#pragma once
#if !defined( YOLK_DX11_SCREEN_H_ )
#define YOLK_DX11_SCREEN_H_

#if !defined( YOLK_SCENE_SCREEN_H_ )
#include "scene/screen.h"
#endif

#if !defined( YOLK_SCENE_TEXTURE_H_ )
#include "scene/texture.h"
#endif

#if !defined( YOLK_SCENE_PROGRAM_H_ )
#include "scene/program.h"
#endif

#if !defined( YOLK_SCENE_RENDERSTATES_H_ )
#include "scene/renderstates.h"
#endif

namespace Dx11 {
	class Screen : public Scene::Screen {
	public:
		friend class Gfx;
		void display( Scene::TextureHandlePtr toDisplay ) override;

	protected:
		Core::ScopedResourceHandle<Scene::SamplerStateHandle>		pointClampSamplerHandle;
		Core::ScopedResourceHandle<Scene::ProgramHandle>			copyProgramHandle;
		Core::ScopedResourceHandle<Scene::ProgramHandle>			msaaResolveProgramHandle;

		Core::ScopedResourceHandle<Scene::TextureHandle>			backHandle;
		DXGISwapChainPtr											swapChain;
	};
}

#endif
