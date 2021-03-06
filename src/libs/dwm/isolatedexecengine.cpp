#include "dwm.h"

#include "llvm/Support/MemoryBuffer.h"

#include "mmu.h"
#include "sandboxmemorymanager.h"
#include "trustedregion.h"
#include "ieethreadcontext.h"
#include "isolatedexecengine.h"
#include "world.h"

extern "C" void _ovly_debug_event() {
}

IsolatedExecEngine::IsolatedExecEngine( uint32_t sandboxSize, 
										uint32_t sandboxStackSize,
										uint32_t sandboxTrustedRegionSize,
										World* _world ) :
	sandboxMem( 0 ),
	reservedMem( 0 ),
	world( _world )
{
	// allocate the sandbox memory space and make it all 
	// read/write for use to prepare it
	// 4 MiB is all they get for now 

#if CPU_BIT_SIZE == 32
	// 32 bit we have to conserve virtual address space and use
	// other techniques to ensure no peeking into the non sandbox region
	MMU::get()->allocPages( &sandboxMem, sandboxSize );
	reservedMem = sandboxMem; // one and the same for 32 bit

#	if 0
	//CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
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
	MMU::get()->protectPages( mem, size, MMU::PAGE_READ | MMU::PAGE_WRITE );
	return mem;
}
void IsolatedExecEngine::sandboxFree( void* ptr ) {
	// TODO
}

void IsolatedExecEngine::terminate() {
//	untrustedThread->interrupt();
//	try {
//		boost::this_thread::interruption_point();
//	} catch( ... ) {
//	};

	CORE_DELETE untrustedThread;
	untrustedThread = nullptr;
}

// TODO thread safe thread allocation
#define MAX_UNTRUSTED_THREADS_PER_UNTRUSTED_PROCESS 1024
static IEEThreadContext g_ThreadCtxs[ MAX_UNTRUSTED_THREADS_PER_UNTRUSTED_PROCESS ];
static int g_ThreadIdx = 0;
void IsolatedExecEngine::process( const std::string& elfstr, void (*apiInstall)( TrustedRegion* ) ) {
	using namespace llvm;
#if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
	static_assert( sizeof(IEEThreadContext)== 80, "IEEThreadContext size has changed" );
#elif CPU_FAMILY == CPU_X64
	static_assert( sizeof(IEEThreadContext)== 80, "IEEThreadContext size has changed" );
#endif
	IEEThreadContext* threadCtx = g_ThreadCtxs + g_ThreadIdx;
	g_ThreadIdx++;
	memset( threadCtx, 0, sizeof(IEEThreadContext) );

	// add the thunks
	void* sttStart = dyld->getSymbolAddress( llvm::StringRef("SwitchToTrustedLinux") );
	void* sttEnd = dyld->getSymbolAddress( llvm::StringRef("SwitchToTrustedLinuxEnd") );
	trustedRegion->setThunkCode( sttStart, sttEnd );
	trustedRegion->setThreadContext( threadCtx );
	// wipe out (HLT) the untrusted thunk code now its been copied into trusted space
	memset( sttStart, 0xFE, (uintptr_t)sttEnd - (uintptr_t)sttStart ); 

	apiInstall( trustedRegion );

	// transfer the elf into the loader and get its ready to go
	MemoryBuffer* mb = MemoryBuffer::getMemBuffer( elfstr );
	dyld->loadObject( mb );
	dyld->resolveRelocations();

	// no code writting can occur once the protect are set
	mmgr->protect();
	trustedRegion->protect();

	_ovly_debug_event();
	
	threadCtx->untrusted_stack = (uint64_t)mmgr->getStackStart() - 8;
	unsigned int tmp = 0;
#if PLATFORM_OS == MS_WINDOWS
	_controlfp_s( &tmp, 0, 0);
#endif
	threadCtx->fcw =  (uint16_t) tmp;

	threadCtx->membase = (uint64_t) 0;// DEBUG sandboxMem;
	threadCtx->untrusted_rip = (uint64_t)dyld->getSymbolAddress( llvm::StringRef("__libc_init") );
//	threadCtx->untrusted_rip = (uint64_t)dyld->getSymbolAddress( llvm::StringRef("main") );
	assert( threadCtx->untrusted_rip != 0 );

	threadCtx->owner = this;
	typedef void (*switch_ptr)( const IEEThreadContext* ctx );
	switch_ptr switchp = (switch_ptr) dyld->getSymbolAddress( llvm::StringRef("SwitchToUntrustedSSELinux") );
	assert( switchp != nullptr );

	untrustedThread = CORE_NEW Core::thread( [&]() {
		switchp( threadCtx );
	});

	untrustedThread->join();
}