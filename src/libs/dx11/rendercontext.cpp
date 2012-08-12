//!-----------------------------------------------------
//!
//! \file DeferredLightGBuffers11.cpp
//! The G Buffers for the deferred lighting system
//! for true Dx11 hardware assume sample control, UAV
//! etc.
//! Needs NUM_MRT_TARGETS x width * height * AA_LEVEL * TARGET_BIT_DEPTH/4 VRAM
//! i.e. 1/2 GB for 1280x720 4xMSAA 32 bit with D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT == 8 targets!!
//!
//!-----------------------------------------------------

#include "graphics_pch.h"
#include "graphics.h"
#include "texture.h"
#include "core/resourceman.h"
#include "rendercontext.h"

namespace Graphics {
	void RenderContext::useRenderTarget( TexturePtr pTarget, TexturePtr pDepthTarget ) {
		ID3D11RenderTargetView* rtViews[] = { (ID3D11RenderTargetView*)pTarget->m_extraView };
		deviceContext->OMSetRenderTargets( 1, rtViews, (ID3D11DepthStencilView*)pDepthTarget->m_extraView );
	}

	void RenderContext::useRenderTarget( ID3D11View* pTarget, ID3D11DepthStencilView* pDepthTarget ) {
		ID3D11RenderTargetView* rtViews[] = { (ID3D11RenderTargetView*)pTarget };
		deviceContext->OMSetRenderTargets( 1, rtViews, pDepthTarget );
	}

	void RenderContext::useRenderTarget( TexturePtr pTarget ) {
		ID3D11RenderTargetView* rtViews[] = { (ID3D11RenderTargetView*)pTarget->m_extraView };
		deviceContext->OMSetRenderTargets( 1, rtViews, NULL );
	}

	void RenderContext::useDepthOnlyRenderTarget( TexturePtr pDepthTarget ) {
		deviceContext->OMSetRenderTargets( 0, NULL, (ID3D11DepthStencilView*)pDepthTarget->m_extraView );
	}

	void RenderContext::useDepthOnlyRenderTarget( ID3D11DepthStencilView* pDepthTarget ) {
		deviceContext->OMSetRenderTargets( 0, NULL, pDepthTarget );
	}


	void RenderContext::useRenderTargets( unsigned int numTargets, TexturePtr* pTargets, TexturePtr pDepthTarget ) {
		ID3D11RenderTargetView* rtViews[8];
		assert( numTargets < 8 );
		for( unsigned int i=0;i < numTargets;++i ){ 
			rtViews[i] = (ID3D11RenderTargetView*)pTargets[i]->m_extraView;
		};
		ID3D11DepthStencilView* dTarget = (pDepthTarget != NULL) ? 
						(ID3D11DepthStencilView*) pDepthTarget->m_extraView : NULL;
		deviceContext->OMSetRenderTargets( numTargets, rtViews, dTarget );
	}
}