//--------------------------------------------------------------------------------------
// File: DDSTextureLoader.cpp
//
// Functions for loading a DDS texture without using D3DX
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "gl.h"
#include "core/fileio.h"

#include "dds.h"

//--------------------------------------------------------------------------------------
bool LoadDDSFromFile( const char* fileName, uint8_t** ppHeapData,
										DDS_HEADER** ppHeader,
										uint8_t** ppBitData, size_t* pBitSize ) {

	Core::File file( fileName );

	const uint64_t fileSize = file.bytesLeft();

	// Need at least enough data to fill the header and magic number to be a valid DDS
	if( fileSize < (uint64_t)(sizeof(DDS_HEADER)+sizeof(uint32_t)) ) {
		return false;
	}

	// ensure memory alloc size is < 32 bit on 32 bit builds
	const size_t memSize = (size_t) fileSize;
	if( fileSize != (uint64_t)memSize ) {
		return false;
	}
	*ppHeapData = CORE_NEW_ARRAY uint8_t[ memSize ];
	if( !( *ppHeapData ) ) {
		return false;
	}

	uint64_t bytesRead = file.read( *ppHeapData, fileSize );
	if( bytesRead != fileSize ) {
		CORE_DELETE_ARRAY( *ppHeapData );
		return false;
	}

	file.close();

	// DDS files always start with the same magic number ("DDS ")
	uint32_t dwMagicNumber = *(uint32_t*)( *ppHeapData );
	if( dwMagicNumber != DDS_MAGIC ) {
		CORE_DELETE_ARRAY( *ppHeapData );
		return false;
	}

	DDS_HEADER* pHeader = reinterpret_cast<DDS_HEADER*>( *ppHeapData + sizeof( uint32_t ) );

	// Verify header to validate DDS file
	if( pHeader->dwSize != sizeof(DDS_HEADER) || 
		pHeader->ddspf.dwSize != sizeof(DDS_PIXELFORMAT) ) {
		CORE_DELETE_ARRAY( *ppHeapData );
		return false;
	}

	// Check for DX10 extension
	bool bDXT10Header = false;
	if ( (pHeader->ddspf.dwFlags & DDS_FOURCC) && 
		(DDS_MAKEFOURCC( 'D', 'X', '1', '0' ) == pHeader->ddspf.dwFourCC) ) {
		// Must be long enough for both headers and magic value
		if( fileSize < (uint64_t) (sizeof(DDS_HEADER)+sizeof(uint32_t)+sizeof(DDS_HEADER_DXT10)) ) {
			CORE_DELETE_ARRAY( *ppHeapData );
			return false;
		}

		bDXT10Header = true;
	}

	// setup the pointers in the process request
	*ppHeader = pHeader;
	uint32_t offset = sizeof( uint32_t ) + sizeof( DDS_HEADER )
				 + (bDXT10Header ? sizeof( DDS_HEADER_DXT10 ) : 0);
	*ppBitData = *ppHeapData + offset;
	*pBitSize = (size_t)(fileSize - offset);

	return true;
}

void GetSurfaceInfo(	unsigned int width, unsigned int height, DXGI_FORMAT fmt, 
						unsigned int * pNumBytes, unsigned int * pRowBytes, unsigned int * pNumRows ) {
	unsigned int numBytes = 0;
	unsigned int rowBytes = 0;
	unsigned int numRows = 0;

	bool bc = true;
	int bcnumBytesPerBlock = 0;
	switch (fmt) {
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		bcnumBytesPerBlock = 8;
		break;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		bcnumBytesPerBlock = 16;
		break;

	default:
		bc = false;
		break;
	}

	if( bc ) {
		int numBlocksWide = 0;
		if( width > 0 )
			numBlocksWide = std::max<unsigned int>( 1, width / 4 );
		int numBlocksHigh = 0;
		if( height > 0 )
			numBlocksHigh = std::max<unsigned int>( 1, height / 4 );
		rowBytes = numBlocksWide * bcnumBytesPerBlock;
		numRows = numBlocksHigh;
	} else {
		unsigned int bpp = DXGIFormat::GetBitWidth( fmt );
		rowBytes = ( width * bpp + 7 ) / 8; // round up to nearest byte
		numRows = height;
	}
	numBytes = rowBytes * numRows;
	if( pNumBytes != NULL )
		*pNumBytes = numBytes;
	if( pRowBytes != NULL )
		*pRowBytes = rowBytes;
	if( pNumRows != NULL )
		*pNumRows = numRows;
}


//--------------------------------------------------------------------------------------
#define ISBITMASK( r,g,b,a ) ( ddpf.dwRBitMask == r && ddpf.dwGBitMask == g && ddpf.dwBBitMask == b && ddpf.dwABitMask == a )

//--------------------------------------------------------------------------------------
DXGI_FORMAT GetDXGIFormat( const DDS_PIXELFORMAT& ddpf ) {
	if( ddpf.dwFlags & DDS_RGB ) {
		switch (ddpf.dwRGBBitCount) {
		case 32:
			// DXGI_FORMAT_B8G8R8A8_UNORM_SRGB & DXGI_FORMAT_B8G8R8X8_UNORM_SRGB should be
			// written using the DX10 extended header instead since these formats require
			// DXGI 1.1
			//
			// This code will use the fallback to swizzle RGB to BGR in memory for standard
			// DDS files which works on 10 and 10.1 devices with WDDM 1.0 drivers
			//
			// NOTE: We don't use DXGI_FORMAT_B8G8R8X8_UNORM or DXGI_FORMAT_B8G8R8X8_UNORM
			// here because they were defined for DXGI 1.0 but were not required for D3D10/10.1

			if( ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0xff000000) )
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			if( ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) )
				return DXGI_FORMAT_R8G8B8A8_UNORM; // No D3DFMT_X8B8G8R8 in DXGI

			// Note that many common DDS reader/writers swap the
			// the RED/BLUE masks for 10:10:10:2 formats. We assumme
			// below that the 'correct' header mask is being used. The
			// more robust solution is to use the 'DX10' header extension and
			// specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly
			if( ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) )
				return DXGI_FORMAT_R10G10B10A2_UNORM;

			if( ISBITMASK(0x0000ffff,0xffff0000,0x00000000,0x00000000) )
				return DXGI_FORMAT_R16G16_UNORM;

			if( ISBITMASK(0xffffffff,0x00000000,0x00000000,0x00000000) )
				// Only 32-bit color channel format in D3D9 was R32F
				return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
			break;

		case 24:
			// No 24bpp DXGI formats
			break;

		case 16:
			// 5:5:5 & 5:6:5 formats are defined for DXGI, but are deprecated for D3D10+

			if( ISBITMASK(0x0000f800,0x000007e0,0x0000001f,0x00000000) )
				return DXGI_FORMAT_B5G6R5_UNORM;
			if( ISBITMASK(0x00007c00,0x000003e0,0x0000001f,0x00008000) )
				return DXGI_FORMAT_B5G5R5A1_UNORM;
			if( ISBITMASK(0x00007c00,0x000003e0,0x0000001f,0x00000000) )
				return DXGI_FORMAT_B5G5R5A1_UNORM; // No D3DFMT_X1R5G5B5 in DXGI

			// No 4bpp or 3:3:2 DXGI formats
			break;
		}
	} else if( ddpf.dwFlags & DDS_LUMINANCE ) {
		if( 8 == ddpf.dwRGBBitCount ) {
			if( ISBITMASK(0x000000ff,0x00000000,0x00000000,0x00000000) )
				return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension

			// No 4bpp DXGI formats
		}

		if( 16 == ddpf.dwRGBBitCount ) {
			if( ISBITMASK(0x0000ffff,0x00000000,0x00000000,0x00000000) )
				return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
			if( ISBITMASK(0x000000ff,0x00000000,0x00000000,0x0000ff00) )
				return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
		}
	}
	else if( ddpf.dwFlags & DDS_ALPHA ) {
		if( 8 == ddpf.dwRGBBitCount ) {
			return DXGI_FORMAT_A8_UNORM;
		}
	}
	else if( ddpf.dwFlags & DDS_FOURCC ) {
		if( DDS_MAKEFOURCC( 'D', 'X', 'T', '1' ) == ddpf.dwFourCC )
			return DXGI_FORMAT_BC1_UNORM;
		if( DDS_MAKEFOURCC( 'D', 'X', 'T', '3' ) == ddpf.dwFourCC )
			return DXGI_FORMAT_BC2_UNORM;
		if( DDS_MAKEFOURCC( 'D', 'X', 'T', '5' ) == ddpf.dwFourCC )
			return DXGI_FORMAT_BC3_UNORM;

		if( DDS_MAKEFOURCC( 'B', 'C', '4', 'U' ) == ddpf.dwFourCC )
			return DXGI_FORMAT_BC4_UNORM;
		if( DDS_MAKEFOURCC( 'B', 'C', '4', 'S' ) == ddpf.dwFourCC )
			return DXGI_FORMAT_BC4_SNORM;

		if( DDS_MAKEFOURCC( 'A', 'T', 'I', '2' ) == ddpf.dwFourCC )
			return DXGI_FORMAT_BC5_UNORM;
		if( DDS_MAKEFOURCC( 'B', 'C', '5', 'S' ) == ddpf.dwFourCC )
			return DXGI_FORMAT_BC5_SNORM;

		if( DDS_MAKEFOURCC( 'R', 'G', 'B', 'G' ) == ddpf.dwFourCC )
			return DXGI_FORMAT_R8G8_B8G8_UNORM;
		if( DDS_MAKEFOURCC( 'G', 'R', 'G', 'B' ) == ddpf.dwFourCC )
			return DXGI_FORMAT_G8R8_G8B8_UNORM;

		// Check for D3DFORMAT enums being set here
		switch( ddpf.dwFourCC ) {
		case 36: //D3DFMT_A16B16G16R16: // 36
			return DXGI_FORMAT_R16G16B16A16_UNORM;

		case 110: //D3DFMT_Q16W16V16U16: // 110
			return DXGI_FORMAT_R16G16B16A16_SNORM;

		case 111: //D3DFMT_R16F: // 111
			return DXGI_FORMAT_R16_FLOAT;

		case 112: //D3DFMT_G16R16F: // 112
			return DXGI_FORMAT_R16G16_FLOAT;

		case 113: //D3DFMT_A16B16G16R16F: // 113
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case 114: //D3DFMT_R32F: // 114
			return DXGI_FORMAT_R32_FLOAT;

		case 115: //D3DFMT_G32R32F: // 115
			return DXGI_FORMAT_R32G32_FLOAT;

		case 116: //D3DFMT_A32B32G32R32F: // 116
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
	}

	return DXGI_FORMAT_UNKNOWN;
}
/*
//--------------------------------------------------------------------------------------
void CreateTextureFromDDS( DDS_HEADER* pHeader, uint8_t* pBitData, unsigned int BitSize, bool bSRGB ) {
	HRESULT hr = S_OK;

	UINT iWidth = pHeader->dwWidth;
	UINT iHeight = pHeader->dwHeight;
	UINT iMipCount = pHeader->dwMipMapCount;
	if( 0 == iMipCount )
		iMipCount = 1;

	// Bound miplevels (affects the memory usage below)
	if ( iMipCount > D3D11_REQ_MIP_LEVELS )
		return HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );

	if ((  pHeader->ddspf.dwFlags & DDS_FOURCC )
		&& (DDS_MAKEFOURCC( 'D', 'X', '1', '0' ) == pHeader->ddspf.dwFourCC ) )
	{
		DDS_HEADER_DXT10* d3d10ext = (DDS_HEADER_DXT10*)( (char*)pHeader + sizeof(DDS_HEADER) );

		// For now, we only support 2D textures
		if ( d3d10ext->resourceDimension != D3D11_RESOURCE_DIMENSION_TEXTURE2D )
			return HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );

		// Bound array sizes (affects the memory usage below)
		if ( d3d10ext->arraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION )
			return HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );

	} else {
		desc.Format = GetDXGIFormat( pHeader->ddspf );

		if (pHeader->dwCubemapFlags != 0
			|| (pHeader->dwHeaderFlags & DDS_HEADER_FLAGS_VOLUME) ) {
			// For now only support 2D textures, not cubemaps or volumes
			return E_FAIL;
		}

  
			// Need more room to try to swizzle 24bpp formats
			// Could also try to expand 4bpp or 3:3:2 formats

			default:
				return HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );
			}
		}
	}
	
	if ( bSRGB )
		desc.Format = MAKE_SRGB( desc.Format );

	UINT NumBytes = 0;
	UINT RowBytes = 0;
	UINT NumRows = 0;
	BYTE* pSrcBits = pBitData;

	UINT index = 0;
	for( UINT j = 0; j < desc.ArraySize; j++ ) {
		UINT w = iWidth;
		UINT h = iHeight;
		for( UINT i = 0; i < iMipCount; i++ ) {
			GetSurfaceInfo( w, h, desc.Format, &NumBytes, &RowBytes, &NumRows );
			pInitData[index].pSysMem = ( void* )pSrcBits;
			pInitData[index].SysMemPitch = RowBytes;
			++index;
	
			pSrcBits += NumBytes;
			w = w >> 1;
			h = h >> 1;
			if( w == 0 )
				w = 1;
			if( h == 0 )
				h = 1;
		}
	}

	SAFE_DELETE_ARRAY( pInitData );

	return hr;
}
*/