#include "swfruntime.h"
#include "gui/swfparser/SwfBitmap.h"
#include "gui/swfparser/SwfColourTransform.h"
#include "gui/swfparser/SwfFillStyle.h"
#include "scene/constantcache.h"
#include "basepath.h"
#include "player.h"
#include "swfman.h"
#include "bitmap.h"
#include "utils.h"
#include "fstexture.h"

namespace Swf {

FSTexture::FSTexture( Player* _player, SwfBitmapFillStyle* _bitmapFill ) :
	FillStyle(_player),
	bitmapFill( _bitmapFill ) {
	if (_bitmapFill->BitmapID() != 0xFFFF)
	{
		bitmapTex = _player->getBitmap( bitmapFill->BitmapID() );

		matrix = Math::IdentityMatrix();
		matrix = Math::MultiplyMatrix(matrix, Math::CreateScaleMatrix(1.0f / bitmapTex->bitmap->width, 1.0f / bitmapTex->bitmap->height, 0.0f));
		matrix = Math::MultiplyMatrix(Math::InverseMatrix( Convert(_bitmapFill->Matrix())), matrix);
		matrix = Math::MultiplyMatrix(matrix, Math::CreateScaleMatrix(bitmapTex->scale.x, bitmapTex->scale.y, 0.0f));
		matrix = Math::MultiplyMatrix(matrix, Math::CreateTranslationMatrix(bitmapTex->offset.x, bitmapTex->offset.y, 0.0f));

		// we use matrix._14 to mark texture vs colour in the shader
		matrix._14 = -1.0f;

		namespace s = Scene;
		s::DataBuffer::CreationInfo insbcs ( s::Resource::BufferCtor( s::RCF_BUF_CONSTANT | s::RCF_ACE_IMMUTABLE, sizeof(Math::Matrix4x4), &matrix ) );
		constBufferHandle = s::DataBufferHandle::create( "FSTexture_insb", &insbcs, Core::RESOURCE_FLAGS::RMRF_DONTCACHE );

		switch( bitmapFill->fillType ) {
		case NonSmoothedClippedBitmapFill:		samplerHandle.reset( Scene::SamplerStateHandle::create( SAMPLER_STATE_POINT_CLAMP ) ); break;
		case NonSmoothedRepeatingBitmapFill:	samplerHandle.reset( Scene::SamplerStateHandle::create( SAMPLER_STATE_POINT_WRAP ) ); break;
		case ClippedBitmapFill:					samplerHandle.reset( Scene::SamplerStateHandle::create( SAMPLER_STATE_ANISO16_CLAMP ) ); break;
		case RepeatingBitmapFill:				samplerHandle.reset( Scene::SamplerStateHandle::create( SAMPLER_STATE_ANISO16_WRAP ) ); break;
		default:
			CORE_ASSERT( false );
		}
	}
}


void FSTexture::apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) {
	float alpha = _colourTransform->mul[3] + _colourTransform->add[3];
	if(alpha < 1e-2f) {
		return;
	}

	// bind vertex and index buffers					
	auto vb = _path->vertexBufferHandle.tryAcquire();
	if( !vb ) { return; }
	auto insb = constBufferHandle.tryAcquire();
	if( !insb ) { return; }
	auto ib = _path->indexBufferHandle.tryAcquire();
	if( !ib ) { return; }
	auto tex = bitmapTex->textureHandle.tryAcquire();
	if( !tex ) { return; }
	auto ss = samplerHandle.acquire();

	_ctx->bind( Scene::ST_FRAGMENT, 0, ss );
	_ctx->bind( Scene::ST_FRAGMENT, 0, tex );
	_ctx->bindVB( 0, vb, sizeof(float) * 2 );
	_ctx->bindCB( Scene::ST_VERTEX, Scene::CF_USER_BLOCK0, insb );
	_ctx->bindIB( ib, sizeof(uint16_t) );
	_ctx->drawIndexed( Scene::PT_TRIANGLE_LIST, _path->numIndices );

}

}