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

struct tmp_lconv
{
  char *decimal_point;
  char *thousands_sep;
  char *grouping;
  char *int_curr_symbol;
  char *currency_symbol;
  char *mon_decimal_point;
  char *mon_thousands_sep;
  char *mon_grouping;
  char *positive_sign;
  char *negative_sign;
  char int_frac_digits;
  char frac_digits;
  char p_cs_precedes;
  char p_sep_by_space;
  char n_cs_precedes;
  char n_sep_by_space;
  char p_sign_posn;
  char n_sign_posn;
  char int_n_cs_precedes;
  char int_n_sep_by_space;
  char int_n_sign_posn;
  char int_p_cs_precedes;
  char int_p_sep_by_space;
  char int_p_sign_posn;
} TMP_C_LCONV = 
{
	".", ",", ",",
	"£", "£",
	".",",",",",
	"+","-",
	2,2,
	0,0,
	0,0,
	0,0,
	0,0,0,
	0,0,0
};

// trusted functions can currently have 3 user params (besides the threadCtx) any must be integers or sandbox pointers
// if pointer must adjust, and beware of other sandbox threads altering (copy before use in most cases!)
// float support is minimal, not mixing is likely to work for the 4 four float/float vector parameters on 64 bit at least
// TODO improve support for floats + varargs (which won't work) + handle stack parameters
void InstallTrustedFuncs( TrustedRegion* trustedRegion ) {
	auto func0 = []( const IEEThreadContext* threadCtx, const uintptr_t sbTextptr ) { 
		const char* text = (const char*)( threadCtx->membase + sbTextptr);
		LOG(INFO) << "DgStringOut: " << text << "\n"; 
	};

	trustedRegion->addFunctionTrampoline( "DgStringOut", func0 );
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

	typedef void (*main_ptr)( const IEEThreadContext* ctx );
	main_ptr mainp = (main_ptr) dyld->getSymbolAddress( llvm::StringRef("SwitchToUntrustedSSE") );
	mainp( threadCtx );
	// never ever ever gets here!
}