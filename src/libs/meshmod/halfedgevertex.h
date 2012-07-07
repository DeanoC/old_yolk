#pragma once
/** \file MeshModHalfEdgeVertex.h
   A vertex holding a list of half edges that join to this vertex.
   (c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_HALF_EDGE_VERTEX_H_ )
#define MESH_MOD_HALF_EDGE_VERTEX_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined( MESH_MOD_VARICONTAINER_H_ )
#	include "varicontainer.h"
#endif

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{
	namespace VertexData
	{
		struct HalfEdge
		{
			EdgeIndexContainer edgeIndexList;
			static const std::string getName() { return "HalfEdge"; };
		};
	}
	//! indices of half edges that starts with this vertex
	typedef ImplElements< VertexData::HalfEdge, Vertex_ > HalfEdgeVertexElements;

} // end MeshMod

#endif // MESH_MOD_HALF_EDGE_VERTEX_H_
