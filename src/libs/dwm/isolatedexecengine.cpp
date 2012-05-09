#include "dwm.h"

#include "llvm/Object/ELF.h"
#include "llvm/Support/Endian.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/JitMemoryManager.h"

#include "native_client/src/shared/gio/gio.h"
#include "native_client/src/trusted/service_runtime/nacl_all_modules.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/shared/platform/nacl_sync.h"
#include "native_client/src/shared/platform/nacl_exit.h"
#include "native_client/src/shared/platform/nacl_sync_checked.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"


#include "isolatedexecengine.h"

extern "C" void NaClLogAddressSpaceLayout( struct NaClApp *nap );
extern "C" int NaCl_page_alloc(void   **p, size_t size);
extern "C" int NaCl_mprotect(void *addr, size_t len, int prot );

// like assert but x always is executed in all builds TODO release
#define CHECK( x ) ec = (x); if( ec ) { LOG(ERROR) << "ERR " << ec.message() << "\n"; }
#define NACL_MAX_ADDR_BITS  (30)

// The MCJIT memory manager is a layer between the standard JITMemoryManager
// and the RuntimeDyld interface that maps objects, by name, onto their
// matching LLVM IR counterparts in the module(s) being compiled.
class MemoryManager : public llvm::RTDyldMemoryManager {
	virtual void anchor(){};
	llvm::JITMemoryManager *JMM;
public:
	MemoryManager( llvm::JITMemoryManager *jmm ) :
		JMM( jmm ) {}
	// We own the JMM, so make sure to delete it.
	~MemoryManager() { delete JMM; }

	uint8_t *allocateDataSection(uintptr_t Size, unsigned Alignment,
								 unsigned SectionID) {
		return JMM->allocateSpace(Size, Alignment);
	}

	uint8_t *allocateCodeSection(uintptr_t Size, unsigned Alignment,
								 unsigned SectionID) {
		return JMM->allocateSpace(Size, Alignment);
	}

	virtual void *getPointerToNamedFunction(const std::string &Name,
											bool AbortOnFailure = true) {
		return JMM->getPointerToNamedFunction(Name, AbortOnFailure);
	}
};

#include "llvm/ADT/DenseMap.h"
#include "llvm/ExecutionEngine/JITMemoryManager.h"
#include "llvm/Support/Allocator.h"

namespace llvm {

class Function;
class GlobalValue;

struct FreeListNode {
	uintptr_t address;
	uintptr_t size;
	FreeListNode *Prev;
	FreeListNode *Next;
	

	FreeListNode( uintptr_t a, uintptr_t s ) : address(a), size(s) {}

	FreeListNode *RemoveFromFreeList() {
		assert(Next->Prev == this && Prev->Next == this && "Freelist broken!");
		Next->Prev = Prev;
		return Prev->Next = Next;
	}
	void AddToFreeList(FreeListNode *FreeList) {
		Next = FreeList;
		Prev = FreeList->Prev;
		Prev->Next = this;
		Next->Prev = this;
	}

	static void Init(FreeListNode **Head) {
		// Make sure there is always at least one entry in the free list
		*Head = new FreeListNode(0,0);
		(*Head)->Next = (*Head)->Prev = *Head;
	}

	static void Destroy(FreeListNode *Head) {
		FreeListNode *n = Head->Next;
		while(n != Head) {
			FreeListNode *next = n->Next;
			delete n;
			n = next;
		}
		delete Head;
	}
};

// normally memslab points to the allocated memory
class YolkSlabAllocator : public llvm::SlabAllocator {
public:
	YolkSlabAllocator( uintptr_t _membase, uintptr_t _memend ) : 
		membase( _membase ), memend( _memend ) {

		FreeListNode::Init( &head );
		curbase = membase;
	}
	virtual ~YolkSlabAllocator() {};
	virtual MemSlab *Allocate(size_t Size);
	virtual void Deallocate(MemSlab *Slab);
private:
	uintptr_t membase;
	uintptr_t memend;
	uintptr_t curbase;
	FreeListNode* head;
};

// TODO 
MemSlab *YolkSlabAllocator::Allocate(size_t Size) {
	if( curbase >= memend )
		return nullptr;

	MemSlab* slab = (MemSlab*)curbase;
	curbase = curbase + Size;
	slab->NextPtr = (MemSlab*)curbase;
	slab->Size = Size;
	return slab;
}

void YolkSlabAllocator::Deallocate(MemSlab *Slab) {
}

class NaClJITMemoryManagerX : public JITMemoryManager {
	static const size_t kDataSlabSize = 16 * 1024;
	static const size_t kCodeSlabSize = 64 * 1024;

	typedef DenseMap<uintptr_t, size_t> AllocationTable;

	uintptr_t AllocatableRegionStart;
	uintptr_t NextCode;

	// Allocation metadata must be kept separate from code, so the free list is
	// allocated with new rather than being a header in the code blocks
	FreeListNode *CodeFreeListHead;
	FreeListNode *CurrentCodeBlock;
	// Mapping from pointer to allocated function, to size of allocation
	AllocationTable AllocatedFunctions;

	// Since Exception tables are allocated like functions (i.e. we don't know
	// ahead of time how large they are) we use the same allocation method for
	// simplicity even though it's not strictly necessary to separate the
	// allocation metadata from the allocated data.
	FreeListNode *		DataFreeListHead;
	FreeListNode*		CurrentDataBlock;
	AllocationTable		AllocatedTables;
	YolkSlabAllocator	dataSlabAllocator;
	BumpPtrAllocator	DataAllocator;

	uint8_t *GOTBase;     // Target Specific reserved memory

	FreeListNode *allocateCodeSlab(size_t MinSize);
	FreeListNode *allocateDataSlab(size_t MinSize);

	// Functions for allocations using one of the free lists
	FreeListNode *FreeListAllocate(uintptr_t &ActualSize, FreeListNode *Head,
		FreeListNode * (NaClJITMemoryManagerX::*allocate)(size_t));

	void FreeListFinishAllocation(FreeListNode *Block, FreeListNode *Head,
		uintptr_t AllocationStart, uintptr_t AllocationEnd, AllocationTable &table);

	void FreeListDeallocate(FreeListNode *Head, AllocationTable &Table,
							void *Body);
 public:
	// TODO(dschuff): how to find the real value? is it a flag?
	static const int kBundleSize = 32;
	static const intptr_t kJumpMask = -32;
	NaClJITMemoryManagerX( uintptr_t membase, uintptr_t memend );
	virtual ~NaClJITMemoryManagerX();
	static inline bool classof(const JITMemoryManager*) { return true; }

	/// setMemoryWritable - No-op on NaCl - code is never writable
	virtual void setMemoryWritable() {}

	/// setMemoryExecutable - No-op on NaCl - data is never executable
	virtual void setMemoryExecutable() {}

	/// setPoisonMemory - No-op on NaCl - nothing unvalidated is ever executable
	virtual void setPoisonMemory(bool poison) {}

	/// getPointerToNamedFunction - This method returns the address of the
	/// specified function. As such it is only useful for resolving library
	/// symbols, not code generated symbols.
	///
	/// If AbortOnFailure is false and no function with the given name is
	/// found, this function silently returns a null pointer. Otherwise,
	/// it prints a message to stderr and aborts.
	///
	virtual void *getPointerToNamedFunction(const std::string &Name,
											bool AbortOnFailure = true){
		// TODO
		return 0;
	}

	//===--------------------------------------------------------------------===//
	// Global Offset Table Management
	//===--------------------------------------------------------------------===//

	/// AllocateGOT - If the current table requires a Global Offset Table, this
	/// method is invoked to allocate it.  This method is required to set HasGOT
	/// to true.
	virtual void AllocateGOT();

	/// getGOTBase - If this is managing a Global Offset Table, this method should
	/// return a pointer to its base.
	virtual uint8_t *getGOTBase() const { return GOTBase; }

	//===--------------------------------------------------------------------===//
	// Main Allocation Functions
	//===--------------------------------------------------------------------===//

	/// startFunctionBody - When we start JITing a function, the JIT calls this
	/// method to allocate a block of free RWX memory, which returns a pointer to
	/// it.  If the JIT wants to request a block of memory of at least a certain
	/// size, it passes that value as ActualSize, and this method returns a block
	/// with at least that much space.  If the JIT doesn't know ahead of time how
	/// much space it will need to emit the function, it passes 0 for the
	/// ActualSize.  In either case, this method is required to pass back the size
	/// of the allocated block through ActualSize.  The JIT will be careful to
	/// not write more than the returned ActualSize bytes of memory.
	virtual uint8_t *startFunctionBody(const Function *F,
									 uintptr_t &ActualSize);

	/// endFunctionBody - This method is called when the JIT is done codegen'ing
	/// the specified function.  At this point we know the size of the JIT
	/// compiled function.  This passes in FunctionStart (which was returned by
	/// the startFunctionBody method) and FunctionEnd which is a pointer to the
	/// actual end of the function.  This method should mark the space allocated
	/// and remember where it is in case the client wants to deallocate it.
	virtual void endFunctionBody(const Function *F, uint8_t *FunctionStart,
								 uint8_t *FunctionEnd);

	/// allocateCodeSection - Allocate a memory block of (at least) the given
	/// size suitable for executable code. The SectionID is a unique identifier
	/// assigned by the JIT and passed through to the memory manager for
	/// the instance class to use if it needs to communicate to the JIT about
	/// a given section after the fact.
	virtual uint8_t *allocateCodeSection(uintptr_t Size, unsigned Alignment,
										 unsigned SectionID);

	/// allocateDataSection - Allocate a memory block of (at least) the given
	/// size suitable for data. The SectionID is a unique identifier
	/// assigned by the JIT and passed through to the memory manager for
	/// the instance class to use if it needs to communicate to the JIT about
	/// a given section after the fact.
	virtual uint8_t *allocateDataSection(uintptr_t Size, unsigned Alignment,
										 unsigned SectionID);

	virtual uint8_t* allocateStub( const llvm::GlobalValue *, unsigned int, unsigned int ) {
		assert( false );
		return nullptr;
	}

	/// allocateSpace - Allocate a memory block of the given size.  This method
	/// cannot be called between calls to startFunctionBody and endFunctionBody.
	virtual uint8_t *allocateSpace(intptr_t Size, unsigned Alignment);

	/// allocateGlobal - Allocate memory for a global.
	virtual uint8_t *allocateGlobal(uintptr_t Size, unsigned Alignment);

	/// deallocateFunctionBody - Free the specified function body.  The argument
	/// must be the return value from a call to startFunctionBody() that hasn't
	/// been deallocated yet.  This is never called when the JIT is currently
	/// emitting a function.
	virtual void deallocateFunctionBody(void *Body);

	/// startExceptionTable - When we finished JITing the function, if exception
	/// handling is set, we emit the exception table.
	virtual uint8_t* startExceptionTable(const Function* F,
										 uintptr_t &ActualSize);

	/// endExceptionTable - This method is called when the JIT is done emitting
	/// the exception table.
	virtual void endExceptionTable(const Function *F, uint8_t *TableStart,
								 uint8_t *TableEnd, uint8_t* FrameRegister);

	/// deallocateExceptionTable - Free the specified exception table's memory.
	/// The argument must be the return value from a call to startExceptionTable()
	/// that hasn't been deallocated yet.  This is never called when the JIT is
	/// currently emitting an exception table.
	virtual void deallocateExceptionTable(void *ET);

	virtual size_t GetDefaultCodeSlabSize() { return kCodeSlabSize; }
	virtual size_t GetDefaultDataSlabSize() { return kDataSlabSize; }
};

// The way NaCl linking is currently setup, there is a gap between the text
// segment and the rodata segment where we can fill dyncode. The text ends
// at etext, but there's no symbol for the start of rodata. Currently the
// linker script puts it at 0x11000000
// If we run out of space there, we can also allocate below the text segment
// and keep going downward until we run into code loaded by the dynamic
// linker. (TODO(dschuff): make that work)
// For now, just start at etext and go until we hit rodata

// It's an open issue that lazy jitting is not thread safe (PR5184). However
// NaCl's dyncode_create solves exactly this problem, so in the future
// this allocator could (should?) be made thread safe

NaClJITMemoryManagerX::NaClJITMemoryManagerX( uintptr_t membase, uintptr_t memend ) :
	dataSlabAllocator( membase, memend ),
	DataAllocator(4096, 4096, dataSlabAllocator ),
	NextCode(AllocatableRegionStart), GOTBase(NULL) {

	AllocatableRegionStart = (uintptr_t)NACL_TRAMPOLINE_END;
	NextCode = AllocatableRegionStart;

	FreeListNode::Init(&CodeFreeListHead);
	FreeListNode::Init(&DataFreeListHead);

	LOG(INFO) << "NaClJITMemoryManagerX: AllocatableRegionStart " <<
		AllocatableRegionStart << " Limit " << memend << "\n";
}

NaClJITMemoryManagerX::~NaClJITMemoryManagerX() {
	delete [] GOTBase;
	FreeListNode::Destroy(CodeFreeListHead);
	FreeListNode::Destroy(DataFreeListHead);
}

FreeListNode *NaClJITMemoryManagerX::allocateCodeSlab(size_t MinSize) {
	FreeListNode *node = new FreeListNode( NextCode, std::max(kCodeSlabSize, MinSize) );
	NextCode += node->size;

	LOG(INFO) << "allocated code slab " << NextCode - node->size << "-" <<
		NextCode << "\n";
	return node;
}

FreeListNode *NaClJITMemoryManagerX::allocateDataSlab(size_t MinSize) {
	size_t size = std::max(kDataSlabSize, MinSize);
	FreeListNode *node = new FreeListNode( (uintptr_t)DataAllocator.Allocate(size, kBundleSize), size);
	return node;
}


FreeListNode *NaClJITMemoryManagerX::FreeListAllocate(uintptr_t &ActualSize,
	FreeListNode *Head,
	FreeListNode * (NaClJITMemoryManagerX::*allocate)(size_t)) {
	FreeListNode *candidateBlock = Head;
	FreeListNode *iter = Head->Next;

	uintptr_t largest = candidateBlock->size;
	// Search for the largest free block
	while (iter != Head) {
		if (iter->size > largest) {
			largest = iter->size;
			candidateBlock = iter;
		}
		iter = iter->Next;
	}

	if (largest < ActualSize || largest == 0) {
		candidateBlock = (this->*allocate)(ActualSize);
	} else {
		candidateBlock->RemoveFromFreeList();
	}
	return candidateBlock;
}

void NaClJITMemoryManagerX::FreeListFinishAllocation(
											FreeListNode *Block,
											FreeListNode *Head, 
											uintptr_t AllocationStart, 
											uintptr_t AllocationEnd,
											AllocationTable &Table ) {
	assert(AllocationEnd > AllocationStart);
	assert(Block->address == AllocationStart);
	
	uintptr_t End = RoundUpToAlignment(AllocationEnd, kBundleSize);
	assert(End <= Block->address + Block->size);
	int AllocationSize = End - Block->address;
	Table[AllocationStart] = AllocationSize;

	Block->size -= AllocationSize;
	if (Block->size >= kBundleSize * 2) {
		//TODO(dschuff): better heuristic?
		Block->address = End;
		Block->AddToFreeList(Head);
	} else {
		delete Block;
	}

	LOG(INFO) <<"FinishAllocation size "<< AllocationSize <<" end "<<End<<"\n";
}

void NaClJITMemoryManagerX::FreeListDeallocate(	FreeListNode *Head,
												AllocationTable &Table,
												void *Body ) {
	uintptr_t Allocation = (uintptr_t) Body;
	LOG(INFO) << "deallocating "<< Allocation << " ";
	assert(Table.count(Allocation) && "FreeList Deallocation not found in table");

	FreeListNode *Block = new FreeListNode( Allocation, Table[Allocation] );
	Block->AddToFreeList(Head);

	LOG(INFO) << "deallocated "<< Allocation<< " size " << Block->size <<"\n";
}

uint8_t *NaClJITMemoryManagerX::startFunctionBody(const Function *F,
												 uintptr_t &ActualSize) {
	CurrentCodeBlock = FreeListAllocate(ActualSize, CodeFreeListHead,
									&NaClJITMemoryManagerX::allocateCodeSlab);

	LOG(INFO) << "startFunctionBody CurrentBlock " << CurrentCodeBlock <<
		" addr " << CurrentCodeBlock->address << "\n";

	ActualSize = CurrentCodeBlock->size;
	return (uint8_t *) CurrentCodeBlock->address;
}

void NaClJITMemoryManagerX::endFunctionBody(const Function *F,
											 uint8_t *FunctionStart,
											 uint8_t *FunctionEnd) {
	LOG(INFO) << "endFunctionBody ";
	FreeListFinishAllocation(CurrentCodeBlock, CodeFreeListHead,
							 (uintptr_t) FunctionStart, (uintptr_t)FunctionEnd, AllocatedFunctions);

}

uint8_t *NaClJITMemoryManagerX::allocateCodeSection(uintptr_t Size,
													 unsigned Alignment,
													 unsigned SectionID) {
	llvm_unreachable("Implement me! (or don't.)");
}

uint8_t *NaClJITMemoryManagerX::allocateDataSection(uintptr_t Size,
													 unsigned Alignment,
													 unsigned SectionID) {
	return (uint8_t *)DataAllocator.Allocate(Size, Alignment);
}

void NaClJITMemoryManagerX::deallocateFunctionBody(void *Body) {
	LOG(INFO) << "deallocateFunctionBody, ";
	if (Body) FreeListDeallocate(CodeFreeListHead, AllocatedFunctions, Body);
}

uint8_t *NaClJITMemoryManagerX::allocateSpace(intptr_t Size,
											 unsigned Alignment) {
	uint8_t *r = (uint8_t*)DataAllocator.Allocate(Size, Alignment);
	LOG(INFO) << "allocateSpace " << Size <<"/"<<Alignment<<" ret "<<r<<"\n";
	return r;
}

uint8_t *NaClJITMemoryManagerX::allocateGlobal(uintptr_t Size,
												unsigned Alignment) {
	uint8_t *r = (uint8_t*)DataAllocator.Allocate(Size, Alignment);
	LOG(INFO) << "allocateGlobal " << Size <<"/"<<Alignment<<" ret "<<r<<"\n";
	return r;
}

uint8_t* NaClJITMemoryManagerX::startExceptionTable(const Function* F,
													 uintptr_t &ActualSize) {
	CurrentDataBlock = FreeListAllocate(ActualSize, DataFreeListHead,
										&NaClJITMemoryManagerX::allocateDataSlab);
	LOG(INFO) << "startExceptionTable CurrentBlock " << CurrentDataBlock <<
		" addr " << CurrentDataBlock->address << "\n";
	ActualSize = CurrentDataBlock->size;
	return (uint8_t *)CurrentDataBlock->address;
}

void NaClJITMemoryManagerX::endExceptionTable(const Function *F,
											 uint8_t *TableStart,
						 uint8_t *TableEnd, uint8_t* FrameRegister) {
	LOG(INFO) << "endExceptionTable ";
	FreeListFinishAllocation(CurrentDataBlock, DataFreeListHead,
							 (uintptr_t)TableStart, (uintptr_t)TableEnd, AllocatedTables);
}

void NaClJITMemoryManagerX::deallocateExceptionTable(void *ET) {
	LOG(INFO) << "deallocateExceptionTable, ";
	if (ET) FreeListDeallocate(DataFreeListHead, AllocatedTables, ET);
}

// Copy of DefaultJITMemoryManager's implementation
void NaClJITMemoryManagerX::AllocateGOT() {
	assert(GOTBase == 0 && "Cannot allocate the got multiple times");
	GOTBase = new uint8_t[sizeof(void*) * 8192];
	HasGOT = true;
}

}

#define NACL_ABI_PROT_READ        0x1   /* Page can be read.  */
#define NACL_ABI_PROT_WRITE       0x2   /* Page can be written.  */
#define NACL_ABI_PROT_EXEC        0x4   /* Page can be executed.  */

IsolatedExecEngine::IsolatedExecEngine()
{
}


void IsolatedExecEngine::process( const std::string& elfstr ) {
	using namespace llvm;

	std::string Error;

	struct GioFile                gout;
	struct NaClApp                state;
	struct NaClApp                *nap;

	NaClAllModulesInit();

	// TODO Gio/Core::InOutInterface shim
	if (!GioFileRefCtor(&gout, stdout)) { 
		LOG(ERROR) << "Could not create general standard output channel\n";
		return;
	}

	NaClLogSetGio( &gout.base );
	NaClLogSetVerbosity( 2 );

	if (!NaClAppCtor(&state)) {
		LOG(ERROR) << "Error while constructing app state\n";
		return;
	}
	nap = &state;

	// setup the basic nacl process
	NaClAppInitialDescriptorHookup(nap);
	//	NaClLogAddressSpaceLayout(nap);

	// process loader 
	/* NACL_MAX_ADDR_BITS < 32 */
	if (nap->addr_bits > NACL_MAX_ADDR_BITS) {
		LOG(ERROR) << "Error : NACL_MAX_ADDR_BITS\n";
		return;
	}

	// only support one text, one rodata and one data segment
	// not sure why yet, buts its what nacl supports so...
	bool gotTextSegment = false;
	bool gotDataSegment = false;
	bool gotRODataSegment = false;

	uint64_t maxvaddr = 0;
	llvm::error_code ec;

	// allocate the sandbox memory space and make it all 
	// read/write for use to prepare it
	// 4 MiB is all they get for now 
	void* yorkSandboxMem = 0;
	static const unsigned int yorkSandboxSize = 4 * 1024 * 1024;
	NaCl_page_alloc( &yorkSandboxMem, yorkSandboxSize );
	NaCl_mprotect( yorkSandboxMem, yorkSandboxSize, NACL_ABI_PROT_READ | NACL_ABI_PROT_WRITE );
	// setup the linker and memory manager
	NaClJITMemoryManagerX mmgr( (uintptr_t) yorkSandboxMem, (uintptr_t)yorkSandboxMem + yorkSandboxSize );
	MemoryManager jitmmgr( &mmgr );
	RuntimeDyld dyld( &jitmmgr );

	// transfer the elf string into an elf object 
	MemoryBuffer* mb = MemoryBuffer::getMemBuffer( elfstr );
	dyld.loadObject( mb );
	dyld.resolveRelocations();
	void* mainp = dyld.getSymbolAddress( llvm::StringRef("main") );
/*	object::ELFObjectFile<support::little, false> elfob( mb, ec );

	auto sec = elfob.begin_sections();
	while( sec != elfob.end_sections() ) {
		// skip 0 sized segments
		uint64_t secSize = 0;
		CHECK( sec->getSize( secSize ) );
		if( secSize == 0 ) {
			goto FinishedSegmentDecode;					
		}

		// log the name of the segment
		StringRef sr;
		CHECK( sec->getName( sr ) );
		LOG(INFO) << "ELF Segment : " << sr.str() << "\n";


		// start and end virtual address
		uint64_t svaddr = ~0;
		uint64_t evaddr = ~0;
		CHECK( sec->getAddress( svaddr ) );
		if( ec ) goto FinishedSegmentDecode;					
		if( svaddr >= (1ul << (uint64_t)nap->addr_bits) ) {
			LOG(ERROR) << "Segment start above " << nap->addr_bits << " bit address space\n";
			goto FinishedSegmentDecode;					
		}
		evaddr = svaddr + secSize;
		if( evaddr >= (1ul << (uint64_t)nap->addr_bits) ) {
			LOG(ERROR) << "Segment end above " << nap->addr_bits << " bit address space\n";
			goto FinishedSegmentDecode;					
		}
		maxvaddr = std::max( maxvaddr, evaddr );


		auto rawSec = reinterpret_cast<const Elf_Shdr *>(sec->getRawDataRefImpl().p );
		// we default to ignoring most segments, the few we care about
		// are set up
		// text segment (code)
		bool result = false;
		CHECK( sec->isText( result ) );
		if( result ) {
			if( gotTextSegment ) {
				LOG(INFO) << "only 1 text segment used\n";
				goto FinishedSegmentDecode;
			}
			if( rawSec->sh_flags & SHF_WRITE ) {
				// if writable, ignore
				LOG(INFO) << "ignoring writable text segment\n";
				goto FinishedSegmentDecode;
			}
			if( !(rawSec->sh_flags & SHF_EXECINSTR) ) {
				// if not executable, ignore
				LOG(INFO) << "ignoring non executable text segment\n";
				goto FinishedSegmentDecode;					
			}
			if( svaddr != NACL_TRAMPOLINE_END ) {
				// text must start at the 2nd 64K
				// page (after the trampoline)
				LOG(INFO) << "ignoring not at end of trampoline text segment\n";
				goto FinishedSegmentDecode;
			}

			nap->static_text_end = NACL_TRAMPOLINE_END + evaddr;
			nap->dynamic_text_start = 0;
			nap->dynamic_text_end = 0;

			goto FinishedSegmentDecode;
		} 
		// data or rodata segment
		CHECK( sec->isData( result ) );
		if( result ) {
			if( rawSec->sh_flags & SHF_EXECINSTR ) {
				// [ro]data can't be executable
				LOG(INFO) << "ignoring executable data segment\n";
				goto FinishedSegmentDecode;
			}
			if( rawSec->sh_flags & SHF_WRITE ) {
				// its the data segment
				if( gotDataSegment ) {
					LOG(INFO) << "only 1 data segment used\n";
					goto FinishedSegmentDecode;

				}
				nap->data_start = svaddr;
				// we don't actually really use this during the process
				// the data segment covers all the data space, this is purely
				// where teh next segment starts
				nap->data_end = evaddr;
			} else {
				// its the rodata segment
				if( gotRODataSegment ) {
					LOG(INFO) << "only 1 rodata segment used\n";
					goto FinishedSegmentDecode;
				}
				nap->rodata_start = svaddr;
				// rodate_end == max virtual space
				// nap->rodata_end = evaddr;
			}
			goto FinishedSegmentDecode;
		}
		// bss segment
		CHECK( sec->isBSS( result ) );
		if( result ) {
			goto FinishedSegmentDecode;
		}

		// ignore any others at the mo
FinishedSegmentDecode:
		sec.increment(ec);
	}
	if( !gotTextSegment  ) {
		// must have a text segment to be valid
		LOG(INFO) << "Err missing text segment\n";		
	}
	*/
	// fix up address space
	nap->stack_size = NaClRoundAllocPage(nap->stack_size);
	if( nap->data_start == 0) {
		if( nap->rodata_start == 0 ) {
			// its legal for both to be 0 but need to fix up some stuff
			if (NaClRoundAllocPage(maxvaddr) - maxvaddr < NACL_HALT_SLED_SIZE) {
				maxvaddr += NACL_MAP_PAGESIZE; // for the halt sled
			}
		}
		maxvaddr = NaClRoundAllocPage(maxvaddr);
	}

	// where malloc starts
	nap->break_addr = maxvaddr;
	// where the data segment ends
	nap->data_end = maxvaddr;

	nap->bundle_size = NACL_INSTR_BLOCK_SIZE;

/*	auto sym = elfob.begin_symbols();
	while( sym != elfob.end_symbols() ) {
		StringRef sr;
		sym->getName( sr );
		LOG(INFO) << sr.str();
		sym.increment(ec);
	}
	*/
	// update the load status and notify any threads waiting on the processes cv
	NaClXMutexLock(&nap->mu);
//	nap->module_load_status = errcode;
	NaClXCondVarBroadcast(&nap->cv);
	NaClXMutexUnlock(&nap->mu);

	// temp argc/v array
	char* argv[] = { "nacl" };
	if (!NaClCreateMainThread(nap, 1, argv, NULL ) ) {
		LOG(ERROR) << "creating main thread failed\n";
	}

	// stall for exit
	int ret_code = NaClWaitForMainThreadToExit(nap);

	// clean up 
	NaClAllModulesFini();
	NaClExit(ret_code);
}