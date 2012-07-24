#pragma once
//!-----------------------------------------------------
//!
//! \file heightfield.h
//! the engine graphics heightfield class
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_HEIGHTFIELD_H)
#define YOLK_SCENE_HEIGHTFIELD_H

#if !defined( YOLK_SCENE_RENDERABLE_H_ )
#include "renderable.h"
#endif

#if !defined( YOLK_SCENE_WOBFILE_H )
#include "wobfile.h"
#endif

namespace Scene {
	// forward decl
	class RenderContext;

	class Heightfield : public Renderable {
	public:
		//! Default ctor creates an empty Heightfield
		Heightfield();
		//! loads the Heightfield provided
		Heightfield( const char* pFilename );
		
		//! dtor
		~Heightfield();

		//! inherited from Renderable
		virtual void render( RenderContext* context, const int pipelineIndex );
		
		virtual void debugDraw( RenderContext* context ) const;

		virtual uint32_t getActualRenderablesOfType( R_TYPE _type, uint32_t arraySize, const Renderable** outArray ) const {
			if( _type == R_HEIGHTFIELD && arraySize > 0) {
				outArray[0] = this;
				return 1;
			} else {
				return 0;
			}
		}

		virtual uint32_t getActualRenderablesOfType( R_TYPE _type, uint32_t arraySize, Renderable** outArray ) {
			if( _type == R_HEIGHTFIELD && arraySize > 0) {
				outArray[0] = this;
				return 1;
			} else {
				return 0;
			}
		}

//		TextureHandlePtr getResource() const {
//			return textureHandle->acquire();
//		}

	protected:
//		TextureHandlePtr			textureHandle;
		Math::Matrix4x4				transformMatrix;
		Core::TransformNode			simpleTransformNode;
		Math::Matrix4x4				prevWVP;
	};
}; // end namespace Graphics

#endif