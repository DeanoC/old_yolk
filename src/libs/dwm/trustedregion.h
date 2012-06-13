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
	TrustedRegion( void* _region, size_t _size );

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

	uint8_t* addNotLinkedTrampoline( const std::string& name, void* ptr ) {
		// this memory leaks for now but its only a bit of debug strings so ...
		char* cname = (char*)malloc( name.size() + 1 );
		memcpy( cname, name.c_str(), name.size() );
		cname[ name.size() ] = 0;

		uint8_t* storedPtr = encodeFuncWithParam( ptr, cname );

		// stash address of untrusted trampoline to trusted thunk with
		// the name provided
		nameMap[ name ] = storedPtr;		
		return storedPtr;
	}

	uint8_t* addFunctionTrampoline( const std::string& name, void* ptr ) {
		assert( !thunkCode.empty() ); // require the thunk code to be set
		assert( threadCtx != nullptr );
		uint8_t* storedPtr = getAddress( name );
		if( storedPtr != nullptr ) {
			return storedPtr;
		}

		storedPtr = encodeFunc( ptr );
		// stash address of untrusted trampoline to trusted thunk with
		// the name provided
		nameMap[ name ] = storedPtr;		
		return storedPtr;

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
		MMU::get()->protectPages( thunker, thunkSegSize, MMU::PAGE_READ | MMU::PAGE_WRITE );
	}

	void protect() {
		MMU::get()->protectPages( thunker, thunkSegSize, MMU::PAGE_READ | MMU::PAGE_EXEC );
	}

	// pair of function to avoid passing trusted address to the untrused side, any trusted address should
	// be send through here. Also a clean up functions
	uint32_t trustedAddressToHandle( uintptr_t addr );
	uintptr_t handleToTrustedAddress( uint32_t handle );
	void removeTrustedAddress( uintptr_t addr );

private:

	uint8_t* encodeFunc( void* ptr );

	uint8_t* encodeFuncWithParam( void* ptr, void* param );

	uint8_t* encodeFuncWithStart( void* ptr, uint8_t* startThunkPtr );

	const uint8_t* 								region;
	const size_t 								size;
	uint8_t*									curPtr;
					
	void*										thunker;
	size_t										thunkSegSize;
	std::vector<uint8_t*>						thunkCode;
	uint8_t*									curThunkPtr;

	void*										threadCtx;
	std::unordered_map< std::string, uint8_t*> 	nameMap;
	std::unordered_map< uint32_t, uintptr_t> 	trustedAddrToHandle;
	uint32_t									curTrustedAddrHandle;
};
#endif