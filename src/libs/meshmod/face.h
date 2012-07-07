#pragma once
/** \file MeshModVertex.h
   A vertex system.
   Holds  a number of named vertex elements, 
   contains a list of similar vertices (same position, different elements (uv's, normals)
   contains a list of polyMeshModns each vertex is attached to
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_FACE_H_ )
#define MESH_MOD_FACE_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined( MESH_MOD_VARICONTAINER_H_ )
#	include "varicontainer.h"
#endif
#if !defined( MESH_MOD_FACEDATA_H_ )
#	include "facedata.h"
#endif

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{

	//---------------------------------------------------------------------------
	// Classes and structures
	//---------------------------------------------------------------------------
	namespace FaceData {
		struct Face {
			// one of the half edge (which one doesn't matter)
			EdgeIndex	anyHalfEdge;

			Face(){};
			Face( const EdgeIndex edge ) :
				anyHalfEdge(edge)
			{}

			static const std::string getName() { return "Face"; };
				
		};
	}

	typedef VariContainer< Elements<Face_> > FaceElementsContainer;

	typedef ImplElements< FaceData::Face, Face_ > FaceFaceElements;
	typedef ImplElements< FaceData::PlaneEquation, Face_ > PlaneEquationFaceElements;

	typedef ImplElements< FaceData::SortMapper, Face_ > SortMapperFaceElements;

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
