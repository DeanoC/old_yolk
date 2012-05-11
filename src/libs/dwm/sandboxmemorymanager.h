/*
 * /Z/Projects/Cyberspace/yolk_repo/src/libs/dwm/sandboxmemory.h
 *
 * (c) Copyright Cloud Pixies Ltd.
 */
#pragma once
#ifndef DWM_SANDBOXMEMORY_H_
#define DWM_SANDBOXMEMORY_H_ 1

#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Allocator.h"
#include "llvm/ExecutionEngine/JitMemoryManager.h"

#define NACL_TRAMPOLINE_END			64*1024

namespace llvm {
	class Function;
	class GlobalValue;
}
struct FreeListNode;

// normally memslab points to the allocated memory
class YolkSlabAllocator : public llvm::SlabAllocator {
public:
	YolkSlabAllocator( uintptr_t _membase, uintptr_t _memend );

	virtual ~YolkSlabAllocator() {};
	virtual llvm::MemSlab *Allocate(size_t Size);
	virtual void Deallocate(llvm::MemSlab *Slab);

	const uintptr_t membase;
	const uintptr_t memend;
private:
	uintptr_t curbase;
	FreeListNode* head;
};

class SandboxMemoryManager : 
	public llvm::JITMemoryManager, 
	public llvm::RTDyldMemoryManager {
 public:
	static const size_t 	kDataSlabSize = 16 * 1024;
	static const size_t 	kCodeSlabSize = 64 * 1024;
	static const int 		kBundleSize = 32;
	static const intptr_t 	kJumpMask = -32;

	SandboxMemoryManager( uintptr_t membase, uintptr_t memend, uintptr_t stackSize );

	virtual ~SandboxMemoryManager();

	static inline bool classof(const llvm::JITMemoryManager*) { return true; }

	/// setMemoryWritable - No-op on NaCl - code is never writable
	virtual void setMemoryWritable() {}

	/// setMemoryExecutable - No-op on NaCl - data is never executable
	virtual void setMemoryExecutable() {}

	/// setPoisonMemory - No-op on NaCl - nothing unvalidated is ever executable
	virtual void setPoisonMemory(bool poison) {}

	virtual void *getPointerToNamedFunction(const std::string &Name,
											bool AbortOnFailure = true) {
		LOG(INFO) << "Code contains a externed function " << Name << " that doesn't exist, if called will crash\n";
 		// this is a valid sandbox addres, but any call to it will go pop
		return (void*)slabAllocator.membase;
	}

	// set code to RX and data to RW TODO RO data sections
	void protect();

	// set all to RW no X, only allow in trusted code
	void unprotect();

	// top of the stack, stack goes downwards so Start > End 
	void* getStackStart() const { return (void*)stackStart; }
	// bottom of the stack, stack goes downwards so End has a lower value than Start 
	void* getStackEnd() const { return (void*)stackEnd; }

	//===--------------------------------------------------------------------===//
	// Global Offset Table Management
	//===--------------------------------------------------------------------===//
	virtual void AllocateGOT();
	virtual uint8_t *getGOTBase() const { return GOTBase; }

	//===--------------------------------------------------------------------===//
	// Main Allocation Functions
	//===--------------------------------------------------------------------===//

	virtual uint8_t *startFunctionBody(const llvm::Function *F,
									 uintptr_t &ActualSize);

	virtual void endFunctionBody(const llvm::Function *F, uint8_t *FunctionStart,
								 uint8_t *FunctionEnd);

	virtual uint8_t *allocateCodeSection(uintptr_t Size, unsigned Alignment,
										 unsigned SectionID);

	virtual uint8_t *allocateDataSection(uintptr_t Size, unsigned Alignment,
										 unsigned SectionID);

	virtual uint8_t* allocateStub( const llvm::GlobalValue *, unsigned int, unsigned int ) {
		assert( false );
		return nullptr;
	}

	virtual uint8_t *allocateSpace(intptr_t Size, unsigned Alignment);

	virtual uint8_t *allocateGlobal(uintptr_t Size, unsigned Alignment);

	virtual void deallocateFunctionBody(void *Body);

	virtual uint8_t* startExceptionTable(const llvm::Function* F,
										 uintptr_t &ActualSize);

	virtual void endExceptionTable(const llvm::Function *F, uint8_t *TableStart,
								 uint8_t *TableEnd, uint8_t* FrameRegister);

	virtual void deallocateExceptionTable(void *ET);

private:
	typedef llvm::DenseMap<uintptr_t, size_t> 					AllocationTable;
	typedef std::map<unsigned int, std::pair<uintptr_t,size_t>> SectionTable;

	uintptr_t stackSize;
	uintptr_t stackStart;	// stack goes downwards so Start > End
	uintptr_t stackEnd;

	// Allocation metadata must be kept separate from code, so the free list is
	// allocated with new rather than being a header in the code blocks
	FreeListNode*			codeFreeListHead;
	FreeListNode*			currentCodeBlock;
	FreeListNode*			dataFreeListHead;
	FreeListNode*			currentDataBlock;
	llvm::BumpPtrAllocator	dataAllocator;
	llvm::BumpPtrAllocator	codeAllocator;

	// Mapping from pointer to allocated function/table, to size of allocation
	AllocationTable 		allocatedFunctions;
	AllocationTable			allocatedTables;

	// actual memory manager
	YolkSlabAllocator		slabAllocator;
	// trace sectionID to address and type
	SectionTable 			codeSectionTable;
	SectionTable 			dataSectionTable;

	uint8_t *GOTBase;     // Target Specific reserved memory

	FreeListNode *allocateCodeSlab( size_t MinSize );
	FreeListNode *allocateDataSlab( size_t MinSize );

	// Functions for allocations using one of the free lists
	FreeListNode *FreeListAllocate( uintptr_t &actualSize, FreeListNode *head,
					FreeListNode * (SandboxMemoryManager::*allocate)(size_t) );

	void FreeListFinishAllocation( FreeListNode *block, FreeListNode *head,
						uintptr_t allocationStart, uintptr_t allocationEnd, 
						AllocationTable &table );

	void FreeListDeallocate( FreeListNode *head, AllocationTable &table, void *body );

};


#endif	