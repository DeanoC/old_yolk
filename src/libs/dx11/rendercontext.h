//!-----------------------------------------------------
//!
//! \file rendercontext.h
//! a render context is a self contain structure with
//! render pipeline 'globals', allows multiple threads
//! to render different world etc. at the same time
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_RENDERCONTEXT_H_ )
#define YOLK_DX11_RENDERCONTEXT_H_
#pragma once

#if !defined( YOLK_CORE_FRUSTUM_H_ )
#include "core/frustum.h"
#endif

#if !defined( YOLK_SCENE_RENDERCONTEXT_H_ )
#include "scene/rendercontext.h"
#endif

#if !defined( YOLK_SCENE_DATABUFFER_H_ )
#include "scene/databuffer.h"
#endif

namespace Dx11 {

	class RenderContext : public Scene::RenderContext {
	public:
		RenderContext( D3DDeviceContextPtr dvc );
		//---------------------------------------------------------------

		virtual void pushDebugMarker( const char* text ) const override;
		virtual void popDebugMarker() const override;

		virtual void bindRenderTargets( const Scene::TexturePtr& target, const Scene::TexturePtr& depthTarget ) override;
		virtual void bindRenderTarget( const Scene::TexturePtr& target ) override;
		virtual void bindDepthOnlyRenderTarget( const Scene::TexturePtr& depthTarget ) override;
		virtual void bindRenderTargets( unsigned int numTargets, const Scene::TexturePtr* const pTargets, const Scene::TexturePtr& depthTarget ) override;
		virtual void bindRenderTargets( unsigned int numTargets, const Scene::TexturePtr* const pTargets ) override;
		virtual void bindUnorderedViews( unsigned int numViews, const Scene::TexturePtr* const sviews ) override;

			// todo proper viewports
		virtual void bind( const Scene::Viewport& viewport ) override;
	
		virtual void bind( const Scene::ProgramPtr& prg ) override;
		virtual void bind( const Scene::SHADER_TYPES type, const uint32_t unit, const Scene::TexturePtr& tex ) override;
		virtual void bind( const Scene::SHADER_TYPES type, const uint32_t unit, const Scene::DataBufferPtr& sampler ) override;
		virtual void bind( const Scene::SHADER_TYPES type, const uint32_t unit, const Scene::SamplerStatePtr& sampler ) override;

		virtual void bind( const Scene::RenderTargetStatesPtr& targetStates ) override;
		virtual void bind( const Scene::DepthStencilStatePtr& dsStates ) override;
		virtual void bind( const Scene::RasteriserStatePtr& rasteriser ) override;
		virtual void bind( const Scene::VertexInputPtr& vertexInput ) override;

		virtual void bindIndexBuffer( const Scene::DataBufferPtr& ib, int indexBytes ) override;

		virtual void clear( const Scene::TexturePtr& target, const Core::Colour& colour ) override;
		virtual void clearDepthStencil( const Scene::TexturePtr& target, bool clearDepth, float depth, bool clearStencil, uint8_t stencil ) override; 

		virtual void copy( const Scene::DataBufferPtr& dst, const Scene::DataBufferPtr& src ) override;
		virtual void copy( const Scene::TexturePtr& dst, const Scene::TexturePtr& src ) override;

		virtual void draw( Scene::PRIMITIVE_TOPOLOGY topo, uint32_t vertexCount, uint32_t startVertex = 0 ) override;
		virtual void drawIndexed( Scene::PRIMITIVE_TOPOLOGY topo, uint32_t indexCount, uint32_t startIndex = 0, uint32_t baseOffset = 0 ) override;
		virtual void dispatch( uint32_t xThreads, uint32_t yThreads, uint32_t zThreads ) override; // D3D11 NOTE: threads NOT thread groups

		virtual void unbindRenderTargets() override;
		virtual void unbindTexture( const Scene::SHADER_TYPES type, const uint32_t unit, const uint32_t count = 1 ) override; // not usually needed but shuts up d3d11 warnings
		virtual void unbindUnorderedViews() override;
		//---------------------------------------------------------------
		D3DDeviceContextPtr				ctx;
		// whem a compute shader is bound these descrive the thread group size 
		uint32_t					threadGroupXSize;
		uint32_t					threadGroupYSize;
		uint32_t					threadGroupZSize;

	};

}

#endif