#include "dwm.h"

#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/Support/MemoryBuffer.h"

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
	void* yorkSandboxMem = 0;
	static const unsigned int yorkSandboxSize = 4 * 1024 * 1024;
	NaCl_page_alloc( &yorkSandboxMem, yorkSandboxSize );

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