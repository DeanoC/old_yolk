//!-----------------------------------------------------
//!
//! \file rendercontext.h
//! a render context is a self contain structure with
//! render pipeline 'globals', allows multiple threads
//! to render different world etc. at the same time
//!
//!-----------------------------------------------------
#if !defined(WIERD_GRAPHICS_RENDERCONTEXT_H)
#define WIERD_GRAPHICS_RENDERCONTEXT_H
#pragma once

#if !defined( WIERD_CORE_FRUSTUM_H )
#include "core/frustum.h"
#endif


namespace Graphics {
	class RenderMatrixCache;

	class RenderContext {
	public:

		void useRenderTarget( TexturePtr pTarget, TexturePtr pDepthTarget );
		void useRenderTarget( ID3D11View* pTarget, ID3D11DepthStencilView* pDepthTarget );
		void useRenderTarget( TexturePtr pTarget );
		void useDepthOnlyRenderTarget( TexturePtr pDepthTarget );
		void useDepthOnlyRenderTarget( ID3D11DepthStencilView* pDepthTarget );
		void useRenderTargets( unsigned int numTargets, TexturePtr* pTargets, TexturePtr pDepthTarget );

		void setReferenceShader( const ShaderFXPtr shader ) {
			referenceShader = shader;
		}


		RenderMatrixCache*					matrixCache;
		// note in most cases frustum and camera match, but in some debug cases they
		// might not (to allow you see how frustum culling is working or not)
		Core::Frustum*						viewFrustum;

		ID3D11DeviceContext*				deviceContext;

		ShaderFXPtr					referenceShader;

	};

}

#endif