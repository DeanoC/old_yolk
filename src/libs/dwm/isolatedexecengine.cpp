#include "dwm.h"

#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/Support/MemoryBuffer.h"

#include "mmu.h"
#include "sandboxmemorymanager.h"
#include "isolatedexecengine.h"

IsolatedExecEngine::IsolatedExecEngine()
{
}


void IsolatedExecEngine::process( const std::string& elfstr ) {
	using namespace llvm;

	// allocate the sandbox memory space and make it all 
	// read/write for use to prepare it
	// 4 MiB is all they get for now 
	static const unsigned int GiB = 1024 * 1024 * 1024;
	static const unsigned int MiB = 1024 * 1024;

	void* yorkSandboxMem = 0;
	void* yorkReservedMem = 0;
	static const unsigned int yorkSandboxSize = 4 * MiB;

#if CPU_BIT_SIZE == 32
	// 32 bit we have to conserve virtual address space and use
	// other techniques to ensure no peeking into the non sandbox region
	MMU::get()->allocPages( &yorkSandboxMem, yorkSandboxSize );
	yorkReservedMem = yorkSandboxMem; // one and the same for 32 bit

#	if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
	uint16_t dataSel = MMU::get()->allocSelector( true, &yorkSandboxMem, yorkSandboxSize, MMU::PROT_READ | MMU::PROT_WRITE );
	uint16_t codeSel = MMU::get()->allocSelector( false, &yorkSandboxMem, yorkSandboxSize, MMU::PROT_READ | MMU::PROT_WRITE );
#	endif
#else
	// 64 bit we use a massive guard space around the sandbox memory
	// to ensure no leakage
	static const unsigned int bufferSize = 20 * GiB;
	static const unsigned int yorkReserveSize = 
					(4 * GiB) + 	// 4 GiB for the sandbox
				( 2 * bufferSize); 	// 2 * 20GiB space either end
	MMU::get()->reservePages( &yorkReservedMem, yorkReserveSize );
	// the actual memory starts at the 2GiB mark and has yorkSandboxSize
	// of actually commited backed RAM
	yorkSandboxMem = (void*)(((uintptr_t)yorkReservedMem) + bufferSize);
	MMU::get()->allocPages( yorkSandboxMem, yorkSandboxSize); // commit
#endif

	// setup the linker and memory manager
	SandboxMemoryManager mmgr( (uintptr_t) yorkSandboxMem, (uintptr_t)yorkSandboxMem + yorkSandboxSize );
	mmgr.unprotect();
	RuntimeDyld dyld( &mmgr );

	// transfer the elf into the loader and get its ready to go
	MemoryBuffer* mb = MemoryBuffer::getMemBuffer( elfstr );
	dyld.loadObject( mb );
	dyld.resolveRelocations();
	mmgr.protect();

	typedef void (*main_ptr)();
	main_ptr mainp = (main_ptr) dyld.getSymbolAddress( llvm::StringRef("main") );
	mainp();
}