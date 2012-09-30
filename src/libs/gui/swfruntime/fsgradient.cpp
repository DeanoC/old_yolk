#include "swfruntime.h"
#include "gui/swfparser/SwfColourTransform.h"
#include "gui/swfparser/SwfFillStyle.h"
#include "scene/constantcache.h"
#include "basepath.h"
#include "player.h"
#include "swfman.h"
#include "utils.h"
#include "fsgradient.h"

namespace Swf {

FSGradient::FSGradient( Player* _player, SwfGradientFillStyle* _gradientFill) :
	FillStyle(_player),
	gradientFill( _gradientFill ),
	gradTex( nullptr ),
	gpuUpdated( false ) {

	switch (gradientFill->fillType) {
	case LinearGradientFill:
		gradTex = SwfMan::getr().getGradientTextureManager()->allocateLinearGradientTexture(); break;
	case RadialGradientFill:
		gradTex = SwfMan::getr().getGradientTextureManager()->allocateRadialGradientTexture(); break;
	default:
		CORE_ASSERT( false );
	}
	CORE_ASSERT( gradTex );

	// map the _extents to -16K to 16K gradient space
	matrix = Math::IdentityMatrix();
	matrix = Math::CreateTranslationMatrix(16384.0f, 16384.0f, 0.0f );
	matrix = Math::MultiplyMatrix( matrix, Math::CreateScaleMatrix(1.0f / 32768.0f, 1.0f / 32768.0f, 0.0f) );
	matrix = Math::MultiplyMatrix( Math::InverseMatrix( Convert(gradientFill->Matrix())), matrix );
	matrix = Math::MultiplyMatrix( matrix, Math::CreateScaleMatrix(gradTex->scale.x, gradTex->scale.y, 0.0f) );
	matrix = Math::MultiplyMatrix( matrix, Math::CreateTranslationMatrix(gradTex->offset.x, gradTex->offset.y, 0.0f) );
	// we use matrix._14 to mark texture vs colour in the shader
	matrix._14 = -1.0f;

	namespace s = Scene;
	s::DataBuffer::CreationInfo insbcs ( s::Resource::BufferCtor( s::RCF_BUF_CONSTANT | s::RCF_ACE_IMMUTABLE, sizeof(Math::Matrix4x4), &matrix ) );
	constBufferHandle = s::DataBufferHandle::create( "FSGradient_insb", &insbcs, Core::RESOURCE_FLAGS::RMRF_DONTCACHE );
	samplerHandle.reset( Scene::SamplerStateHandle::create( SAMPLER_STATE_ANISO16_CLAMP ) );
}


bool FSGradient::apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) {
	// TODO SOLID_OUTPUT needs to anaylse gradient to see if solid alpha
	float alpha = _colourTransform->mul[3] + _colourTransform->add[3];
	if(alpha < 1e-2f) {
		return false;
	}
	if( gpuUpdated == false ) {
		switch (gradientFill->fillType) {
		case LinearGradientFill:
			SwfMan::getr().getGradientTextureManager()->updateLinearGradient( _ctx, gradTex, gradientFill ); break;
		case RadialGradientFill:
			SwfMan::getr().getGradientTextureManager()->updateRadialGradient( _ctx, gradTex, gradientFill ); break;
		default:
			CORE_ASSERT( false );
		}
		gpuUpdated = true;
	}
	// bind vertex and index buffers					
	auto vb = _path->vertexBufferHandle.tryAcquire();
	if( !vb ) { return false; }
	auto insb = constBufferHandle.tryAcquire();
	if( !insb ) { return false; }
	auto ib = _path->indexBufferHandle.tryAcquire();
	if( !ib ) { return false; }
	auto ss = samplerHandle.acquire();
	_ctx->bind( Scene::ST_FRAGMENT, 0, ss );
	_ctx->bind( Scene::ST_FRAGMENT, 0, gradTex->page->page );
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