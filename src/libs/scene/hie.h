//!-----------------------------------------------------
//!
//! \file Hie.h
//! Hierachy class
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_HIE_H_)
#define YOLK_SCENE_HIE_H_
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

#if !defined( YOLK_SCENE_ENVIRONMENT_H_ )
#	include "scene/environment.h"
#endif

namespace Scene {
	// forward decl
	class RenderContext;

	class Hie : public Renderable {
	public:
		Hie( Core::TransformNode* pRootNode );
		
		// from a Hie resource file
		Hie( const char* pFilename );

		//! dtor
		virtual ~Hie();

		virtual void debugDraw(	RenderContext* context ) const;

		virtual void render( RenderContext* context, const int pipelineName );

		virtual uint32_t getActualRenderablesOfType( R_TYPE _type, uint32_t arraySize, const Renderable** outArray ) const {
			if( _type == R_MESH ) {
				int numMeshes = Math::Min<int>(ownedMeshes.size(), arraySize );
				for( int i=0;i < numMeshes;++i ) {
					outArray[i] = ownedMeshes[i].get();
				}
				return numMeshes;
			} else {
				return 0;
			}
		}
		virtual uint32_t getActualRenderablesOfType( R_TYPE _type, uint32_t arraySize, Renderable** outArray ) {
			if( _type == R_MESH ) {
				int numMeshes = Math::Min<int>(ownedMeshes.size(), arraySize );
				for( int i=0;i < numMeshes;++i ) {
					outArray[i] = ownedMeshes[i].get();
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

		const EnvironmentPtr getEnvironment() const {
			return environment;
		}

	protected:
		int												numNodes;
		boost::scoped_array<uint8_t>					nodeArrayMem;
		Core::TransformNode*							nodeArray;
		boost::scoped_array<Math::Matrix4x4>			matrixArray;
		Core::ScopedResourceHandle<HierResourceHandle> 	hierHandle;
		HierResourcePtr									hier;

		typedef std::vector< std::shared_ptr< Mesh > >	OwnedMeshes;
		OwnedMeshes										ownedMeshes;

		EnvironmentPtr									environment;
		typedef Core::ScopedResourceHandle<Core::BinPropertyResourceHandle> ScopedPropertyHandle;
		boost::scoped_array<ScopedPropertyHandle> 		propertyArray;

	};

	typedef std::shared_ptr<Hie>						HiePtr;
	
}; // end namespace Scene

#endif