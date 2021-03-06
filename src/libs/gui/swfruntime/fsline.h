#ifndef YOLK_GUI_SWFRUNTIME_FSLINE_H_
#define YOLK_GUI_SWFRUNTIME_FSLINE_H_ 1
#pragma once

#ifndef YOLK_GUI_SWFRUNTIME_FILLSTYLE_H_
#	include "fillstyle.h"
#endif
#include "gui/swfparser/SwfRGB.h"

namespace Swf {
	class SwfLineStyle;

	class FSLine : public FillStyle {
	public:
		FSLine( Player* _player, const SwfLineStyle* _lineFill );

		bool apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) override;
	private:
		SwfRGBA 		colour;
		Math::Vector2 	lineWidth;
		Core::ScopedResourceHandle<Scene::DataBufferHandle>			constBufferHandle;
	};
}
#endif