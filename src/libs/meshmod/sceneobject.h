#pragma once
/** \file MeshModSceneObject.h
A Scene object is a base class that things like meshes and camera derive of
as something is in a scene.
A scenenode links together a transform, N SceneObjects and hierachy of
other nodes.

(c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_SCENE_OBJECT_H_ )
#define MESH_MOD_SCENE_OBJECT_H_

#pragma once

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined(MESH_MOD_INDEX_TYPES_H_)
#include "indextypes.h"
#endif


//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{
	/** Scene Object
	**/
	class SceneObject
	{
	public:
		virtual ~SceneObject(){}
		SceneObject( const std::string& _type, const std::string& _name ) :
		  type( _type ), name( _name ){}

		//! returns the type of this object
		const std::string& getType() const { return type; }

		//! all objects in a scene have a name 
		const std::string& getName() const { return name; }

	protected:
		const std::string type; //!< type of this scene object
		const std::string name; //!< name of this scene object
	};
	
	typedef std::shared_ptr<SceneObject> SceneObjectPtr;

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
#endif
