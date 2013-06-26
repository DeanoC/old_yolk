#pragma once
//!-----------------------------------------------------
//!
//! \file mesh.h
//! the engine graphics mesh class
//!
//!-----------------------------------------------------
#if !defined( YOLK_SCENE_MESH_H_ )
#define YOLK_SCENE_MESH_H_

#if !defined( YOLK_SCENE_RENDERABLE_H_ )
#include "renderable.h"
#endif

#if !defined( YOLK_SCENE_WOBFILE_H_ )
#include "wobfile.h"
#endif

namespace Scene {
	// forward decl
	class RenderContext;

	class Mesh : public Renderable {
	public:
		static const uint32_t MESH_TYPE = Core::GenerateID<'M','E','S','H'>::value;

		//! Default ctor creates an empty mesh
		Mesh();
		//! loads the mesh provided and gives it an identity transform
		Mesh( const char* pFilename );

		//! loads the mesh and hooks in the transform provided
		Mesh( const char* pFilename, Core::TransformNode* node );
		
		//! dtor
		~Mesh();

		//--------- RENDERABLE IMPLEMENTATION START -------------

		//! inherited from Renderable
		void render( RenderContext* _context, const Pipeline* _pipeline, const Math::Matrix4x4 _renderMatrix ) const override;
		void renderTransparent( RenderContext* _context, const Pipeline* _pipeline, const Math::Matrix4x4 _renderMatrix ) const override;

		void getRenderablesOfType( uint32_t _type, std::vector<Renderable*>& _out ) const override {
			if( (_type == MESH_TYPE || _type == ALL_TYPES) && isEnabled() ) {
				_out.push_back( (Scene::Renderable*) this );
			}
		}
		void getVisibleRenderablesOfType( const Core::Frustum& _frustum, const uint32_t _type, std::vector< Renderable*>& _out ) const override {
			if( (_type == MESH_TYPE || _type == ALL_TYPES) && isEnabled() ) {
				Core::AABB waabb;
				getWorldAABB( waabb );
				if( _frustum.cullAABB( waabb ) != Core::Frustum::CULL_RESULT::OUTSIDE ) {
					_out.push_back( (Scene::Renderable*) this );
				}
			}
		}
		//--------- RENDERABLE IMPLEMENTATION END -------------

		WobPtr getResource() const {
			return meshHandle->acquire();
		}

	protected:
		WobHandlePtr				meshHandle;
		Math::Matrix4x4*			ownedMatrix;
		mutable Math::Matrix4x4		prevWVP;
	};

	typedef std::shared_ptr<Mesh>						MeshPtr;
	
}; // end namespace Graphics

#endif