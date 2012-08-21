#ifndef YOLK_DX11_RESOURCELOADER_H_
#define YOLK_DX11_RESOURCELOADER_H_ 1
#pragma once

#ifndef YOLK_SCENE_RESOURCELOADER_H_
#include "scene/resourceloader.h"
#endif

namespace Dx11 {
class ResourceLoader : public Scene::ResourceLoader {
public:
	ResourceLoader( Scene::Renderer* _renderer ) : renderer( _renderer ) {}

	Scene::Renderer* getRenderer() const override {
		return renderer;
	}

	virtual Scene::DataBuffer* createDataBuffer( const void* );
	virtual Scene::Texture* createTexture( const void* );
	virtual Scene::VertexInput* createVertexInput( const void* );
	virtual Scene::SamplerState* createSamplerState( const void* );
	virtual Scene::RenderTargetStates* createRenderTargetStates( const void* );
	virtual Scene::DepthStencilState* createDepthStencilState( const void* );
	virtual Scene::RasteriserState* createRasteriserState( const void* );

protected:
	
	void installResourceTypes();
	Scene::Renderer*					renderer;
};

}

#endif