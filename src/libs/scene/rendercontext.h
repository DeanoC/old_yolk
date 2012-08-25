#pragma once
//!-----------------------------------------------------
//!
//! \file rendercontext.h
//! simple redirects to the backend version
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_RENDERCONTEXT_H_)
#define YOLK_SCENE_RENDERCONTEXT_H_

#if !defined(YOLK_SCENE_PROGRAM_H_)
#	include "scene/program.h"
#endif
#if !defined(YOLK_SCENE_CONSTANTCACHE_H_)
#	include "scene/constantcache.h"
#endif
#if !defined(YOLK_SCENE_TEXTURE_H_)
#	include "scene/texture.h"
#endif
#if !defined(YOLK_SCENE_RENDERSTATES_H_)
#	include "scene/renderstates.h"
#endif
#if !defined(YOLK_SCENE_VERTEXINPUT_H_)
#	include "scene/vertexinput.h"
#endif

namespace Scene {
	enum PRIMITIVE_TOPOLOGY {
		PT_POINT_LIST,
		PT_LINE_LIST,
		PT_LINE_STRIP,
		PT_TRIANGLE_LIST,
		PT_TRIANGLE_STRIP,
		PT_LINE_LIST_ADJ,
		PT_LINE_STRIP_ADJ,
		PT_TRIANGLE_LIST_ADJ,
		PT_TRIANGLE_STRIP_ADJ,
	};

	struct Viewport {
		float tlX;
		float tlY;
		float width;
		float height;
		float minDepth;
		float maxDepth;
	};

	class RenderContext {
	public:
		virtual void pushDebugMarker( const char* ) const = 0;
		virtual void popDebugMarker() const = 0;		

		virtual void clear( const TexturePtr& target, const Core::Colour& colour ) = 0;
		virtual void clearDepthStencil( const TexturePtr& target, bool clearDepth, float depth, bool clearStencil, uint8_t stencil ) = 0; 

		virtual void bindRenderTargets( const TexturePtr& target, const TexturePtr& depthTarget ) = 0;
		virtual void bindRenderTarget( const TexturePtr& target ) = 0;
		virtual void bindDepthOnlyRenderTarget( const TexturePtr& depthTarget ) = 0;
		virtual void bindRenderTargets( unsigned int numTargets, const TexturePtr* const pTargets, const TexturePtr& depthTarget ) = 0;
		virtual void bindRenderTargets( unsigned int numTargets, const TexturePtr* const pTargets ) = 0;

		// todo proper viewports
		virtual void bind( const Viewport& viewport ) = 0;
	
		virtual void bind( const ProgramPtr& prg ) = 0;
		virtual void bind( const SHADER_TYPES type, const uint32_t unit, const TexturePtr& tex ) = 0;
		virtual void bind( const SHADER_TYPES type, const uint32_t unit, const SamplerStatePtr sampler ) = 0;
		virtual void bind( const RenderTargetStatesPtr targetStates ) = 0;
		virtual void bind( const DepthStencilStatePtr dsStates ) = 0;
		virtual void bind( const RasteriserStatePtr rasteriser ) = 0;
		virtual void bind( const VertexInputPtr vertexInput ) = 0;

		virtual void bindIndexBuffer( const DataBufferPtr ib, int indexBytes ) = 0;

		virtual void draw( PRIMITIVE_TOPOLOGY topo, uint32_t vertexCount, uint32_t startVertex = 0 ) = 0;
		virtual void drawIndexed( PRIMITIVE_TOPOLOGY topo, uint32_t indexCount, uint32_t startIndex = 0, uint32_t baseOffset = 0 ) = 0;


		virtual void unbindRenderTargets() = 0;

		Scene::ConstantCache& getConstantCache() { return constantCache; }

	protected:
		Scene::ConstantCache					constantCache;
	};
}

#endif // end YOLK_SCENE_RENDERCONTEXT_H_