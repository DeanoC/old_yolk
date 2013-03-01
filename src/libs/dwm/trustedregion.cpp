#include "core/core.h"
#include "trustedregion.h"

TrustedRegion::TrustedRegion( void* _region, size_t _size ) :
	region( (const uint8_t*) _region ),
	size( _size ),
	curPtr( (uint8_t*) region ),
	curTrustedAddrHandle( 1 ) {

	thunkSegSize = 64 * 1024;
	thunker = nullptr;
	threadCtx = nullptr;
	MMU::get()->allocPages( &thunker, thunkSegSize );
	unprotect();
	curThunkPtr = (uint8_t*)thunker;
}

uint32_t TrustedRegion::trustedAddressToHandle( uintptr_t addr ) {
	trustedAddrToHandle[ curTrustedAddrHandle++ ] = addr;
	return curTrustedAddrHandle-1;
}
uintptr_t TrustedRegion::handleToTrustedAddress( uint32_t handle ) {
	return trustedAddrToHandle[ handle ];
}
void TrustedRegion::removeTrustedAddress( uintptr_t addr ) {
	auto it = trustedAddrToHandle.find( addr );
	if( it != trustedAddrToHandle.end() ) {
		trustedAddrToHandle.erase( it );
	}	
}

uint8_t* TrustedRegion::encodeFunc( void* ptr ) {
	assert( !thunkCode.empty() ); // require the thunk code to be set
	assert( curThunkPtr < (uint8_t*)thunker + thunkSegSize );

	// copy and fill in the thunk first
	curThunkPtr = (uint8_t*) Core::alignTo( (uintptr_t)curThunkPtr, 32 );
	uint8_t* startThunkPtr = curThunkPtr;
	return encodeFuncWithStart( ptr, startThunkPtr );
}

uint8_t* TrustedRegion::encodeFuncWithParam( void* ptr, void* param ) {
	assert( !thunkCode.empty() ); // require the thunk code to be set
	assert( curThunkPtr < (uint8_t*)thunker + thunkSegSize );

	// copy and fill in the thunk first
	curThunkPtr = (uint8_t*) Core::alignTo( (uintptr_t)curThunkPtr, 32 );
	uint8_t* startThunkPtr = curThunkPtr;

	curThunkPtr[0] = 0x48;	// x86-64 mov mem+2, rdi (will be moved to 2nd param by thunk)
	curThunkPtr[1] = 0xBF;
	curThunkPtr += 2;
	memcpy( curThunkPtr, &param, sizeof(void*) ); // copy param ptr
	curThunkPtr += sizeof(void*);

	return encodeFuncWithStart( ptr, startThunkPtr );
}

uint8_t* TrustedRegion::encodeFuncWithStart( void* ptr, uint8_t* startThunkPtr ) {
	curThunkPtr[0] = 0x48;	// x86-64 mov mem+2, rcx
	curThunkPtr[1] = 0xB9;
	memcpy( curThunkPtr + 2, &threadCtx, sizeof(void*) ); // copy ctx ptr
	curThunkPtr += 2 + sizeof(void*);

	curThunkPtr[0] = 0x48;	// x86-64 mov mem+2, rax
	curThunkPtr[1] = 0xB8;
	memcpy( curThunkPtr + 2, &ptr, sizeof(void*) ); // copy ptr
	curThunkPtr += 2 + sizeof(void*);

	memcpy( curThunkPtr, &thunkCode[0], thunkCode.size() );
	curThunkPtr += thunkCode.size();

	uint8_t* mem = (uint8_t*) Core::alignTo( (uintptr_t)curPtr, 32 );
	// then point the untrusted trampoline to the thunk
	curPtr = mem;
	curPtr[0] = 0x48;	// x86-64 mov mem+2, rax
	curPtr[1] = 0xB8;
	memcpy( curPtr + 2, &startThunkPtr, sizeof(void*) ); // copy ptr
	curPtr += 2 + sizeof(void*);

	curPtr[0] = 0xFF; // x86-64 jmp rax
	curPtr[1] = 0xE0; 
	curPtr += 2;

	return mem;
}
