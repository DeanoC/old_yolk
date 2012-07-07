#pragma once
/** \file MeshModIndexTypes.h
	MeshMod is largely index based, therefore these are the basic
	unit used by this library
   (c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_INDEX_TYPES_H_ )
#define MESH_MOD_INDEX_TYPES_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{

	//---------------------------------------------------------------------------
	// Classes and structures
	//---------------------------------------------------------------------------
	//! vertex index type.
	typedef unsigned int VertexIndex;
	//! edge index type.
	typedef unsigned int EdgeIndex;
	//! face index type
	typedef unsigned int FaceIndex;
	//! material index type
	typedef unsigned int MaterialIndex;
	//! scene node index
	typedef unsigned int SceneNodeIndex;

	//! a list of vertex indices
	typedef std::vector<VertexIndex> VertexIndexContainer;
	//! a list of edge indices
	typedef std::vector<EdgeIndex> EdgeIndexContainer;
	//! a list of face indices
	typedef std::vector<FaceIndex> FaceIndexContainer;

	//! value used to indicate an index is not assigned/deleted/invalid etc.
	static const unsigned int MM_INVALID_INDEX = ~0;		

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
