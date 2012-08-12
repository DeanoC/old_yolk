//!-----------------------------------------------------
//!
//! \file gfxresources.cpp
//! any graphics resources are handled here
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "graphics.h"
#include "core/resourceman.h"
#include "vbmanager.h"
#include "ibmanager.h"
#include "wobfile.h"
#include "shadermanager.h"
#include "texture.h"
#include "textureatlas.h"
#include "gpubvh.h"

#include "gfxresources.h"

namespace 
{

//! Callback from the resource manager to create a wob resource
Core::shared_ptr<Core::ResourceBase> WobCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;
	using namespace Graphics;

	if( flags & RMRF_LOADOFFDISK ) {
		Core::shared_ptr<WobResource> pResource( CORE_NEW WobResource );
		pResource->m_spHeader = WobLoad( pName );

		WobFileHeader* pHeader = pResource->m_spHeader.get();

		// allocate graphics materials
		pResource->m_saMaterials.reset( CORE_NEW MaterialWob[pHeader->uiNumMaterials] );
		for( int i =0;i < pHeader->uiNumMaterials; i++ ) {
			WobMaterial& wobMaterial = pHeader->pMaterials[i];
			MaterialWob& material = pResource->m_saMaterials[i];
			material.CreateFromWobMaterial( wobMaterial );
		}

		WobResource::s_loadPreDiscardCallback( pResource.get() );
		
		for( int i =0;i < pHeader->uiNumMaterials; i++ ) {
			WobMaterial& wobMaterial = pHeader->pMaterials[i];
			MaterialWob& material = pResource->m_saMaterials[i];
			wobMaterial.pEngineMaterial = &material;
		}

		CORE_DELETE_ARRAY pResource->m_spHeader->pDiscardable;
		pResource->m_spHeader->pDiscardable = 0;

		return shared_ptr<ResourceBase>(pResource);
	} else {
		assert( false && "Can only load off disk" );
		return shared_ptr<ResourceBase>();
	}
}

void GraphicsMeshPreDiscardLoadCallback( Graphics::WobResource* pResource ) {
	using namespace Graphics;
	WobFileHeader* pHeader = pResource->m_spHeader.get();

#if 0
	// TODO make bvhsah options via load structure
	pResource->m_bvh.reset( CORE_NEW Graphics::GpuBvh() );
	pResource->m_bvh->setRootAABB( Core::AABB(pHeader->minAABB, pHeader->maxAABB) );

	for( int i =0;i < pHeader->uiNumMaterials; i++ ) {
		WobMaterial& wobMaterial = pHeader->pMaterials[i];
		MaterialWob& material = pResource->m_saMaterials[i];
		bool indexIs32bit = false;
		if( wobMaterial.uiFlags & WobMaterial::WM_32BIT_INDICES ) {
			indexIs32bit = true;
		}
		pResource->m_bvh->AddIndexTriSource( wobMaterial.uiNumIndices, wobMaterial.pIndexData, wobMaterial.pVertexData, indexIs32bit, material.GetVertexSize() );
	}
	pResource->m_bvh->BuildLBVH( 32, 32, 20 );
#endif
}

//! Callback from the resource manager to create a Hie resource
Core::shared_ptr<Core::ResourceBase> HieCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;
	using namespace Graphics;

	if( flags & RMRF_LOADOFFDISK ) {
		Core::shared_ptr<WobHierachyResource> pResource( CORE_NEW WobHierachyResource );
		pResource->m_spHeader = WobHieLoad( pName );

		return shared_ptr<ResourceBase>(pResource);
	} else {
		assert( false && "Can only load off disk" );
		return shared_ptr<ResourceBase>();
	}
}


//! Callback from the resource manager to create a shader FX resource
Core::shared_ptr<Core::ResourceBase> ShaderFXCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;
	using namespace Graphics;

	if( flags & RMRF_LOADOFFDISK ) {
		ShaderFXPtr pResource( ShaderManager::Get()->InternalLoadShaderFX( handle, pName, (flags & RMRF_PRELOAD) == RMRF_PRELOAD ) );

		return shared_ptr<ResourceBase>(pResource);
	} else
	{
		assert( false && "Can only load off disk" );
		return shared_ptr<ResourceBase>();
	}
}

//! Callback from the resource manager to create a shader FX resource
Core::shared_ptr<Core::ResourceBase> StaticShaderCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;
	using namespace Graphics;

	if( flags & RMRF_LOADOFFDISK ) {
		StaticShader* pResource( ShaderManager::Get()->InternalLoadStaticShader( handle, pName, (flags & RMRF_PRELOAD) == RMRF_PRELOAD ) );

		return shared_ptr<ResourceBase>(pResource);
	} else
	{
		assert( false && "Can only load off disk" );
		return shared_ptr<ResourceBase>();
	}
}

//! Callback from the resource manager to create a texture resource
Core::shared_ptr<Core::ResourceBase> TextureCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;
	using namespace Graphics;

	if( flags & RMRF_LOADOFFDISK ) {
		bool bPreLoad = false;
		if( flags & RMRF_PRELOAD ) {
			bPreLoad = true;
		}
		TexturePtr pResource( Texture::InternalLoadTexture( handle, pName, bPreLoad ) );
		return shared_ptr<ResourceBase>( pResource );
	} else if( flags & RMRF_INMEMORYCREATE ) {
		const Texture::CreationStruct* pStruct = (const Texture::CreationStruct*) pData;
		TexturePtr pResource( Texture::InternalCreateTexture( pStruct ) );
#if defined( _DEBUG )
		if( pResource ) {
			pResource->m_baseTexture->SetPrivateData( WKPDID_D3DDebugObjectName, strlen(pName), pName );
		}
#endif

		return shared_ptr<ResourceBase>( pResource );
	} else {
		assert( false && "Unknown Resource Type" );
		return shared_ptr<ResourceBase>();
	}
}

//! Callback from the resource manager to create a texture atlas resource
Core::shared_ptr<Core::ResourceBase> TextureAtlasCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData ) {
	using namespace Core;
	using namespace Graphics;

	if( flags & RMRF_LOADOFFDISK ) {
		TextureAtlasPtr pResource( TextureAtlas::InternalLoadTextureAtlas( pName ) );
		return shared_ptr<ResourceBase>(pResource);
	} else {
		assert( false && "Can only load off disk" );
		return shared_ptr<ResourceBase>();
	}
}


} // end anon namespace 

namespace Graphics {
	WobResource::PreDiscardLoadCallback WobResource::s_loadPreDiscardCallback = GraphicsMeshPreDiscardLoadCallback;
	void Gfx::InstallResourceTypes() {
		using namespace Core;
		ResourceMan::Get()->RegisterResourceType( WobType, WobCreateResource, &SimpleResourceDestroyer<WobResource>, sizeof(WobResourceHandle), NULL, 0 , "Meshes/" );
		ResourceMan::Get()->RegisterResourceType( WobHierType, HieCreateResource, &SimpleResourceDestroyer<WobResource>, sizeof(WobResourceHandle), NULL, 0 , "Hie/" );
		ResourceMan::Get()->RegisterResourceType( StaticShaderType, StaticShaderCreateResource, &SimpleResourceDestroyer<StaticShader>, sizeof(StaticShaderHandle), NULL, 0, "Shaders/" );
		ResourceMan::Get()->RegisterResourceType( ShaderFXType, ShaderFXCreateResource, &SimpleResourceDestroyer<ShaderFX>, sizeof(ShaderFXHandle), NULL, 0, "" );
		ResourceMan::Get()->RegisterResourceType( TextureType, TextureCreateResource, &SimpleResourceDestroyer<Texture>, sizeof(TextureHandle), &TextureHandle::DestroyHandle, 0, "Textures/" );
		ResourceMan::Get()->RegisterResourceType( TextureAtlasType, TextureAtlasCreateResource, &SimpleResourceDestroyer<TextureAtlas>, sizeof(TextureAtlasHandle), NULL, 0, "Textures/" );

	}
} // end Graphics namespace
