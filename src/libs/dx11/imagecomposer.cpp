//!-----------------------------------------------------
//!
//! \file imagecomposer.cpp
//! the sprite engine system
//!
//!-----------------------------------------------------

#include "graphics_pch.h"
#include "graphics.h"
#include "core/colour.h"
#include "textureatlas.h"
#include "wobfile.h"
#include "vdeclmanager.h"
#include "rendercontext.h"
#include "core/resourceman.h"
#include "rendermatrixcache.h"
#include "imagecomposer.h"

namespace Graphics
{
ImageComposer::ImageComposer( int iMaxSpritesPerLayer ) :
	m_iMaxSpritesPerLayer( iMaxSpritesPerLayer )
{
	m_context = Gfx::Get()->getMainRenderContext(); // TODO give each composer a proper context
	WobVertexElement SimpleSpriteElements[] = { { WobVertexElement::WVTU_POSITION, WobVertexElement::WVTT_FLOAT2 },
												{ WobVertexElement::WVTU_TEXCOORD0, WobVertexElement::WVTT_FLOAT2 },
												{ WobVertexElement::WVTU_COLOUR0, WobVertexElement::WVTT_BYTEARGB } };
	
	WobVertexElement LerpedSpriteElements[] = { { WobVertexElement::WVTU_POSITION, WobVertexElement::WVTT_FLOAT2 },
												{ WobVertexElement::WVTU_TEXCOORD0, WobVertexElement::WVTT_FLOAT2 },
												{ WobVertexElement::WVTU_TEXCOORD1, WobVertexElement::WVTT_FLOAT2 },
												{ WobVertexElement::WVTU_TEXCOORD2, WobVertexElement::WVTT_FLOAT1 },
												{ WobVertexElement::WVTU_COLOUR0, WobVertexElement::WVTT_BYTEARGB } };

	WobVertexElement SolidColourElements[]	= { { WobVertexElement::WVTU_POSITION, WobVertexElement::WVTT_FLOAT2 },
												{ WobVertexElement::WVTU_COLOUR0, WobVertexElement::WVTT_BYTEARGB } };

	m_Shader[ SIMPLE_SPRITE ] = ShaderFXHandle::Load( "SimpleSprite" );
	m_Shader[ LERPED_SPRITE ] = ShaderFXHandle::Load( "LerpedSprite" );
	m_Shader[ SOLID_COLOUR ] = ShaderFXHandle::Load( "SolidColour2D" );
	m_VDeclHandle[ SIMPLE_SPRITE ] = VDeclManager::Get()->GetVertexDeclarationHandle( 3, SimpleSpriteElements, m_Shader[ SIMPLE_SPRITE ], "" );
	m_VDeclHandle[ LERPED_SPRITE ] = VDeclManager::Get()->GetVertexDeclarationHandle( 5, LerpedSpriteElements, m_Shader[ LERPED_SPRITE ], "" );
	m_VDeclHandle[ SOLID_COLOUR ] = VDeclManager::Get()->GetVertexDeclarationHandle( 2, SolidColourElements, m_Shader[ SOLID_COLOUR ], "" );
}

ImageComposer::~ImageComposer()
{
	Core::ResourceMan::Get()->CloseResource( m_Shader[SOLID_COLOUR] );
	Core::ResourceMan::Get()->CloseResource( m_Shader[LERPED_SPRITE] );
	Core::ResourceMan::Get()->CloseResource( m_Shader[SIMPLE_SPRITE] );
}

void ImageComposer::clearImage()
{
//	Graphics::Gfx::Get()->GetDevice()->Clear(0,0, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0);
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
	Layer& layer = m_Layers[layerNum];

	// assign a sorting page for this sprite
	Layer::PageKey key(pTexture, NULL,renderStates,SIMPLE_SPRITE);
	Layer::PageMap::iterator pmIt = layer.m_PageMap.find( key );
	if( pmIt == layer.m_PageMap.end() ) {
		VBManager::VBInstance vertexBuffer = VBManager::Get()->AllocVertices( m_iMaxSpritesPerLayer * 6 * sizeof(SimpleSpriteVertex), true );

		// insert a new page
		std::pair< Layer::PageMap::iterator, bool> insertPair = 
				layer.m_PageMap.insert(	Layer::PageMap::value_type(key, 
										Layer::Page(vertexBuffer, m_Shader[ SIMPLE_SPRITE ]->Acquire()) 
										) );
		pmIt = insertPair.first;
		assert( insertPair.second );

		Layer::Page& mpage = pmIt->second;
		mpage.m_mapped = mpage.m_VertexBuffer.Lock( m_context, VBManager::VBInstance::DISCARD );
	}

	Layer::Page& page = pmIt->second;

	assert( ((page.m_NumVertices / 6) + 1) < m_iMaxSpritesPerLayer );

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*)page.m_mapped;
	const uint32_t packCol = Core::RGBAColour::packARGB( colour.getRGBAColour() );

	// fill in the vertex data
	pVertexData[ page.m_NumVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y+size.y) );
	pVertexData[ page.m_NumVertices + 0 ].uv = Math::Vector2(0, 1);
	pVertexData[ page.m_NumVertices + 0 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.m_NumVertices + 1 ].uv = Math::Vector2(1, 1);
	pVertexData[ page.m_NumVertices + 1 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 2 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.m_NumVertices + 2 ].uv = Math::Vector2(0, 0);
	pVertexData[ page.m_NumVertices + 2 ].colour = packCol;
	page.m_NumVertices += 3;
	pVertexData[ page.m_NumVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.m_NumVertices + 0 ].uv = Math::Vector2(0, 0);
	pVertexData[ page.m_NumVertices + 0 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.m_NumVertices + 1 ].uv = Math::Vector2(1, 1);
	pVertexData[ page.m_NumVertices + 1 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y-size.y) );
	pVertexData[ page.m_NumVertices + 2 ].uv = Math::Vector2(1, 0);
	pVertexData[ page.m_NumVertices + 2 ].colour = packCol;
	page.m_NumVertices += 3;

}

void ImageComposer::putSprite(	const TextureAtlasPtr&			atlas,
								unsigned int					texIndex,
								unsigned int					renderStates,
								const Math::Vector2&			pos,
								const Math::Vector2&			fullsize,
								const Core::Colour&				colour,
								unsigned int					layerNum )
{
	assert( (layerNum < MAX_LAYERS) && "Invalid layer" );

	const TextureAtlas::SubTexture& sprite = atlas->GetSubTexture( texIndex );
	TextureHandlePtr texture = atlas->GetPackedTexture( sprite.m_Index );

	// put this sprite on the appropaite layer
	Layer& layer = m_Layers[layerNum];

	// assign a sorting page for this sprite
	Layer::PageKey key(texture, NULL,renderStates,SIMPLE_SPRITE);
	Layer::PageMap::iterator pmIt = layer.m_PageMap.find( key );
	if( pmIt == layer.m_PageMap.end() ) {
		VBManager::VBInstance vertexBuffer = VBManager::Get()->AllocVertices( m_iMaxSpritesPerLayer * 6 * sizeof(SimpleSpriteVertex), true );

		// insert a new page
		std::pair< Layer::PageMap::iterator, bool> insertPair = 
				layer.m_PageMap.insert(	Layer::PageMap::value_type(key, 
										Layer::Page(vertexBuffer, m_Shader[ SIMPLE_SPRITE ]->Acquire()) 
										) );
		pmIt = insertPair.first;
		assert( insertPair.second );

		Layer::Page& mpage = pmIt->second;
		mpage.m_mapped = mpage.m_VertexBuffer.Lock( m_context, VBManager::VBInstance::DISCARD );
	}

	Layer::Page& page = pmIt->second;
	assert( ((page.m_NumVertices / 6) + 1) < m_iMaxSpritesPerLayer );

	Math::Vector2			size = fullsize;

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*) page.m_mapped;
	const uint32_t packCol = Core::RGBAColour::packARGB( colour.getRGBAColour() );

	// fill in the vertex data
	pVertexData[ page.m_NumVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y+size.y) );
	pVertexData[ page.m_NumVertices + 0 ].uv = Math::Vector2(sprite.m_u0, sprite.m_v1);
	pVertexData[ page.m_NumVertices + 0 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.m_NumVertices + 1 ].uv = Math::Vector2(sprite.m_u1, sprite.m_v1);
	pVertexData[ page.m_NumVertices + 1 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 2 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.m_NumVertices + 2 ].uv = Math::Vector2(sprite.m_u0, sprite.m_v0);
	pVertexData[ page.m_NumVertices + 2 ].colour = packCol;
	page.m_NumVertices += 3;
	pVertexData[ page.m_NumVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.m_NumVertices + 0 ].uv = Math::Vector2(sprite.m_u0, sprite.m_v0);
	pVertexData[ page.m_NumVertices + 0 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.m_NumVertices + 1 ].uv = Math::Vector2(sprite.m_u1, sprite.m_v1);
	pVertexData[ page.m_NumVertices + 1 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y-size.y) );
	pVertexData[ page.m_NumVertices + 2 ].uv = Math::Vector2(sprite.m_u1, sprite.m_v0);
	pVertexData[ page.m_NumVertices + 2 ].colour = packCol;
	page.m_NumVertices += 3;

}

void ImageComposer::putSubSprite(	const TextureAtlasPtr&			atlas,
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

	const TextureAtlas::SubTexture& sprite = atlas->GetSubTexture( texIndex );
	TextureHandlePtr texture = atlas->GetPackedTexture( sprite.m_Index );

	// put this sprite on the appropaite layer
	Layer& layer = m_Layers[layerNum];

	// assign a sorting page for this sprite
	Layer::PageKey key(texture, NULL,renderStates,SIMPLE_SPRITE);
	Layer::PageMap::iterator pmIt = layer.m_PageMap.find( key );
	if( pmIt == layer.m_PageMap.end() ) {
		VBManager::VBInstance vertexBuffer = VBManager::Get()->AllocVertices( m_iMaxSpritesPerLayer * 6 * sizeof(SimpleSpriteVertex), true );

		// insert a new page
		std::pair< Layer::PageMap::iterator, bool> insertPair = 
				layer.m_PageMap.insert(	Layer::PageMap::value_type(key, 
										Layer::Page(vertexBuffer, m_Shader[ SIMPLE_SPRITE ]->Acquire()) 
										) );
		pmIt = insertPair.first;
		assert( insertPair.second );

		Layer::Page& mpage = pmIt->second;
		mpage.m_mapped = mpage.m_VertexBuffer.Lock( m_context, VBManager::VBInstance::DISCARD );
	}

	Layer::Page& page = pmIt->second;
	assert( ((page.m_NumVertices / 6) + 1) < m_iMaxSpritesPerLayer );

	Math::Vector2			size = fullsize;

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*) page.m_mapped;
	float uSize = sprite.m_u1-sprite.m_u0;
	float vSize = sprite.m_v1-sprite.m_v0;

	float u0 = sprite.m_u0+subTexTL.x*uSize;
	float v0 = sprite.m_v0+subTexTL.y*vSize;
	float u1 = sprite.m_u0+subTexBR.x*uSize;
	float v1 = sprite.m_v0+subTexBR.y*vSize;

	const uint32_t packCol = Core::RGBAColour::packARGB( colour.getRGBAColour() );

	// fill in the vertex data
	pVertexData[ page.m_NumVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y+size.y) );
	pVertexData[ page.m_NumVertices + 0 ].uv = Math::Vector2(u0, v1);
	pVertexData[ page.m_NumVertices + 0 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.m_NumVertices + 1 ].uv = Math::Vector2(u1, v1);
	pVertexData[ page.m_NumVertices + 1 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 2 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.m_NumVertices + 2 ].uv = Math::Vector2(u0, v0);
	pVertexData[ page.m_NumVertices + 2 ].colour = packCol;
	page.m_NumVertices += 3;
	pVertexData[ page.m_NumVertices + 0 ].pos = Math::Vector2( pos.x-size.x, -(pos.y-size.y) );
	pVertexData[ page.m_NumVertices + 0 ].uv = Math::Vector2(u0, v0);
	pVertexData[ page.m_NumVertices + 0 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 1 ].pos = Math::Vector2( pos.x+size.x, -(pos.y+size.y) );
	pVertexData[ page.m_NumVertices + 1 ].uv = Math::Vector2(u1, v1);
	pVertexData[ page.m_NumVertices + 1 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 2 ].pos = Math::Vector2( pos.x+size.x, -(pos.y-size.y) );
	pVertexData[ page.m_NumVertices + 2 ].uv = Math::Vector2(u1, v0);
	pVertexData[ page.m_NumVertices + 2 ].colour = packCol;
	page.m_NumVertices += 3;

}

void ImageComposer::putSprite(	const TextureAtlasPtr&			atlas,
								unsigned int					texIndex0,
								unsigned int					texIndex1,
								float							timeFactor,
								unsigned int					renderStates,
								const Math::Vector2&			pos,
								const Math::Vector2&			fullsize,
								const Core::Colour&				colour,
								unsigned int					layerNum )
{
	assert( (layerNum < MAX_LAYERS) && "Invalid layer" );

	const TextureAtlas::SubTexture& sprite0 = atlas->GetSubTexture( texIndex0 );
	TextureHandlePtr texture0 = atlas->GetPackedTexture( sprite0.m_Index );
	const TextureAtlas::SubTexture& sprite1 = atlas->GetSubTexture( texIndex1 );
	TextureHandlePtr texture1 = atlas->GetPackedTexture( sprite1.m_Index );

	// put this sprite on the appropaite layer
	Layer& layer = m_Layers[layerNum];

	// assign a sorting page for this sprite
	Layer::PageKey key(texture0, texture1,renderStates,LERPED_SPRITE);
	Layer::PageMap::iterator pmIt = layer.m_PageMap.find( key );
	if( pmIt == layer.m_PageMap.end() ) {
		VBManager::VBInstance vertexBuffer = VBManager::Get()->AllocVertices( m_iMaxSpritesPerLayer * 6 * sizeof(SimpleSpriteVertex), true );

		// insert a new page
		std::pair< Layer::PageMap::iterator, bool> insertPair = 
				layer.m_PageMap.insert(	Layer::PageMap::value_type(key, 
										Layer::Page(vertexBuffer, m_Shader[ SIMPLE_SPRITE ]->Acquire()) 
										) );
		pmIt = insertPair.first;
		assert( insertPair.second );

		Layer::Page& mpage = pmIt->second;
		mpage.m_mapped = mpage.m_VertexBuffer.Lock( m_context, VBManager::VBInstance::DISCARD );
	}

	Math::Vector2			size = fullsize;

	Layer::Page& page = pmIt->second;
	assert( ((page.m_NumVertices / 6) + 1) < m_iMaxSpritesPerLayer );


	// fill in the vertex data
	LerpedSpriteVertex* pVertexData = (LerpedSpriteVertex*) page.m_mapped;
	const uint32_t packCol = Core::RGBAColour::packARGB( colour.getRGBAColour() );

	pVertexData[ page.m_NumVertices + 0 ].pos = pos;
	pVertexData[ page.m_NumVertices + 0 ].uv0 = Math::Vector2(sprite0.m_u0, sprite0.m_v0);
	pVertexData[ page.m_NumVertices + 0 ].uv1 = Math::Vector2(sprite1.m_u0, sprite1.m_v0);
	pVertexData[ page.m_NumVertices + 0 ].time = timeFactor;
	pVertexData[ page.m_NumVertices + 0 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 1 ].pos = pos + Math::Vector2( size.x, 0);
	pVertexData[ page.m_NumVertices + 1 ].uv0 = Math::Vector2(sprite0.m_u1, sprite0.m_v0);
	pVertexData[ page.m_NumVertices + 1 ].uv1 = Math::Vector2(sprite1.m_u1, sprite1.m_v0);
	pVertexData[ page.m_NumVertices + 1 ].time = timeFactor;
	pVertexData[ page.m_NumVertices + 1 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 2 ].pos = pos + Math::Vector2( 0, -size.y );
	pVertexData[ page.m_NumVertices + 2 ].uv0 = Math::Vector2(sprite0.m_u0, sprite0.m_v1);
	pVertexData[ page.m_NumVertices + 2 ].uv1 = Math::Vector2(sprite1.m_u0, sprite1.m_v1);
	pVertexData[ page.m_NumVertices + 2 ].time = timeFactor;
	pVertexData[ page.m_NumVertices + 2 ].colour = packCol;
	page.m_NumVertices += 3;
	pVertexData[ page.m_NumVertices + 0 ].pos = pos + Math::Vector2( 0, -size.y );
	pVertexData[ page.m_NumVertices + 0 ].uv0 = Math::Vector2(sprite0.m_u0, sprite0.m_v1);
	pVertexData[ page.m_NumVertices + 0 ].uv1 = Math::Vector2(sprite1.m_u0, sprite1.m_v1);
	pVertexData[ page.m_NumVertices + 0 ].time = timeFactor;
	pVertexData[ page.m_NumVertices + 0 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 1 ].pos = pos + Math::Vector2( size.x, 0);
	pVertexData[ page.m_NumVertices + 1 ].uv0 = Math::Vector2(sprite0.m_u1, sprite0.m_v0);
	pVertexData[ page.m_NumVertices + 1 ].uv1 = Math::Vector2(sprite1.m_u1, sprite1.m_v0);
	pVertexData[ page.m_NumVertices + 1 ].time = timeFactor;
	pVertexData[ page.m_NumVertices + 1 ].colour = packCol;
	pVertexData[ page.m_NumVertices + 2 ].pos = pos + Math::Vector2( size.x, -size.y );
	pVertexData[ page.m_NumVertices + 2 ].uv0 = Math::Vector2(sprite0.m_u1, sprite0.m_v1);
	pVertexData[ page.m_NumVertices + 2 ].uv1 = Math::Vector2(sprite1.m_u1, sprite1.m_v1);
	pVertexData[ page.m_NumVertices + 2 ].time = timeFactor;
	pVertexData[ page.m_NumVertices + 2 ].colour = packCol;
	page.m_NumVertices += 3;

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
	Layer& layer = m_Layers[layerNum];

	// assign a sorting page for this quad
	Layer::PageKey key(NULL, NULL,renderStates,SOLID_COLOUR);
	Layer::PageMap::iterator pmIt = layer.m_PageMap.find( key );
	if( pmIt == layer.m_PageMap.end() ) {
		VBManager::VBInstance vertexBuffer = VBManager::Get()->AllocVertices( m_iMaxSpritesPerLayer * 6 * sizeof(SimpleSpriteVertex), true );

		// insert a new page
		std::pair< Layer::PageMap::iterator, bool> insertPair = 
				layer.m_PageMap.insert(	Layer::PageMap::value_type(key, 
										Layer::Page(vertexBuffer, m_Shader[ SIMPLE_SPRITE ]->Acquire()) 
										) );
		pmIt = insertPair.first;
		assert( insertPair.second );

		Layer::Page& mpage = pmIt->second;
		mpage.m_mapped = mpage.m_VertexBuffer.Lock( m_context, VBManager::VBInstance::DISCARD );
	}

	Layer::Page& page = pmIt->second;

	assert( ((page.m_NumVertices / 6) + 1) < m_iMaxSpritesPerLayer );

	SolidColourVertex* pVertexData = (SolidColourVertex*) page.m_mapped;

	// fill in the vertex data
	pVertexData[ page.m_NumVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.m_NumVertices + 0 ].colour = Core::RGBAColour::packARGB( tlcol.getRGBAColour() );
	pVertexData[ page.m_NumVertices + 1 ].pos = Math::Vector2(tr.x,-tr.y);
	pVertexData[ page.m_NumVertices + 1 ].colour = Core::RGBAColour::packARGB( trcol.getRGBAColour() );
	pVertexData[ page.m_NumVertices + 2 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.m_NumVertices + 2 ].colour = Core::RGBAColour::packARGB( brcol.getRGBAColour() );
	page.m_NumVertices += 3;
	pVertexData[ page.m_NumVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.m_NumVertices + 0 ].colour = Core::RGBAColour::packARGB( tlcol.getRGBAColour() );
	pVertexData[ page.m_NumVertices + 1 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.m_NumVertices + 1 ].colour = Core::RGBAColour::packARGB( brcol.getRGBAColour() );
	pVertexData[ page.m_NumVertices + 2 ].pos = Math::Vector2(bl.x,-bl.y);
	pVertexData[ page.m_NumVertices + 2 ].colour = Core::RGBAColour::packARGB( blcol.getRGBAColour() );
	page.m_NumVertices += 3;

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
	Layer& layer = m_Layers[layerNum];

	// assign a sorting page for this quad
	Layer::PageKey key(pTexture, NULL,renderStates,SIMPLE_SPRITE );
	Layer::PageMap::iterator pmIt = layer.m_PageMap.find( key );
	if( pmIt == layer.m_PageMap.end() ) {
		VBManager::VBInstance vertexBuffer = VBManager::Get()->AllocVertices( m_iMaxSpritesPerLayer * 6 * sizeof(SimpleSpriteVertex), true );

		// insert a new page
		std::pair< Layer::PageMap::iterator, bool> insertPair = 
				layer.m_PageMap.insert(	Layer::PageMap::value_type(key, 
										Layer::Page(vertexBuffer, m_Shader[ SIMPLE_SPRITE ]->Acquire()) 
										) );
		pmIt = insertPair.first;
		assert( insertPair.second );

		Layer::Page& mpage = pmIt->second;
		mpage.m_mapped = mpage.m_VertexBuffer.Lock( m_context, VBManager::VBInstance::DISCARD );
	}

	Layer::Page& page = pmIt->second;

	assert( ((page.m_NumVertices / 6) + 1) < m_iMaxSpritesPerLayer );

	SimpleSpriteVertex* pVertexData = (SimpleSpriteVertex*) page.m_mapped;

	// fill in the vertex data
	pVertexData[ page.m_NumVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.m_NumVertices + 0 ].uv = Math::Vector2(uvtl.x,1.0f-uvtl.y);
	pVertexData[ page.m_NumVertices + 0 ].colour = Core::RGBAColour::packARGB( tlcol.getRGBAColour() );
	pVertexData[ page.m_NumVertices + 1 ].pos = Math::Vector2(tr.x,-tr.y);
	pVertexData[ page.m_NumVertices + 1 ].uv = Math::Vector2( uvbr.x, 1.0f-uvtl.y );
	pVertexData[ page.m_NumVertices + 1 ].colour = Core::RGBAColour::packARGB( trcol.getRGBAColour() );
	pVertexData[ page.m_NumVertices + 2 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.m_NumVertices + 2 ].uv = Math::Vector2(uvbr.x,1.0f-uvbr.y);
	pVertexData[ page.m_NumVertices + 2 ].colour = Core::RGBAColour::packARGB( brcol.getRGBAColour() );
	page.m_NumVertices += 3;
	pVertexData[ page.m_NumVertices + 0 ].pos = Math::Vector2(tl.x,-tl.y);
	pVertexData[ page.m_NumVertices + 0 ].uv = Math::Vector2(uvtl.x,1.0f-uvtl.y);
	pVertexData[ page.m_NumVertices + 0 ].colour = Core::RGBAColour::packARGB( tlcol.getRGBAColour() );
	pVertexData[ page.m_NumVertices + 1 ].pos = Math::Vector2(br.x,-br.y);
	pVertexData[ page.m_NumVertices + 1 ].uv = Math::Vector2(uvbr.x,1.0f-uvbr.y);
	pVertexData[ page.m_NumVertices + 1 ].colour = Core::RGBAColour::packARGB( brcol.getRGBAColour() );
	pVertexData[ page.m_NumVertices + 2 ].pos = Math::Vector2(bl.x,-bl.y);
	pVertexData[ page.m_NumVertices + 2 ].uv = Math::Vector2(uvtl.x,1.0f-uvbr.y);
	pVertexData[ page.m_NumVertices + 2 ].colour = Core::RGBAColour::packARGB( blcol.getRGBAColour() );
	page.m_NumVertices += 3;

}


void ImageComposer::Render() {

	for( unsigned int i=0;i < MAX_LAYERS;++i) {
		Layer::PageMap::iterator pmIt = m_Layers[i].m_PageMap.begin();
		while( pmIt != m_Layers[i].m_PageMap.end() ) {
			const Layer::PageKey& pagekey = pmIt->first;
			Layer::Page& page = pmIt->second;
			if( page.m_mapped != NULL ) {
				page.m_VertexBuffer.Unlock(m_context);
				page.m_mapped = NULL;
			}
			ID3DX11EffectTechnique*	technique = page.m_pShader->m_pEffectTechnique;

			if( pagekey.m_pTexture0 ) {
				page.m_pShader->setVariable( "g_Texture0", pagekey.m_pTexture0 );
			}
			if( pagekey.m_pTexture1 ) {
				page.m_pShader->setVariable( "g_Texture1", pagekey.m_pTexture1 );
			}
	
			D3DX11_TECHNIQUE_DESC techDesc;
			technique->GetDesc( &techDesc );

			for( uint32_t j = 0;j < techDesc.Passes;++j) {
	            technique->GetPassByIndex( j )->Apply( 0,  Gfx::Get()->GetMainContext() );

				switch( pagekey.m_renderStates ) {
/*				case NORMAL:
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
					break;
				case COLOUR_MASK:
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
				break;
				case ALPHA_MASK:
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA );
				break;
				case ALPHA_TEST:
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHAREF, 127 );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
					break;
				case ALPHA_BLEND:
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHAREF, 1 );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
					break;
				case PM_OVER:
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHAREF, 1 );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
					Gfx::Get()->GetDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
					break;*/
				}
				VDeclManager::Get()->SetVertexDecleration( m_context, m_VDeclHandle[ pagekey.m_eType ] );
				page.m_VertexBuffer.Bind( m_context, 0, VDeclManager::Get()->GetVertexSize(m_VDeclHandle[ pagekey.m_eType ]) );
				m_context->deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
				m_context->deviceContext->Draw( page.m_NumVertices,0 );
			}

			page.m_VertexBuffer.Unbind( m_context, 0 );
			VBManager::Get()->FreeVertices( page.m_VertexBuffer );

			++pmIt;
		}
		m_Layers[i].m_PageMap.clear();
	}

//	Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
//	Gfx::Get()->GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
}

bool ImageComposer::Layer::PageKey::operator <(const Graphics::ImageComposer::Layer::PageKey &b) const
{
	if( m_renderStates == b.m_renderStates )
	{
		if( m_eType == m_eType )
		{
			if( m_pTexture0 == b.m_pTexture0 )
			{
				return m_pTexture1 < m_pTexture1;
			} else
			{
				return m_pTexture0 < m_pTexture0;
			}
		} else
		{
			return m_eType < b.m_eType;
		}
	} else 
	{
		return m_renderStates < b.m_renderStates;
	}
}
}