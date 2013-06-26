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

#if !defined( YOLK_CORE_AABB_H )
#include "core/aabb.h"
#endif

#if !defined( YOLK_CORE_FRUSTUM_H )
#include "core/frustum.h"
#endif

// forward decl
namespace Scene {
	class RenderContext;
	class Pipeline;
}

namespace Scene {
	//!-----------------------------------------------------
	//!
	//! Base 3D renderable class
	//!
	//!-----------------------------------------------------
	class Renderable : public std::enable_shared_from_this<Renderable> {
	public:
		static const uint32_t ALL_TYPES = Core::GenerateID<'A','L','L'>::value;
		Renderable() :
			transformNode( nullptr ),
			localAabb(),
			enabled( true ) {
		}

		Renderable( Core::TransformNode* transNode ) :	
		  	transformNode( transNode ),
			localAabb(),
			enabled( true ) {
		}

		//! dtor
		virtual ~Renderable(){};

		//--------- INTERFACE START -------------

		//! most renderable need some update (at least snapshoting the render matrix), override to do more than this
		virtual void renderUpdate() { getTransformNode()->setRenderMatrix(); };

		//! called to render this things opaque parts.
		//! note: if a compound type use getActual to pass the embedded objects, 
		//! so this may never be called on the compound type
		virtual void render( RenderContext* _context, const Pipeline* _pipeline, const Math::Matrix4x4 _renderMatrix ) const {};

		//! called to render this things transparent parts.
		//! note: if a compound type use getActual to pass the embedded objects, 
		virtual void renderTransparent( RenderContext* _context, const Pipeline* _pipeline, const Math::Matrix4x4 _renderMatrix ) const {};

		//! this is a method to get a flat array of renderables. For simple
		//! renderable it will 0 or 1 entry, however for compound renderables
		//! it can render a number of renderables. Its also allows easy filtering
		//! by type. Will return upto _maxArraySize actual renderables of _type in _outArray,
		//! \param _type 4 char code to tell what type to return (ALL_TYPES for all)
		//! \param _out vector to be added to (don't remove anything)
		virtual void getRenderablesOfType( 	uint32_t _type, std::vector< Renderable*>& _out ) const = 0;

		//! this is a method to get a flat array of visible renderables. For simple
		//! renderable it will 0 or 1 entry, however for compound renderables
		//! it can render a number of renderables. Its also allows easy filtering
		//! by type. Will return upto _maxArraySize actual renderables of _type in _outArray,
		//! \param _frustum frustum to be check against
		//! \param _type 4 char code to tell what type to return (ALL_TYPES for all)
		//! \param _out vector to be added to (don't remove anything)
		virtual void getVisibleRenderablesOfType( const Core::Frustum& _frustum, const uint32_t _type, std::vector< Renderable*>& _out ) const = 0;

		//--------- INTERFACE END -------------

/*		virtual void debugDraw( RenderContext* context ) const {
			Core::AABB waabb;
			getWorldAABB( waabb );
			waabb.drawDebug( Core::RGBAColour(1,1,1,1), Math::IdentityMatrix() );
		}*/


		//! get the transform node directly for attaching
		virtual const Core::TransformNode* getTransformNode() const { return transformNode; }
		virtual Core::TransformNode* getTransformNode() { return transformNode; }

		virtual const Core::AABB& getLocalAabb() const { return localAabb; }

		virtual void getWorldAABB( Core::AABB& waabb ) const {
			if( getTransformNode() != NULL ) {
				waabb = getLocalAabb().transform( getWorldMatrix() );
			} else{
				waabb = getLocalAabb();
			}
		}
		virtual void getRenderAABB( Core::AABB& waabb ) const {
			if( getTransformNode() != NULL ) {
				waabb = getLocalAabb().transform( getRenderMatrix() );
			} else{
				waabb = getLocalAabb();
			}
		}

		virtual const Math::Matrix4x4& getWorldMatrix() const {
			return getTransformNode()->getWorldMatrix();
		}

		virtual const Math::Matrix4x4& getRenderMatrix() const {
			return getTransformNode()->getRenderMatrix();
		}

		virtual void enable() { enabled = true; }
		virtual void disable() { enabled = false; }
		virtual bool isEnabled() const { return enabled; }
	protected:
		Core::TransformNode*		transformNode;
		Core::AABB					localAabb;
		bool						enabled;
	};

	typedef std::shared_ptr<Renderable>						RenderablePtr;

}; // end namespace Scene

#endif //YOLK_GRAPHICS_RENDERABLE_H