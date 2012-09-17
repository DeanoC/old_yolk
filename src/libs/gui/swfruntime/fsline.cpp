#include "swfruntime.h"
#include "gui/swfparser/SwfLineStyle.h"
#include "gui/swfparser/SwfColourTransform.h"
#include "player.h"
#include "fsline.h"

namespace Swf {
FSLine::FSLine( Player* _player, const SwfLineStyle* _lineFill ) : FillStyle( _player ) {
	if(_lineFill != NULL){
		colour = _lineFill->colour;
		if(_lineFill->width > 0) {
			lineWidth = Math::Vector2( (float) _lineFill->width, (float) _lineFill->width);
		} else {
			lineWidth = Math::Vector2(10.0f,10.0f);
		} 
	} else {
		lineWidth = Math::Vector2(10.0f,10.0f);
	}
}
FillStyle::APPLY_RESULT FSLine::testApply( const SwfColourTransform* _colourTransform ){
	// TODO screen width seems hard coded here?!
	Math::Vector2 width = Math::TransformNormal(lineWidth, player->getTwipToNdx()) * 480; 
	float alpha = colour.a * _colourTransform->mul[3] + _colourTransform->add[3];
	if(Math::Length(width) < 1e-2f)
		return NO_OUTPUT;
	if( alpha < 1e-2f )
		return NO_OUTPUT;
	else if( alpha > 0.99f)
		return SOLID_OUTPUT;
	else
		return BLEND_OUTPUT;			
}

FillStyle::APPLY_RESULT FSLine::apply(const SwfColourTransform* _colourTransform) {
	// colour = colour * colourTransform.Mul + colourtransform.Add			
	float alpha = colour.a * _colourTransform->mul[3] + _colourTransform->add[3];
//	CALL_GL( glColor4f(	colour.r * _colourTransform->mul[0] + _colourTransform->add[0], 
//						colour.g * _colourTransform->mul[1] + _colourTransform->add[1], 
//						colour.b * _colourTransform->mul[2] + _colourTransform->add[2], 
//						alpha ) );
	LOG(INFO) << "TODO FSLine setColour\n";

	Math::Vector2 width = Math::TransformNormal(lineWidth, player->getTwipToPixels()) ;
	LOG(INFO) << "TODO FSLine width\n";
//	CALL_GL( glLineWidth(Math::Length(width)) );

	return testApply( _colourTransform ); 	
}

}