//!-----------------------------------------------------
//!
//! \file swfman.h
//!
//!-----------------------------------------------------
#if !defined( YOLK_SWFRUNTIME_SWFMAN_H_ )
#define YOLK_SWFRUNTIME_SWFMAN_H_
#pragma once

#include "core/singleton.h"
#include "scene/vertexinput.h"
#include "scene/program.h"
#include "scene/rendercontext.h"
#include "scene/renderstates.h"

namespace Swf {
	class GradientTextureManager;

	//! The main singleton for the Swf runtime
	class SwfMan : public Core::Singleton<SwfMan> {
	public:
		friend class GradientTextureManager;
		~SwfMan();

		void bind( Scene::RenderContext* _ctx );

		GradientTextureManager* getGradientTextureManager() { return gradientTextureManager; }

	private:
		friend class Core::Singleton<SwfMan>;
		//! ctor
		SwfMan();

		GradientTextureManager*											gradientTextureManager;

		Core::ScopedResourceHandle<Scene::VertexInputHandle>			vertexInputHandle;
		Core::ScopedResourceHandle<Scene::ProgramHandle>				mainProgramHandle;
		Core::ScopedResourceHandle<Scene::RasteriserStateHandle>		rasterStateHandle;
		Core::ScopedResourceHandle<Scene::RenderTargetStatesHandle>		renderStateHandle;
		Core::ScopedResourceHandle<Scene::SamplerStateHandle>			clampSamplerHandle;
		Core::ScopedResourceHandle<Scene::SamplerStateHandle>			pointClampSamplerHandle;

	};
}

#endif