#pragma once
//!-----------------------------------------------------
//!
//! \file mesh.h
//! the engine graphics mesh class
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_MESH_H)
#define YOLK_SCENE_MESH_H

#if !defined( YOLK_SCENE_RENDERABLE_H_ )
#include "renderable.h"
#endif

#if !defined( YOLK_SCENE_WOBFILE_H )
#include "wobfile.h"
#endif

namespace Scene {
	// forward decl
	class RenderContext;

	class Mesh : public Renderable {
	public:
		//! Default ctor creates an empty mesh
		Mesh();
		//! loads the mesh provided and gives it an identity transform
		Mesh( const char* pFilename );

		//! loads the mesh and hooks in the transform provided
		Mesh( const char* pFilename, Core::TransformNode* node );
		
		//! dtor
		~Mesh();

		//! inherited from Renderable
		virtual void render( RenderContext* context, Pipeline* pipeline ) override;
		
		virtual uint32_t getActualRenderablesOfType( R_TYPE _type, uint32_t arraySize, const Renderable** outArray ) const override {
			if( _type == R_MESH && arraySize > 0) {
				outArray[0] = this;
				return 1;
			} else {
				return 0;
			}
		}

		WobResourcePtr getResource() const {
			return meshHandle->acquire();
		}

	protected:
		WobResourceHandlePtr		meshHandle;
		Math::Matrix4x4*			ownedMatrix;
		Math::Matrix4x4				prevWVP;
	};
}; // end namespace Graphics

#endif