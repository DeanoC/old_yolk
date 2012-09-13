//!-----------------------------------------------------
//!
//! \file resourceloader.cpp
//! async loader/creator handler + thread
//!
//!-----------------------------------------------------

#include "screen.h"
#include "core/resourceman.h"
#include "wobfile.h"
#include "hierfile.h"
#include "textureatlas.h"
#include "imagecomposer.h"
#include "texture.h"
#include "program.h"
#include "databuffer.h"
#include "vertexinput.h"
#include "renderstates.h"
#include "debugprims.h"
#include "programman.h"

// doesn't really belong here, its higher level library than scene but scene needs a seperation itself, 
// so for now and make life simple, just bung its loader here
#include "gui/swfruntime/swfruntime.h"
#include "gui/swfruntime/swfplayer.h"

#include <boost/asio.hpp>
#include "resourceloader.h"

namespace Scene {

class ResourceLoaderImpl {
public:
	friend class ResourceLoader;
	ResourceLoaderImpl();
	~ResourceLoaderImpl();

	void renderThreadUpdate( Scene::ImageComposer* composer );

	void installResourceTypes();
	static void PushOntoQueues( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  );
	static void AcquirePumper();

	static std::shared_ptr<boost::asio::io_service>		loaderIo;
	static std::shared_ptr<boost::asio::io_service>		renderIo;
	static ResourceLoader* 								parent;
	static ResourceLoaderImpl*							renderLoaderImpl;
	static std::atomic<int>								workCounter;
	static Core::thread::id 							renderThreadId;
	static Core::thread::id 							loaderThreadId;

	Core::thread*										loaderThread;
	Scene::TextureAtlasHandlePtr 						loadTextureAtlas;
	std::unique_ptr<boost::asio::io_service::work>		workUnit;
};

std::shared_ptr<boost::asio::io_service> 			ResourceLoaderImpl::loaderIo;
std::shared_ptr<boost::asio::io_service> 			ResourceLoaderImpl::renderIo;
ResourceLoader* 									ResourceLoaderImpl::parent(0);
ResourceLoaderImpl*									ResourceLoaderImpl::renderLoaderImpl(nullptr);
std::atomic<int> 									ResourceLoaderImpl::workCounter(0);
Core::thread::id 									ResourceLoaderImpl::renderThreadId;
Core::thread::id 									ResourceLoaderImpl::loaderThreadId;

#define RENDER( x ) res = std::static_pointer_cast<ResourceBase>( x ## Ptr( ResourceLoaderImpl::parent->create ## x( data ) ) ); \
					if( flags & Core::RMRF_SCENE_DEFINED ) { CORE_DELETE (const x ::CreationInfo*) data; };

void ProcessRender( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* name, const void* data  ) {
	using namespace Core;
	using namespace Swf;
	std::shared_ptr<Core::ResourceBase> res;

	// route type to their specific creation functions 
	switch( handle->getType() ) {
	case WobType: RENDER( Wob ); break;
	case HieType: RENDER( Hie ); break;
	case TextureType: RENDER( Texture ); break;
	case TextureAtlasType: RENDER( TextureAtlas ); break;
	case ProgramType: RENDER( Program ); break;
	case DataBufferType: RENDER( DataBuffer ); break;
	case VertexInputType: RENDER( VertexInput );  break;
	case SamplerStateType: RENDER( SamplerState ); break;
	case RenderTargetStatesType: RENDER( RenderTargetStates ); break;
	case DepthStencilStateType: RENDER( DepthStencilState ); break;
	case RasteriserStateType: RENDER( RasteriserState ); break;
	case SwfPlayerType: RENDER( SwfPlayer ); break;
	default:;
	}

	--ResourceLoaderImpl::workCounter;
	Core::ResourceMan::get()->internalAcquireComplete( handle, res );
}

#undef RENDER
#define LOADER( x ) cs = (void*) ResourceLoaderImpl::parent->preCreate( (flags & Core::RMRF_LOADOFFDISK) ? pName : nullptr, (const x ::CreationInfo*) pData )

void ProcessLoader( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	void* cs = nullptr;
	// route type to their specific creation functions, they should alloc and fill a new creation struct
	switch( handle->getType() ) {
	case WobType: LOADER( Wob ); break;
	case HieType: LOADER( Hie ); break;
	case TextureType: LOADER( Texture ); break;
	case TextureAtlasType: LOADER( TextureAtlas ); break;
	case ProgramType: LOADER( Program ); break;
	case DataBufferType: LOADER( DataBuffer ); break;
	case SamplerStateType: LOADER( SamplerState ); break;
	case VertexInputType: LOADER( VertexInput );  break;
	case RenderTargetStatesType: LOADER( RenderTargetStates ); break;
	case DepthStencilStateType: LOADER( DepthStencilState ); break;
	case RasteriserStateType: LOADER( RasteriserState ); break;
	case Swf::SwfPlayerType: LOADER( Swf::SwfPlayer ); break;
	default:;
	}
	CORE_ASSERT( cs != nullptr );

	// defines that the creation structure should be frees after ProcessRender
	// TODO we are leaking
//	flags = (Core::RESOURCE_FLAGS) (flags | Core::RMRF_SCENE_DEFINED);
	
	ResourceLoaderImpl::renderIo->post( std::bind<void>( ProcessRender, handle, flags, pName, cs ) );
}

#undef LOADER

void ResourceLoaderImpl::PushOntoQueues( const Core::ResourceHandleBase* handle, 
                                                                	Core::RESOURCE_FLAGS flags, 
                                                                    const char* pName, 
                                                                    const void* pData  ) {
	ResourceLoaderImpl::workCounter++;
	ResourceLoaderImpl::loaderIo->post( std::bind<void>( ProcessLoader, handle, flags, pName, pData ) ); 
}

void ResourceLoaderImpl::AcquirePumper() {
	if( ResourceLoaderImpl::renderThreadId == std::this_thread::get_id() ) {
		ResourceLoaderImpl::renderIo->poll_one();
	} else {
		ResourceLoaderImpl::loaderIo->poll_one();
	}
}

void ResourceLoaderImpl::installResourceTypes() {
	using namespace Core;

	ResourceMan::get()->registerAcquirePump( AcquirePumper );

	#define SRD(x) SimpleResourceDestroyer< x >
	#define SO(x) sizeof( x )
	#define REG ResourceMan::get()->registerResourceType

	auto cb = &ResourceLoaderImpl::PushOntoQueues;

	REG ( WobType, cb, &SRD(Wob), SO(WobHandle), 
							NULL, 0 , "Meshes/" );
	REG ( HieType, cb, &SRD(Hie), SO(HieHandle), 
							NULL, 0 , "Hier/" );
	REG ( TextureType, cb, &SRD(Texture), SO(TextureHandle), 
							NULL, 0, "Textures/" );
	REG ( TextureAtlasType, cb, &SRD(TextureAtlas), SO(TextureAtlasHandle), 
							NULL, 0, "Textures/" );
	REG ( ProgramType, cb, &SRD(Program), SO(ProgramHandle), 
							NULL, 0, "" );
	REG( DataBufferType, cb, &SRD(DataBuffer), SO(DataBufferHandle), 
							NULL, 0, "" );
	REG( VertexInputType, cb, &SRD(VertexInput), SO(VertexInputHandle), 
							NULL, 0, "" );
	REG( SamplerStateType, cb, &SRD(SamplerState), SO(SamplerStateHandle), 
							NULL, 0, "" );
	REG( RenderTargetStatesType, cb, &SRD(RenderTargetStates), SO(RenderTargetStatesHandle), 
							NULL, 0, "" );
	REG( DepthStencilStateType, cb, &SRD(DepthStencilState), SO(DepthStencilStateHandle), 
							NULL, 0, "" );
	REG( RasteriserStateType, cb, &SRD(RasteriserState), SO(RasteriserStateHandle), 
							NULL, 0, "" );
	REG( Swf::SwfPlayerType, cb, &SRD(Swf::SwfPlayer), SO(Swf::SwfPlayerHandle), 
							NULL, 0, "Ui/" );
	#undef SRD
	#undef SO
	#undef REG

	loadTextureAtlas = TextureAtlasHandle::load( "base-ui.tat" );
}

ResourceLoaderImpl::ResourceLoaderImpl() {
	renderLoaderImpl = this;

	loaderIo = std::make_shared<boost::asio::io_service>();
	renderIo = std::make_shared<boost::asio::io_service>();

	// loaderIo happens on a seperate thread (can be made multiple threads)
	// as doesn't call anything that requires the primary render thread
	loaderThread = CORE_NEW Core::thread( 
		[&] {
			loaderThreadId = std::this_thread::get_id();
			GC_stack_base stackBase;
			GC_get_stack_base( &stackBase );
			GC_register_my_thread( &stackBase );
			workUnit = std::unique_ptr<boost::asio::io_service::work>( CORE_NEW boost::asio::io_service::work( *loaderIo ) );
			loaderIo->run();
		}
	);

	// must be pumped manually during frame handling
	renderThreadId = std::this_thread::get_id();
	installResourceTypes();
}

void ResourceLoaderImpl::renderThreadUpdate( Scene::ImageComposer* composer ) {
	renderIo->poll();

	if( composer != nullptr ) {
		composer->putSprite( loadTextureAtlas, 1, 
						ImageComposer::ALPHA_BLEND, 
						Math::Vector2( 0.85f, 0.95f ),
						Math::Vector2( 0.15f, 0.05f ),
						Core::RGBAColour::unpackARGB(0x80FFFFFF),
						1 );

		if( ResourceLoaderImpl::workCounter > 0) {
			composer->putSprite( loadTextureAtlas, 0, 
							ImageComposer::ALPHA_BLEND, 
							Math::Vector2( 0.85f, 0.95f ),
							Math::Vector2( 0.15f, 0.05f ),
							Core::RGBAColour::unpackARGB(0xFFFFFFFF),
							0 );
		}

	}

}

ResourceLoaderImpl::~ResourceLoaderImpl() {
	loadTextureAtlas->close();
	workUnit.reset();
	loaderThread->join();
	CORE_DELETE loaderThread;
	loaderIo.reset();
	renderIo.reset();
}

ResourceLoader::ResourceLoader() :
	impl( *(CORE_NEW ResourceLoaderImpl()) ) {
	impl.parent = this;
}

ResourceLoader::~ResourceLoader() {
	CORE_DELETE( &impl );
}

void ResourceLoader::installResourceTypes() {
	impl.installResourceTypes();
}

void ResourceLoader::renderThreadUpdate( Scene::ImageComposer* composer ) {
	impl.renderThreadUpdate( composer );	
}

Hie* ResourceLoader::createHie( const void* data ) {
	return (Hie*)data;
}
TextureAtlas* ResourceLoader::createTextureAtlas( const void* data ) {
	return (TextureAtlas*)data;
}
Program* ResourceLoader::createProgram( const void* data ) {
	return Program::internalCreate( data );
}
Wob* ResourceLoader::createWob( const void* data ) {
	return Wob::internalCreate( getRenderer(), data );
}
Swf::SwfPlayer* ResourceLoader::createSwfPlayer( const void* data ) {
	return (Swf::SwfPlayer*)data;
}

const void* ResourceLoader::preCreate( const char* name, const DataBuffer::CreationInfo *loader ) {
	return DataBuffer::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const Hie::CreationInfo *loader ) {
	return Hie::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const Program::CreationInfo *loader ) {
	return Program::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const Texture::CreationInfo *loader ) {
	return Texture::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const TextureAtlas::CreationInfo *loader ) {
	return TextureAtlas::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const VertexInput::CreationInfo* loader ) {
	return VertexInput::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const Wob::CreationInfo* loader ) {
	return Wob::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const SamplerState::CreationInfo* loader ) {
	return SamplerState::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const RenderTargetStates::CreationInfo* loader ) {
	return RenderTargetStates::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const DepthStencilState::CreationInfo* loader ) {
	return DepthStencilState::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const RasteriserState::CreationInfo* loader ) {
	return RasteriserState::internalPreCreate( name, loader );
}
const void* ResourceLoader::preCreate( const char* name, const  Swf::SwfPlayer::CreationInfo* loader ) {
	return Swf::SwfPlayer::internalPreCreate( name, loader );
}

}