#ifndef YOLK_GUI_SWFRUNTIME_FSGRADIENT_H_
#define YOLK_GUI_SWFRUNTIME_FSGRADIENT_H_ 1
#pragma once

#ifndef YOLK_GUI_SWFRUNTIME_FILLSTYLE_H_
#	include "fillstyle.h"
#endif
#include "gui/swfparser/swfrgb.h"
#include "gradienttexturemanager.h"

namespace Swf {
	class FSGradient : public FillStyle {
	public:
		FSGradient( Player* _player, SwfGradientFillStyle* _gradientFill );

		bool apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) override;
	private:
		SwfGradientFillStyle*										gradientFill;
		Math::Matrix4x4												matrix;
		GradientTexture*											gradTex;
		Core::ScopedResourceHandle<Scene::DataBufferHandle>			constBufferHandle;
		Core::ScopedResourceHandle<Scene::SamplerStateHandle>		samplerHandle;
		bool														gpuUpdated;
	};
}
#endif