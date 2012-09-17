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
		FSSolidColour( Player* _player, const SwfRGBA&  _colour ) : FillStyle ( _player ), colour( _colour ){}

		FillStyle::APPLY_RESULT testApply(const SwfColourTransform* _colourTransform) override;
		FillStyle::APPLY_RESULT apply(const SwfColourTransform* _colourTransform) override;
	private:
		SwfRGBA colour;
	};
}
#endif