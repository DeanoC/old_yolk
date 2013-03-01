#ifndef YOLK_DX11_RENDER_STATES_H_
#define YOLK_DX11_RENDER_STATES_H_ 1

#ifndef YOLK_SCENE_RENDER_STATES_H_
#include "scene/renderstates.h"
#endif

namespace Dx11 {
	class SamplerState : public Scene::SamplerState {
	public:
		D3DSamplerStatePtr		samplerState;
		
		static Scene::SamplerState* internalCreate( const Scene::SamplerState::CreationInfo* creation );
	};

	class RenderTargetStates : public Scene::RenderTargetStates {
	public:
		int						numActive;
		D3DBlendStatePtr		renderTargetState;

		static Scene::RenderTargetStates* internalCreate( const Scene::RenderTargetStates::CreationInfo* creation );
	};
	class DepthStencilState : public Scene::DepthStencilState {
	public:
		D3DDepthStencilStatePtr depthStencilState;

		static Scene::DepthStencilState* internalCreate( const Scene::DepthStencilState::CreationInfo* creation );
	};

	class RasteriserState : public Scene::RasteriserState {
	public:
		D3DRasterizerStatePtr	rasteriserState;
		
		static Scene::RasteriserState* internalCreate( const Scene::RasteriserState::CreationInfo* creation );
	};

}
#endif