//!-----------------------------------------------------
//!
//! \file texture_pc.cpp
//! the texture functions on PC
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

Texture::Texture() :
	m_baseTexture(0), 
	m_primaryView(0),
	m_extraView(0)
{
}

Texture::~Texture() {
	//
	SAFE_RELEASE( m_extraView );
	SAFE_RELEASE( m_primaryView );
	SAFE_RELEASE( m_baseTexture );
}

Texture* Texture::InternalLoadTexture( const Core::ResourceHandleBase* baseHandle, const char* pFileName, bool bPreLoad ) {
	Core::FilePath path( pFileName );
	path = path.ReplaceExtension( ".dds" );
	const char* pTextureFileName = path.value().c_str();

	TextureHandlePtr handle = (TextureHandlePtr) baseHandle; 

	HRESULT hr;

	if( FAILED( handle->asyncResult ) && handle->asyncResultPtr == NULL ) {
		// shader has started compiling but hasn't finished so wait
		while( handle->asyncResult == E_PENDING && handle->asyncResultPtr == NULL ) {
			DXFAIL( Graphics::Gfx::Get()->GetThreadPump()->ProcessDeviceWorkItems( 1 ) );
		}
		if( handle->asyncResult != E_PENDING && handle->asyncResultPtr == NULL ) {
			if( FAILED( handle->asyncResult ) ) {
				DXWARN( handle->asyncResult );			
			}
			LOG(INFO) << "Texture : " << pTextureFileName << " not found, using Defaulted.dds\n";
			Texture::CreationStruct fromExist( CF_FROM_D3D_TEXTURE ) ;
			ID3D11Resource* res = Graphics::Gfx::Get()->getDefaultedTexture()->Acquire().get()->m_baseTexture;
			ID3D11Texture2D* pD3DTex;
			hr = res->QueryInterface( __uuidof( *pD3DTex ), ( LPVOID* )&pD3DTex );
			fromExist.platformParam0 = (uintptr_t)pD3DTex;
			Texture* tex = InternalCreateTexture( &fromExist );
			return tex;
		}
	} else if( bPreLoad ) {
		// just doing a preload so kick one off
		handle->asyncResultPtr = 0;
		handle->asyncResult = E_PENDING;

		hr = D3DX11CreateTextureFromFile(
					Gfx::Get()->GetDevice(),
					pTextureFileName,
					NULL,
					Gfx::Get()->GetThreadPump(),
					(ID3D11Resource**)&handle->asyncResultPtr,
					&handle->asyncResult
					);
		DXWARN( hr );
		return NULL;
	} else {
		// stalling async load
		handle->asyncResultPtr = 0;
		handle->asyncResult = E_PENDING;

		hr = D3DX11CreateTextureFromFile(
					Gfx::Get()->GetDevice(),
					pTextureFileName,
					NULL,
					Gfx::Get()->GetThreadPump(),
					(ID3D11Resource**)&handle->asyncResultPtr,
					&handle->asyncResult
					);
		DXWARN( hr );
		return InternalLoadTexture( baseHandle, pFileName, false );
	}

	handle->asyncResult = S_OK;
	Texture* pResource = CORE_NEW Texture();

	D3D11_TEXTURE2D_DESC texDesc;
	ID3D11Texture2D* pTexture = (ID3D11Texture2D*) &(*handle->asyncResultPtr);
	handle->asyncResultPtr.Detach();
	pTexture->GetDesc( &texDesc );
	pResource->m_iWidth = texDesc.Width;
	pResource->m_iHeight = texDesc.Height;
	pResource->m_iFormat = texDesc.Format;
	pResource->m_iMipLevels = texDesc.MipLevels;
	pResource->m_baseTexture = pTexture;
	DXFAIL( Gfx::Get()->GetDevice()->CreateShaderResourceView(pTexture, NULL, (ID3D11ShaderResourceView**) &pResource->m_primaryView ) );

#if defined( _DEBUG )
	pResource->m_baseTexture->SetPrivateData( WKPDID_D3DDebugObjectName, strlen(pFileName), pFileName );
#endif

	return pResource;
}
Texture* Texture::InternalCreateTexture( const Texture::CreationStruct* pStructOrig ) {
	Texture* pResource = CORE_NEW Texture();
	D3D11_TEXTURE2D_DESC texDesc = { 0 };

	// copy and point to TODO big ugly but quick...
	Texture::CreationStruct copyTCS = *pStructOrig;
	Texture::CreationStruct* pStruct = &copyTCS;

	HRESULT hr;

	texDesc.Width = pStruct->iWidth;
	texDesc.Height = pStruct->iHeight;
	texDesc.MipLevels = pStruct->iMipLevels;
	texDesc.ArraySize = 1;
	texDesc.Format = pStruct->texFormat;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;

	if( pStruct->iFlags & (CF_RAW_BUFFER | CF_STRUCTURED_BUFFER) ) {
		pStruct->iFlags |= CF_BUFFER;
	}
	if( pStruct->iFlags & (CF_UAV_COUNTER | CF_UAV_APPEND | CF_DRAW_INDIRECT) ) {
		pStruct->iFlags |= CF_BUFFER | CF_UAV;
	}

	if( pStruct->iFlags & CF_UAV ) {
		texDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		assert( (pStruct->iFlags & CF_MULTISAMPLE) == false ); // Dx11 limitation :(
	}
	if( pStruct->iFlags & CF_KEYEDMUTEX ) {
		texDesc.MiscFlags|= D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
	}
	if( pStruct->iFlags & CF_CPU_READ  ) {
		texDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		texDesc.Usage = D3D11_USAGE_DYNAMIC;
	}
	if( pStruct->iFlags & CF_CPU_WRITE ) {
		texDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		texDesc.Usage = D3D11_USAGE_DYNAMIC;
	}
	if( pStruct->iFlags & CF_ARRAY ) {
		assert( (pStruct->iFlags & CF_3D) == false );
		// arrays of cubemaps are just depth * 6 for creation purpose
		if( pStruct->iFlags & CF_CUBE_MAP ) {
			assert( (pStruct->iFlags & CF_1D) == false );
			texDesc.ArraySize = pStruct->iDepth * 6;
			texDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		} else {
			texDesc.ArraySize = pStruct->iDepth;
		}
	} else 	if( pStruct->iFlags & CF_CUBE_MAP ) {
		assert( (pStruct->iFlags & CF_3D) == false );
		assert( (pStruct->iFlags & CF_1D) == false );
		// non arrayed cube map
		texDesc.ArraySize = 6;
		texDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
	}
	if( pStruct->iFlags & CF_STREAM_OUT ) {
		texDesc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
	}
	if( pStruct->iFlags & CF_VERTEX_BUFFER ) {
		texDesc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
	}

	if( pStruct->iFlags & CF_FROM_D3D_TEXTURE ) {
		// todo support non 2D from d3d textures, if rquired
		ID3D11Texture2D* pTexture = (ID3D11Texture2D*) pStruct->fromD3DTex;
		// ovveride passed in with ones in the existing textures
		pTexture->GetDesc( &texDesc );
		pResource->m_baseTexture = pTexture;
	} else if( pStruct->iFlags & CF_RENDER_TARGET) {
		if( pStruct->iFlags & CF_MULTISAMPLE && pStruct->platformParam0 > 1) {
			texDesc.SampleDesc.Count = pStruct->sampleCount;
			texDesc.SampleDesc.Quality = 0; // TODO quality
		}

		if( DXGIFormat::IsDepthStencilFormat( texDesc.Format ) ) {
			texDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;

			// Dx10 backwards compat, can't bind depth surfaces to shaders if multisampling.
			if( Graphics::Gfx::Get()->getShaderModel() == Gfx::SM4_0 && texDesc.SampleDesc.Count > 1 ) {
				texDesc.BindFlags &= ~D3D11_BIND_SHADER_RESOURCE;
			}
		} else {
			texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}
	}

	Core::scoped_array<D3D11_SUBRESOURCE_DATA> initialData;

	if( pStruct->iFlags & CF_PRE_FILL ) {
		assert( texDesc.Usage == 0 );
		assert( (pStruct->iFlags & CF_MULTISAMPLE) == false );

		// each mip is a sub resource and each array slice is
		uint32_t numMipLevels = (pStruct->iMipLevels > 0) ? pStruct->iMipLevels : 1;
		uint32_t numSubResources = texDesc.ArraySize * numMipLevels;
		texDesc.Usage = D3D11_USAGE_IMMUTABLE;
		initialData.reset( CORE_NEW_ARRAY D3D11_SUBRESOURCE_DATA[numSubResources]);
		uint8_t* memPtr = (uint8_t*) pStruct->prefillData;
		for( unsigned int i =0;i < texDesc.ArraySize; ++i ) {
			for( unsigned int j = 0; j < numMipLevels; ++j ) {
				initialData[(i*numMipLevels)+j].pSysMem = memPtr;
				if( pStruct->iFlags & CF_1D) {
					memPtr += initialData[(i*numMipLevels)+j].SysMemPitch >> j;
				} else if( pStruct->iFlags & CF_3D ) {
					TODO_ASSERT(pStruct->iFlags & CF_3D);
				} else {
					initialData[(i*numMipLevels)+j].SysMemPitch = pStruct->prefillPitch  >> j;
					memPtr += (initialData[(i*numMipLevels)+j].SysMemPitch * (texDesc.Height>>j));
				}
			}
		}
	}

	if( (pStruct->iFlags & CF_FROM_D3D_TEXTURE) == false ) {
		DXGI_FORMAT typelessFormat = texDesc.Format;
		if( (pStruct->iFlags & CF_KEYEDMUTEX) == false ) {
			typelessFormat = DXGIFormat::GetTypelessTextureFormat( texDesc.Format );
		}

		if( pStruct->iFlags & CF_BUFFER ) {
			D3D11_BUFFER_DESC bufDesc;
			bufDesc.BindFlags = texDesc.BindFlags;
			bufDesc.CPUAccessFlags = texDesc.CPUAccessFlags;
			bufDesc.MiscFlags = texDesc.MiscFlags;
			bufDesc.Usage = texDesc.Usage;


			if( pStruct->iFlags & CF_RAW_BUFFER ) {
				bufDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
				bufDesc.ByteWidth = texDesc.Width;
			} else if( pStruct->iFlags & CF_STRUCTURED_BUFFER ) {
				bufDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
				bufDesc.ByteWidth = texDesc.Width * pStruct->structureSize;
				bufDesc.StructureByteStride = pStruct->structureSize;
			} else {
				bufDesc.ByteWidth = texDesc.Width * (DXGIFormat::GetBitWidth(texDesc.Format) / 8);
			}
			if( pStruct->iFlags & CF_DRAW_INDIRECT ) {
				bufDesc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
			}

			ID3D11Buffer* pBuffer;
			DXFAIL( Gfx::Get()->GetDevice()->CreateBuffer(&bufDesc, initialData.get(), &pBuffer ) );
			pResource->m_baseTexture = pBuffer;

		} else if( pStruct->iFlags & CF_1D ) {
			assert( (pStruct->iFlags & CF_MULTISAMPLE) == false );
			D3D11_TEXTURE1D_DESC texTexDesc;
			texTexDesc.ArraySize = texDesc.ArraySize;
			texTexDesc.BindFlags = texDesc.BindFlags;
			texTexDesc.CPUAccessFlags = texDesc.CPUAccessFlags;
			texTexDesc.Format = typelessFormat;
			texTexDesc.MipLevels = texDesc.MipLevels;
			texTexDesc.MiscFlags = texDesc.MiscFlags;
			texTexDesc.Usage = texDesc.Usage;
			texTexDesc.Width = texDesc.Width;
			ID3D11Texture1D* pTexture;
			DXFAIL( Gfx::Get()->GetDevice()->CreateTexture1D(&texTexDesc, initialData.get(), &pTexture ) );
			pResource->m_baseTexture = pTexture;
		} else if( pStruct->iFlags & CF_3D ) {
			assert( (pStruct->iFlags & CF_MULTISAMPLE) == false );
			D3D11_TEXTURE3D_DESC texTexDesc;
			texTexDesc.BindFlags = texDesc.BindFlags;
			texTexDesc.CPUAccessFlags = texDesc.CPUAccessFlags;
			texTexDesc.Format = typelessFormat;
			texTexDesc.MipLevels = texDesc.MipLevels;
			texTexDesc.MiscFlags = texDesc.MiscFlags;
			texTexDesc.Usage = texDesc.Usage;
			texTexDesc.Width = texDesc.Width;
			texTexDesc.Height = texDesc.Height;
			texTexDesc.Depth = pStruct->iDepth;
			ID3D11Texture3D* pTexture;
			DXFAIL( Gfx::Get()->GetDevice()->CreateTexture3D(&texTexDesc, initialData.get(), &pTexture ) );
			texDesc.ArraySize = texTexDesc.Depth; // pretend a 3D is an array just for a short bit
			pResource->m_baseTexture = pTexture;
		} else {
			D3D11_TEXTURE2D_DESC texTexDesc = texDesc;
			texTexDesc.Format = typelessFormat;
			ID3D11Texture2D* pTexture;
			DXFAIL( Gfx::Get()->GetDevice()->CreateTexture2D(&texTexDesc, initialData.get(), &pTexture ) );
			pResource->m_baseTexture = pTexture;
		}
	}

	pResource->m_iWidth = texDesc.Width;
	pResource->m_iHeight = texDesc.Height;
	pResource->m_iDepth = texDesc.ArraySize;
	pResource->m_iFormat = texDesc.Format;
	pResource->m_iMipLevels = texDesc.MipLevels;

	// primary view
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	bool noPrimaryView = false;
	if( DXGIFormat::IsDepthStencilFormat( texDesc.Format ) ) {
		srDesc.Format = DXGIFormat::GetShaderDepthTextureFormat( texDesc.Format );
		// Dx10 backwards compat!
		if( Graphics::Gfx::Get()->getShaderModel() == Gfx::SM4_0 && texDesc.SampleDesc.Count > 1 ) {
			noPrimaryView = true;
		}
	} else {
		srDesc.Format = texDesc.Format;
	}

	if( noPrimaryView == false  ) {
		if( pStruct->iFlags & CF_BUFFER ) {
			// a buffer view is a shader resource view but typed as a buffer rather than a texture
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			if( pStruct->iFlags & CF_STRUCTURED_BUFFER ) {
				srDesc.Format = DXGI_FORMAT_UNKNOWN;
			}
			srDesc.BufferEx.NumElements = pResource->m_iWidth;
			srDesc.BufferEx.FirstElement = 0;
			if( pStruct->iFlags & CF_RAW_BUFFER ) {
				srDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			} else {
				srDesc.BufferEx.Flags = 0;
			}
		} else if( pStruct->iFlags & CF_1D ) {
			// 1d and arrays of them
			if( pStruct->iFlags & CF_ARRAY ) {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
				srDesc.Texture1DArray.MipLevels = texDesc.MipLevels;
				srDesc.Texture1DArray.MostDetailedMip = 0;
				srDesc.Texture1DArray.ArraySize = texDesc.ArraySize;
				srDesc.Texture1DArray.FirstArraySlice = 0;
			} else {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
				srDesc.Texture1D.MipLevels = texDesc.MipLevels;
				srDesc.Texture1D.MostDetailedMip = 0;
			}
		} else if( pStruct->iFlags & CF_CUBE_MAP ) {
			// cube and arrays of them
			if( pStruct->iFlags & CF_ARRAY ) {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
				srDesc.TextureCubeArray.NumCubes = texDesc.ArraySize / 6;
				srDesc.TextureCubeArray.First2DArrayFace = 0;
				srDesc.TextureCubeArray.MipLevels = texDesc.MipLevels;
				srDesc.TextureCubeArray.MostDetailedMip = 0;
			} else {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				srDesc.TextureCube.MipLevels = texDesc.MipLevels;
				srDesc.TextureCube.MostDetailedMip = 0;
			}
		} else if( pStruct->iFlags & CF_3D ) {
			// just 3d, currently no arrays of 3D
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			srDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srDesc.Texture2D.MostDetailedMip = 0;
		} else if( texDesc.SampleDesc.Count > 1 ) {
			// 2D multisample and arrays of them
			if( pStruct->iFlags & CF_ARRAY ) {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
				srDesc.Texture2DMSArray.ArraySize = texDesc.ArraySize;
				srDesc.Texture2DMSArray.FirstArraySlice = 0;
			} else {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
			}
		} else {
			// good ol' fashioned 2D, well maybe an array of them...
			if( pStruct->iFlags & CF_ARRAY ) {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				srDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
				srDesc.Texture2DArray.FirstArraySlice = 0;
				srDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
				srDesc.Texture2DArray.MostDetailedMip = 0;
			} else {
				srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srDesc.Texture2D.MipLevels = texDesc.MipLevels;
				srDesc.Texture2D.MostDetailedMip = 0;
			}
		}

		DXFAIL( Gfx::Get()->GetDevice()->CreateShaderResourceView(pResource->m_baseTexture, &srDesc, (ID3D11ShaderResourceView**)&pResource->m_primaryView ) );
	}
	// extra view
	if( pStruct->iFlags & CF_RENDER_TARGET ) {
		if( DXGIFormat::IsDepthStencilFormat( texDesc.Format ) ) {
			D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc;
			dsDesc.Flags = 0;
			dsDesc.Format = texDesc.Format;
			if( texDesc.SampleDesc.Count > 1 ) {
				dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			} else {
				dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				dsDesc.Texture2D.MipSlice = 0;
			}
			DXFAIL( Gfx::Get()->GetDevice()->CreateDepthStencilView( pResource->m_baseTexture, &dsDesc, (ID3D11DepthStencilView**)&pResource->m_extraView ) );

		} else {
			D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
			rtDesc.Format = texDesc.Format;
			if( texDesc.SampleDesc.Count > 1 ) {
				rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			} else {
				rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				rtDesc.Texture2D.MipSlice = 0;
			}
			DXFAIL( Gfx::Get()->GetDevice()->CreateRenderTargetView( pResource->m_baseTexture, &rtDesc, (ID3D11RenderTargetView**) &pResource->m_extraView ) );
		}
	} else if( pStruct->iFlags & CF_UAV ) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		if( pStruct->iFlags & CF_BUFFER ) {
			if( pStruct->iFlags & CF_STRUCTURED_BUFFER ) {
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			} else {
				uavDesc.Format = texDesc.Format;
			}
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.Flags = 0;
			uavDesc.Buffer.NumElements = pResource->m_iWidth;

			if( pStruct->iFlags & CF_UAV_COUNTER ) {
				uavDesc.Buffer.Flags  |= D3D11_BUFFER_UAV_FLAG_COUNTER;
			}
			if( pStruct->iFlags & CF_UAV_APPEND ) {
				uavDesc.Buffer.Flags  |= D3D11_BUFFER_UAV_FLAG_APPEND ;
			}
			if( pStruct->iFlags & CF_RAW_BUFFER ) {
				uavDesc.Buffer.Flags  |= D3D11_BUFFER_UAV_FLAG_RAW;
			}

		} else {
			uavDesc.Format = texDesc.Format;
			if( pStruct->iFlags & CF_1D ) {
				if( pStruct->iFlags & CF_ARRAY ) {
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
					uavDesc.Texture1DArray.ArraySize = texDesc.ArraySize;
					uavDesc.Texture1DArray.FirstArraySlice = 0;
					uavDesc.Texture1DArray.MipSlice = 0;
				} else {
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
					uavDesc.Texture1D.MipSlice = 0;
				}
			} else if( pStruct->iFlags & CF_3D ) {
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
					uavDesc.Texture3D.FirstWSlice = 0;
					uavDesc.Texture3D.MipSlice = 0;
					uavDesc.Texture3D.WSize = texDesc.ArraySize;
			} else {
				if( pStruct->iFlags & CF_ARRAY ) {
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
					uavDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
					uavDesc.Texture2DArray.FirstArraySlice = 0;
					uavDesc.Texture2DArray.MipSlice = 0;
				} else {
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
					uavDesc.Texture2D.MipSlice = 0;
				}
			}
		}
		DXFAIL( Gfx::Get()->GetDevice()->CreateUnorderedAccessView( pResource->m_baseTexture, &uavDesc, (ID3D11UnorderedAccessView**) &pResource->m_extraView ) );
	}

	return pResource;
}

void* Texture::Lock( RenderContext* context, Texture::CPU_ACCESS access, uint32_t& iOutPitch ) {
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mapper;
	DXWARN( context->deviceContext->Map( m_baseTexture, 0, (D3D11_MAP) access, 0, &mapper ) );

	iOutPitch = mapper.RowPitch;
	return mapper.pData;
}

void Texture::Unlock( RenderContext* context ) {
	context->deviceContext->Unmap( m_baseTexture, 0 );
}

}; // end namespace Graphics