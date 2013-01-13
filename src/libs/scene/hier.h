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
		Hier( Core::TransformNode* pRootNode );
		
		// from a Hie resource file
		Hier( const char* pFilename );

		//! dtor
		virtual ~Hier();

		virtual void render( RenderContext* context, Pipeline* pipeline ) override;
		virtual void renderTransparent( RenderContext* context, Pipeline* pipeline ) override;

		virtual uint32_t getActualRenderablesOfType( R_TYPE _type, uint32_t arraySize, Renderable** outArray ) const override {
			if( !isEnabled() ) return 0;
			if( _type == R_MESH || _type == R_ALL) {
				uint32_t numMeshes = 0;
				for( uint32_t i=0;i < ownedMeshes.size();++i ) {
					if( ownedMeshes[i]->isEnabled() ) {
						outArray[i] = ownedMeshes[i].get();
						numMeshes++;
						if( numMeshes >= arraySize ) return numMeshes;
					}
				}
				return numMeshes;
			} else {
				return 0;
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