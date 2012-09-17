#ifndef YOLK_GUI_SWFRUNTIME_FSLINE_H_
#define YOLK_GUI_SWFRUNTIME_FSLINE_H_ 1
#pragma once

#ifndef YOLK_GUI_SWFRUNTIME_FILLSTYLE_H_
#	include "fillstyle.h"
#endif
#include "gui/swfparser/swfrgb.h"

namespace Swf {
	class SwfLineStyle;

	class FSLine : public FillStyle {
	public:
		FSLine( Player* _player, const SwfLineStyle* _lineFill );

		FillStyle::APPLY_RESULT testApply( const SwfColourTransform* _colourTransform ) override;
		FillStyle::APPLY_RESULT apply( const SwfColourTransform* _colourTransform ) override;
	private:
		SwfRGBA 		colour;
		Math::Vector2 	lineWidth;
	};
}
#endif