//!-----------------------------------------------------
//!
//! \file ibmanager.cpp
//! manager for the index allocations
//!
//!-----------------------------------------------------

#include "graphics_pch.h"
#include "rendercontext.h"
#include "ibmanager.h"

namespace
{
	static const uint32_t s_static_ib_size = 128 * 1024; // static IB 128K alloc
	static const uint32_t s_dynamic_ib_size = 128 * 1024; // dynamic IB 128K alloc
}

namespace Graphics
{

IBManager::~IBManager() {
	// delete all IB's left
	ListIBAlloc::iterator vbIt;
	vbIt = m_ListIBAllocs.begin();
	while( vbIt != m_ListIBAllocs.end() )
	{
		D3D11_BUFFER_DESC desc;
		vbIt->m_pIB->GetDesc( &desc );
		LOG(INFO) << "Index Buffer of size " << desc.ByteWidth << " not released\n";

		vbIt->m_pIB->Release(); 
		++vbIt;
	}
}

void IBManager::Reset() {
}

const IBManager::IBInstance IBManager::AllocIndices( const uint32_t iSizeInBytes, const bool b32Bit, const bool bDynamic ) {
	HRESULT hr;

	ListIBAlloc::iterator vbIt;

	// scan allocs for free space
	vbIt = m_ListIBAllocs.begin();
	while( vbIt != m_ListIBAllocs.end() ) {
		if( vbIt->m_iFreeSpace >= iSizeInBytes && 
			bDynamic == vbIt->m_bDynamic && 
			vbIt->m_32bit == b32Bit ) {
			break;
		}
		++vbIt;
	}

	// okay I have some logic bugs to work out with dynamic and DISCARD so for now don't
	// share
	if( bDynamic == true ) {
		vbIt = m_ListIBAllocs.end();
	}

	// if we don't find a free space, alloc a new one
	if( vbIt == m_ListIBAllocs.end() ) {
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
			bufferDesc.BindFlags        = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags        = 0;

			DXFAIL( Gfx::Get()->GetDevice()->CreateBuffer( &bufferDesc, NULL, &pVB ) );
		} else {
			iAllocSize = std::max( iSizeInBytes, s_static_ib_size );
//			DXFAIL( Gfx::Get()->GetDevice()->CreateVertexBuffer( iAllocSize, 0, 0, D3DPOOL_MANAGED, &pVB, 0 ) );
		}
		m_ListIBAllocs.push_back( IBAlloc( pVB, iAllocSize, bDynamic, b32Bit) );
		vbIt = --m_ListIBAllocs.end();
	}

	assert( vbIt->m_iFreeSpace >= iSizeInBytes );

	const uint32_t startB = vbIt->m_iFirstUnused;
	vbIt->m_iFirstUnused += iSizeInBytes;
	vbIt->m_iFreeSpace -= iSizeInBytes;
	vbIt->m_iRefCount++;

	IBInstance instance;
	instance.m_pIB = vbIt->m_pIB;
	instance.m_iStartInBytes = startB;
	instance.m_iSizeInBytes = iSizeInBytes;
	instance.m_bDynamic = bDynamic;
	instance.m_32bit = b32Bit;
	return instance;
}
const IBManager::IBInstance IBManager::AllocIndices( void* _data, const uint32_t _sizeInBytes, const bool b32Bit ) {
	HRESULT hr;

	ID3D11Buffer* pVB;
	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage            = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth        = _sizeInBytes;
	bufferDesc.BindFlags        = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags   = 0;
	bufferDesc.MiscFlags        = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = _data;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	DXFAIL( Gfx::Get()->GetDevice()->CreateBuffer( &bufferDesc, &initData, &pVB ) );
	m_ListIBAllocs.push_back( IBAlloc( pVB, _sizeInBytes, false, b32Bit) );
	ListIBAlloc::iterator vbIt = --m_ListIBAllocs.end();

	const uint32_t startB = vbIt->m_iFirstUnused;
	vbIt->m_iFirstUnused += _sizeInBytes;
	vbIt->m_iFreeSpace -= _sizeInBytes;
	vbIt->m_iRefCount++;

	IBInstance instance;
	instance.m_pIB = vbIt->m_pIB;
	instance.m_iStartInBytes = startB;
	instance.m_iSizeInBytes = _sizeInBytes;
	instance.m_bDynamic = false;
	instance.m_32bit = b32Bit;
	return instance;
}

void IBManager::FreeIndices( const IBInstance& ibInstance )
{
	ListIBAlloc::iterator vbIt;

	// scan allocs for free space
	vbIt = m_ListIBAllocs.begin();
	while( vbIt != m_ListIBAllocs.end() )
	{
		if( vbIt->m_pIB == ibInstance.m_pIB )
		{
			vbIt->m_iRefCount--;
			if( vbIt->m_iRefCount == 0 )
			{
				vbIt->m_pIB->Release(); 
				m_ListIBAllocs.erase( vbIt );
			}
			return;
		}
		++vbIt;
	}
}

void* IBManager::IBInstance::Lock( RenderContext* context, LOCK_FLAGS flags ) {
	HRESULT hr;
	D3D11_MAP iFlags = D3D11_MAP_WRITE;
	if( m_bDynamic ) {	
		if( flags & DISCARD ) {
			iFlags = D3D11_MAP_WRITE_DISCARD;
		} else if( flags & NOOVERWRITE ) {
			iFlags = D3D11_MAP_WRITE_NO_OVERWRITE;
		}
	} else {
		assert( false );
	}
	D3D11_MAPPED_SUBRESOURCE mapped;
	DXFAIL( context->deviceContext->Map( m_pIB, 0, iFlags, 0, &mapped ) );

	return ((uint8_t*)mapped.pData) + m_iStartInBytes;
}

void IBManager::IBInstance::Unlock( RenderContext* context ) {
	context->deviceContext->Unmap( m_pIB, 0 );
}

void IBManager::IBInstance::Bind( RenderContext* context ) {
	if( m_32bit ) {
		context->deviceContext->IASetIndexBuffer( m_pIB, DXGI_FORMAT_R32_UINT, 0 );
	} else {
		context->deviceContext->IASetIndexBuffer( m_pIB, DXGI_FORMAT_R16_UINT, 0 );
	}
}

} // end namespace