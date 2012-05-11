#include "dwm.h"

#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/Support/MemoryBuffer.h"

#include "mmu.h"
#include "sandboxmemorymanager.h"
#include "isolatedexecengine.h"

IsolatedExecEngine::IsolatedExecEngine( uint32_t sandboxSize, uint32_t sandboxStackSize ) :
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
					((uint64_t)4 * GiB) + 	// 4 GiB for the sandbox
				( (uint64_t)2 * bufferSize); 	// 2 * 20GiB space either end
	MMU::get()->reservePages( &reservedMem, reserveSize );
	// the actual memory starts at the 20GiB mark and has sandboxSize
	// of actually commited backed RAM

	sandboxMem = (void*)( ( ( (uintptr_t)reservedMem ) + bufferSize ) & 0xFFFFFFFF00000000 );
	MMU::get()->commitPages( &sandboxMem, sandboxSize);
#endif

	// setup the linker and memory manager
	mmgr = CORE_NEW SandboxMemoryManager( (uintptr_t) sandboxMem, ((uintptr_t)sandboxMem) + sandboxSize, sandboxStackSize );
	dyld = CORE_NEW llvm::RuntimeDyld( mmgr );

}
IsolatedExecEngine::~IsolatedExecEngine(){
	CORE_DELETE dyld;
	CORE_DELETE mmgr;
}

void IsolatedExecEngine::addLibrary( const std::string& elfstr ) {
	using namespace llvm;
	MemoryBuffer* mb = MemoryBuffer::getMemBuffer( elfstr );
	dyld->loadObject( mb );
}


void IsolatedExecEngine::process( const std::string& elfstr ) {
	using namespace llvm;

	// transfer the elf into the loader and get its ready to go
	MemoryBuffer* mb = MemoryBuffer::getMemBuffer( elfstr );
	dyld->loadObject( mb );
	
	dyld->resolveRelocations();
	mmgr->protect();

	IEEThreadContext threadCtx;
	static_assert( sizeof(threadCtx)== 168, "eek" );
	memset( &threadCtx, 0, sizeof(threadCtx) );
	threadCtx.membase = (uint64_t) sandboxMem;
	threadCtx.r15 = threadCtx.membase;
	threadCtx.rsp = (uint64_t)mmgr->getStackStart() - 8;
	threadCtx.rbp = threadCtx.rsp;
	*(uint64_t*)threadCtx.rsp = (uint64_t) &([]{ LOG(INFO) << "done\n"; });
	threadCtx.newrip = (uint64_t)dyld->getSymbolAddress( llvm::StringRef("main") );
	typedef void (*main_ptr)( const IEEThreadContext& ctx );
	main_ptr mainp = (main_ptr) dyld->getSymbolAddress( llvm::StringRef("SwitchToUntrustedSSE") );
	mainp( threadCtx );
}