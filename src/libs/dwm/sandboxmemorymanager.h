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
#include "llvm/ExecutionEngine/JITMemoryManager.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "trustedregion.h"

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

	uint8_t* AllocateRaw(size_t Size);

	const uintptr_t membase;
	const uintptr_t memend;
private:
	uintptr_t curbase;
	FreeListNode* head;
};
#define kDataSlabSize (16 * 1024)
#define kCodeSlabSize (64 * 1024)


class SandboxMemoryManager : 
	public llvm::JITMemoryManager, 
	public llvm::RTDyldMemoryManager {
 public:
	static const int 		kBundleSize = 32;
	static const intptr_t 	kJumpMask = -32;

	SandboxMemoryManager( uintptr_t membase, uintptr_t memend, size_t _stackSize, size_t _trustedSize );

	virtual ~SandboxMemoryManager();

	static inline bool classof(const llvm::JITMemoryManager*) { return true; }

	/// setMemoryWritable - No-op on NaCl - code is never writable
	virtual void setMemoryWritable() {}

	/// setMemoryExecutable - No-op on NaCl - data is never executable
	virtual void setMemoryExecutable() {}

	/// setPoisonMemory - No-op on NaCl - nothing unvalidated is ever executable
	virtual void setPoisonMemory(bool poison) {}

	virtual void *getPointerToNamedFunction(const std::string &name, bool AbortOnFailure = true);

	// set code to RX and data to RW TODO RO data sections
	void protect();

	// set all to RW no X, only allow in trusted code
	void unprotect();

	// top of the stack, stack goes downwards so Start > End 
	void* getStackStart() const { return (void*)stackStart; }
	// bottom of the stack, stack goes downwards so End has a lower value than Start 
	void* getStackEnd() const { return (void*)stackEnd; }

	// trusted address of the trusted pages, these are read/only pages at the bottom
	// of untrusted space, use to store trampolines, tls indices etc. Anything you 
	// need to be safely stored, be careful not to leak info that could be used by 
	// hackers
	void* getTrustedStart() const { return (void*) trustedStart; }

	size_t getTrustedSize() const { return trustedSize; }

	// sets trusted region for lookup but does not own it 
	void setTrustedRegion( TrustedRegion* region ) { trustedRegion = region; }

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

	size_t 		stackSize;
	uintptr_t 	stackStart;	// stack goes downwards so Start > End
	uintptr_t 	stackEnd;
	uintptr_t	trustedStart; // a read-only portion of untrusted space for trampolines, tls index etc
	size_t		trustedSize;

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

	TrustedRegion*	trustedRegion; // used for external variable look ups

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