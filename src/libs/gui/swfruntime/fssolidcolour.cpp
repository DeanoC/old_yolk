#include "swfruntime.h"
#include "gui/swfparser/SwfColourTransform.h"
#include "scene/constantcache.h"
#include "basepath.h"
#include "fssolidcolour.h"

namespace Swf {
FSSolidColour::FSSolidColour( Player* _player, const SwfRGBA&  _colour ) : 
	FillStyle ( _player ), 
	colour( _colour ) {

	Math::Matrix4x4 instanceData( _colour.r, _colour.g, _colour.b, _colour.a,
								  0,0,0,0,
								  0,0,0,0,
								  0,0,0,0 );
	namespace s = Scene;
	s::DataBuffer::CreationInfo insbcs ( s::Resource::BufferCtor(
		s::RCF_BUF_CONSTANT | s::RCF_ACE_IMMUTABLE, 
		sizeof(Math::Matrix4x4), &instanceData
	) );
	instanceBufferHandle = s::DataBufferHandle::create( "FSSolidColour_insb", &insbcs, Core::RESOURCE_FLAGS::RMRF_DONTCACHE );

	instanceBufferHandle.acquire(); // cos our immutable data lies on the stack, so we must ensure it lasts till after an acquire
}

void FSSolidColour::apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) {
	float alpha = colour.a * _colourTransform->mul[3] + _colourTransform->add[3];
	if( alpha < 1e-2f )
		return;
//	CALL_GL( glColor4f(	colour.r * _colourTransform->mul[0] + _colourTransform->add[0], 
//						colour.g * _colourTransform->mul[1] + _colourTransform->add[1], 
//						colour.b * _colourTransform->mul[2] + _colourTransform->add[2], 
//						alpha ) );

	// bind vertex and index buffers					
	auto vb = _path->vertexBufferHandle.tryAcquire();
	if( !vb ) { return; }
	auto insb = instanceBufferHandle.tryAcquire();
	if( !insb ) { return; }
	auto ib = _path->indexBufferHandle.tryAcquire();
	if( !ib ) { return; }

	_ctx->bindVB( 0, vb, sizeof(float) * 2 );
	_ctx->bindCB( Scene::ST_VERTEX, Scene::CF_USER_BLOCK0, insb );
	_ctx->bindIB( ib, sizeof(uint16_t) );
	_ctx->drawIndexed( Scene::PT_TRIANGLE_LIST, _path->numIndices );

}

/*
      SwfRFSGradient::SwfRFSGradient(SwfPlayer* _player, SwfGradientFillStyle* _gradientFill) :
	SwfRuntimeFillStyle(_player)
      {
          gradTex = _player->GradientTextureManager()->AllocateGradient(_gradientFill);
          // map the _extents to -16K to 16K gradient space
          matrix = Math::IdentityMatrix();
          matrix = Math::CreateTranslationMatrix(16384.0f, 16384.0f, 0.0f);
          matrix = Math::MultiplyMatrix(matrix, Math::CreateScaleMatrix(1.0f / 32768.0f, 1.0f / 32768.0f, 0.0f));
          matrix = Math::MultiplyMatrix(Math::InverseMatrix( Convert(_gradientFill->Matrix())), matrix);
          matrix = Math::MultiplyMatrix(matrix, Math::CreateScaleMatrix(gradTex->scale.x, gradTex->scale.y, 0.0f));
		matrix = Math::MultiplyMatrix(matrix, Math::CreateTranslationMatrix(gradTex->offset.x, gradTex->offset.y, 0.0f));
      }
      SwfRuntimeFillStyle::APPLY_RESULT SwfRFSGradient::TestApply(const SwfColourTransform* _colourTransform){
	// TODO SOLID_OUTPUT needs to anaylse gradient to see if solid alpha
	float alpha = _colourTransform->mul[3] + _colourTransform->add[3];
	if(alpha < 1e-2f)
		return NO_OUTPUT;
	else
		return BLEND_OUTPUT;	
}

SwfRuntimeFillStyle::APPLY_RESULT SwfRFSGradient::Apply(const SwfColourTransform* _colourTransform)
{
	// Enable use of the texture
	CALL_GL( glActiveTexture(GL_TEXTURE0) );
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
		return BLEND_OUTPUT;
}
SwfRFSTexture::SwfRFSTexture(SwfPlayer* _player, SwfBitmapFillStyle* _bitmapFill) :
	SwfRuntimeFillStyle(_player)
{
	if (_bitmapFill->BitmapID() != 0xFFFF)
	{
		bitmapTex = _player->GetBitmap(_bitmapFill->BitmapID());
		matrix = Math::IdentityMatrix();
		matrix = Math::MultiplyMatrix(matrix, Math::CreateScaleMatrix(1.0f / bitmapTex->bitmap->width, 1.0f / bitmapTex->bitmap->height, 0.0f));
		matrix = Math::MultiplyMatrix(Math::InverseMatrix( Convert(_bitmapFill->Matrix())), matrix);
		matrix = Math::MultiplyMatrix(matrix, Math::CreateScaleMatrix(bitmapTex->scale.x, bitmapTex->scale.y, 0.0f));
		matrix = Math::MultiplyMatrix(matrix, Math::CreateTranslationMatrix(bitmapTex->offset.x, bitmapTex->offset.y, 0.0f));
	} 
}
SwfRuntimeFillStyle::APPLY_RESULT SwfRFSTexture::TestApply(const SwfColourTransform* _colourTransform){
	// TODO SOLID_OUTPUT needs to anaylse texture to see if solid alpha
	float alpha = _colourTransform->mul[3] + _colourTransform->add[3];
	if(alpha < 1e-2f)
		return NO_OUTPUT;
	else
		return BLEND_OUTPUT;	
}
SwfRuntimeFillStyle::APPLY_RESULT SwfRFSTexture::Apply(const SwfColourTransform* _colourTransform)
{
	// Enable use of the texture
	CALL_GL( glActiveTexture(GL_TEXTURE0) );
	CALL_GL( glEnable(GL_TEXTURE_2D) );
	CALL_GL( glMatrixMode(GL_TEXTURE) );
	CALL_GL( glLoadMatrixf(matrix) );
	CALL_GL( glBindTexture(GL_TEXTURE_2D, bitmapTex->texNum) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_COMBINE) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_RGB, GL_MODULATE) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_ALPHA, GL_MODULATE) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC0_RGB, GL_PRIMARY_COLOR) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC1_RGB, GL_TEXTURE) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC0_ALPHA, GL_PRIMARY_COLOR) );
	CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_SRC1_ALPHA, GL_TEXTURE) );
	CALL_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	CALL_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
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
	return TestApply(_colourTransform);
}
SwfRFSLine::SwfRFSLine(SwfPlayer* _player, SwfLineStyle* _lineFill) :
	SwfRuntimeFillStyle(_player)
{
	if(_lineFill != NULL){
		colour = _lineFill->colour;
		if(_lineFill->width > 0) {
			lineWidth = Math::Vector2(_lineFill->width,_lineFill->width);
		} else {
			lineWidth = Math::Vector2(10.0f,10.0f);
		} 
	} else {
		lineWidth = Math::Vector2(10.0f,10.0f);
	}
}
SwfRuntimeFillStyle::APPLY_RESULT SwfRFSLine::TestApply(const SwfColourTransform* _colourTransform){			
	Math::Vector2 width = Math::TransformNormal(lineWidth, player->GetTwipToNdx()) * 480;
	if(Math::Length(width) < 1e-2f)
		return NO_OUTPUT;
	float alpha = colour.a * _colourTransform->mul[3] + _colourTransform->add[3];
	if(alpha < 1e-2f)
		return NO_OUTPUT;
	else if( alpha > 0.99f)
		return SOLID_OUTPUT;
	else
		return BLEND_OUTPUT;
}

SwfRuntimeFillStyle::APPLY_RESULT SwfRFSLine::Apply(const SwfColourTransform* _colourTransform){			
	// colour = colour * colourTransform.Mul + colourtransform.Add			
	float alpha = colour.a * _colourTransform->mul[3] + _colourTransform->add[3];
	CALL_GL( glColor4f(	colour.r * _colourTransform->mul[0] + _colourTransform->add[0], 
						colour.g * _colourTransform->mul[1] + _colourTransform->add[1], 
						colour.b * _colourTransform->mul[2] + _colourTransform->add[2], 
						alpha ) );
						
	Math::Vector2 width = Math::TransformNormal(lineWidth, player->GetTwipToPixels()) ;
	
	CALL_GL( glLineWidth(Math::Length(width)) );
	
	return TestApply(_colourTransform);
}
*/
}