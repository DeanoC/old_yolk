/**
 @file	databuffer.cpp

 @brief	Implements the data buffer objects.
 */
#include "dx11.h"
#include "databuffer.h"

namespace {

// map DATA_BUFFER_MAP_ACCESS to Dx11
static const D3D11_MAP DBMA_Map[] = {
	D3D11_MAP_READ,										// DBMA_READ_ONLY
	D3D11_MAP_WRITE,									// DBMA_WRITE_ONLY
	D3D11_MAP_READ_WRITE,							// DBMA_READ_WRITE
};

}

namespace Dx11 {

void* DataBuffer::map( Scene::RenderContext* scontext, Scene::DATA_BUFFER_MAP_ACCESS _access, Scene::DATA_BUFFER_MAP_FLAGS _flags, size_t offset, size_t bytes ) {
	RenderContext* context = static_cast<RenderContext*>( scontext );
	using namespace Scene;
	D3D11_MAP access = DBMA_Map[ _access ];

	if( access == D3D11_MAP_WRITE ) {
		if( _flags & DBMF_DISCARD  ) {
			access = D3D11_MAP_WRITE_DISCARD;
		} else if( _flags & DBMF_UNSYNC ) {
			access = D3D11_MAP_WRITE_NO_OVERWRITE;
		}
	}

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mapped;
	DXFAIL( context->ctx->Map( resource.get(), 0, access, 0, &mapped ) );

	return ((uint8_t*)mapped.pData) + offset;
}

void DataBuffer::unmap( Scene::RenderContext* scontext  ) {
	RenderContext* context = static_cast<RenderContext*>( scontext );
	context->ctx->Unmap( resource.get(), 0 );
}

DataBuffer* DataBuffer::internalCreate(	const void* data ) {
	using namespace Scene;
	auto creation = (const DataBuffer::CreationInfo*) data;

	size_t size = (size_t)Core::alignTo( creation->width, DataBuffer::MIN_BUFFER_SIZE);
	uint32_t usage = 0;
	uint32_t cpuAccess = 0;
	D3D11_SUBRESOURCE_DATA init;
	D3D11_SUBRESOURCE_DATA* initer = nullptr;

	// can't read and write from teh cpu to the same buffer
	CORE_ASSERT( (( creation->flags & RCF_ACE_CPU_WRITE ) == false) || (( creation->flags & RCF_ACE_CPU_READ ) == false) );

	if( creation->flags & RCF_ACE_CPU_STAGING ) {
		usage = D3D11_USAGE_STAGING; // CPU read-backs
		cpuAccess = D3D11_CPU_ACCESS_READ;
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
	}
	CORE_ASSERT( usage != 0 );

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

	HRESULT hr;
	ID3D11Buffer* buffer;
	DXFAIL( Gfx::getr()()->CreateBuffer( &bufferDesc, initer, &buffer ) );
	
	DataBuffer* dbuffer = CORE_NEW DataBuffer( D3DBufferPtr( buffer, false ) );
	dbuffer->size = size;

	return dbuffer;
}

}