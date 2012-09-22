#ifndef YOLK_GUI_SWFRUNTIME_FSSOLIDCOLOUR_H_
#define YOLK_GUI_SWFRUNTIME_FSSOLIDCOLOUR_H_ 1
#pragma once

#ifndef YOLK_GUI_SWFRUNTIME_FILLSTYLE_H_
#	include "fillstyle.h"
#endif
#include "gui/swfparser/swfrgb.h"

namespace Swf {
	class FSSolidColour : public FillStyle {
	public:
		FSSolidColour( Player* _player, const SwfRGBA&  _colour );

		void apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) override;
	private:
		SwfRGBA colour;
		Core::ScopedResourceHandle<Scene::DataBufferHandle>			constBufferHandle;
	};
}
#endif