//!-----------------------------------------------------
//!
//! \file resourceloader.cpp
//! GL async loader/creator handler + thread
//! owns the Gl LOAD_CONTEXT for talking to the GPU
//!
//! TODO possible multiple ResourceLoader at the same time
//! TODO find out why some have to be done sync, if doen async
//! TODO GL errors abound :(
//!-----------------------------------------------------

#include "ogl.h"
#include "core/resourceman.h"
#include "scene/wobfile.h"
#include "scene/hierfile.h"
#include "scene/textureatlas.h"
#include "scene/imagecomposer.h"
#include "texture.h"
#include "program.h"
#include "databuffer.h"
#include "vao.h"
#include "xbo.h"
#include "wobbackend.h"
#include "gfx.h"
#include "shaderman.h"
#include "rendercontext.h"

#include "resourceloader.h"

namespace Gl {

class ResourceLoaderImpl {
public:
	friend class Gfx;
	friend class ResourceLoader;
	ResourceLoaderImpl();
	~ResourceLoaderImpl();

	void showLoadingIfNeeded( Scene::ImageComposer* composer );

	void installResourceTypes();
	static void PushOntoLoaderContext( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  );
	static std::shared_ptr<boost::asio::io_service>	io;
	static std::shared_ptr<boost::asio::io_service::strand>	ioStrand;

	std::shared_ptr<Core::thread>						loaderThread;
	Scene::TextureAtlasHandlePtr 						loadTextureAtlas;
	static std::atomic<int>								workCounter;
};
std::shared_ptr<boost::asio::io_service> ResourceLoaderImpl::io;
std::shared_ptr<boost::asio::io_service::strand>	ResourceLoaderImpl::ioStrand;
std::atomic<int> ResourceLoaderImpl::workCounter(0);

//! Callback from the resource manager to create a texture resource
std::shared_ptr<Core::ResourceBase> TextureCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;

	if( flags & RMRF_LOADOFFDISK ) {
		Scene::TexturePtr pResource( Texture::internalPreCreate( handle, pName, bPreLoad ) );
		return std::static_pointer_cast<ResourceBase>( pResource );
	} else if( flags & RMRF_INMEMORYCREATE ) {
		const Texture::CreationStruct* pStruct = (const Texture::CreationStruct*) pData;
		Scene::TexturePtr pResource( Texture::internalCreate( pStruct ) );
		return std::static_pointer_cast<ResourceBase>( pResource );
	} else {
		assert( false && "Unknown Resource Type" );
		return std::shared_ptr<ResourceBase>();
	}

}

std::shared_ptr<Core::ResourceBase> TextureAtlasCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;

	if( flags & RMRF_LOADOFFDISK ) {
		bool bPreLoad = false;
		if( flags & RMRF_PRELOAD ) {
			bPreLoad = true;
		}
		Scene::TextureAtlasPtr pResource( Scene::TextureAtlas::internalPreCreate( handle, pName, bPreLoad ) );
		return std::static_pointer_cast<ResourceBase>( pResource );
	} else if( flags & RMRF_INMEMORYCREATE ) {
//		const Texture::CreationStruct* pStruct = (const Texture::CreationStruct*) pData;
//		TexturePtr pResource( Texture::internalCreateTexture( pStruct ) );
		return std::shared_ptr<ResourceBase>();
	} else {
		assert( false && "Unknown Resource Type" );
		return std::shared_ptr<ResourceBase>();
	}
}

//! Callback from the resource manager to create a program resource
std::shared_ptr<Core::ResourceBase> ProgramCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;
	using namespace Scene;

	const Program::CreationStruct* creation = (const Program::CreationStruct*) pData;

	// for programs at the moment create or load is the same thing, as source is internal,
	// may choose to store a disk cache blob for faster compiling in future
	Scene::ProgramPtr pResource( Gfx::get()->getShaderMan()->internalCreate( handle, pName, creation ) );
	return std::static_pointer_cast<ResourceBase>( pResource );

}

//! Callback from the resource manager to create a data buffer
std::shared_ptr<Core::ResourceBase> DataBufferCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;
	using namespace Scene;

	const DataBuffer::CreationStruct* creation = (const DataBuffer::CreationStruct*) pData;

	// currently only in memory is supported
	Scene::DataBufferPtr pResource( DataBuffer::internalCreate( handle, pName, creation ) );
	return std::static_pointer_cast<ResourceBase>( pResource );
}

//! Callback from the resource manager to create a vao buffer
std::shared_ptr<Core::ResourceBase> VaoCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;
	using namespace Scene;

	const VertexInput::CreationStruct* creation = (const VertexInput::CreationStruct*) pData;

	// currently only in memory is supported
	VertexInputPtr pResource( Vao::internalCreate( handle, pName, creation ) );
	return std::static_pointer_cast<ResourceBase>( pResource );
}

//! Callback from the resource manager to create a data buffer
std::shared_ptr<Core::ResourceBase> XboCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;

	// currently only in memory is supported
	XboPtr pResource( Xbo::internalCreate( handle, pName ) );
	return std::static_pointer_cast<ResourceBase>( pResource );
}


//! Callback from the resource manager to create a wob resource
std::shared_ptr<Core::ResourceBase> WobCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;
	using namespace Scene;

	Gfx* gfx = Gfx::get();

	if( flags & RMRF_LOADOFFDISK ) {
		std::shared_ptr<WobResource> pResource( CORE_NEW WobResource );
		pResource->header = WobLoad( pName );
		WobFileHeader* header = pResource->header.get();

		pResource->backEnd.reset( CORE_NEW WobBackEnd( (int) gfx->getNumPipelines()) );

		for( size_t i = 0; i < gfx->getNumPipelines(); ++i ) {
			Scene::Pipeline* pipe = gfx->getPipeline( i );
			pipe->conditionWob( pName, pResource.get() );
		}

//		ResourceLoaderImpl::ioStrand->post( [header]() {	
//			CORE_DELETE_ARRAY header->pDiscardable.p;
//			header->pDiscardable.p = 0;
//		});

		return std::shared_ptr<ResourceBase>(pResource);
	} else {
		CORE_ASSERT( false && "Can only load off disk" );
		return std::shared_ptr<ResourceBase>();
	}
}

//! Callback from the resource manager to create a Hie resource
std::shared_ptr<Core::ResourceBase> HierCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;
	using namespace Scene;

	if( flags & RMRF_LOADOFFDISK ) {
		std::shared_ptr<HierResource> pResource( CORE_NEW HierResource );
		pResource->header = HierLoad( pName );

		return std::shared_ptr<ResourceBase>(pResource);
	} else {
		CORE_ASSERT( false && "Can only load off disk" );
		return std::shared_ptr<ResourceBase>();
	}
}

void ProcessLoader( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Scene;

	std::shared_ptr<Core::ResourceBase> res;
	// route type to their specific creation functions 
	switch( handle->getType() ) {
	case WobType: res = WobCreateResource( handle, flags, pName, pData ); break;
	case HierType: res = HierCreateResource( handle, flags, pName, pData ); break;
	case TextureType: res = TextureCreateResource( handle, flags, pName, pData ); break;
	case TextureAtlasType: res = TextureAtlasCreateResource( handle, flags, pName, pData ); break;
	case ProgramRType: res = ProgramCreateResource( handle, flags, pName, pData ); break;
	case DataBufferRType: res = DataBufferCreateResource( handle, flags, pName, pData ); break;
	case XboRType: res = XboCreateResource( handle, flags, pName, pData ); break;
//	case VaoRType: res = VaoCreateResource( handle, flags, pName, pData ); break; 
	default:
		LOG(FATAL) << "ProcessLoader being passed a resource it cannot handle\n";
	}

	--ResourceLoaderImpl::workCounter;
	Core::ResourceMan::get()->internalAcquireComplete( handle, res );
}

void ResourceLoaderImpl::PushOntoLoaderContext( const Core::ResourceHandleBase* handle, 
                                                                              	Core::RESOURCE_FLAGS flags, 
                                                                              	const char* pName, 
                                                                              	const void* pData  ) {
	using namespace Scene;

	// some things have to be done sync due to GL context oddness, this distinguishs the boys from the men
	std::shared_ptr<Core::ResourceBase> res;
	switch( handle->getType() ) {
	case WobType: res = WobCreateResource( handle, flags, pName, pData ); break;
	case HierType: res = HierCreateResource( handle, flags, pName, pData ); break;
	case TextureType: res = TextureCreateResource( handle, flags, pName, pData );break;
	case TextureAtlasType: res = TextureAtlasCreateResource( handle, flags, pName, pData );break;
	case ProgramRType: res = ProgramCreateResource( handle, flags, pName, pData ); break;
	case DataBufferRType: res = DataBufferCreateResource( handle, flags, pName, pData ); break;
	case XboRType: res = XboCreateResource( handle, flags, pName, pData ); break;
	case VinRType: res = VaoCreateResource( handle, flags, pName, pData ); break; 
	default: {
			ResourceLoaderImpl::workCounter++;
			ioStrand->post( boost::bind<void>( ProcessLoader, handle, flags, pName, pData ) ); 
			return;
		}
 	}

	Core::ResourceMan::get()->internalAcquireComplete( handle, res );
}

void ResourceLoaderImpl::installResourceTypes() {
	using namespace Core;
	using namespace Scene;
	using namespace Gl;

	#define SRD(x) SimpleResourceDestroyer< x >
	#define SO(x) sizeof( x )
	#define REG ResourceMan::get()->registerResourceType

	auto cb = &ResourceLoaderImpl::PushOntoLoaderContext;

	REG ( WobType, cb, &SRD(WobResource), SO(WobResourceHandle), 
							NULL, 0 , "Meshes/" );
	REG ( HierType, cb, &SRD(HierResource), SO(HierResourceHandle), 
							NULL, 0 , "Hier/" );
	REG ( TextureType, cb, &SRD(Texture), SO(TextureHandle), 
							NULL, 0, "Textures/" );
	REG ( TextureAtlasType, cb, &SRD(TextureAtlas), SO(TextureAtlasHandle), 
							NULL, 0, "Textures/" );
	REG ( ProgramRType, cb, &SRD(Program), SO(ProgramHandle), 
							NULL, 0, "" );
	REG( DataBufferRType, cb, &SRD(DataBuffer), SO(DataBufferHandle), 
							NULL, 0, "" );
	REG( VinRType, cb, &SRD(VertexInput), SO(VertexInputHandle), 
							NULL, 0, "" );
	REG( XboRType, cb, &SRD(Xbo), SO(XboHandle), 
							NULL, 0, "" );

	#undef SRD
	#undef SO
	#undef REG

	loadTextureAtlas = TextureAtlasHandle::load( "base-ui.tat" );

}

ResourceLoaderImpl::ResourceLoaderImpl() {
	io = std::make_shared<boost::asio::io_service>();
	ioStrand = std::make_shared<boost::asio::io_service::strand>( *io );

	std::atomic<bool> sig( false );

	loaderThread = std::make_shared<Core::thread>( 
		[&] {
			auto ctx = Gl::Gfx::get()->getThreadContext( Gl::Gfx::LOAD_CONTEXT );
			ctx->threadActivate();
			sig = true;
			boost::asio::io_service::work w( *io );
			io->run();
		}
	);
	
	while( sig == false ) {
		// wait for load thread context to activate;
	};
}

void ResourceLoaderImpl::showLoadingIfNeeded( Scene::ImageComposer* composer ) {
	using namespace Scene;
	if( ResourceLoaderImpl::workCounter > 0) {
		composer->putSprite( loadTextureAtlas, 0, 
						ImageComposer::ALPHA_BLEND, 
						Math::Vector2( 0.85f, 0.95f ),
						Math::Vector2( 0.15f, 0.05f ),
						Core::RGBAColour::unpackARGB(0xFFFFFFFF),
						0 );
	}
	composer->putSprite( loadTextureAtlas, 1, 
					ImageComposer::ALPHA_BLEND, 
//					Math::Vector2( 0.85f, 0.95f ),
					Math::Vector2( 0.0f, 0.0f ),
					Math::Vector2( 0.15f, 0.05f ),
					Core::RGBAColour::unpackARGB(0x80FFFFFF),
					1 );

	auto texAtlas = loadTextureAtlas->acquire();
	composer->texturedRect( texAtlas->getPackedTexture(0), 
					ImageComposer::ALPHA_BLEND, 
					Math::Vector2( 0.0f, 0.0f ),
					Math::Vector2( 0.5f, 0.5f ),
					Core::RGBAColour::unpackARGB(0xFFFFFFFF),
					Math::Vector2( 0.0f, 0.0f ),
					Math::Vector2( 1.0f, 1.0f ),
					1 );

}

ResourceLoaderImpl::~ResourceLoaderImpl() {
	io.reset();
}

ResourceLoader::ResourceLoader() :
	impl( *(CORE_NEW ResourceLoaderImpl()) ) {
}

ResourceLoader::~ResourceLoader() {
	CORE_DELETE( &impl );
}

void ResourceLoader::installResourceTypes() {
	impl.installResourceTypes();
}

void ResourceLoader::showLoadingIfNeeded( Scene::ImageComposer* composer ) {
	impl.showLoadingIfNeeded( composer );	
}

}