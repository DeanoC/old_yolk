//!-----------------------------------------------------
//!
//! \file texture.cpp
//! the texture functions on dx11
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "gfx.h"
#include "texture.h"
#include "rendercontext.h"
#include "core/resourceman.h"
#include "core/file_path.h"

namespace Dx11 {

Scene::Texture* Texture::internalCreate( const void* data ) {
	using namespace Scene;
	HRESULT hr;

	// copy and point to TODO big ugly but quick...
	Texture::CreationInfo copyTCS = *((Texture::CreationInfo*)data);
	Texture::CreationInfo* creation = &copyTCS;

	DXGI_FORMAT fmt = DXGIFormat::getDXGIFormat( creation->format );
	CORE_ASSERT( (creation->flags & RCF_D3D_FROM_OS ) || (fmt != DXGI_FORMAT_UNKNOWN) );

	uint32_t usage = 0;
	uint32_t cpuAccess = 0;
	boost::scoped_array< D3D11_SUBRESOURCE_DATA > initialData;

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
		// each mip is a sub resource and each array slice is
		uint32_t numMipLevels = (creation->mipLevels > 0) ? creation->mipLevels : 1;
		uint32_t numSubResources = creation->slices * numMipLevels;
		initialData.reset( CORE_NEW_ARRAY D3D11_SUBRESOURCE_DATA[numSubResources] );
		uint8_t* memPtr = (uint8_t*) creation->prefillData;
		for( unsigned int i =0;i < creation->slices; ++i ) {
			for( unsigned int j = 0; j < numMipLevels; ++j ) {
				initialData[(i*numMipLevels)+j].pSysMem = memPtr;
				if(creation->flags & RCF_TEX_1D ) {
					memPtr += initialData[(i*numMipLevels)+j].SysMemPitch >> j;
				} else if(creation->flags & RCF_TEX_2D ) {
					initialData[(i*numMipLevels)+j].SysMemPitch = creation->prefillPitch >> j;
					memPtr += (initialData[(i*numMipLevels)+j].SysMemPitch * (creation->height>>j));
				} else if(creation->flags & RCF_TEX_3D ) {
					TODO_ASSERT( creation->flags & RCF_TEX_3D );
				}
			}
		}
		usage = D3D11_USAGE_IMMUTABLE;
		cpuAccess = 0;
	} else if( creation->flags & RCF_ACE_CPU_READ ) {	
		usage = D3D11_USAGE_DYNAMIC;
		cpuAccess = D3D11_CPU_ACCESS_READ;
	} else {
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
		CORE_ASSERT( creation->samples <= 1 ); // dx11/hw limit no multisample uav
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
	if( creation->flags & RCF_OUT_RENDER_TARGET ) {
		if( DXGIFormat::isDepthStencilFormat( fmt ) ) {
			bind |= D3D11_BIND_DEPTH_STENCIL;
		} else {
			bind |= D3D11_BIND_RENDER_TARGET;
		}
	}
	if( creation->flags & RCF_TEX_CUBE_MAP ) {
		misc |= D3D11_RESOURCE_MISC_TEXTURECUBE;
	}

	D3DResourcePtr resource;

	DXGI_FORMAT typelessFmt = DXGIFormat::getTypelessTextureFormat( fmt );

	if( creation->flags & RCF_TEX_1D ) {
		// no Multisample 1D textures
		CORE_ASSERT( creation->samples <= 1 );
		D3D11_TEXTURE1D_DESC texDesc;
		texDesc.Width = creation->width;
		texDesc.MipLevels = creation->mipLevels;
		texDesc.ArraySize = creation->slices;
		texDesc.Format = typelessFmt;
		texDesc.BindFlags = bind;
		texDesc.CPUAccessFlags = cpuAccess;
		texDesc.MiscFlags = misc;

		ID3D11Texture1D* texture;
		DXFAIL( Gfx::getr()()->CreateTexture1D( &texDesc, initialData.get(), &texture ) );
		resource = D3DResourcePtr( texture, false );

	} else if( creation->flags & RCF_TEX_2D ) {

		D3D11_TEXTURE2D_DESC texDesc;

		ID3D11Texture2D* texture;
		if( creation->flags & RCF_D3D_FROM_OS ) {
			// todo support non 2D from d3d textures, if rquired
			texture = (ID3D11Texture2D*) creation->referenceTex;
			// ovveride passed in with ones in the existing textures
			texture->GetDesc( &texDesc );
			creation->width = texDesc.Width;
			creation->height = texDesc.Height;
			creation->mipLevels = texDesc.MipLevels;
			creation->slices = texDesc.ArraySize;
			creation->mipLevels = texDesc.MipLevels;
			creation->samples = texDesc.SampleDesc.Count;
			fmt = texDesc.Format;
			creation->format = DXGIFormat::getGenericFormat( fmt ); 
		} else {
			texDesc.Width = creation->width;
			texDesc.Height = creation->height;
			texDesc.MipLevels = creation->mipLevels;
			texDesc.ArraySize = creation->slices;
			texDesc.Format = typelessFmt;
			texDesc.SampleDesc.Count = creation->samples;
			texDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
			texDesc.Usage = (D3D11_USAGE) usage;
			texDesc.BindFlags = bind;
			texDesc.CPUAccessFlags = cpuAccess;
			texDesc.MiscFlags = misc;
			DXFAIL( Gfx::getr()()->CreateTexture2D( &texDesc, initialData.get(), &texture ) );
		}
		resource = D3DResourcePtr( texture, false );

	} else if( creation->flags & RCF_TEX_3D ) {
		// No multisample or arrays for 3D textures
		CORE_ASSERT( creation->slices <= 1 );
		CORE_ASSERT( creation->samples <= 1 );
		D3D11_TEXTURE3D_DESC texDesc;
		texDesc.Width = creation->width;
		texDesc.Height = creation->height;
		texDesc.Depth = creation->depth;
		texDesc.MipLevels = creation->mipLevels;
		texDesc.Format = typelessFmt;
		texDesc.BindFlags = bind;
		texDesc.CPUAccessFlags = cpuAccess;
		texDesc.MiscFlags = misc;

		ID3D11Texture3D* texture;
		DXFAIL( Gfx::getr()()->CreateTexture3D( &texDesc, initialData.get(), &texture ) );
		resource = D3DResourcePtr( texture, false );
	}

	Texture* tex = CORE_NEW Texture( resource );
	tex->format = creation->format;
	tex->width = creation->width;
	tex->height = creation->height;
	tex->depth = creation->depth;
	tex->slices = creation->slices;
	tex->mipLevels = creation->mipLevels;
	tex->samples = creation->samples;
	tex->d3dFormat = fmt;

	// create default views for this texture
	if( bind & D3D11_BIND_SHADER_RESOURCE ) {
		CreationInfo vs = Scene::Resource::ViewCtor( 	creation->flags & (RCF_TEX_1D | RCF_TEX_2D | RCF_TEX_3D | RCF_TEX_CUBE_MAP),
										tex->width, tex->height, tex->depth, tex->slices, tex->mipLevels, tex->samples,
										tex->format );
		tex->createView( SHADER_RESOURCE_VIEW, &vs );
	}
	if( bind & D3D11_BIND_RENDER_TARGET ) {		
		CreationInfo vs = Scene::Resource::ViewCtor( 	creation->flags & (RCF_TEX_1D | RCF_TEX_2D | RCF_TEX_3D | RCF_TEX_CUBE_MAP),
										tex->width, tex->height, tex->depth, tex->slices, tex->mipLevels, tex->samples,
										tex->format );
		tex->createView( RENDER_TARGET_VIEW, &vs );
	}
	if( bind & D3D11_BIND_DEPTH_STENCIL ) {		
		CreationInfo vs = Scene::Resource::ViewCtor( 	creation->flags & (RCF_TEX_1D | RCF_TEX_2D | RCF_TEX_3D | RCF_TEX_CUBE_MAP),
										tex->width, tex->height, tex->depth, tex->slices, tex->mipLevels, tex->samples,
										tex->format );
		tex->createView( DEPTH_STENCIL_VIEW, &vs );
	}
	if( bind & RCF_OUT_UNORDERED_ACCESS ) {
		CreationInfo vs = Scene::Resource::ViewCtor( 	creation->flags & (RCF_TEX_1D | RCF_TEX_2D | RCF_TEX_3D | RCF_TEX_CUBE_MAP),
										tex->width, tex->height, tex->depth, tex->slices, tex->mipLevels, tex->samples,
										tex->format );
		tex->createView( UNORDERED_ACCESS_VIEW, &vs );
	}

	return tex;
}
/*
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
*/

}; // end namespace Dx11