//!-----------------------------------------------------
//!
//! \file resource.cpp
//! view managment for dx11 resources
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "gfx.h"
#include "texture.h"
#include "rendercontext.h"
#include "core/resourceman.h"
#include "core/file_path.h"

namespace Dx11
{

uint32_t Resource::createView( uint32_t viewType, Scene::Resource::CreationInfo* creation ) {

	bool custom = !!(viewType & CUSTOM_VIEW);
	viewType = viewType & ~CUSTOM_VIEW;
	CORE_ASSERT( viewType <= UNORDERED_ACCESS_VIEW );
	uint32_t index;
	if( custom ) {
		views.resize( views.size() + 1 );
		index = (uint32_t)(views.size() - 1);
	} else {
		index = viewType;
		views.resize( Math::Max( viewType+1, (uint32_t)views.size()) );		
	}
	switch( viewType ) {
		case SHADER_RESOURCE_VIEW: 		createSRView( index, creation ); break;
		case DEPTH_STENCIL_VIEW:		createDSView( index, creation ); break;
		case RENDER_TARGET_VIEW:		createRTView( index, creation ); break;
		case UNORDERED_ACCESS_VIEW:		createUAView( index, creation ); break;
		default: CORE_ASSERT( false );	
	}
	return index;
}

void Resource::createSRView( uint32_t index, Scene::Resource::CreationInfo* creation ) {
	using namespace Scene;
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	srDesc.Format = DXGIFormat::getDXGIFormat( creation->format );

	if( DXGIFormat::isDepthStencilFormat( srDesc.Format ) ) {
		srDesc.Format = DXGIFormat::getShaderDepthTextureFormat( srDesc.Format );
	}

	if( creation->flags & (RCF_BUF_CONSTANT | RCF_BUF_VERTEX | RCF_BUF_INDEX | RCF_BUF_STREAMOUT | RCF_BUF_GENERAL) ) {
		// a buffer view is a shader resource view but typed as a buffer rather than a texture
		srDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		if( creation->flags & RCF_PRG_STRUCTURED ) {
			srDesc.Format = DXGI_FORMAT_UNKNOWN;
		}
		srDesc.BufferEx.NumElements = creation->width;
		srDesc.BufferEx.FirstElement = 0;
		if( creation->flags & RCF_PRG_BYTE_ACCESS ) {
			srDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		} else {
			srDesc.BufferEx.Flags = 0;
		}
	} else if( creation->flags & RCF_TEX_1D ) {
		// 1d and arrays of them
		if( creation->slices > 1 ) {
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
			srDesc.Texture1DArray.MipLevels = creation->mipLevels;
			srDesc.Texture1DArray.MostDetailedMip = 0;
			srDesc.Texture1DArray.ArraySize = creation->slices;
			srDesc.Texture1DArray.FirstArraySlice = 0;
		} else {
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
			srDesc.Texture1D.MipLevels = creation->mipLevels;
			srDesc.Texture1D.MostDetailedMip = 0;
		}
	} else if( creation->flags & RCF_TEX_CUBE_MAP ) {
		// cube and arrays of them
		if( creation->slices > 1 ) {
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
			srDesc.TextureCubeArray.NumCubes = creation->slices / 6;
			srDesc.TextureCubeArray.First2DArrayFace = 0;
			srDesc.TextureCubeArray.MipLevels = creation->mipLevels;
			srDesc.TextureCubeArray.MostDetailedMip = 0;
		} else {
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srDesc.TextureCube.MipLevels = creation->mipLevels;
			srDesc.TextureCube.MostDetailedMip = 0;
		}
	} else if( creation->flags & RCF_TEX_3D ) {
		// 3d arrays don't exist yet
		CORE_ASSERT( creation->slices <= 1 );
/*		if( creation->slices > 1 ) {
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3DARRAY;
			srDesc.Texture3DArray.ArraySize = creation->slices;
			srDesc.Texture3DArray.MipLevels = creation->mipLevels;
			srDesc.Texture3DArray.MostDetailedMip = 0;
		} else */ {
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			srDesc.Texture3D.MipLevels = creation->mipLevels;
			srDesc.Texture3D.MostDetailedMip = 0;			
		}
	} else if( creation->flags & RCF_TEX_2D ) {
		if( creation->samples > 1 ) {
			// 2D multisample and arrays of them
			if( creation->slices > 1 ) {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
				srDesc.Texture2DMSArray.ArraySize = creation->slices;
				srDesc.Texture2DMSArray.FirstArraySlice = 0;
			} else {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
			}
		} else { 
			if( creation->slices > 1 ) {
				// good ol' fashioned 2D, well maybe an array of them...
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				srDesc.Texture2DArray.ArraySize = creation->slices;
				srDesc.Texture2DArray.FirstArraySlice = 0;
				srDesc.Texture2DArray.MipLevels = creation->mipLevels;
				srDesc.Texture2DArray.MostDetailedMip = 0;
			} else {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srDesc.Texture2D.MipLevels = creation->mipLevels;
				srDesc.Texture2D.MostDetailedMip = 0;
			}
		}
	}

	HRESULT hr;
	ID3D11ShaderResourceView* view;
	DXFAIL( Gfx::getr()()->CreateShaderResourceView( resource.get(), &srDesc, &view ) );
	views[ index ] = D3DViewPtr( view, false );
}
void Resource::createDSView( uint32_t index, Scene::Resource::CreationInfo* creation ) {
	D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc;
	dsDesc.Flags = 0;
	dsDesc.Format = DXGIFormat::getDXGIFormat( creation->format );
	if( creation->samples > 1 ) {
		dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	} else {
		dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsDesc.Texture2D.MipSlice = 0;
	}
	HRESULT hr;
	ID3D11DepthStencilView* view;
	DXFAIL( Gfx::getr()()->CreateDepthStencilView( resource.get(), &dsDesc, &view ) );
	views[ index ] = D3DViewPtr( view, false );

}
void Resource::createRTView( uint32_t index, Scene::Resource::CreationInfo* creation ) {
	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	rtDesc.Format = DXGIFormat::getDXGIFormat( creation->format );
	if( creation->samples > 1 ) {
		rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	} else {
		rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtDesc.Texture2D.MipSlice = 0;
	}
	HRESULT hr;
	ID3D11RenderTargetView* view;
	DXFAIL( Gfx::getr()()->CreateRenderTargetView( resource.get(), &rtDesc, &view ) );
	views[ index ] = D3DViewPtr( view, false );

}

void Resource::createUAView( uint32_t index, Scene::Resource::CreationInfo* creation ) {
	using namespace Scene;
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGIFormat::getDXGIFormat( creation->format );

	if( creation->flags & (RCF_BUF_CONSTANT | RCF_BUF_VERTEX | RCF_BUF_INDEX | RCF_BUF_STREAMOUT | RCF_BUF_GENERAL) ) {
		if( creation->flags & RCF_PRG_STRUCTURED ) {
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		}
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = creation->width;

		if( creation->flags & RCF_OUT_UA_COUNTER ) {
			uavDesc.Buffer.Flags  |= D3D11_BUFFER_UAV_FLAG_COUNTER;
		}
		if( creation->flags & RCF_OUT_UA_APPEND ) {
			uavDesc.Buffer.Flags  |= D3D11_BUFFER_UAV_FLAG_APPEND;
		}
		if( creation->flags & RCF_PRG_BYTE_ACCESS ) {
			uavDesc.Buffer.Flags  |= D3D11_BUFFER_UAV_FLAG_RAW;
		}

	} else {
		if( creation->flags & RCF_TEX_1D ) {
			if( creation->slices > 1 ) {
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
				uavDesc.Texture1DArray.ArraySize = creation->slices;
				uavDesc.Texture1DArray.FirstArraySlice = 0;
				uavDesc.Texture1DArray.MipSlice = 0;
			} else {
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
				uavDesc.Texture1D.MipSlice = 0;
			}
		} else if( creation->flags & RCF_TEX_2D ) {
			if( creation->slices > 1 ) {
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
				uavDesc.Texture2DArray.ArraySize = creation->slices;
				uavDesc.Texture2DArray.FirstArraySlice = 0;
				uavDesc.Texture2DArray.MipSlice = 0;
			} else {
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;
			}
		} else if( creation->flags & RCF_TEX_3D ) {
			// 3d arrays don't exist in the api yet
			CORE_ASSERT( creation->slices <=1 );
			/*if( creation->slices > 1 ) {
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3DARRAY;
				uavDesc.Texture3DArray.ArraySize = creation->slices;
				uavDesc.Texture3DArray.FirstWSlice = 0;
				uavDesc.Texture3DArray.MipSlice = 0;
				uavDesc.Texture3DArray.WSize = creation->depth;
			} else */
			{
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
				uavDesc.Texture3D.FirstWSlice = 0;
				uavDesc.Texture3D.MipSlice = 0;
				uavDesc.Texture3D.WSize = creation->depth;
			}
		}
	}
	HRESULT hr;
	ID3D11UnorderedAccessView* view;
	DXFAIL( Gfx::getr()()->CreateUnorderedAccessView( resource.get(), &uavDesc, &view ) );
	views[ index ] = D3DViewPtr( view, false );
}


}