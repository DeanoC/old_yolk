//!-----------------------------------------------------
//!
//! \file renderworld.h
//! Holder class for an entire world as seen by the 
//! renderer
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_RENDERWORLD_H )
#define YOLK_DX11_RENDERWORLD_H
#pragma once


namespace Dx11 {

	class RenderContext;
	class Renderable;
	class DeferredLightGBuffers10;
	class DeferredLightGBuffers11;
	class SM1view;

	class RenderWorld {
	public:
		RenderWorld();
		~RenderWorld();

		//! add an Renderable to the world
		virtual void addRenderable( Renderable* renderable );

		//! remove this Renderable from the world
		virtual void removeRenderable( Renderable* renderable );

		virtual void render( RenderContext* context );

		virtual void debugDraw( RenderContext* context );

	protected:
		void render11( RenderContext* context );

		void renderVisibilityMaps( RenderContext* context );

		void renderRenderables(  RenderContext* context, const Core::string& renderPassName );

		typedef Core::vector< Renderable* > RenderableContainer;

		RenderableContainer				renderables;
		DeferredLightGBuffers11*		gbuffers11;
		SM1view*						testSM;
	};
}

#endif