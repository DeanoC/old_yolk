#pragma once
/** \file MeshModHalfEdgeVertex.h
   A vertex holding a list of half edges that join to this vertex.
   (c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_POINT_REP_VERTEX_H_ )
#define MESH_MOD_POINT_REP_VERTEX_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined( MESH_MOD_VARICONTAINER_H_ )
#	include "modvaricontainer.h"
#endif

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{
	namespace VertexData
	{
		struct PointRep
		{
			VertexIndex next;
			PointRep(){}
			PointRep(const VertexIndex n_) : 
				next(n_)
			{}
			static const std::string getName() { return "PointRep"; };
		};
	}
	//! index of next vertex sharing position but with other data ( cyclic list )
	typedef ImplElements< VertexData::PointRep, Vertex_ > PointRepVertexElements;

} // end MeshMod

#endif // MESH_MOD_POINT_REP_VERTEX_H_
