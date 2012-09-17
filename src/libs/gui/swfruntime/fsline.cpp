#include "swfruntime.h"
#include "gui/swfparser/SwfLineStyle.h"
#include "gui/swfparser/SwfColourTransform.h"
#include "basepath.h"
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

void FSLine::apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) {
	float alpha = colour.a * _colourTransform->mul[3] + _colourTransform->add[3];
	Math::Vector2 width = Math::TransformNormal(lineWidth, player->getTwipToPixels()) ;

	if( alpha < 1e-2f )
		return;
	if(Math::Length(width) < 1e-2f)
		return;
//	CALL_GL( glColor4f(	colour.r * _colourTransform->mul[0] + _colourTransform->add[0], 
//						colour.g * _colourTransform->mul[1] + _colourTransform->add[1], 
//						colour.b * _colourTransform->mul[2] + _colourTransform->add[2], 
//						alpha ) );
	LOG(INFO) << "TODO FSLine width\n";

	// bind vertex and index buffers					
	auto vb = _path->vertexBufferHandle.tryAcquire();
	if( !vb ) { return; }
	auto ib = _path->indexBufferHandle.tryAcquire();
	if( !ib ) { return; }

	_ctx->bindVB( 0, vb, sizeof(float) * 2 );
	_ctx->bindIB( ib, sizeof(uint16_t) );
	_ctx->drawIndexed( Scene::PT_LINE_LIST, _path->numIndices );

}

}