//!-----------------------------------------------------
//!
//! \file renderstates.cpp
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "gfx.h"
#include "rendercontext.h"
#include "renderstates.h"

namespace Dx11 {

D3D11_FILTER FM_Map[] = {
	D3D11_FILTER_MIN_MAG_MIP_POINT,
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR,
	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_MIN_MAG_MIP_LINEAR,
	D3D11_FILTER_ANISOTROPIC,
	D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
	D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
	D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
	D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
	D3D11_FILTER_COMPARISON_ANISOTROPIC
};

D3D11_TEXTURE_ADDRESS_MODE AM_Map[] = {
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_MIRROR,
	D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_BORDER,
	D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
};

D3D11_COMPARISON_FUNC CF_Map[] = {
	D3D11_COMPARISON_NEVER,
	D3D11_COMPARISON_LESS,
	D3D11_COMPARISON_EQUAL,
	D3D11_COMPARISON_LESS_EQUAL,
	D3D11_COMPARISON_GREATER,
	D3D11_COMPARISON_NOT_EQUAL,
	D3D11_COMPARISON_GREATER_EQUAL,
	D3D11_COMPARISON_ALWAYS 
};

D3D11_BLEND BM_Map[] = {
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ONE,
	D3D11_BLEND_SRC_COLOR,
	D3D11_BLEND_INV_SRC_COLOR,
	D3D11_BLEND_SRC_ALPHA,
	D3D11_BLEND_INV_SRC_ALPHA,
	D3D11_BLEND_DEST_ALPHA,
	D3D11_BLEND_INV_DEST_ALPHA,
	D3D11_BLEND_DEST_COLOR,
	D3D11_BLEND_INV_DEST_COLOR,
	D3D11_BLEND_SRC_ALPHA_SAT,
	D3D11_BLEND_BLEND_FACTOR,
	D3D11_BLEND_INV_BLEND_FACTOR,
	D3D11_BLEND_SRC1_COLOR,
	D3D11_BLEND_INV_SRC1_COLOR,
	D3D11_BLEND_SRC1_ALPHA,
	D3D11_BLEND_INV_SRC1_ALPHA,
};

D3D11_BLEND_OP BO_Map[] = {
	D3D11_BLEND_OP_ADD,
	D3D11_BLEND_OP_SUBTRACT,
	D3D11_BLEND_OP_REV_SUBTRACT,
	D3D11_BLEND_OP_MIN,
	D3D11_BLEND_OP_MAX,
};

D3D11_STENCIL_OP SO_Map[] = {
	D3D11_STENCIL_OP_KEEP,
	D3D11_STENCIL_OP_ZERO,
	D3D11_STENCIL_OP_REPLACE,
	D3D11_STENCIL_OP_INCR_SAT,
	D3D11_STENCIL_OP_DECR_SAT,
	D3D11_STENCIL_OP_INVERT,
	D3D11_STENCIL_OP_INCR,
	D3D11_STENCIL_OP_DECR,	
};

D3D11_FILL_MODE FIM_Map[] = {
	D3D11_FILL_WIREFRAME,
	D3D11_FILL_SOLID,
};

D3D11_CULL_MODE CUM_Map[] = {
	D3D11_CULL_NONE,
	D3D11_CULL_FRONT,
	D3D11_CULL_BACK,
};

Scene::SamplerState* SamplerState::internalCreate( const Scene::SamplerState::CreationInfo* creation ) {
	using namespace Scene;
	D3D11_SAMPLER_DESC desc;
	desc.Filter = FM_Map[ creation->filter ];
	desc.AddressU = AM_Map[ creation->addressU ];
	desc.AddressV = AM_Map[ creation->addressV ];
	desc.AddressW = AM_Map[ creation->addressW ];
	desc.ComparisonFunc = CF_Map[ creation->compareFunc ];
	desc.MipLODBias = creation->mipLODBias;
	desc.MinLOD = creation->minLOD;
	desc.MaxLOD = creation->maxLOD;
	desc.MaxAnisotropy = creation->maxAnisotropy;
	desc.BorderColor[0] = creation->borderColour[0];
	desc.BorderColor[1] = creation->borderColour[1];
	desc.BorderColor[2] = creation->borderColour[2];
	desc.BorderColor[3] = creation->borderColour[3];

	ID3D11SamplerState* state;
	Gfx::getr()()->CreateSamplerState( &desc, &state );
	SamplerState* ss = CORE_NEW SamplerState;
	ss->samplerState = D3DSamplerStatePtr( state, false );
	return ss;
}

Scene::RenderTargetStates* RenderTargetStates::internalCreate( const Scene::RenderTargetStates::CreationInfo* creation ) {
	using namespace Scene;

	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = (creation->flags & CreationInfo::ALPHA_TO_COV);
	desc.IndependentBlendEnable = (creation->numActive > 1);

	for( int i = 0; i < creation->numActive; ++i ) {
		const TargetState& cts = creation->renderTargetState[i];
		desc.RenderTarget[i].BlendEnable = (cts.flags & TargetState::BLEND_ENABLE);
		desc.RenderTarget[i].SrcBlend = BM_Map[ cts.blendSrcColour ];
		desc.RenderTarget[i].DestBlend = BM_Map[ cts.blendDstColour ];
		desc.RenderTarget[i].BlendOp = BO_Map[ cts.blendColourOp ];
		desc.RenderTarget[i].SrcBlendAlpha = BM_Map[ cts.blendSrcAlpha ];
		desc.RenderTarget[i].DestBlendAlpha = BM_Map[ cts.blendDstAlpha ];
		desc.RenderTarget[i].BlendOpAlpha = BO_Map[ cts.blendAlphaOp ];

		desc.RenderTarget[i].RenderTargetWriteMask = 
			((cts.writeEnable & TWE_RED) ? D3D11_COLOR_WRITE_ENABLE_RED : 0) |
			((cts.writeEnable & TWE_GREEN) ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0) |
			((cts.writeEnable & TWE_BLUE) ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0) |
			((cts.writeEnable & TWE_ALPHA) ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0);

	}
	ID3D11BlendState* state;
	Gfx::getr()()->CreateBlendState( &desc, &state );
	RenderTargetStates* ss = CORE_NEW RenderTargetStates;
	ss->renderTargetState = D3DBlendStatePtr( state, false );
	ss->numActive = creation->numActive;

	return ss;
}

Scene::DepthStencilState* DepthStencilState::internalCreate( const Scene::DepthStencilState::CreationInfo* creation ) {
	using namespace Scene;

	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = (creation->flags & CreationInfo::DEPTH_ENABLE);
	desc.StencilEnable = (creation->flags & CreationInfo::STENCIL_ENABLE);
	desc.DepthWriteMask = (creation->flags & CreationInfo::DEPTH_WRITE) ? 
				D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = CF_Map[ creation->depthCompare ];
	desc.StencilReadMask = creation->stencilReadMask;
	desc.StencilWriteMask = creation->stencilWriteMask;
	desc.FrontFace.StencilFailOp = SO_Map[ creation->stencilFrontFailOp ];
	desc.FrontFace.StencilDepthFailOp = SO_Map[ creation->stencilFrontDepthFailOp ];
	desc.FrontFace.StencilPassOp = SO_Map[ creation->stencilFrontPassOp ];
	desc.FrontFace.StencilFunc = CF_Map[ creation->stencilFrontCompare ];
	desc.BackFace.StencilFailOp = SO_Map[ creation->stencilBackFailOp ];
	desc.BackFace.StencilDepthFailOp = SO_Map[ creation->stencilBackDepthFailOp ];
	desc.BackFace.StencilPassOp = SO_Map[ creation->stencilBackPassOp ];
	desc.BackFace.StencilFunc = CF_Map[ creation->stencilBackCompare ];

	ID3D11DepthStencilState* state;
	Gfx::getr()()->CreateDepthStencilState( &desc, &state );
	DepthStencilState* ss = CORE_NEW DepthStencilState;
	ss->depthStencilState = D3DDepthStencilStatePtr( state, false );
	return ss;

}

Scene::RasteriserState* RasteriserState::internalCreate( const Scene::RasteriserState::CreationInfo* creation ) {
	using namespace Scene;
	D3D11_RASTERIZER_DESC desc;
	desc.FrontCounterClockwise = (creation->flags & RasteriserState::CreationInfo::FRONT_CCW);
	desc.DepthClipEnable = !(creation->flags & RasteriserState::CreationInfo::DEPTH_CLAMP);
	desc.ScissorEnable = (creation->flags & RasteriserState::CreationInfo::SCISSOR);
	desc.MultisampleEnable = (creation->flags & RasteriserState::CreationInfo::MULTISAMPLE);
	desc.AntialiasedLineEnable = (creation->flags & RasteriserState::CreationInfo::AA_LINES);
	desc.FillMode = FIM_Map[ creation->fillMode ];
	desc.CullMode = CUM_Map[ creation->cullMode ];
	desc.DepthBias = creation->depthBias;
	desc.DepthBiasClamp = creation->depthBiasClamp;
	desc.SlopeScaledDepthBias = creation->depthScaledSlopeBias;

	ID3D11RasterizerState* state;
	Gfx::getr()()->CreateRasterizerState( &desc, &state );
	RasteriserState* ss = CORE_NEW RasteriserState;
	ss->rasteriserState = D3DRasterizerStatePtr( state, false );
	return ss;

}

}