#pragma once
//!-----------------------------------------------------
//!
//! \file renderable.h
//! base class of an object that can be rendered with
//! an arbitary transform
//! Shared base of mesh and sprite3D
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_RENDERABLE_H_)
#define YOLK_SCENE_RENDERABLE_H_

#if !defined( WIERD_CORE_TRANSFORM_NODE_H )
#include "core/transform_node.h"
#endif

#if !defined( WIERD_CORE_AABB_H )
#include "core/aabb.h"
#endif

namespace Scene {
	// forward decl
	class RenderContext;

	//!-----------------------------------------------------
	//!
	//! Base 3D renderable class
	//!
	//!-----------------------------------------------------
	class Renderable {
	public:
		enum R_TYPE {
			R_MESH,
			R_HEIGHTFIELD,
		};

		Renderable( Core::TransformNode* transNode ) :
		  	transformNode( transNode ),
			localAabb() {
		}

		//! dtor
		virtual ~Renderable(){};

		//! called to render this thing
		virtual void render( RenderContext* context, const int pipelineIndex ) = 0;

		virtual void debugDraw( RenderContext* context ) const {
			getWorldAABB().drawDebug( Core::RGBAColour(1,1,1,1), Math::IdentityMatrix() );
		}

		//! get the transform node directly for attaching
		const Core::TransformNode* getTransformNode() const { return transformNode; }
		Core::TransformNode* getTransformNode() { return transformNode; }

		const Core::AABB& getLocalAabb() const { return localAabb; }

		Core::AABB getWorldAABB() const {
			if( transformNode != NULL ) {
				return localAabb.transform( transformNode->getWorldTransform() );
			} else{
				return localAabb;
			}
		}

		// this is a method to get a flat array of renderables for simple
		// renderable it will 0 or 1 entry, however for compound renderables
		// it can render a number of renderables. Its also allows easy filtering
		// by type
		// return upto arraySize actual renderables of R_TYPE in outArray,
		// returns number inf array
		virtual uint32_t getActualRenderablesOfType( R_TYPE _type, uint32_t arraySize, const Renderable** outArray ) const = 0;
		virtual uint32_t getActualRenderablesOfType( R_TYPE _type, uint32_t arraySize, Renderable** outArray ) = 0;

	protected:
		Core::TransformNode*		transformNode;
		Core::AABB					localAabb;
	};

	typedef std::shared_ptr<Renderable>						RenderablePtr;

}; // end namespace Scene

#endif //YOLK_GRAPHICS_RENDERABLE_H