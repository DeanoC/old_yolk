#include "dwm.h"

#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/Support/MemoryBuffer.h"

#include "mmu.h"
#include "sandboxmemorymanager.h"
#include "trustedregion.h"
#include "isolatedexecengine.h"


#if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
# error TODO
#elif CPU_FAMILY == CPU_X64
struct IEEThreadContext {
	uint64_t  	rbx;				//  0
	uint64_t  	r12;				//  8
	uint64_t  	r13;				// 16
	uint64_t  	r14;				// 24
	uint64_t  	untrusted_stack;	// 32
  	uint16_t    fcw;				// 40
  	uint16_t	padd[3];			// 42
	uint64_t  	membase;			// 48
	// the above are the untrusted initial registers
	uint64_t	untrusted_rip;		// 56
	uint64_t	trusted_stack;		// 64
};									// 72

#endif

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

void InstallTrustedFuncs( TrustedRegion* trustedRegion ) {
	auto func0 = []() -> int { LOG(INFO) << "_malloc_r called with no implementation!\n"; return 0; };
	auto func1 = []() -> int { LOG(INFO) << "__sprintf called with no implementation!\n"; return 0; };
	auto func2 = []() -> int { LOG(INFO) << "_free_r called with no implementation!\n"; return 0; };
	auto func3 = []() -> int { LOG(INFO) << "_lseek called with no implementation!\n"; return 0; };
	auto func4 = []() -> int { LOG(INFO) << "_raise called with no implementation!\n"; return 0; };
	auto func5 = []() -> int { LOG(INFO) << "__fpclassifyd called with no implementation!\n"; return 0; };
	auto func6 = []() -> int { LOG(INFO) << "_exit called with no implementation!\n"; return 0; };
	auto func7 = []() -> int { LOG(INFO) << "_read called with no implementation!\n"; return 0; };
	auto func8 = []() -> int { LOG(INFO) << "_write called with no implementation!\n"; return 0; };
	auto func9 = []() -> int { LOG(INFO) << "_close called with no implementation!\n"; return 0; };
	auto func10 = []() -> int { LOG(INFO) << "_localeconv_r called with no implementation!\n"; return 0; };
	auto func11 = []() -> int { LOG(INFO) << "_isatty called\n"; return 0; };
	auto func12 = []() -> int { LOG(INFO) << "_calloc_r called with no implementation!\n"; return 0; };

	trustedRegion->addFunctionTrampoline( "_malloc_r", func0 );
	trustedRegion->addFunctionTrampoline( "__sprintf", func1 );
	trustedRegion->addFunctionTrampoline( "_free_r", func2 );
	trustedRegion->addFunctionTrampoline( "_lseek", func3 );
	trustedRegion->addFunctionTrampoline( "_raise", func4 );
	trustedRegion->addFunctionTrampoline( "__fpclassifyd", func5 );
	trustedRegion->addFunctionTrampoline( "_exit", func6 );
	trustedRegion->addFunctionTrampoline( "_read", func7 );
	trustedRegion->addFunctionTrampoline( "_write", func8 );
	trustedRegion->addFunctionTrampoline( "_close", func9 );
	trustedRegion->addFunctionTrampoline( "_localeconv_r", func10 );
	trustedRegion->addFunctionTrampoline( "_isatty", func11 );
	trustedRegion->addFunctionTrampoline( "_calloc_r", func12 );
}

// TODO thread safe thread allocation
#define MAX_UNTRUSTED_THREADS_PER_UNTRUSTED_PROCESS 1024
static IEEThreadContext g_ThreadCtxs[ MAX_UNTRUSTED_THREADS_PER_UNTRUSTED_PROCESS ];
static int g_ThreadIdx = 0;
void IsolatedExecEngine::process( const std::string& elfstr ) {
	using namespace llvm;

	static_assert( sizeof(IEEThreadContext)== 72, "IEEThreadContext size has changed" );
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

	// transfer the elf into the loader and get its ready to go
	MemoryBuffer* mb = MemoryBuffer::getMemBuffer( elfstr );
	dyld->loadObject( mb );
	dyld->resolveRelocations();

	mmgr->protect();
	trustedRegion->protect();


	threadCtx->untrusted_stack = (uint64_t)mmgr->getStackStart() - 8; // dummy slot for initial pop off stack which is never used
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

	typedef void (*main_ptr)( const IEEThreadContext* ctx );
	main_ptr mainp = (main_ptr) dyld->getSymbolAddress( llvm::StringRef("SwitchToUntrustedSSE") );
	mainp( threadCtx );
	// never ever ever gets here!
}