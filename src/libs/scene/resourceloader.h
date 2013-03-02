#ifndef YOLK_SCENE_RESOURCELOADER_H_
#define YOLK_SCENE_RESOURCELOADER_H_ 1
#pragma once

#ifndef YOLK_CORE_SINGLE_H_
#	include "core/singleton.h"
#endif
#ifndef YOLK_SCENE_HIER_H_
#include "scene/hier.h"
#endif
#ifndef YOLK_SCENE_PROGRAM_H_
#include "scene/program.h"
#endif
#ifndef YOLK_SCENE_TEXTUREATLAS_H_
#include "scene/textureatlas.h"
#endif
#ifndef YOLK_SCENE_DATABUFFER_H_
#include "scene/databuffer.h"
#endif
#ifndef YOLK_SCENE_VERTEXINPUT_H_
#include "scene/vertexinput.h"
#endif
#ifndef YOLK_SCENE_TEXTUREATLAS_H_
#include "scene/textureatlas.h"
#endif
#ifndef YOLK_SCENE_WOB_H_
#include "scene/wobfile.h"
#endif
#ifndef YOLK_SCENE_RENDERSTATES_H_
#include "scene/renderstates.h"
#endif

// stuck here into re-org
#ifndef YOLK_GUI_SWFRUNTIME_SWFPLAYER_H_
#include "gui/swfruntime/player.h"
#endif

namespace Scene {
// forward decl
class ImageComposer;
class ResourceLoaderImpl;
class Renderer;

// resource loader should be created (dereived of course)by the graphics sysytem on the primary render thead
// it keeps the thread id for acquire pumping, so better make sure its right
class ResourceLoader : public Core::Singleton< ResourceLoader, true > {
public:
	friend class Core::Singleton<ResourceLoader,true>;
	// should be called near the end of the frame, also will print a loading graohic
	void renderThreadUpdate(ImageComposer* composer );

	virtual Renderer* getRenderer() const = 0;

	// don't use directly, go via resource system
	virtual Hie* createHie( const void* data );
	virtual Program* createProgram( const void* data );
	virtual TextureAtlas* createTextureAtlas( const void* data );
	virtual DataBuffer* createDataBuffer( const void* data ) = 0;
	virtual Texture* createTexture( const void* data ) = 0;
	virtual VertexInput* createVertexInput( const void* data ) = 0;
	virtual Wob* createWob( const void* data );
	virtual SamplerState* createSamplerState( const void* data ) = 0;
	virtual RenderTargetStates* createRenderTargetStates( const void* data ) = 0;
	virtual DepthStencilState* createDepthStencilState( const void* data ) = 0;
	virtual RasteriserState* createRasteriserState( const void* data ) = 0;
	virtual Swf::Player* createSwfPlayer( const void* data );

	virtual const void* preCreate( const char* name, const DataBuffer::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const Hie::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const Program::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const Texture::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const TextureAtlas::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const VertexInput::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const Wob::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const SamplerState::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const RenderTargetStates::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const DepthStencilState::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const RasteriserState::CreationInfo* loader );
	virtual const void* preCreate( const char* name, const Swf::Player::CreationInfo* loader );

protected:
	ResourceLoader();
	virtual ~ResourceLoader();

	void installResourceTypes();
	ResourceLoaderImpl& impl;

};

}

#endif