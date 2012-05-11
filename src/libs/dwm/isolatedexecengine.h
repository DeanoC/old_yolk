/**
 @file	Z:\Projects\Cyberspace\yolk_repo\src\libs\dwm\isolatedexecengine.h

 Declares the isolatedexecengine class.
 */
 
#ifndef DWM_ISOLATEDEXECENGINE_H_
#define DWM_ISOLATEDEXECENGINE_H_

class SandboxMemoryManager;
namespace llvm { class RuntimeDyld; }

class IsolatedExecEngine {
public:
	static const unsigned int GiB = 1024 * 1024 * 1024;
	static const unsigned int MiB = 1024 * 1024;

	IsolatedExecEngine( uint32_t sandboxSize, uint32_t sandboxStackSize );
	~IsolatedExecEngine();

	void addLibrary( const std::string& elfstr );
	void process( const std::string& elfstr );
private:
	void* sandboxMem;
	void* reservedMem;
	SandboxMemoryManager*	mmgr;
	llvm::RuntimeDyld*		dyld;		
};

#if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
# error TODO
#elif CPU_FAMILY == CPU_X64
struct IEEThreadContext {
	uint64_t  	rax;		// offset in bytes: rax = 0,
	uint64_t  	rbx;		// 8
	uint64_t  	rcx;		// 16
	uint64_t  	rdx;		// 24
	uint64_t  	rbp;		// 32
	uint64_t  	rsi;		// 40
	uint64_t  	rdi;		// 48
	uint64_t  	rsp;		// 56
	uint64_t  	r8;			// 64
	uint64_t  	r9;			// 72
	uint64_t  	r10;		// 80
	uint64_t  	r11;		// 88
	uint64_t  	r12;		// 96
	uint64_t  	r13;		// 104
	uint64_t  	r14;		// 112
	uint64_t  	r15;		// 120
	uint64_t	rip;		// 128
	uint64_t	newrip;		// 136
  	uint64_t  	sysret;		// 144
  	uint16_t    fcw;		// 152
  	uint16_t    sysfcw;		// 154
  	uint32_t	padd;		// 156
  	uint64_t	membase;	// 164
};							// 168 // total size in bytes
#endif

#endif