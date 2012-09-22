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

}


void FSGradient::apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) {
	// TODO SOLID_OUTPUT needs to anaylse gradient to see if solid alpha
	float alpha = _colourTransform->mul[3] + _colourTransform->add[3];
	if(alpha < 1e-2f) {
		return;
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
	if( !vb ) { return; }
	auto insb = constBufferHandle.tryAcquire();
	if( !insb ) { return; }
	auto ib = _path->indexBufferHandle.tryAcquire();
	if( !ib ) { return; }

	_ctx->bind( Scene::ST_FRAGMENT, 0, gradTex->page->page );
	_ctx->bindVB( 0, vb, sizeof(float) * 2 );
	_ctx->bindCB( Scene::ST_VERTEX, Scene::CF_USER_BLOCK0, insb );
	_ctx->bindIB( ib, sizeof(uint16_t) );
	_ctx->drawIndexed( Scene::PT_TRIANGLE_LIST, _path->numIndices );

	// Enable use of the texture
/*	CALL_GL( glActiveTexture(GL_TEXTURE0) );
	CALL_GL( glEnable(GL_TEXTURE_2D) );
	CALL_GL( glMatrixMode(GL_TEXTURE) );
	CALL_GL( glLoadMatrixf(matrix) );
	CALL_GL( glBindTexture(GL_TEXTURE_2D, gradTex->textureNum) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_COMBINE) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_RGB, GL_MODULATE) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_ALPHA, GL_MODULATE) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC0_RGB, GL_PRIMARY_COLOR) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC1_RGB, GL_TEXTURE) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC0_ALPHA, GL_PRIMARY_COLOR) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC1_ALPHA, GL_TEXTURE) );
	CALL_GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
	CALL_GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
	CALL_GL( glActiveTexture(GL_TEXTURE1) );
	CALL_GL( glEnable(GL_TEXTURE_2D) );
	CALL_GL( glBindTexture(GL_TEXTURE_2D, player->GetStockTexture(OPAQUE_WHITE_TEXTURE)) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_COMBINE) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_RGB, GL_ADD) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_ALPHA, GL_ADD) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC0_RGB, GL_PREVIOUS) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC1_RGB, GL_CONSTANT) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC0_ALPHA, GL_PREVIOUS) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC1_ALPHA, GL_CONSTANT) );
	CALL_GL( glEnableClientState(GL_TEXTURE_COORD_ARRAY) );
	CALL_GL( glClientActiveTexture(GL_TEXTURE0) );
	CALL_GL( glTexCoordPointer(2, GL_FLOAT, 0, 0) );
	CALL_GL( glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR, _colourTransform->add) );
	CALL_GL( glColor4f(_colourTransform->mul[0],_colourTransform->mul[1],_colourTransform->mul[2],_colourTransform->mul[3]));
	// TODO SOLID_OUTPUT needs to anaylse gradient to see if solid alpha
	float alpha = _colourTransform->mul[3] + _colourTransform->add[3];
	if(alpha < 1e-2f)
		return NO_OUTPUT;
	else
		return BLEND_OUTPUT;*/
}

}