#include "core/core.h"

#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/JITMemoryManager.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/system_error.h"
#include "mmu.h"
#include "ieethreadcontext.h"

#include "sandboxmemorymanager.h"

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

// It's an open issue that lazy jitting is not thread safe (PR5184). However
// NaCl's dyncode_create solves exactly this problem, so in the future
// this allocator could (should?) be made thread safe

SandboxMemoryManager::SandboxMemoryManager( uintptr_t membase, 
											uintptr_t memend, 
											size_t _stackSize,
											size_t _trustedSize ) :
	stackSize(_stackSize),
	trustedSize( _trustedSize ),
	slabAllocator( membase, memend - stackSize ),
	codeAllocator( 4096, 4096, slabAllocator ),
	dataAllocator( 4096, 4096, slabAllocator ),
	GOTBase(NULL) {

	FreeListNode::Init(&codeFreeListHead);
	FreeListNode::Init(&dataFreeListHead);

	unprotect();

	// write x86 HLT opcode over the entire space
	memset( (void*)membase, 0xF4, memend - membase );

	// reserve 4K at the base of memory for the 0 page, catch most nullptr violations (r or w)
	slabAllocator.Allocate( MMU::get()->getPageSize() );
	// next pages is r/o memory of the trusted block 
	trustedStart = (uintptr_t) slabAllocator.Allocate( trustedSize );

	LOG(INFO) << "SandboxMemoryManager: Pwned " << (void*)membase << " - " << (void*)memend << "\n";

	stackStart = slabAllocator.memend - MMU::get()->getPageSize();
	stackEnd = stackStart - (stackSize - (2 * MMU::get()->getPageSize()));
}

SandboxMemoryManager::~SandboxMemoryManager() {
	delete [] GOTBase;
	FreeListNode::Destroy(codeFreeListHead);
	FreeListNode::Destroy(dataFreeListHead);
}

FreeListNode *SandboxMemoryManager::allocateCodeSlab(size_t minSize) {
	// a call to here, occurs when we need code space outside a existing section
	// I think this can only be dynamic code allocations (else the elf will have
	// allocated a section and have an ID). So we createa a slab sized section
	// with an ID of ~0 (-1)
	size_t size = std::max( (size_t)kCodeSlabSize, minSize);
	return new FreeListNode( (uintptr_t)allocateCodeSection(size, kBundleSize,~0), size);
}

FreeListNode *SandboxMemoryManager::allocateDataSlab(size_t minSize) {
	// see allocateCodeSlab just replace Code with Data
	size_t size = std::max( (size_t)kDataSlabSize, minSize);
	return new FreeListNode( (uintptr_t)allocateDataSection(size, kBundleSize,~0), size);
}


FreeListNode *SandboxMemoryManager::FreeListAllocate(uintptr_t &actualSize,
					FreeListNode *head,
					FreeListNode * (SandboxMemoryManager::*allocate)(size_t)) 
{
	FreeListNode *candidateBlock = head;
	FreeListNode *iter = head->Next;

	uintptr_t largest = candidateBlock->size;
	// Search for the largest free block
	while (iter != head) {
		if (iter->size > largest) {
			largest = iter->size;
			candidateBlock = iter;
		}
		iter = iter->Next;
	}

	if (largest < actualSize || largest == 0) {
		candidateBlock = (this->*allocate)(actualSize);
	} else {
		candidateBlock->RemoveFromFreeList();
	}
	return candidateBlock;
}

void SandboxMemoryManager::FreeListFinishAllocation(
											FreeListNode *block,
											FreeListNode *head, 
											uintptr_t allocationStart, 
											uintptr_t allocationEnd,
											AllocationTable &table ) {
	assert(allocationEnd > allocationStart);
	assert(block->address == allocationStart);
	
	uintptr_t end = Core::alignTo(allocationEnd, kBundleSize);
	assert(end <= block->address + block->size);
	uintptr_t allocationSize = end - block->address;
	table[allocationStart] = allocationSize;

	block->size -= allocationSize;
	if (block->size >= kBundleSize * 2) {
		//TODO(dschuff): better heuristic?
		block->address = end;
		block->AddToFreeList(head);
	} else {
		delete block;
	}

	LOG(INFO) <<"FinishAllocation size "<< allocationSize <<" end "<<end<<"\n";
}

void SandboxMemoryManager::FreeListDeallocate(	FreeListNode *head,
												AllocationTable &table,
												void *body ) {
	uintptr_t allocation = (uintptr_t) body;
	LOG(INFO) << "deallocating "<< allocation << " ";
	assert(table.count(allocation) && "FreeList Deallocation not found in table");

	FreeListNode *block = new FreeListNode( allocation, table[allocation] );
	block->AddToFreeList(head);

	LOG(INFO) << "deallocated "<< allocation<< " size " << block->size <<"\n";
}

uint8_t *SandboxMemoryManager::startFunctionBody(const llvm::Function *F,
												 uintptr_t &actualSize) {
	currentCodeBlock = FreeListAllocate(actualSize, codeFreeListHead,
									&SandboxMemoryManager::allocateCodeSlab);

	LOG(INFO) << "startFunctionBody CurrentBlock " << currentCodeBlock <<
		" addr " << currentCodeBlock->address << "\n";

	actualSize = currentCodeBlock->size;
	return (uint8_t *) currentCodeBlock->address;
}

void SandboxMemoryManager::endFunctionBody(const llvm::Function *F,
											 uint8_t *functionStart,
											 uint8_t *functionEnd) {
	LOG(INFO) << "endFunctionBody ";
	FreeListFinishAllocation(currentCodeBlock, codeFreeListHead,
							 (uintptr_t) functionStart, (uintptr_t)functionEnd, 
							 allocatedFunctions);

}

uint8_t *SandboxMemoryManager::allocateCodeSection(	uintptr_t size,
													unsigned alignment,
													unsigned sectionID ) {
	LOG(INFO) << "allocateCodeSection " << size << "/" << alignment << "\n";
	uintptr_t r = (uintptr_t)codeAllocator.Allocate( size, alignment );
	assert( r < stackEnd );
	codeSectionTable[ sectionID ] = std::pair<uintptr_t,size_t>(r,(size_t)size);
	return (uint8_t*)r;
}

uint8_t *SandboxMemoryManager::allocateDataSection(	uintptr_t size,
													unsigned alignment,
													unsigned sectionID ) {
	LOG(INFO) << "allocateDataSection " << size << "/" << alignment << "\n";
	uintptr_t r = (uintptr_t)dataAllocator.Allocate( size, alignment );
	assert( r < stackEnd );
	dataSectionTable[ sectionID ] = std::pair<uintptr_t,size_t>(r,(size_t)size);
	return (uint8_t*)r;
}

void SandboxMemoryManager::deallocateFunctionBody( void* body ) {
	LOG(INFO) << "deallocateFunctionBody, ";
	if (body) FreeListDeallocate( codeFreeListHead, allocatedFunctions, body );
}

uint8_t *SandboxMemoryManager::allocateSpace(intptr_t size,
											 unsigned alignment) {
	LOG(INFO) << "allocateSpace " << size << "/" << alignment << "\n";
	// todo REWRITE data allocators, not fit for purpose and can be used
	// to crash the trusted side post running untrusted code :O
	if( size < 4096 ) {
		uintptr_t r = (uintptr_t) dataAllocator.Allocate( size, alignment );
		assert( r < stackEnd );
		return (uint8_t*)r;
	} else {
		uintptr_t r = (uintptr_t) slabAllocator.AllocateRaw( size );
		assert( r < stackEnd );
		return (uint8_t*) r;
	}
}

uint8_t *SandboxMemoryManager::allocateGlobal(uintptr_t size,
												unsigned alignment) {
	LOG(INFO) << "allocateGlobal " << size << "/" << alignment << "\n";
	uintptr_t r = (uintptr_t) dataAllocator.Allocate( size, alignment );
	assert( r < stackEnd );
	return (uint8_t*) r;
}

uint8_t* SandboxMemoryManager::startExceptionTable(const llvm::Function* f,
													 uintptr_t &actualSize) {
	currentDataBlock = FreeListAllocate( actualSize, dataFreeListHead,
										&SandboxMemoryManager::allocateDataSlab );
	LOG(INFO) << "startExceptionTable CurrentBlock " << currentDataBlock <<
		" addr " << currentDataBlock->address << "\n";
	actualSize = currentDataBlock->size;
	return (uint8_t *)currentDataBlock->address;
}

void SandboxMemoryManager::endExceptionTable(const llvm::Function *F,
											 uint8_t *tableStart,
						 uint8_t *tableEnd, uint8_t* frameRegister) {
	LOG(INFO) << "endExceptionTable ";
	FreeListFinishAllocation(currentDataBlock, dataFreeListHead,
							 (uintptr_t)tableStart, (uintptr_t)tableEnd, allocatedTables);
}

void SandboxMemoryManager::deallocateExceptionTable(void *et) {
	LOG(INFO) << "deallocateExceptionTable, ";
	if (et) FreeListDeallocate(dataFreeListHead, allocatedTables, et);
}

// Copy of DefaultJITMemoryManager's implementation
void SandboxMemoryManager::AllocateGOT() {
	assert(GOTBase == 0 && "Cannot allocate the got multiple times");
	GOTBase = new uint8_t[sizeof(void*) * 8192];
	HasGOT = true;
}

void SandboxMemoryManager::protect() {
	// make the entire range untouchable at first
	MMU::get()->protectPages( (void*) slabAllocator.membase, 
					(size_t)((slabAllocator.memend + stackSize) - slabAllocator.membase), 
					MMU::PAGE_NONE );
	SectionTable::const_iterator it;
	for( it = codeSectionTable.cbegin(); it != codeSectionTable.cend(); ++it ) {
		// get address and size
		auto addr = it->second.first;
		auto size = it->second.second;

		MMU::get()->protectPages( 	(void*) addr, 
									(size_t)size, 
									MMU::PAGE_READ | MMU::PAGE_EXEC );
	}
	for( it = dataSectionTable.cbegin(); it != dataSectionTable.cend(); ++it ) {
		// get address and size
		// TODO ro date section
		auto addr = it->second.first;
		auto size = it->second.second;
		MMU::get()->protectPages( 	(void*) addr, 
									(size_t)size, 
									MMU::PAGE_READ | MMU::PAGE_WRITE );
	}
	// the untrusted stack is R/W except for the top and bottom 64K which
	// are guard pages
	MMU::get()->protectPages( 	(void*)stackEnd, 
								stackStart - stackEnd, 
								MMU::PAGE_READ | MMU::PAGE_WRITE );

	// read/exec trusted region TODO? split into data and code region?
	// see how its used, currently its mostly used for tramps + a few variables
	// these variable aren't likely to become a escape vector
	MMU::get()->protectPages( 	(void*)trustedStart, 
								trustedSize, 
								MMU::PAGE_READ | MMU::PAGE_EXEC );

}
void SandboxMemoryManager::unprotect() {
	// make entire memory range R/W but not executable, only
	// trusted code should ever be in this state
	// NOTE *MUST* be done at start as the range starts protected
	MMU::get()->protectPages( (void*) slabAllocator.membase, 
					(size_t)((slabAllocator.memend + stackSize) - slabAllocator.membase), 
					MMU::PAGE_READ | MMU::PAGE_WRITE );

}

YolkSlabAllocator::YolkSlabAllocator( uintptr_t _membase, uintptr_t _memend ) : 
	membase( _membase ), memend( _memend ) {

	FreeListNode::Init( &head );
	curbase = membase;
}

// TODO 
llvm::MemSlab *YolkSlabAllocator::Allocate(size_t Size) {
	if( curbase >= memend )
		return nullptr;

	llvm::MemSlab* slab = (llvm::MemSlab*)curbase;
	slab->Size = Core::alignTo( Size, MMU::get()->getPageSize() );	
	curbase = curbase + slab->Size;
	return slab;
}

// TODO 
uint8_t* YolkSlabAllocator::AllocateRaw(size_t Size) {
	if( curbase >= memend )
		return nullptr;

	uint8_t* mem = (uint8_t*) curbase;
	Size = Core::alignTo( Size, MMU::get()->getPageSize() );	
	curbase = curbase + Size;
	return mem;
}

void YolkSlabAllocator::Deallocate(llvm::MemSlab *Slab) {
}

static void DbgNotLinked(  const IEEThreadContext* threadCtx, const char* name ) {
	LOG(INFO) << "Not Linked FUNC : " << name << " called!! CRASH 3.2.1.... \n"; 
}

void *SandboxMemoryManager::getPointerToNamedFunction(const std::string &name, bool AbortOnFailure) {
	if( name[0] == '.' ) {
		// local var, never a function so ignore
		return (void*)slabAllocator.membase;
	}
	auto addr = trustedRegion->getAddress( name );
	if( addr == nullptr ) {
		addr = trustedRegion->addNotLinkedTrampoline( name, (void*)&DbgNotLinked );
//			LOG(INFO) << "Code contains a trusted function " << name << " that doesn't exist, if called it will likely crash\n";
	}
	if( addr == nullptr ) {
			// this is a valid sandbox addres, but any call to it will go pop
		LOG(INFO) << "Code contains a trusted function " << name << " that doesn't exist, if called it will crash\n";
		return (void*)slabAllocator.membase;
	}

	return (void*)addr;
}
