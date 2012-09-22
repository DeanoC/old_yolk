/**
 @file	databuffer.cpp

 @brief	Implements the data buffer objects.
 */
#include "dx11.h"
#include "databuffer.h"


namespace Dx11 {

DataBuffer* DataBuffer::internalCreate(	const void* data ) {
	using namespace Scene;
	auto creation = (const Scene::Resource::CreationInfo*) data;

	size_t size = (size_t)Core::alignTo( creation->width, DataBuffer::MIN_BUFFER_SIZE);
	uint32_t usage = 0;
	uint32_t cpuAccess = 0;
	D3D11_SUBRESOURCE_DATA init;
	D3D11_SUBRESOURCE_DATA* initer = nullptr;

	// can't read and write from teh cpu to the same buffer
	CORE_ASSERT( (( creation->flags & RCF_ACE_CPU_WRITE ) == false) || (( creation->flags & RCF_ACE_CPU_READ ) == false) );

	if( creation->flags & RCF_ACE_CPU_STAGING ) {
		usage = D3D11_USAGE_STAGING; // CPU to GPU transfers
		if( creation->flags & RCF_ACE_CPU_WRITE ) {
			cpuAccess = D3D11_CPU_ACCESS_WRITE;
		} else {
			cpuAccess = D3D11_CPU_ACCESS_READ;
		}
	} else if( creation->flags & RCF_ACE_CPU_WRITE ) {
		if( creation->flags & RCF_ACE_ONCE ) {
			usage = D3D11_USAGE_DEFAULT;
		} else {
			usage = D3D11_USAGE_DYNAMIC;
		}
		cpuAccess = D3D11_CPU_ACCESS_WRITE;
	} else if( creation->flags & RCF_ACE_GPU_WRITE_ONLY ) {
		usage = D3D11_USAGE_DEFAULT;
		cpuAccess = 0;
	} else if( creation->flags & RCF_ACE_IMMUTABLE ) {
		usage = D3D11_USAGE_IMMUTABLE;
		cpuAccess = 0;
		initer = &init;
		init.pSysMem = creation->prefillData;
	} else if( creation->flags & RCF_ACE_CPU_READ ) {	
		usage = D3D11_USAGE_DYNAMIC;
		cpuAccess = D3D11_CPU_ACCESS_READ;
	}else {
		usage = D3D11_USAGE_DEFAULT;
		cpuAccess = 0;
	}

	uint32_t bind = 0;
	uint32_t misc = 0;
	if( creation->flags & RCF_BUF_CONSTANT ) {
		bind |= D3D11_BIND_CONSTANT_BUFFER;
	}
	if( creation->flags & RCF_BUF_VERTEX ) {
		bind |= D3D11_BIND_VERTEX_BUFFER;
	}
	if( creation->flags & RCF_BUF_INDEX ) {
		bind |= D3D11_BIND_INDEX_BUFFER;
	}
	if( creation->flags & RCF_BUF_STREAMOUT ) {
		bind |= D3D11_BIND_STREAM_OUTPUT;
	}
	if( creation->flags & RCF_PRG_READ ) {
		bind |= D3D11_BIND_SHADER_RESOURCE;
	}
	if( creation->flags & RCF_OUT_UNORDERED_ACCESS ) {
		bind |= D3D11_BIND_UNORDERED_ACCESS;
	}
	if( creation->flags & RCF_PRG_BYTE_ACCESS ) {
		misc |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	}
	if( creation->flags & RCF_PRG_STRUCTURED ) {
		misc |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		size = creation->width; // don't mess with structured buffers sizes
	}
	if( creation->flags & RCF_ACE_GPU_INDIRECT ) {
		misc |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
	}

	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage            = (D3D11_USAGE) usage;
	bufferDesc.ByteWidth        = (UINT)size;
	bufferDesc.BindFlags        = bind;
	bufferDesc.CPUAccessFlags   = cpuAccess;
	bufferDesc.MiscFlags        = misc;
	bufferDesc.StructureByteStride = creation->structureSize;

	HRESULT hr;
	ID3D11Buffer* buffer;
	DXFAIL( Gfx::getr()()->CreateBuffer( &bufferDesc, initer, &buffer ) );
	
	DataBuffer* dbuffer = CORE_NEW DataBuffer( D3DBufferPtr( buffer, false ) );
	dbuffer->size = size;

	// create default views for this buffer
	if( bind & D3D11_BIND_SHADER_RESOURCE ) {
		dbuffer->createView( SHADER_RESOURCE_VIEW, creation );
	}
	if( bind & D3D11_BIND_UNORDERED_ACCESS ) {
		dbuffer->createView( UNORDERED_ACCESS_VIEW, creation );
	}
	return dbuffer;
}

}
