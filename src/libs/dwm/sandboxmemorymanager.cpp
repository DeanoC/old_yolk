#include "core/core.h"

#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/JitMemoryManager.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/system_error.h"

#include "sandboxmemorymanager.h"

// It's an open issue that lazy jitting is not thread safe (PR5184). However
// NaCl's dyncode_create solves exactly this problem, so in the future
// this allocator could (should?) be made thread safe

SandboxMemoryManager::SandboxMemoryManager( uintptr_t membase, uintptr_t memend ) :
	slabAllocator( membase, memend ),
	codeAllocator( 4096, 4096, slabAllocator ),
	dataAllocator( 4096, 4096, slabAllocator ),
	GOTBase(NULL) {

	FreeListNode::Init(&codeFreeListHead);
	FreeListNode::Init(&dataFreeListHead);

	LOG(INFO) << "SandboxMemoryManager: Pwned " << membase <<
								" - " << memend << "\n";
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
	size_t size = std::max(kCodeSlabSize, minSize);
	return new FreeListNode( (uintptr_t)allocateCodeSection(size, kBundleSize,~0), size);
}

FreeListNode *SandboxMemoryManager::allocateDataSlab(size_t minSize) {
	// see allocateCodeSlab just replace Code with Data
	size_t size = std::max(kDataSlabSize, minSize);
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
	int allocationSize = end - block->address;
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
	codeSectionTable[ sectionID ] = std::pair<uintptr_t,size_t>(r,(size_t)size);
	return (uint8_t*)r;
}

uint8_t *SandboxMemoryManager::allocateDataSection(	uintptr_t size,
													unsigned alignment,
													unsigned sectionID ) {
	LOG(INFO) << "allocateDataSection " << size << "/" << alignment << "\n";
	uintptr_t r = (uintptr_t)dataAllocator.Allocate( size, alignment );
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
	return (uint8_t*)dataAllocator.Allocate( size, alignment );
}

uint8_t *SandboxMemoryManager::allocateGlobal(uintptr_t size,
												unsigned alignment) {
	LOG(INFO) << "allocateGlobal " << size << "/" << alignment << "\n";
	return (uint8_t*)dataAllocator.Allocate( size, alignment );
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
	NaCl_mprotect( 	(void*)slabAllocator.membase, 
					(size_t)(slabAllocator.memend - slabAllocator.membase), 
					0 );
	SectionTable::const_iterator it;
	for( it = codeSectionTable.cbegin(); it != codeSectionTable.cend(); ++it ) {
		// get address and size
		auto addr = it->second.first;
		auto size = it->second.second;

		NaCl_mprotect( 	(void*) addr, 
					(size_t)size, 
					NACL_ABI_PROT_READ | NACL_ABI_PROT_EXEC );
	}
	for( it = dataSectionTable.cbegin(); it != dataSectionTable.cend(); ++it ) {
		// get address and size
		// TODO ro date section
		auto addr = it->second.first;
		auto size = it->second.second;
		NaCl_mprotect( 	(void*) addr, 
					(size_t)size, 
					NACL_ABI_PROT_READ | NACL_ABI_PROT_WRITE );
	}

}
void SandboxMemoryManager::unprotect() {
	// make entire memory range R/W but not executable, only
	// trusted code should ever be in this state
	// NOTE *MUST* be done at start as the range starts protected
	NaCl_mprotect( 	(void*) slabAllocator.membase, 
					(size_t)(slabAllocator.memend - slabAllocator.membase), 
					NACL_ABI_PROT_READ | NACL_ABI_PROT_WRITE );

}
// TODO 
llvm::MemSlab *YolkSlabAllocator::Allocate(size_t Size) {
	if( curbase >= memend )
		return nullptr;

	llvm::MemSlab* slab = (llvm::MemSlab*)curbase;
	curbase = curbase + Size;
	slab->Size = Size;
	return slab;
}

void YolkSlabAllocator::Deallocate(llvm::MemSlab *Slab) {
}