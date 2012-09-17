//!-----------------------------------------------------
//!
//! \file imagecomposer.cpp
//! the 2D + sprite engine system
//!
//!-----------------------------------------------------

#include "scene.h"
#include "core/colour.h"
#include "core/resourceman.h"
#include "textureatlas.h"
#include "rendercontext.h"
#include "databuffer.h"
#include "constantcache.h"
#include "vertexinput.h"

#include "imagecomposer.h"

namespace Scene {

const uint32_t ImageComposer::SizeOfRenderType[ImageComposer::MAX_RENDER_TYPE] = {
	sizeof( ImageComposer::SIMPLE_SPRITE ),
	sizeof( ImageComposer::SOLID_COLOUR ),
};

const VertexInput::CreationInfo ImageComposer::VaoCS[ImageComposer::MAX_RENDER_TYPE] = {
	{
		3, 
		{	{ VE_POSITION, 	VT_FLOAT2 },
			{ VE_TEXCOORD0,	VT_FLOAT2 },
			{ VE_COLOUR0, 	VT_BYTEARGB }, },
		{	{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM },
			{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM },
			{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM } }
	},
	{
		2, 
		{	{ VE_POSITION, VT_FLOAT2 },
			{ VE_COLOUR0, VT_BYTEARGB }, },
		{	{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM },
			{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM } }
	}
};

ImageComposer::ImageComposer( int _maxSpritesPerLayer ) :
	maxSpritesPerLayer( _maxSpritesPerLayer )
{
	program[ SIMPLE_SPRITE ].reset( ProgramHandle::load( "sprite_basic" ) );
	program[ SOLID_COLOUR ].reset( ProgramHandle::load( "2dcolour" ) );

	linearClampSampler.reset( SamplerStateHandle::create( "_SS_Linear_Clamp" ) );
	for( int i = 0; i < MAX_LAYERS; ++i ) {
		layers[i].layerNum = i;
	}

	blendState[ NORMAL ].reset( RenderTargetStatesHandle::create( "_RTS_NoBlend_WriteAll" ) );
	blendState[ COLOUR_MASK ].reset( RenderTargetStatesHandle::create( "_RTS_NoBlend_WriteColour" ) );
	blendState[ ALPHA_MASK ].reset( RenderTargetStatesHandle::create( "_RTS_NoBlend_WriteAlpha" ) );
	blendState[ ALPHA_BLEND ].reset( RenderTargetStatesHandle::create( "_RTS_Over_WriteAll" ) );
	blendState[ PM_OVER ].reset( RenderTargetStatesHandle::create( "_RTS_PMOver_WriteAll" ) );
}

ImageComposer::Layer::Page& ImageComposer::findOrCreatePage( ImageComposer::Layer& layer, ImageComposer::Layer::PageKey& key ) {
	using namespace Scene;
	Layer::PageMap::iterator pmIt = layer.pageMap.find( key );
	if( pmIt == layer.pageMap.end() ) {
		// insert a new page

		const size_t sizeInBytes = maxSpritesPerLayer * 6 * SizeOfRenderType[ key.type ];
		// allocate vertex buffer
		DataBuffer::CreationInfo vbcs( Resource::BufferCtor(
			RCF_BUF_VERTEX | RCF_ACE_CPU_WRITE, 
			sizeInBytes
		) );
		std::stringstream nam;
		nam << "_imagecompPage" << layer.pageMap.size() << "_" << layer.layerNum;
		const std::string name = nam.str();
		DataBufferHandlePtr vertexBufferHandle = DataBufferHandle::create( name.c_str(), &vbcs );

		uint8_t* sysBuffer = CORE_NEW_ARRAY uint8_t[ sizeInBytes ];

		// copy vao creation struct template
		VertexInput::CreationInfo vcs = VaoCS[ key.type ];
		// fill in this vertex buffer
		for( int i = 0;i < vcs.elementCount; ++i ) {
			vcs.data[i].buffer =  vertexBufferHandle;
		}
		const std::string vaoName = name + VertexInput::genEleString( vcs.elementCount, vcs.elements );
		VertexInputHandlePtr vaoHandle = VertexInputHandle::create( vaoName.c_str(), &vcs );

		layer.pageMap[ key ] = Layer::Page( vertexBufferHandle, vaoHandle, program[ key.type ].get() );
		Layer::Page& page = layer.pageMap[ key ];

		page.mapped.reset( sysBuffer );
		page.map();

		return page;
	} else {
		if( pmIt->second.mapped == nullptr ) {
			pmIt->second.map();
		}
		return pmIt->second;
	}
}

void ImageComposer::putTexture(	const Scene::TextureHandlePtr&	pTexture,
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

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*)page.mapped.get();

	// fill in the vertex data
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(0, 1);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(0, 0);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(1, 1);
	pVertexData[ page.numVertices + 2 ].colour = colour.packARGB();
	page.numVertices += 3;
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(0, 0);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(1, 0);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(1, 1);
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

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*) page.mapped.get();
	// fill in the vertex data
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(sprite.u0, sprite.v1);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(sprite.u1, sprite.v1);
	pVertexData[ page.numVertices + 2 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(sprite.u0, sprite.v0);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	page.numVertices += 3;
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(sprite.u0, sprite.v0);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(sprite.u1, sprite.v0);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(sprite.u1, sprite.v1);
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

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*) page.mapped.get();
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
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(u0, v0);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(u1, v1);
	pVertexData[ page.numVertices + 2 ].colour = colour.packARGB();
	page.numVertices += 3;
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(u0, v0);
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y-size.y) );
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(u1, v0);
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(u1, v1);
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

	SolidColourVertex* pVertexData = (SolidColourVertex*) page.mapped.get();
	// fill in the vertex data
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.numVertices + 0 ].colour = tlcol.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.numVertices + 1 ].colour = brcol.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2(tr.x,-tr.y);
	pVertexData[ page.numVertices + 2 ].colour = trcol.packARGB();
	page.numVertices += 3;
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.numVertices + 0 ].colour = tlcol.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2(bl.x,-bl.y);
	pVertexData[ page.numVertices + 1 ].colour = blcol.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.numVertices + 2 ].colour = brcol.packARGB();
	page.numVertices += 3;

}

void ImageComposer::texturedRect(	const Scene::TextureHandlePtr&		pTexture,
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


void ImageComposer::texturedQuad( const Scene::TextureHandlePtr&		pTexture,
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

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*) page.mapped.get();

	// fill in the vertex data
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(uvtl.x,1.0f-uvtl.y);
	pVertexData[ page.numVertices + 0 ].colour = tlcol.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(uvbr.x,1.0f-uvbr.y);
	pVertexData[ page.numVertices + 1 ].colour = brcol.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2(tr.x,-tr.y);
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2( uvbr.x, 1.0f-uvtl.y );
	pVertexData[ page.numVertices + 2 ].colour = trcol.packARGB();
	page.numVertices += 3;
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.numVertices + 0 ].uv = Math::Vector2(uvtl.x,1.0f-uvtl.y);
	pVertexData[ page.numVertices + 0 ].colour = tlcol.packARGB();
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2(bl.x,-bl.y);
	pVertexData[ page.numVertices + 1 ].uv = Math::Vector2(uvtl.x,1.0f-uvbr.y);
	pVertexData[ page.numVertices + 1 ].colour = blcol.packARGB();
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.numVertices + 2 ].uv = Math::Vector2(uvbr.x,1.0f-uvbr.y);
	pVertexData[ page.numVertices + 2 ].colour = brcol.packARGB();
	page.numVertices += 3;

}


void ImageComposer::render( RenderContext* context ) {
	using namespace Scene;

//	glDisable( GL_CULL_FACE );
//	glDisable( GL_DEPTH_TEST );

	auto samplerState = linearClampSampler.acquire();

	for( unsigned int i=0;i < MAX_LAYERS;++i) {
		Layer::PageMap::iterator pmIt = layers[i].pageMap.begin();
		while( pmIt != layers[i].pageMap.end() ) {
			const Layer::PageKey& pagekey = pmIt->first;
			Layer::Page& page = pmIt->second;
			if( page.numVertices == 0 ) {
				++pmIt;
				continue;
			}

			page.unmap();
			DataBufferPtr db = page.vertexBufferHandle->acquire();
			if( !db ) { ++pmIt; continue; }
			void* gpuVerts = (void*) db->map( context, DBMA_WRITE_ONLY, DBMF_DISCARD );
			memcpy( gpuVerts, page.mapped.get(), page.numVertices * 6 * SizeOfRenderType[ pagekey.type ] );
			db->unmap( context );
			auto icProgram = program[ pagekey.type ].acquire();
			auto vao = page.vaoHandle->tryAcquire();
			if( !vao ) { ++pmIt; continue; }
			vao->validate( icProgram );

			context->bind( vao );

			if( pagekey.texture0 ) {
				auto tex = pagekey.texture0->tryAcquire();
				if( tex ) {
					context->bind( ST_FRAGMENT, 0, samplerState );
					context->bind( ST_FRAGMENT, 0, tex );
				}
			}

			context->getConstantCache().setMatrixBypassCache( CVN_VIEW_PROJ, Math::IdentityMatrix() );
			context->getConstantCache().updateGPU( context, icProgram );
			context->bind( icProgram );

			auto blender = blendState[ pagekey.renderStates ].acquire();
			context->bind( blender );
			context->draw( PT_TRIANGLE_LIST, page.numVertices );
			page.numVertices = 0;
			++pmIt;
		}
	}

//	glDisable( GL_BLEND );
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