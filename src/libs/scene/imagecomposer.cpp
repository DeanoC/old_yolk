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
	sizeof( SimpleSpriteVertex ),			// SIMPLE_SPRITE
	sizeof( SolidColourVertex ),			// SOLID_COLOUR
	sizeof( SimpleSpriteVertex ), // DISTANCE_FIELD
};

const VertexInput::CreationInfo ImageComposer::VaoCS[ImageComposer::MAX_RENDER_TYPE] = {
	{	// SIMPLE_SPRITE vertex
		3, 
		{	{ VE_POSITION, 	VT_FLOAT2 },
			{ VE_TEXCOORD0,	VT_FLOAT2 },
			{ VE_COLOUR0, 	VT_BYTEARGB }, },
		{	{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM },
			{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM },
			{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM } }
	},
	{	// SOLID_COLOUR vertex
		2, 
		{	{ VE_POSITION, VT_FLOAT2 },
			{ VE_COLOUR0, VT_BYTEARGB }, },
		{	{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM },
			{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM } }
	},
	{	// DISTANCE_FIELD
		3, 
		{ { VE_POSITION, VT_FLOAT2 },
		{ VE_TEXCOORD0, VT_FLOAT2 },
		{ VE_COLOUR0, VT_BYTEARGB }, },
		{ { nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM },
		{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM },
		{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM } }
	}
};

const Scene::GPUConstants::ICGPUConstants ImageComposer::defaultConstants = {
	Math::Vector4(1.0f, 0, 0, 0),
};

ImageComposer::ImageComposer(	unsigned int _targetWidth,
								unsigned int _targetHeight,
								unsigned int _targetDPI,
								unsigned int _maxSpritesPerLayer ) :
	targetWidth(_targetWidth),
	targetHeight(_targetHeight),
	targetDPI(_targetDPI),
	maxSpritesPerLayer(_maxSpritesPerLayer)
{
	program[ SIMPLE_SPRITE ].reset( ProgramHandle::load( "sprite_basic" ) );
	program[ SOLID_COLOUR ].reset( ProgramHandle::load( "2dcolour" ) );
	program[ DISTANCE_FIELD ].reset( ProgramHandle::load("distance_field") );

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
			(uint32_t) sizeInBytes
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

		DataBuffer::CreationInfo cbcs(Resource::BufferCtor(
			RCF_BUF_CONSTANT | RCF_ACE_CPU_WRITE, (uint32_t)sizeof(Scene::GPUConstants::ICGPUConstants)
			));
		const auto constantsHandle = DataBufferHandle::create(std::string("imagecomposer").c_str(), &cbcs);

		layer.pageMap[key] = Layer::Page{ vertexBufferHandle, vaoHandle, program[key.type].get(), constantsHandle };
		Layer::Page& page = layer.pageMap[ key ];

		page.mapped.reset(sysBuffer, [](uint8_t* arr) { CORE_DELETE_ARRAY( arr ); });
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
	Layer::PageKey key( pTexture, renderStates, SIMPLE_SPRITE, defaultConstants );
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
	Layer::PageKey key(texture, renderStates, SIMPLE_SPRITE, defaultConstants);
	Layer::Page& page = findOrCreatePage( layer, key );

	assert( ((page.numVertices / 6) + 1) < maxSpritesPerLayer );

	Math::Vector2 size = fullsize;

	// support texture packer rotation by checking u0 < 0

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*) page.mapped.get();
	// fill in the vertex data
	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x - size.x, -(pos.y + size.y) );
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x - size.x, -(pos.y - size.y) );
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x + size.x, -(pos.y + size.y) );

	if (sprite.u0 < 0) {
		pVertexData[page.numVertices + 0].uv = Math::Vector2(-sprite.u0, sprite.v0);
		pVertexData[page.numVertices + 1].uv = Math::Vector2( sprite.u1, sprite.v0);
		pVertexData[page.numVertices + 2].uv = Math::Vector2(-sprite.u0, sprite.v1);
	}
	else {
		pVertexData[page.numVertices + 0].uv = Math::Vector2(sprite.u0, sprite.v1);
		pVertexData[page.numVertices + 1].uv = Math::Vector2(sprite.u0, sprite.v0);
		pVertexData[page.numVertices + 2].uv = Math::Vector2(sprite.u1, sprite.v1);
	}
	pVertexData[ page.numVertices + 0 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 2 ].colour = colour.packARGB();
	pVertexData[ page.numVertices + 1 ].colour = colour.packARGB();
	page.numVertices += 3;

	pVertexData[ page.numVertices + 0 ].pos = Math::Vector2( pos.x - size.x, -(pos.y - size.y) );
	pVertexData[ page.numVertices + 1 ].pos = Math::Vector2( pos.x + size.x, -(pos.y - size.y) );
	pVertexData[ page.numVertices + 2 ].pos = Math::Vector2( pos.x + size.x, -(pos.y + size.y) );
	if (sprite.u0 < 0) {
		pVertexData[page.numVertices + 0].uv = Math::Vector2(sprite.u1, sprite.v0);
		pVertexData[page.numVertices + 1].uv = Math::Vector2(sprite.u1, sprite.v1);
		pVertexData[page.numVertices + 2].uv = Math::Vector2(-sprite.u0, sprite.v1);
	}
	else {
		pVertexData[page.numVertices + 0].uv = Math::Vector2(sprite.u0, sprite.v0);
		pVertexData[page.numVertices + 1].uv = Math::Vector2(sprite.u1, sprite.v0);
		pVertexData[page.numVertices + 2].uv = Math::Vector2(sprite.u1, sprite.v1);
	}
	pVertexData[page.numVertices + 0].colour = colour.packARGB();
	pVertexData[page.numVertices + 1].colour = colour.packARGB();
	pVertexData[page.numVertices + 2].colour = colour.packARGB();
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
	Layer::PageKey key(texture, renderStates, SIMPLE_SPRITE, defaultConstants);
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

Math::Vector2 ImageComposer::putChar(const FontHandlePtr&			_font,
							const uint32_t					_glyph,
							unsigned int					_renderStates,
							const Math::Vector2&			_pos,
							const float						_pt,
							const Core::Colour&				_colour,
							unsigned int					_layerNum)
{
	assert((_layerNum < MAX_LAYERS) && "Invalid layer");

	const auto font = _font->acquire();
	const Font::Glyph& fontGlyph = font->getGlyph(_glyph);
	CORE_ASSERT(fontGlyph.unicode == 0 || fontGlyph.unicode == _glyph);

	TextureAtlasPtr atlas = font->getAtlas()->tryAcquire();
	if (!atlas) { return Math::Vector2(0,0); }

	const float dpiScale = (float)font->getDPI() / (float)targetDPI;
	const Math::Vector2 scrScale(	(2.0f / (targetWidth * dpiScale))* (_pt / 72.0f),
									(2.0f / (targetHeight * dpiScale) * (_pt / 72.0f)));

	// special case ' '
	if (_glyph == ' ') {
		return Math::ComponentMultiply(Math::Vector2((float)fontGlyph.advanceX, (float)-fontGlyph.advanceY), scrScale);
	}

	const Math::Vector2 pos = _pos + Math::ComponentMultiply(Math::Vector2((float)fontGlyph.offsetX, (float)-fontGlyph.offsetY), scrScale);
	const TextureAtlas::SubTexture& sprite = atlas->getSubTexture( fontGlyph.sprite);
	TextureHandlePtr texture = atlas->getPackedTexture(fontGlyph.page);

	// put this sprite on the appropaite layer
	Layer& layer = layers[_layerNum];

	Scene::GPUConstants::ICGPUConstants constants;
	// TODO change this based on pt size wanted smoothly
	if (_pt < 37) {
		constants.fontSharpness.x = 0.7f;
	}
	else if (_pt < 60) {
		constants.fontSharpness.x = 1.0f;
	}
	else if (_pt < 200) {
		constants.fontSharpness.x = 1.6f;
	}
	else {
		constants.fontSharpness.x = 2.0f;
	}

	// assign a sorting page for this sprite
	Layer::PageKey key(texture, _renderStates, DISTANCE_FIELD, constants);
	Layer::Page& page = findOrCreatePage(layer, key);

	assert(((page.numVertices / 6) + 1) < maxSpritesPerLayer);


	const Math::Vector2 size = Math::ComponentMultiply( Math::Vector2(fontGlyph.width,fontGlyph.height), scrScale);

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*)page.mapped.get();

	// fill in the vertex data
	pVertexData[page.numVertices + 0].pos = Math::Vector2(pos.x, -(pos.y + size.y));
	pVertexData[page.numVertices + 1].pos = Math::Vector2(pos.x, -pos.y);
	pVertexData[page.numVertices + 2].pos = Math::Vector2(pos.x + size.x, -(pos.y + size.y));
	pVertexData[page.numVertices + 3].pos = Math::Vector2(pos.x, -pos.y);
	pVertexData[page.numVertices + 4].pos = Math::Vector2(pos.x + size.x, -pos.y);
	pVertexData[page.numVertices + 5].pos = Math::Vector2(pos.x + size.x, -(pos.y + size.y));

	if (sprite.u0 < 0) {
		pVertexData[page.numVertices + 0].uv.x = -sprite.u0;
		pVertexData[page.numVertices + 0].uv.y = sprite.v0;
		pVertexData[page.numVertices + 1].uv.x = sprite.u1;
		pVertexData[page.numVertices + 1].uv.y = sprite.v0;
		pVertexData[page.numVertices + 2].uv.x = -sprite.u0;
		pVertexData[page.numVertices + 2].uv.y = sprite.v1;

		pVertexData[page.numVertices + 3].uv.x = sprite.u1;
		pVertexData[page.numVertices + 3].uv.y = sprite.v0;
		pVertexData[page.numVertices + 4].uv.x = sprite.u1;
		pVertexData[page.numVertices + 4].uv.y = sprite.v1;
		pVertexData[page.numVertices + 5].uv.x = -sprite.u0;
		pVertexData[page.numVertices + 5].uv.y = sprite.v1;
	}
	else {
		pVertexData[page.numVertices + 0].uv.x = sprite.u0;
		pVertexData[page.numVertices + 0].uv.y = sprite.v1;
		pVertexData[page.numVertices + 1].uv.x = sprite.u0;
		pVertexData[page.numVertices + 1].uv.y = sprite.v0;
		pVertexData[page.numVertices + 2].uv.x = sprite.u1;
		pVertexData[page.numVertices + 2].uv.y = sprite.v1;

		pVertexData[page.numVertices + 3].uv.x = sprite.u0;
		pVertexData[page.numVertices + 3].uv.y = sprite.v0;
		pVertexData[page.numVertices + 4].uv.x = sprite.u1;
		pVertexData[page.numVertices + 4].uv.y = sprite.v0;
		pVertexData[page.numVertices + 5].uv.x = sprite.u1;
		pVertexData[page.numVertices + 5].uv.y = sprite.v1;
	}
	const auto packedColour = _colour.packARGB();
	pVertexData[page.numVertices + 0].colour = packedColour;
	pVertexData[page.numVertices + 1].colour = packedColour;
	pVertexData[page.numVertices + 2].colour = packedColour;
	pVertexData[page.numVertices + 3].colour = packedColour;
	pVertexData[page.numVertices + 4].colour = packedColour;
	pVertexData[page.numVertices + 5].colour = packedColour;

	page.numVertices += 6;

	auto ret = Math::ComponentMultiply(Math::Vector2((float)fontGlyph.advanceX, 0), scrScale);
	ret.y = size.y;

	return ret;
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
	Layer::PageKey key( NULL, renderStates, SOLID_COLOUR, defaultConstants );
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
	Layer::PageKey key(pTexture, renderStates, SIMPLE_SPRITE, defaultConstants);
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

// not thread safe, *AT LEAST* due to sharing of ICGPUConstants
void ImageComposer::render( RenderContext* context ) {
	using namespace Scene;

	auto samplerState = linearClampSampler.acquire();

	CORE_ASSERT(YOLK_GPU_GET_OFFSET_IN_BLOCK(ICGPUConstants, fontSharpness) == offsetof(Scene::GPUConstants::ICGPUConstants, fontSharpness));

	for( unsigned int i=0;i < MAX_LAYERS;++i) {

		// each layer can have multiple pages of different GPU programs & state
		for (auto& pm : layers[i].pageMap) {
			const Layer::PageKey& pagekey = pm.first;
			Layer::Page& page = pm.second;
			if( page.numVertices == 0 ) { continue; }

			// TODO threading issue here :(
			// close the CPU vertex buffer
			page.unmap();
			
			// copy composers vertices to GPU
			DataBufferPtr db = page.vertexBufferHandle.acquire();
			if( !db ) { continue; }
			void* gpuVerts = (void*) db->map( context, (RESOURCE_MAP_ACCESS)( RMA_WRITE | RMA_DISCARD ) );
			memcpy( gpuVerts, page.mapped.get(), page.numVertices * SizeOfRenderType[ pagekey.type ] );
			db->unmap( context );

			// setup the GPU programs and vertex access HW
			auto icProgram = program[ pagekey.type ].acquire();
			auto vao = page.vaoHandle.tryAcquire();
			if( !vao ) { continue; }
			vao->validate( icProgram );

			context->bind( vao );

			// if using a texture set it up
			if( pagekey.texture0 ) {
				auto tex = pagekey.texture0->tryAcquire();
				if( tex ) {
					context->bind( ST_FRAGMENT, 0, samplerState );
					context->bind( ST_FRAGMENT, 0, tex );
				}
			}
			//  upload an identity matrix for view proj
			context->getConstantCache().setMatrixBypassCache( CVN_VIEW_PROJ, Math::IdentityMatrix() );
			context->getConstantCache().updateGPU( context, icProgram );

			// TODO make a general flag rather than based on type for when other passes use custom constants
			DataBufferPtr cb = page.constantsHandle.acquire();
			void* buffer = cb->map(context, (RESOURCE_MAP_ACCESS)(RMA_WRITE | RMA_DISCARD));
			memcpy(buffer, &pagekey.gpuConstants, sizeof(Scene::GPUConstants::ICGPUConstants));
			cb->unmap(context);

			context->bindCB(Scene::ST_FRAGMENT, Scene::CF_USER_BLOCK0, cb);

			context->bind( icProgram );

			// setup blender state
			auto blender = blendState[ pagekey.renderStates ].acquire();
			context->bind( blender );
			// now actually draw it
			context->draw( PT_TRIANGLE_LIST, page.numVertices );
			page.numVertices = 0;
		}
	}

}

bool ImageComposer::Layer::PageKey::operator <(const ImageComposer::Layer::PageKey &b) const {
	if( renderStates == b.renderStates ) {
		if( type == type ){
			if (type == DISTANCE_FIELD && texture0 == b.texture0 ) {
				return memcmp(&gpuConstants, &b.gpuConstants, sizeof(Scene::GPUConstants::ICGPUConstants)) < 0;
			} else {
				return texture0 < b.texture0;
			}
		} else {
			return type < b.type;
		}
	} else {
		return renderStates < b.renderStates;
	}
}

}