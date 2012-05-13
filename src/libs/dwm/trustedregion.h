/*
 * /Z/Projects/Cyberspace/yolk_repo/src/libs/dwm/trustedregion.h
 *
 * (c) Copyright Cloud Pixies Ltd.
 */
#pragma once
#ifndef DWM_TRUSTED_REGION_H_
#define DWM_TRUSTED_REGION_H_ 1

 #include "mmu.h"

class TrustedRegion {
public:
	TrustedRegion( void* _region, size_t _size ) :
		region( (const uint8_t*) _region ),
		size( _size ),
		curPtr( (uint8_t*) region ) {

		thunkSegSize = 64 * 1024;
		thunker = nullptr;
		threadCtx = nullptr;
		MMU::get()->allocPages( &thunker, thunkSegSize );
		unprotect();
		curThunkPtr = (uint8_t*)thunker;
	}

	template< typename T> uint8_t* addVariable( const std::string& name, const T& val ) {
		T* mem = (T*) Core::alignTo( ((uintptr_t) curPtr), 32 );
		*mem = val;
		nameMap[ name ] = (uint8_t*)mem;
		curPtr += sizeof(T);
		return (uint8_t*) mem;
	}

	void setThunkCode( const void* thunkCodeStart, const void* thunkCodeEnd) {
		size_t thunkCodeSize = (uintptr_t)thunkCodeEnd - (uintptr_t)thunkCodeStart;
		assert( thunkCodeSize != 0 );
		thunkCode.resize( thunkCodeSize );
		memcpy( &thunkCode[0], thunkCodeStart, thunkCodeSize );
	}
	void setThreadContext( void* ctx ) {
		threadCtx = ctx;
	}

	uint8_t* addFunctionTrampoline( const std::string& name, void* ptr ) {
		assert( !thunkCode.empty() ); // require the thunk code to be set
		assert( threadCtx != nullptr );

		// copy and fill in the thunk first
		curThunkPtr = (uint8_t*) Core::alignTo( (uintptr_t)curThunkPtr, 32 );
		uint8_t* startThunkPtr = curThunkPtr;
		curThunkPtr[0] = 0x48;	// x86-64 mov mem+2, rax
		curThunkPtr[1] = 0xB8;
		curThunkPtr += 2;
		memcpy( curThunkPtr, &threadCtx, sizeof(void*) ); // copy ctx ptr
		curThunkPtr += sizeof(void*);
		curThunkPtr[0] = 0x49;	// x86-64 mov rax, r15
		curThunkPtr[1] = 0x89;
		curThunkPtr[2] = 0xC7;
		curThunkPtr += 3;

		curThunkPtr[0] = 0x48;	// x86-64 mov mem+2, rax
		curThunkPtr[1] = 0xB8;
		curThunkPtr += 2;
		memcpy( curThunkPtr, &ptr, sizeof(void*) ); // copy ptr
		curThunkPtr += sizeof(void*);
		memcpy( curThunkPtr, &thunkCode[0], thunkCode.size() );
		curThunkPtr += thunkCode.size();

		uint8_t* mem = (uint8_t*) Core::alignTo( (uintptr_t)curPtr, 32 );
		// then point the untrusted trampoline to the thunk
		curPtr = mem;
		curPtr[0] = 0x48;	// x86-64 mov mem+2, rax
		curPtr[1] = 0xB8;
		curPtr += 2;
		memcpy( curPtr, &startThunkPtr, sizeof(void*) ); // copy ptr
		curPtr += sizeof(void*);
		curPtr[0] = 0xFF; // x86-64 jmp rax
		curPtr[1] = 0xE0; 
		curPtr += 2;

		// stash address of untrusted trampoline to trusted thunk with
		// the name provided
		nameMap[ name ] = (uint8_t*)mem;		
		return (uint8_t*)mem;
	}

	uint8_t* getAddress( const std::string& name ) const { 
		auto ret = nameMap.find(name); 
		if( ret != nameMap.cend() ) {
			return ret->second;
		} else {
			return nullptr;
		}
	}

	void unprotect() {		
		MMU::get()->protectPages( thunker, thunkSegSize, MMU::PROT_READ | MMU::PROT_WRITE );
	}

	void protect() {
		MMU::get()->protectPages( thunker, thunkSegSize, MMU::PROT_READ | MMU::PROT_EXEC );
	}

private:
	const uint8_t* 		region;
	const size_t 		size;
	uint8_t*			curPtr;

	void*				thunker;
	size_t				thunkSegSize;
	std::vector<uint8_t*>	thunkCode;
	uint8_t*			curThunkPtr;

	void*				threadCtx;
	std::map< std::string, uint8_t*> nameMap;
};
#endif