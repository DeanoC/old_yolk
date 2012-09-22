#ifndef YOLK_GUI_SWFRUNTIME_FSTEXTURE_H_
#define YOLK_GUI_SWFRUNTIME_FSTEXTURE_H_ 1
#pragma once

#ifndef YOLK_GUI_SWFRUNTIME_FILLSTYLE_H_
#	include "fillstyle.h"
#endif
#include "gui/swfparser/swfrgb.h"

namespace Swf {
	class Bitmap;

	class FSTexture : public FillStyle {
	public:
		FSTexture( Player* _player, SwfBitmapFillStyle* _gradientFill );

		bool apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) override;
	private:
		Bitmap*														bitmapTex;
		SwfBitmapFillStyle*											bitmapFill;
		Math::Matrix4x4												matrix;
		Core::ScopedResourceHandle<Scene::DataBufferHandle>			constBufferHandle;
		Core::ScopedResourceHandle<Scene::SamplerStateHandle>		samplerHandle;
	};
}
#endif