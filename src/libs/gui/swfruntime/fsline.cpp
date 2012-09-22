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

	Math::Matrix4x4 instanceData( colour.getLinearRed(), colour.getLinearGreen(), colour.getLinearBlue(), colour.getAlpha(),
								  0,0,0,0,
								  0,0,0,0,
								  0,0,0,0 );
	namespace s = Scene;
	s::DataBuffer::CreationInfo insbcs ( s::Resource::BufferCtor(
		s::RCF_BUF_CONSTANT | s::RCF_ACE_IMMUTABLE, 
		sizeof(Math::Matrix4x4), &instanceData
	) );
	constBufferHandle = s::DataBufferHandle::create( "FSLine_insb", &insbcs, Core::RESOURCE_FLAGS::RMRF_DONTCACHE );

	constBufferHandle.acquire(); // cos our immutable data lies on the stack, so we must ensure it lasts till after an acquire

}

bool FSLine::apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) {
	float alpha = colour.getAlpha() * _colourTransform->mul[3] + _colourTransform->add[3];
//	Math::Vector2 width = Math::TransformNormal(lineWidth, player->getTwipToPixels()) ;

	if( alpha < 1e-2f )
		return false;
	// TODO line width
//	if(Math::Length(width) < 1e-2f)
//		return false;

	// bind vertex and index buffers					
	auto vb = _path->vertexBufferHandle.tryAcquire();
	if( !vb ) { return false; }
	auto insb = constBufferHandle.tryAcquire();
	if( !insb ) { return false; }
	auto ib = _path->indexBufferHandle.tryAcquire();
	if( !ib ) { return false; }

	_ctx->bindVB( 0, vb, sizeof(float) * 2 );
	_ctx->bindCB( Scene::ST_VERTEX, Scene::CF_USER_BLOCK0, insb );
	_ctx->bindIB( ib, sizeof(uint16_t) );
	_ctx->drawIndexed( Scene::PT_LINE_LIST, _path->numIndices );
	return true;
}

}