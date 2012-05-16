#include "dwm.h"

#include "llvm/Support/MemoryBuffer.h"

#include "mmu.h"
#include "sandboxmemorymanager.h"
#include "trustedregion.h"
#include "ieethreadcontext.h"
#include "isolatedexecengine.h"

extern void InstallVmApiFuncs( TrustedRegion* trustedRegion );

IsolatedExecEngine::IsolatedExecEngine( uint32_t sandboxSize, 
										uint32_t sandboxStackSize,
										uint32_t sandboxTrustedRegionSize ) :
	sandboxMem( 0 ),
	reservedMem( 0 )	
{
	// allocate the sandbox memory space and make it all 
	// read/write for use to prepare it
	// 4 MiB is all they get for now 

#if CPU_BIT_SIZE == 32
	// 32 bit we have to conserve virtual address space and use
	// other techniques to ensure no peeking into the non sandbox region
	MMU::get()->allocPages( &yorkSandboxMem, sandboxSize );
	yorkReservedMem = yorkSandboxMem; // one and the same for 32 bit

#	if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
	uint16_t dataSel = MMU::get()->allocSelector( true, &sandboxMem, sandboxSize, MMU::PROT_READ | MMU::PROT_WRITE );
	uint16_t codeSel = MMU::get()->allocSelector( false, &sandboxMem, sandboxSize, MMU::PROT_READ | MMU::PROT_WRITE );
#	endif
#else
	// 64 bit we use a massive guard space around the sandbox memory
	// to ensure no leakage
	static const uint64_t bufferSize = (uint64_t)40 * GiB;
	static const uint64_t reserveSize = 
					((uint64_t)4 * GiB) + 		// 4 GiB for the sandbox
				( (uint64_t)2 * bufferSize); 	// 2 * 40GiB space either end
	MMU::get()->reservePages( &reservedMem, reserveSize );
	// the actual memory starts at about 40GiB mark and has sandboxSize
	// of actually commited backed RAM

	sandboxMem = (void*)( ( ( (uintptr_t)reservedMem ) + bufferSize ) & 0xFFFFFFFF00000000 );
	MMU::get()->commitPages( &sandboxMem, sandboxSize);
#endif

	// setup the linker and memory manager
	mmgr = CORE_NEW SandboxMemoryManager( 	(uintptr_t) sandboxMem, 
											((uintptr_t)sandboxMem) + sandboxSize, 
											sandboxStackSize,
											sandboxTrustedRegionSize );
	trustedRegion = CORE_NEW TrustedRegion( mmgr->getTrustedStart(), 
											mmgr->getTrustedSize() );

	mmgr->setTrustedRegion( trustedRegion );
	dyld = CORE_NEW llvm::RuntimeDyld( mmgr );
}
IsolatedExecEngine::~IsolatedExecEngine(){
	CORE_DELETE trustedRegion;
	CORE_DELETE dyld;
	CORE_DELETE mmgr;
}

void IsolatedExecEngine::addLibrary( const std::string& elfstr ) {
	using namespace llvm;
	MemoryBuffer* mb = MemoryBuffer::getMemBuffer( elfstr );
	dyld->loadObject( mb );

}

void* IsolatedExecEngine::sandboxAllocate( size_t size) {
	size = Core::alignTo( size, MMU::get()->getPageSize() );
	uint8_t* mem = mmgr->allocateSpace( size, (unsigned int) MMU::get()->getPageSize() );
	MMU::get()->protectPages( mem, size, MMU::PROT_READ | MMU::PROT_WRITE );
	return mem;
}
void IsolatedExecEngine::sandboxFree( void* ptr ) {
	// TODO
}

// trusted functions can currently have 3 user params (besides the threadCtx) any must be integers or sandbox pointers
// if pointer must adjust, and beware of other sandbox threads altering (copy before use in most cases!)
// float support is minimal, not mixing is likely to work for the 4 four float/float vector parameters on 64 bit at least
// TODO improve support for floats + varargs (which won't work) + handle stack parameters
void InstallTrustedFuncs( TrustedRegion* trustedRegion ) {
	auto no_func = []( const IEEThreadContext* threadCtx, const uintptr_t sbTextptr ) { 
		LOG(INFO) << "No func: " << threadCtx->untrusted_rip << "\n"; 
	};

	auto func0 = []( const IEEThreadContext* threadCtx, const uintptr_t sbTextptr ) { 
		const char* text = (const char*)( threadCtx->membase + sbTextptr);
		LOG(INFO) << "DgStringOut: " << text << "\n"; 
	};
	auto func1 = [] ( const IEEThreadContext* threadCtx, const uintptr_t sbDst, const int32_t val, const uint32_t size )  -> uintptr_t {
		void* dst = (void*)( threadCtx->membase + sbDst);
		void* ret = memset( dst, val, size );
		return (uintptr_t)ret - threadCtx->membase;
	};
#define TRFUNC(X) \
	auto func##X = [] ( const IEEThreadContext* threadCtx ) { \
		LOG(INFO) << MACRO_TEXT(X) << "\n";\
		DebugBreak(); \
	}; \
	trustedRegion->addFunctionTrampoline( MACRO_TEXT(X), func##X );

	trustedRegion->addFunctionTrampoline( "_no_func_", no_func );
	trustedRegion->addFunctionTrampoline( "DgStringOut", func0 );
	trustedRegion->addFunctionTrampoline( "memset", func1 );
	TRFUNC( __cxa_guard_abort )
	TRFUNC( _Unwind_Resume )
	TRFUNC( powf )
	TRFUNC( memcpy )
	TRFUNC( __assert_func )
	TRFUNC( __dso_handle )
	TRFUNC( __cxa_atexit )
	TRFUNC( __cxa_guard_release )
	TRFUNC( __cxa_pure_virtual )
	TRFUNC( __gxx_personality_v0 )
	TRFUNC( __cxa_guard_acquire )

	TRFUNC( _ZTVN10__cxxabiv121__vmi_class_type_infoE )
	TRFUNC( _Z21btAlignedFreeInternalPv )
	TRFUNC( _ZN15CProfileManager5ResetEv )
	TRFUNC( _ZN20btConvexHullComputer7computeEPKvbiiff )
	TRFUNC( _ZN15CProfileManager12Stop_ProfileEv )
	TRFUNC( _ZN15CProfileManager13Start_ProfileEPKc )
	TRFUNC( _ZdlPv )
	TRFUNC( _ZN15CProfileManager23Increment_Frame_CounterEv )
	TRFUNC( _ZSt9terminatev )
	TRFUNC( _ZTVN10__cxxabiv120__si_class_type_infoE )
	TRFUNC( _Z22btAlignedAllocInternalji )
	TRFUNC( _ZTVN10__cxxabiv117__class_type_infoE )
	TRFUNC( _Znwj )

}

// TODO thread safe thread allocation
#define MAX_UNTRUSTED_THREADS_PER_UNTRUSTED_PROCESS 1024
static IEEThreadContext g_ThreadCtxs[ MAX_UNTRUSTED_THREADS_PER_UNTRUSTED_PROCESS ];
static int g_ThreadIdx = 0;
void IsolatedExecEngine::process( const std::string& elfstr ) {
	using namespace llvm;

	static_assert( sizeof(IEEThreadContext)== 80, "IEEThreadContext size has changed" );
	IEEThreadContext* threadCtx = g_ThreadCtxs + g_ThreadIdx;
	g_ThreadIdx++;
	memset( threadCtx, 0, sizeof(IEEThreadContext) );

	// add the thunks
	void* sttStart = dyld->getSymbolAddress( llvm::StringRef("SwitchToTrusted") );
	void* sttEnd = dyld->getSymbolAddress( llvm::StringRef("SwitchToTrustedEnd") );
	trustedRegion->setThunkCode( sttStart, sttEnd );
	trustedRegion->setThreadContext( threadCtx );
	// wipe out (HLT) the untrusted thunk code now its been copied into trusted space
	memset( sttStart, 0xFE, (uintptr_t)sttEnd - (uintptr_t)sttStart ); 

	InstallTrustedFuncs( trustedRegion );
	InstallVmApiFuncs( trustedRegion );

	// transfer the elf into the loader and get its ready to go
	MemoryBuffer* mb = MemoryBuffer::getMemBuffer( elfstr );
	dyld->loadObject( mb );
	dyld->resolveRelocations();

	// no code writting can occur once the protect are set
	mmgr->protect();
	trustedRegion->protect();
	
	threadCtx->untrusted_stack = (uint64_t)mmgr->getStackStart() - 8;
	unsigned int tmp;
	_controlfp_s( &tmp, 0, 0);
	threadCtx->fcw =  (uint16_t) tmp;

	threadCtx->membase = (uint64_t) sandboxMem;
	threadCtx->untrusted_rip = (uint64_t)dyld->getSymbolAddress( llvm::StringRef("main") );

	// this function never leaves once we enter untrusted code, this thread is commited to untrusted exec
	// and so we use its stack for the trusted calls from untrusted code
	// TODO make sure automatic C++ clean up code is on the stack, post this (including all code getting here)
	// easiest way is to create a new main thread that calls here leaving all the construction to its parent thread.
	threadCtx->trusted_stack = (uint64_t) _AddressOfReturnAddress(); 
	threadCtx->owner = this;

	typedef void (*main_ptr)( const IEEThreadContext* ctx );
	main_ptr mainp = (main_ptr) dyld->getSymbolAddress( llvm::StringRef("SwitchToUntrustedSSE") );
	mainp( threadCtx );
	// never ever ever gets here!
}