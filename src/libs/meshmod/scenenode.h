#pragma once
/** \file MeshModScene.h
	A container for a scene hierachy

(c) 2012 Dean Calver
 */

#if !defined( MESH_MOD_SCENE_NODE_H_ )
#define MESH_MOD_SCENE_NODE_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined(MESH_MOD_INDEX_TYPES_H_)
#include "indextypes.h"
#endif

#if !defined( MESH_MOD_TRANSFORM_H_ )
#include "transform.h"
#endif

#if !defined( MESH_MOD_SCENE_OBJECT_H_ )
#include "sceneobject.h"
#endif

#if !defined( MESH_MOD_PROPERTY_H_ )
#include "property.h"
#endif

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod {
	class SceneNode;

	typedef std::shared_ptr<SceneNode> SceneNodePtr;

	class SceneNode {
	public:
		void addChild( const SceneNodePtr child ) {
			// check for duplicates
			CORE_ASSERT( std::find( children.begin(), children.end(), child ) == children.end() );
			children.push_back( child );
		}
		void removeChild( SceneNodePtr child ) {
			SceneNodeContainer::iterator chIt = 
				std::find( children.begin(), children.end(), child );
			CORE_ASSERT( chIt != children.end() );
			children.erase( chIt );
		}

		uint32_t findChildIndex( const SceneNodePtr child ) const {
			SceneNodeContainer::const_iterator chIt = std::find( children.begin(), children.end(), child );
			return (uint32_t) std::distance( children.begin(), chIt );
		}

		unsigned int getChildCount() const {
			return (unsigned int) children.size();
		}
		SceneNodePtr getChild( SceneNodeIndex index ) {
			return children[index];
		}
		void addObject( SceneObjectPtr obj ) {
			CORE_ASSERT( std::find( objects.begin(), objects.end(), obj ) == objects.end() );
			objects.push_back( obj );
		}
		void removeObject( SceneObjectPtr obj ) {
			SceneObjectContainer::iterator obIt = 
				std::find( objects.begin(), objects.end(), obj );
			CORE_ASSERT( obIt != objects.end() );
			objects.erase( obIt );
		}

		unsigned int getObjectCount() const {
			return (unsigned int) objects.size();
		}
		SceneObjectPtr getObject(unsigned int index) {
			return objects[index];
		}

		// embedded transform TODO animation
		Transform										transform;
		std::string										type;
		std::string										name;
		std::vector< PropertyPtr > 						properties;

	protected:
		typedef std::vector< SceneNodePtr >				SceneNodeContainer;
		typedef std::vector< SceneObjectPtr >			SceneObjectContainer;
		SceneNodeContainer								children;
		SceneObjectContainer							objects;

	};


} // end namespace
//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Externals
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// End Header file
//---------------------------------------------------------------------------
#endif // MESH_MOD_SCENE_NODE_H_
