#pragma once
/** \file MeshModVertex.h
   A vertex system.
   Holds  a number of named vertex elements, 
   contains a list of similar vertices (same position, different elements (uv's, normals)
   contains a list of polyMeshModns each vertex is attached to
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_VERTEX_H_ )
#define MESH_MOD_VERTEX_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined( MESH_MOD_VARICONTAINER_H_ )
#	include "varicontainer.h"
#endif

#if !defined( MESH_MOD_VERTEXDATA_H_ )
#	include "vertexdata.h"
#endif

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{

	//---------------------------------------------------------------------------
	// Classes and structures
	//---------------------------------------------------------------------------



	/**
		A container for vertex elements
	*/
	typedef VariContainer< Elements<Vertex_> >						VertexElementsContainer;

	// boolean marker
	typedef ImplElements< VertexData::BoolMarker, Vertex_ >			BoolMarkerVertexElements;

	// a single float (unspecifed use, upto you use subname to identify)
	typedef ImplElements< VertexData::Float1Tuple, Vertex_ >		Float1TupleVertexElements;

	// a single unsigned int (unspecifed use, upto you use subname to identify)
	typedef ImplElements< VertexData::UnsignedInt1Tuple, Vertex_ >	UnsignedInt1TupleVertexElements;

	// bone indices and weights attached to this vertex
	typedef ImplElements< VertexData::BoneWeights, Vertex_ >		BoneWeightsVertexElements;

	// vertex displacement attached to this vertex 
	typedef ImplElements< VertexData::PositionDelta, Vertex_ >		PositionDeltaVertexElements;

	// an index remapping item mapping sorted to unsorted index
	typedef ImplElements< VertexData::SortMapper, Vertex_ >			SortMapperVertexElements;

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
