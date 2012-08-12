//!-----------------------------------------------------
//!
//! \file vbmanager.cpp
//! manager for the vertex allocations
//!
//!-----------------------------------------------------

#include "graphics_pch.h"
#include "rendercontext.h"
#include "vbmanager.h"

namespace
{
	static const uint32_t s_static_vb_size = 1024 * 1024; // static VB 1Mb alloc
	static const uint32_t s_dynamic_vb_size = 32 * 1024; // dynamic VB 32K alloc (cos no sharing currently...)
}

namespace Graphics
{

VBManager::~VBManager() {
	// delete all VB's left
	ListVBAlloc::iterator vbIt;
	vbIt = m_ListVBAllocs.begin();
	while( vbIt != m_ListVBAllocs.end() ) {
		if( vbIt->m_iRefCount != 0 ) {
			D3D11_BUFFER_DESC desc;
			vbIt->m_pVB->GetDesc( &desc );
			LOG(INFO) << "Vertex Buffer of size " << desc.ByteWidth << " not released\n";
		}

		vbIt->m_pVB->Release(); 
		++vbIt;
	}
}

void VBManager::Reset() {
}

const VBManager::VBInstance VBManager::AllocVertices( const uint32_t iSizeInBytes, const bool bDynamic ) {
	HRESULT hr;

	ListVBAlloc::iterator vbIt;

	// scan allocs for free space
	vbIt = m_ListVBAllocs.begin();
	while( vbIt != m_ListVBAllocs.end() ) {
		if( vbIt->m_iFreeSpace >= iSizeInBytes && bDynamic == vbIt->m_bDynamic ) {
			break;
		}
		++vbIt;
	}

	// if we don't find a free space, alloc a new one
	if( vbIt == m_ListVBAllocs.end() ) {
		// if the size requested is bigger than our alloc size
		// give this allocation its own block
		uint32_t iAllocSize;
		ID3D11Buffer* pVB;
		if( bDynamic ) {
			iAllocSize = iSizeInBytes;//std::max( iSizeInBytes, s_dynamic_vb_size );
			// Fill in a buffer description.
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage            = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth        = iSizeInBytes;
			bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags        = 0;

			DXFAIL( Gfx::Get()->GetDevice()->CreateBuffer( &bufferDesc, NULL, &pVB ) );
		} else {
			iAllocSize = std::max( iSizeInBytes, s_static_vb_size );
			// Fill in a buffer description.
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage            = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth        = iAllocSize;
			bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags   = 0;
			bufferDesc.MiscFlags        = 0;

			DXFAIL( Gfx::Get()->GetDevice()->CreateBuffer( &bufferDesc, NULL, &pVB ) );
		}
		m_ListVBAllocs.push_back( VBAlloc( pVB, iAllocSize, bDynamic) );
		vbIt = --m_ListVBAllocs.end();
	}

	assert( vbIt->m_iFreeSpace >= iSizeInBytes );

	const uint32_t startB = vbIt->m_iFirstUnused;
	vbIt->m_iFirstUnused += iSizeInBytes;
	vbIt->m_iFreeSpace -= iSizeInBytes;
	vbIt->m_iRefCount++;

	VBInstance instance;
	instance.m_pVB = vbIt->m_pVB;
	instance.m_iStartInBytes = startB;
	instance.m_iSizeInBytes = iSizeInBytes;
	instance.m_bDynamic = bDynamic;
	return instance;
}
const VBManager::VBInstance VBManager::AllocVertices( void* _data, const uint32_t _sizeInBytes ) {
	HRESULT hr;

	ID3D11Buffer* pVB;
	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage            = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth        = _sizeInBytes;
	bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags   = 0;
	bufferDesc.MiscFlags        = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = _data;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	DXFAIL( Gfx::Get()->GetDevice()->CreateBuffer( &bufferDesc, &initData, &pVB ) );
	m_ListVBAllocs.push_back( VBAlloc( pVB, _sizeInBytes, false) );
	ListVBAlloc::iterator vbIt = --m_ListVBAllocs.end();

	const uint32_t startB = vbIt->m_iFirstUnused;
	vbIt->m_iFirstUnused += _sizeInBytes;
	vbIt->m_iFreeSpace -= _sizeInBytes;
	vbIt->m_iRefCount++;

	VBInstance instance;
	instance.m_pVB = vbIt->m_pVB;
	instance.m_iStartInBytes = startB;
	instance.m_iSizeInBytes = _sizeInBytes;
	instance.m_bDynamic = false;
	return instance;
}

void VBManager::FreeVertices( const VBInstance& vbInstance ) {
	ListVBAlloc::iterator vbIt;

	// scan allocs for free space
	vbIt = m_ListVBAllocs.begin();
	while( vbIt != m_ListVBAllocs.end() ) {
		if( vbIt->m_pVB == vbInstance.m_pVB ) {
			vbIt->m_iRefCount--;
			if( vbIt->m_iRefCount == 0 ) {
				// ready to be re-used
				vbIt->m_iFreeSpace = vbIt->m_maxFreeSpace;
				vbIt->m_iFirstUnused = 0;
			}
			return;
		}
		++vbIt;
	}
}

void* VBManager::VBInstance::Lock( RenderContext* context, LOCK_FLAGS flags ) {
	HRESULT hr;
	D3D11_MAP iFlags = D3D11_MAP_WRITE;
	if( m_bDynamic ) {	
		if( flags & DISCARD ) {
			iFlags = D3D11_MAP_WRITE_DISCARD;
		} else if( flags & NOOVERWRITE ) {
			iFlags = D3D11_MAP_WRITE_NO_OVERWRITE;
		} else {
			assert( false );
		}
	} else {
		assert( false );
	}
	D3D11_MAPPED_SUBRESOURCE mapped;
	DXFAIL( context->deviceContext->Map( m_pVB, 0, iFlags, 0, &mapped ) );

	return ((uint8_t*)mapped.pData) + m_iStartInBytes;
}

void VBManager::VBInstance::Unlock( RenderContext* context ) {
	context->deviceContext->Unmap( m_pVB, 0 );
}

void VBManager::VBInstance::Bind( RenderContext* context, uint32_t uiStreamNum, uint32_t stride ) {
	ID3D11Buffer* vbs[] = { m_pVB };
	UINT strides[] = { stride };
	UINT offsets[] = { m_iStartInBytes };
	context->deviceContext->IASetVertexBuffers( uiStreamNum, 1, vbs, strides, offsets );
}
void VBManager::VBInstance::Unbind( RenderContext* context, uint32_t uiStreamNum ) {
	ID3D11Buffer* vbs[] = { 0 };
	UINT strides[] = { 0 };
	UINT offsets[] = { 0 };
	context->deviceContext->IASetVertexBuffers( uiStreamNum, 1, vbs, strides, offsets );
}

} // end namespace