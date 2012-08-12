//!-----------------------------------------------------
//!
//! \file imagecomposer.h
//!
//!-----------------------------------------------------
#if !defined(WIERD_GRAPHICS_IMAGECOMPOSER_H)
#define WIERD_GRAPHICS_IMAGECOMPOSER_H

#if PLATFORM == WINDOWS
#	pragma once
#endif

#if !defined( WIERD_GRAPHICS_VBMANAGER_H )
#	include "vbmanager.h"
#endif

#if !defined( WIERD_GRAPHICS_SHADER_H )
#	include "shader.h"
#endif

#if !defined( WIERD_GRAPHICS_TEXTUREATLAS_H )
#	include "textureatlas.h"
#endif

#if !defined( WIERD_CORE_COLOUR_H )
#	include "core/colour.h"
#endif


#include <map>

namespace Graphics
{
//!-----------------------------------------------------
//!
//! An Image composer is the renderer for 2D graphics
//! All coord and sizes are in NDC*float2(1,-1)and are 
//! rendered onto the current render target. 
//! Assumes full post pixel blend ops are available (lots of alpha...)
//! 
//! I flip the y so we match cairo. Basically 0,1 is the
//! nearer the bottom of your monitor
//! Our space 
//!   -1               0                1
//!-1 -----------------|----------------| -1
//!   |                |                |
//!   |                |                |
//!   |                |                |
//!   |                |                |
//! 0 ----------------------------------| 0
//!   |                |                |
//!   |                |                |
//!   |                |                |
//!   |                |                |
//! 1 ----------------------------------| 1
//!   -1               0                1
//!
//! NOTE: Based on the singleton sprite engine.. in future
//! the sprite engine might just become a special case of
//! the image composer system.
//!
//! Layer 0 = far layer (furthest from camera), 
//!       MAX_LAYERS-1 = near layer
//!
//!-----------------------------------------------------
class ImageComposer
{
public:
	static const int MAX_LAYERS = 16;

	// note this are rendered in this order. i.e. NORMAL first, then alpha test then blend etc.
	enum RENDER_STATES
	{
		NORMAL = 0,			//!< res.argb = tex.argb * iterator.argb
		COLOUR_MASK,		//!< res.argb = tex.rgb * iterator.rgb
		ALPHA_MASK,			//!< res.a = tex.a * iterator.a
		ALPHA_TEST,			//!< res.rgba = (tex.a * iterator.a) < ALPHA_REF ? tex.argb * iterator.argb : res.rgba
		ALPHA_BLEND,		//!< res.rgba = OVER( tex.argb * iterator.argb, res.argb, (tex.a * iterator.a) )
		PM_OVER,			//!< Porter/Duff Over with pre-multiplied alpha
	};

	ImageComposer( int iMaxSpritesPerLayer = 64 );
	~ImageComposer();

	//! clear the entire thing is faster than filling, so its provided here
	void clearImage();

	//!-----------------------------------------------------
	//! Places a solid colour rectangle on to the screen at a 
	//! given NDC position and NDC size the sprite can also be 
	//! coloured.
	//!-----------------------------------------------------
	void filledRect(			unsigned int					renderStates,
								const Math::Vector2&			pos,
								const Math::Vector2&			fullsize,
								const Core::Colour&				colour,
								unsigned int					layerNum );
	//!-----------------------------------------------------
	//! Places a solid colour quad on to the screen at a 
	//! given NDC positions.
	//!-----------------------------------------------------
	void filledQuad(			unsigned int					renderStates,
								const Math::Vector2&			tl,
								const Math::Vector2&			tr,
								const Math::Vector2&			br,
								const Math::Vector2&			bl,
								const Core::Colour&				tlcol,
								const Core::Colour&				trcol,
								const Core::Colour&				brcol,
								const Core::Colour&				blcol,
								unsigned int					layerNum );
	//!-----------------------------------------------------
	//! Places a textured colour quad on to the screen at a 
	//! given NDC positions.
	//!-----------------------------------------------------
	void texturedQuad(			const TextureHandlePtr&			pTexture,
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
								unsigned int					layerNum );
	//!-----------------------------------------------------
	//! Places a textured colour quad on to the screen at a 
	//! given NDC position and NDC size the sprite can also be 
	//! coloured.
	//!-----------------------------------------------------
	void texturedRect(			const TextureHandlePtr&			pTexture,
								unsigned int					renderStates,
								const Math::Vector2&			pos,
								const Math::Vector2&			fullsize,
								const Core::Colour&				colour,
								const Math::Vector2&			uvtl,
								const Math::Vector2&			uvbr,
								unsigned int					layerNum );

	//!-----------------------------------------------------
	//! Places a whole texture on to the screen at a given NDC 
	//! position and NDC size the texture can also be coloured.
	//!-----------------------------------------------------
	void putTexture(	const TextureHandlePtr&			pTexture,
						unsigned int					renderStates,
						const Math::Vector2&			pos,
						const Math::Vector2&			size,
						const Core::Colour&				colour,
						unsigned int					layer );


	//!-----------------------------------------------------
	//! Places a sprite image on to the screen at a given NDC 
	//! position and NDC size the sprite can also be coloured.
	//!-----------------------------------------------------
	void putSprite(	const TextureAtlasPtr&			atlas,
					unsigned int					texIndex,
					unsigned int					renderStates,
					const Math::Vector2&			pos,
					const Math::Vector2&			size,
					const Core::Colour&				colour,
					unsigned int					layer );

	//!-----------------------------------------------------
	//! Places part of a sprite image on to the screen at a given NDC 
	//! position and NDC size the sprite can also be coloured.
	//!-----------------------------------------------------
	void putSubSprite(	const TextureAtlasPtr&			atlas,
					unsigned int					texIndex,
					const Math::Vector2&			subTexTL,
					const Math::Vector2&			subTexBR,
					unsigned int					renderStates,
					const Math::Vector2&			pos,
					const Math::Vector2&			size,
					const Core::Colour&				colour,
					unsigned int					layer );
	//!-----------------------------------------------------
	//! Same as PutSprite, except it takes two sprite images
	//! and blends between them in the pixel shader using
	//! the provided time factor. Used for animated sprites
	//!-----------------------------------------------------
	void putSprite(	const TextureAtlasPtr&			atlas,
					unsigned int					texIndex0,
					unsigned int					texIndex1,
					float							timeFactor,
					unsigned int					renderStates,
					const Math::Vector2&			pos,
					const Math::Vector2&			size,
					const Core::Colour&				colour,
					unsigned int					layer );

	//! Cause all the image operations to be submitted
	void Render();

private:

	//! used internally to select the correct rendering settings (vertex decl etc.)
	enum RENDER_TYPE
	{
		SIMPLE_SPRITE,			//!< simple coloured single image sprites
		LERPED_SPRITE,			//!< coloured sprites lerped between 2 images
		SOLID_COLOUR,			//!< a solid colour polygon
		MAX_RENDER_TYPE
	};

	struct SimpleSpriteVertex
	{
		Math::Vector2	pos;
		Math::Vector2	uv;
		uint32_t		colour;
	};						//!< format of a simple sprite

	struct LerpedSpriteVertex
	{
		Math::Vector2	pos;
		Math::Vector2	uv0;			//!< uv0 
		Math::Vector2	uv1;			//!< uv1
		float			time;			//!< time factor blend
		uint32_t		colour;
	};						//!< format of a lerped sprite

	struct SolidColourVertex
	{
		Math::Vector2	pos;
		uint32_t		colour;
	};						//!< format of a solid colour poly

	struct Layer
	{
		//! a sort page, there are 1 or 2 textures, several different rendering state and different types (shaders)
		//! this gives us unique pages only where nessecary. Sprites on the same page are rendered in 1 batch
		struct PageKey
		{
			PageKey( TextureHandlePtr a, TextureHandlePtr b, uint32_t c, RENDER_TYPE d) : m_pTexture0(a), m_pTexture1(b), m_renderStates(c), m_eType(d) {}

			TextureHandlePtr	m_pTexture0;
			TextureHandlePtr			m_pTexture1;
			uint32_t			m_renderStates;
			RENDER_TYPE			m_eType;

			// sorts the page keys for the map lookup
			bool operator<( const PageKey& b ) const;
		};

		struct Page
		{
			Page() : m_NumVertices(0), m_mapped(0) {}
			Page( VBManager::VBInstance a, ShaderFXPtr b) :
				m_VertexBuffer(a), m_NumVertices(0), m_pShader(b) {}

			VBManager::VBInstance	m_VertexBuffer;
			uint32_t				m_NumVertices;
			ShaderFXPtr				m_pShader;
			void*					m_mapped;
		};

		typedef std::map<PageKey, Page>				PageMap;
		PageMap										m_PageMap;
	} m_Layers[MAX_LAYERS];

	uint32_t m_VDeclHandle[MAX_RENDER_TYPE];				//!< Vertex declerations for the sprite types
	ShaderFXHandlePtr	m_Shader[MAX_RENDER_TYPE];			//!< Shader resource for the sprite types
	RenderContext*		m_context;

	const unsigned int m_iMaxSpritesPerLayer;				//! how many sprites per layer can this composer support
};

}

#endif // end WIERD_GRAPHICS_SPRITEENGINE_H