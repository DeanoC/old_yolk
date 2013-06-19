//!-----------------------------------------------------
//!
//! \file Hie.h
//! Hierachy class
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_HIER_H_)
#define YOLK_SCENE_HIER_H_
#pragma once

#if !defined( YOLK_SCENE_RENDERABLE_H_ )
#include "scene/renderable.h"
#endif

#if !defined( YOLK_SCENE_HIERFILE_H_ )
#	include "scene/hierfile.h"
#endif

#if !defined( YOLK_SCENE_MESH_H_ )
#	include "scene/mesh.h"
#endif

#if !defined( YOLK_CORE_CORERESOURC_H_ )
#	include "core/coreresources.h"
#endif

namespace Scene {
	// forward decl
	class RenderContext;
	class Pipeline;

	class Hier : public Renderable {
	public:
		static const uint32_t HIER_TYPE = Core::GenerateID<'H','I','E','R'>::value;
		Hier( Core::TransformNode* pRootNode );
		
		// from a Hie resource file
		Hier( const char* pFilename );

		//! dtor
		virtual ~Hier();

		void renderUpdate() override {
			Renderable::renderUpdate();

			for( uint32_t i=0;i < ownedMeshes.size();++i ) {
				ownedMeshes[i]->renderUpdate();
			}
		}


		void getRenderablesOfType( uint32_t _type, std::vector<Renderable*>& _out ) const override {
			if( !isEnabled() ) return;
			// if someone specifically asked for hier types return us else send back our contained meshes
			if( _type == Mesh::MESH_TYPE || _type == ALL_TYPES ) {
				for( uint32_t i=0;i < ownedMeshes.size();++i ) {
					if( ownedMeshes[i]->isEnabled() ) {
						_out.push_back( ownedMeshes[i].get() );
					}
				}
			} else if( _type == HIER_TYPE ) {
				_out.push_back( (Renderable*) this );
			}
		}
		void getVisibleRenderablesOfType( const Core::Frustum& _frustum, const uint32_t _type, std::vector< Renderable*>& _out ) const override {
			if( !isEnabled() ) return;
			Core::AABB waabb;

			// if someone specifically asked for hier types return us else send back our contained meshes
			if( _type == Mesh::MESH_TYPE || _type == ALL_TYPES ) {
				for( uint32_t i=0;i < ownedMeshes.size();++i ) {
					if( ownedMeshes[i]->isEnabled() ) {
						ownedMeshes[i]->getRenderAABB( waabb );
						if( _frustum.cullAABB( waabb ) != Core::Frustum::CULL_RESULT::OUTSIDE ) {
							_out.push_back( ownedMeshes[i].get() );
						}
					}
				}
			} else if( _type == HIER_TYPE ) {
				getRenderAABB( waabb );
				if( _frustum.cullAABB( waabb ) != Core::Frustum::CULL_RESULT::OUTSIDE ) {
					_out.push_back( (Renderable*) this );
				}
			}
		}


		int getNodeCount() const { return numNodes; }

		Core::TransformNode* getTransformNode( int nodeId = 0 ) {
			CORE_ASSERT( nodeId >= 0 && nodeId < numNodes );
			return &nodeArray[nodeId];
		}

		const Core::TransformNode* getTransformNode( int nodeId = 0 ) const {
			CORE_ASSERT( nodeId >= 0 && nodeId < numNodes );
			return &nodeArray[nodeId];
		}

		Core::BinPropertyResourceHandlePtr getProperties( int nodeId = 0 ) const {
			CORE_ASSERT( nodeId >= 0 && nodeId < numNodes );
			return propertyArray[ nodeId ].get();
		}

		Hier*											cloneInstance() const;

	protected:
														Hier();
		int												numNodes;
		boost::scoped_array<uint8_t>					nodeArrayMem;
		Core::TransformNode*							nodeArray;
		boost::scoped_array<Math::Matrix4x4>			matrixArray;
		Core::ScopedResourceHandle<HieHandle> 			hieHandle;
		HiePtr											hie;

		typedef std::vector< std::shared_ptr< Mesh > >	OwnedMeshes;
		OwnedMeshes										ownedMeshes;

		typedef Core::ScopedResourceHandle<Core::BinPropertyResourceHandle> ScopedPropertyHandle;
		boost::scoped_array<ScopedPropertyHandle> 		propertyArray;

	};

	typedef std::shared_ptr<Hier>						HierPtr;
	
}; // end namespace Scene

#endif