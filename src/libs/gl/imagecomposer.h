//!-----------------------------------------------------
//!
//! \file imagecomposer.h
//!
//!-----------------------------------------------------
#if !defined(YOLK_GL_IMAGECOMPOSER_H_)
#define YOLK_GL_IMAGECOMPOSER_H_
#pragma once

#if !defined( YOLK_GL_TEXTUREATLAS_H_ )
#	include "textureatlas.h"
#endif
#if !defined( YOLK_CORE_COLOUR_H_ )
#	include "core/colour.h"
#endif
#if !defined( YOLK_GL_DATABUFFER_H_ )
#	include "databuffer.h"
#endif
#if !defined( YOLK_GL_VAO_H_ )
#	include "vao.h"
#endif
#if !defined( YOLK_GL_PROGRAMPIPELINEOBJECT_H_ )
#	include "programpipelineobject.h"
#endif

namespace Gl {
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
//!
//! Layer 0 = far layer (furthest from camera), 
//!       MAX_LAYERS-1 = near layer
//!
//!-----------------------------------------------------
class ImageComposer {
public:
	static const int MAX_LAYERS = 16;

	// note this are rendered in this order. i.e. NORMAL first, then alpha test then blend etc.
	enum RENDER_STATES
	{
		NORMAL = 0,			//!< res.argb = tex.argb * iterator.argb
		COLOUR_MASK,		//!< res.argb = tex.rgb * iterator.rgb
		ALPHA_MASK,			//!< res.a = tex.a * iterator.a
		ALPHA_BLEND,		//!< res.argb = OVER( tex.argb * iterator.argb, res.argb, (tex.a * iterator.a) )
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
	//! pos is at the center of where you wish to place it
	//! size is equal either side of pos (i.e. total width == *2)
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
	void putSprite(	const TextureAtlasHandlePtr&	atlas,
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
	void putSubSprite(	const TextureAtlasHandlePtr&	atlas,
						unsigned int					texIndex,
						const Math::Vector2&			subTexTL,
						const Math::Vector2&			subTexBR,
						unsigned int					renderStates,
						const Math::Vector2&			pos,
						const Math::Vector2&			size,
						const Core::Colour&				colour,
						unsigned int					layer );

	//! Cause all the image operations to be submitted
	void render();

private:
	//! used internally to select the correct rendering settings (vertex decl etc.)
	enum RENDER_TYPE {
		SIMPLE_SPRITE,			//!< simple coloured single image sprites
		SOLID_COLOUR,			//!< a solid colour polygon
		MAX_RENDER_TYPE
	};

	struct SimpleSpriteVertex {
		Math::Vector2	pos;
		Math::Vector2	uv;
		uint32_t		colour;
	};						//!< format of a simple sprite

	struct SolidColourVertex {
		Math::Vector2	pos;
		uint32_t		colour;
	};						//!< format of a solid colour poly

	struct Layer {
		//! a sort page, there are 0 or 1 textures, several different rendering state and different types (shaders)
		//! this gives us unique pages only where nessecary. Sprites on the same page are rendered in 1 batch
		struct PageKey {
			PageKey( TextureHandlePtr a, uint32_t b, RENDER_TYPE c ) : 
				texture0(a), renderStates(b), type(c) {}

			TextureHandlePtr	texture0;
			uint32_t			renderStates;
			RENDER_TYPE			type;

			// sorts the page keys for the map lookup
			bool operator<( const PageKey& b ) const;
		};

		struct Page {
			Page() : vertexBufferHandle(nullptr), vertexBuffer( nullptr ),
						numVertices(0), mapped(nullptr) {}

			Page( Scene::DataBufferHandlePtr a, VaoHandlePtr b, Scene::ProgramHandlePtr c ) :
				vertexBufferHandle(a), vaoHandle(b), programHandle(c), 
				vertexBuffer( nullptr ), numVertices(0), 
				mapped(nullptr) {}

			void map() {
				if( vertexBuffer == nullptr ) {
					vertexBuffer = vertexBufferHandle->acquire();
				}
				mapped = (void*) vertexBuffer->map( Scene::DBMA_WRITE_ONLY, Scene::DBMF_DISCARD );
				numVertices = 0;
			}

			void unmap() {
				CORE_ASSERT( mapped != nullptr );
				CORE_ASSERT( vertexBuffer != nullptr );
				vertexBuffer->unmap();
				mapped = nullptr;
			}

			Scene::DataBufferHandlePtr		vertexBufferHandle;
			VaoHandlePtr 					vaoHandle;
			Scene::ProgramHandlePtr			programHandle;

			Scene::DataBufferPtr 			vertexBuffer;
			uint32_t				numVertices;
			void*					mapped;
		};

		typedef std::map<PageKey, Page>				PageMap;
		PageMap										pageMap;
		int 										layerNum;
	} layers[MAX_LAYERS];

	Scene::ProgramHandlePtr	program[MAX_RENDER_TYPE];			//!< programs

	const unsigned int maxSpritesPerLayer;				//! how many sprites per layer can this composer support
	static const uint32_t SizeOfRenderType[MAX_RENDER_TYPE];
	static const Vao::CreationStruct VaoCS[MAX_RENDER_TYPE];
	Layer::Page& findOrCreatePage( Layer& layer, Layer::PageKey& key );


};

}

#endif // end YOLK_GL_IMAGECOMPOSER_H_