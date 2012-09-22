#include "swfruntime.h"
#include "gui/swfparser/SwfColourTransform.h"
#include "scene/constantcache.h"
#include "basepath.h"
#include "swfman.h"
#include "fssolidcolour.h"

namespace Swf {
FSSolidColour::FSSolidColour( Player* _player, const SwfRGBA&  _colour ) : 
	FillStyle ( _player ), 
	colour( _colour ) {

	Math::Matrix4x4 instanceData( _colour.getLinearRed(), _colour.getLinearGreen(), _colour.getLinearBlue(), _colour.getAlpha(),
								  0,0,0,0,
								  0,0,0,0,
								  0,0,0,0 );
	namespace s = Scene;
	s::DataBuffer::CreationInfo insbcs ( s::Resource::BufferCtor(
		s::RCF_BUF_CONSTANT | s::RCF_ACE_IMMUTABLE, 
		sizeof(Math::Matrix4x4), &instanceData
	) );
	constBufferHandle = s::DataBufferHandle::create( "FSSolidColour_insb", &insbcs, Core::RESOURCE_FLAGS::RMRF_DONTCACHE );

	constBufferHandle.acquire(); // cos our immutable data lies on the stack, so we must ensure it lasts till after an acquire
}

bool FSSolidColour::apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) {
	float alpha = colour.getAlpha() * _colourTransform->mul[3] + _colourTransform->add[3];
	if( alpha < 1e-2f )
		return false;

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

	if( _path->isSimple() ) {
		_ctx->drawIndexed( Scene::PT_TRIANGLE_LIST, _path->numIndices );
	} else {
		auto dss = SwfMan::getr().countCrossingDepthStateHandle.acquire();
		auto dssnormal = SwfMan::getr().normalDepthStateHandle.acquire();
		auto dssro = SwfMan::getr().oddDepthStateHandle.acquire();
		auto nors = SwfMan::getr().norenderStateHandle.acquire();
		auto pmrs = SwfMan::getr().renderStateHandle.acquire();
		_ctx->bind( nors );
		_ctx->bind( dss );
		_ctx->drawIndexed( Scene::PT_TRIANGLE_LIST, _path->numIndices );
		_ctx->bind( pmrs );
		_ctx->bind( dssro, 0x1 );
		_ctx->draw( Scene::PT_TRIANGLE_STRIP, 4, _path->extentsRectVertexOffset );
		_ctx->bind( dssnormal );
	}

	return true;
}

}