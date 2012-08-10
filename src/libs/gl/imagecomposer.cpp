//!-----------------------------------------------------
//!
//! \file imagecomposer.cpp
//! the 2D + sprite engine system
//!
//!-----------------------------------------------------

#include "ogl.h"
#include "gfx.h"
#include "core/colour.h"
#include "core/resourceman.h"
#include "textureatlas.h"
#include "rendercontext.h"
#include "constantcache.h"

#include "imagecomposer.h"

namespace Gl {

const uint32_t ImageComposer::SizeOfRenderType[ImageComposer::MAX_RENDER_TYPE] = {
	sizeof( ImageComposer::SIMPLE_SPRITE ),
	sizeof( ImageComposer::SOLID_COLOUR ),
};

const Vao::CreationStruct ImageComposer::VaoCS[ImageComposer::MAX_RENDER_TYPE] = {
	{
		3, nullptr,
		{	{ VE_POSITION, 	VT_FLOAT2 },
			{ VE_TEXCOORD0, VT_FLOAT2 },
			{ VE_COLOUR0, 	VT_BYTEARGB }, },
		{	nullptr, Vao::AUTO_OFFSET, Vao::AUTO_STRIDE, 0,
			nullptr, Vao::AUTO_OFFSET, Vao::AUTO_STRIDE, 0,
			nullptr, Vao::AUTO_OFFSET, Vao::AUTO_STRIDE, 0, }
	},
	{
		2, nullptr,
		{	{ VE_POSITION, VT_FLOAT2 },
			{ VE_COLOUR0, VT_BYTEARGB }, },
		{	nullptr, Vao::AUTO_OFFSET, Vao::AUTO_STRIDE, 0,
			nullptr, Vao::AUTO_OFFSET, Vao::AUTO_STRIDE, 0, }
	}
};

ImageComposer::ImageComposer( int _maxSpritesPerLayer ) :
	maxSpritesPerLayer( _maxSpritesPerLayer )
{
	program[ SIMPLE_SPRITE ] = ProgramHandle::create( "basicsprite" );
	program[ SOLID_COLOUR ] = ProgramHandle::create( "2dcolour" );

	for( int i = 0; i < MAX_LAYERS; ++i ) {
		layers[i].layerNum = i;
	}
}

ImageComposer::~ImageComposer() {
	program[ SIMPLE_SPRITE ]->close();
	program[ SOLID_COLOUR ]->close();
}

void ImageComposer::clearImage() {
//	Graphics::Gfx::Get()->GetDevice()->Clear(0,0, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0);
}

ImageComposer::Layer::Page& ImageComposer::findOrCreatePage( ImageComposer::Layer& layer, ImageComposer::Layer::PageKey& key ) {
	Layer::PageMap::iterator pmIt = layer.pageMap.find( key );
	if( pmIt == layer.pageMap.end() ) {
		// insert a new page

		// allocate vertex buffer
		DataBuffer::CreationStruct vbcs = {
			DBCF_CPU_UPDATES, DBT_VERTEX, 
			maxSpritesPerLayer * 6 * SizeOfRenderType[ key.type ]
		};
		std::stringstream nam;
		nam << "_imagecompPage" << layer.pageMap.size() << "_" << layer.layerNum;
		const std::string name = nam.str();
		DataBufferHandlePtr vertexBufferHandle = DataBufferHandle::create( name.c_str(), &vbcs );

		// copy vao creation struct template
		Vao::CreationStruct vcs = VaoCS[ key.type ];
		// fill in this vertex buffer
		for( int i = 0;i < vcs.elementCount; ++i ) {
			vcs.data[i].buffer =  vertexBufferHandle;
		}
		const std::string vaoName = name + Vao::genEleString( vcs.elementCount, vcs.elements );
		VaoHandlePtr vaoHandle = VaoHandle::create( vaoName.c_str(), &vcs );


		layer.pageMap[ key ] = Layer::Page( vertexBufferHandle, vaoHandle, program[ key.type ] );
		Layer::Page& page = layer.pageMap[ key ];

		page.map();

		return page;
	} else {
		if( pmIt->second.mapped == nullptr ) {
			pmIt->second.map();
		}
		return pmIt->second;
	}
}

void ImageComposer::putTexture(	const TextureHandlePtr&			pTexture,
								unsigned int					renderStates,
								const Math::Vector2&			pos,
								const Math::Vector2&			size,
								const Core::Colour&				colour,
								unsigned int					layerNum )
{
	assert( (layerNum < MAX_LAYERS) && "Invalid layer" );

	// put this sprite on the appropaite layer
	Layer& layer = layers[ layerNum ];

	// assign a sorting page for this sprite
	Layer::PageKey key( pTexture, renderStates, SIMPLE_SPRITE );
	Layer::Page& page = findOrCreatePage( layer, key );

	assert( ((page.numVertices / 6) + 1) < maxSpritesPerLayer );

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*)page.mapped;

	// fill in the vertex data
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(0, 1);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(1, 1);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(0, 0);
	pVertexData[ page.numVertices + 2 ].colour = colour.packARGB();
	page.numVertices += 3;
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(0, 0);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(1, 1);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(1, 0);
	pVertexData[ page.numVertices + 2 ].colour = colour.packARGB();
	page.numVertices += 3;

}

void ImageComposer::putSprite(	const TextureAtlasHandlePtr&	atlasHandle,
								unsigned int					texIndex,
								unsigned int					renderStates,
								const Math::Vector2&			pos,
								const Math::Vector2&			fullsize,
								const Core::Colour&				colour,
								unsigned int					layerNum )
{
	assert( (layerNum < MAX_LAYERS) && "Invalid layer" );

	TextureAtlasPtr atlas = atlasHandle->tryAcquire();
	if( !atlas ) { return; }

	const TextureAtlas::SubTexture& sprite = atlas->getSubTexture( texIndex );
	TextureHandlePtr texture = atlas->getPackedTexture( sprite.index );

	// put this sprite on the appropaite layer
	Layer& layer = layers[ layerNum ];

	// assign a sorting page for this sprite
	Layer::PageKey key(texture, renderStates, SIMPLE_SPRITE);
	Layer::Page& page = findOrCreatePage( layer, key );

	assert( ((page.numVertices / 6) + 1) < maxSpritesPerLayer );

	Math::Vector2 size = fullsize;

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*) page.mapped;
	// fill in the vertex data
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(sprite.u0, sprite.v1);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(sprite.u1, sprite.v1);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(sprite.u0, sprite.v0);
	pVertexData[ page.numVertices + 2 ].colour = colour.packARGB();
	page.numVertices += 3;
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(sprite.u0, sprite.v0);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(sprite.u1, sprite.v1);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(sprite.u1, sprite.v0);
	pVertexData[ page.numVertices + 2 ].colour = colour.packARGB();
	page.numVertices += 3;

}

void ImageComposer::putSubSprite(	
								const TextureAtlasHandlePtr&	atlasHandle,
								unsigned int					texIndex,
								const Math::Vector2&			subTexTL,
								const Math::Vector2&			subTexBR,
								unsigned int					renderStates,
								const Math::Vector2&			pos,
								const Math::Vector2&			fullsize,
								const Core::Colour&				colour,
								unsigned int					layerNum )
{
	assert( (layerNum < MAX_LAYERS) && "Invalid layer" );

	TextureAtlasPtr atlas = atlasHandle->tryAcquire();
	if( !atlas ) { return; }

	const TextureAtlas::SubTexture& sprite = atlas->getSubTexture( texIndex );
	TextureHandlePtr texture = atlas->getPackedTexture( sprite.index );

	// put this sprite on the appropaite layer
	Layer& layer = layers[layerNum];

	// assign a sorting page for this sprite
	Layer::PageKey key( texture, renderStates, SIMPLE_SPRITE);
	Layer::Page& page = findOrCreatePage( layer, key );

	assert( ((page.numVertices / 6) + 1) < maxSpritesPerLayer );

	Math::Vector2			size = fullsize;

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*) page.mapped;
	float uSize = sprite.u1-sprite.u0;
	float vSize = sprite.v1-sprite.v0;

	float u0 = sprite.u0+subTexTL.x*uSize;
	float v0 = sprite.v0+subTexTL.y*vSize;
	float u1 = sprite.u0+subTexBR.x*uSize;
	float v1 = sprite.v0+subTexBR.y*vSize;

	// fill in the vertex data
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(u0, v1);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(u1, v1);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(u0, v0);
	pVertexData[ page.numVertices + 2 ].colour = colour.packARGB();
	page.numVertices += 3;
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(u0, v0);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(u1, v1);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(u1, v0);
	pVertexData[ page.numVertices + 2 ].colour = colour.packARGB();
	page.numVertices += 3;

}

void ImageComposer::filledRect( unsigned int					renderStates,
								const Math::Vector2&			pos,
								const Math::Vector2&			size,
								const Core::Colour&				colour,
								unsigned int					layerNum )
{
	filledQuad( renderStates,	pos + Math::Vector2( -size.x, size.y),
								pos + Math::Vector2( size.x, size.y),
								pos + Math::Vector2( size.x, -size.y ),
								pos + Math::Vector2( -size.x, -size.y ),
								colour,
								colour,
								colour,
								colour,
								layerNum );

}

void ImageComposer::filledQuad( unsigned int					renderStates,
								const Math::Vector2&			tl,
								const Math::Vector2&			tr,
								const Math::Vector2&			br,
								const Math::Vector2&			bl,
								const Core::Colour&				tlcol,
								const Core::Colour&				trcol,
								const Core::Colour&				brcol,
								const Core::Colour&				blcol,
								unsigned int					layerNum )
{
	assert( (layerNum < MAX_LAYERS) && "Invalid layer" );

	// put this quad on the appropaite layer
	Layer& layer = layers[layerNum];

	// assign a sorting page for this quad
	Layer::PageKey key( NULL, renderStates, SOLID_COLOUR );
	Layer::Page& page = findOrCreatePage( layer, key );

	assert( ((page.numVertices / 6) + 1) < maxSpritesPerLayer );

	SolidColourVertex* pVertexData = (SolidColourVertex*) page.mapped;
	// fill in the vertex data
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.numVertices + 0 ].colour = tlcol.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2(tr.x,-tr.y);
	pVertexData[ page.numVertices + 1 ].colour = trcol.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.numVertices + 2 ].colour = brcol.packARGB();
	page.numVertices += 3;
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.numVertices + 0 ].colour = tlcol.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.numVertices + 1 ].colour = brcol.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2(bl.x,-bl.y);
	pVertexData[ page.numVertices + 2 ].colour = blcol.packARGB();
	page.numVertices += 3;

}

void ImageComposer::texturedRect(	const TextureHandlePtr&		pTexture,
									unsigned int					renderStates,
									const Math::Vector2&			pos,
									const Math::Vector2&			size,
									const Core::Colour&				colour,
									const Math::Vector2&			uvtl,
									const Math::Vector2&			uvbr,
									unsigned int					layerNum )
{
	texturedQuad( pTexture, renderStates,	
								pos + Math::Vector2( -size.x, size.y),
								pos + Math::Vector2( size.x, size.y),
								pos + Math::Vector2( size.x, -size.y ),
								pos + Math::Vector2( -size.x, -size.y ),
								colour,
								colour,
								colour,
								colour,
								uvtl,
								uvbr,
								layerNum );

}


void ImageComposer::texturedQuad( const TextureHandlePtr&		pTexture,
								unsigned int					renderStates,
								const Math::Vector2&			tl,
								const Math::Vector2&			tr,
								const Math::Vector2&			br,
								const Math::Vector2&			bl,
								const Core::Colour&				tlcol,
								const Core::Colour&				trcol,
								const Core::Colour&				brcol,
								const Core::Colour&				blcol,
								const Math::Vector2&			uvtl,
								const Math::Vector2&			uvbr,
								unsigned int					layerNum )
{
	assert( (layerNum < MAX_LAYERS) && "Invalid layer" );

	// put this quad on the appropaite layer
	Layer& layer = layers[layerNum];

	// assign a sorting page for this quad
	Layer::PageKey key(pTexture, renderStates, SIMPLE_SPRITE );
	Layer::Page& page = findOrCreatePage( layer, key );

	assert( ((page.numVertices / 6) + 1) < maxSpritesPerLayer );

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*) page.mapped;

	// fill in the vertex data
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(uvtl.x,1.0f-uvtl.y);
	pVertexData[ page.numVertices + 0 ].colour = tlcol.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2(tr.x,-tr.y);
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2( uvbr.x, 1.0f-uvtl.y );
	pVertexData[ page.numVertices + 1 ].colour = trcol.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(uvbr.x,1.0f-uvbr.y);
	pVertexData[ page.numVertices + 2 ].colour = brcol.packARGB();
	page.numVertices += 3;
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(uvtl.x,1.0f-uvtl.y);
	pVertexData[ page.numVertices + 0 ].colour = tlcol.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(uvbr.x,1.0f-uvbr.y);
	pVertexData[ page.numVertices + 1 ].colour = brcol.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2(bl.x,-bl.y);
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(uvtl.x,1.0f-uvbr.y);
	pVertexData[ page.numVertices + 2 ].colour = blcol.packARGB();
	page.numVertices += 3;

}


void ImageComposer::render() {

	auto context = Gfx::get()->getThreadRenderContext( Gfx::RENDER_CONTEXT );

	glDisable(GL_CULL_FACE);

	for( unsigned int i=0;i < MAX_LAYERS;++i) {
		Layer::PageMap::iterator pmIt = layers[i].pageMap.begin();
		while( pmIt != layers[i].pageMap.end() ) {
			const Layer::PageKey& pagekey = pmIt->first;
			Layer::Page& page = pmIt->second;
			if( page.mapped == nullptr ) {
				++pmIt;
				continue;
			}

			page.unmap();

			VaoPtr vao = page.vaoHandle->tryAcquire();
			if( !vao ) {
				++pmIt;
				continue;
			}
			glBindVertexArray( vao->getName() );
			GL_CHECK

			auto icProgram = program[ pagekey.type ]->acquire();
			context->bindWholeProgram( icProgram );
			context->getConstantCache().setMatrixBypassCache( CVN_VIEW_PROJ, Math::IdentityMatrix() );
			context->getConstantCache().updateGPU( icProgram );
			context->getConstantCache().bind();

			TexturePtr tex;
			if( pagekey.texture0 ) {
				tex = pagekey.texture0->tryAcquire();
				if( tex ) {
					glActiveTexture( GL_TEXTURE0 );
					glBindTexture( GL_TEXTURE_2D, tex->getName() );
				}
			}

			switch( pagekey.renderStates ) {
				case NORMAL:
				glDisable( GL_BLEND );
//				Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
				break;
			case COLOUR_MASK:
				glDisable( GL_BLEND );
//				Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
			break;
			case ALPHA_MASK:
				glDisable( GL_BLEND );
//				Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA );
			break;
			case ALPHA_BLEND:
				glEnable( GL_BLEND );
				glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD );
				glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//				Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
				break;
			case PM_OVER:
				glEnable( GL_BLEND );
				glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD );
				glBlendFuncSeparate( GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,  GL_ONE_MINUS_SRC_ALPHA );
//				Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
				break;
			}
			glDrawArrays( GL_TRIANGLES, 0, page.numVertices );
			GL_CHECK;

			++pmIt;
		}
	}

	glDisable( GL_BLEND );
}

bool ImageComposer::Layer::PageKey::operator <(const ImageComposer::Layer::PageKey &b) const {
	if( renderStates == b.renderStates ) {
		if( type == type ){
			return texture0 < texture0;
		} else {
			return type < b.type;
		}
	} else {
		return renderStates < b.renderStates;
	}
}

}