#pragma once
/** \file MeshModHalfEdge.h
   A half edge type.
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_HALFEDGE_H_ )
#define MESH_MOD_HALFEDGE_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//------------------------------------------a---------------------------------
#if !defined( MESH_MOD_VARICONTAINER_H_ )
#	include "varicontainer.h"
#endif
#if !defined( MESH_MOD_EDGE_H_ )
#	include "edge.h"
#endif

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{

	//---------------------------------------------------------------------------
	// Classes and structures
	//---------------------------------------------------------------------------
	namespace EdgeData {
		struct HalfEdge {
			//! index of vertex that starts this half edge
			VertexIndex					startVertexIndex;
			//! index of vertex that ends this half edge
			VertexIndex					endVertexIndex;

			//! other half edges this half edges is similar to (same vertices different polygon)
			EdgeIndexContainer			pairEdges;
			//! face this edge is connected
			FaceIndex					faceIndex;
			//! next edge in face (always clockwise)
			EdgeIndex					nextEdge;

			HalfEdge(){};

			static const std::string getName() { return "HalfEdge"; };
				
		};
	}

	/**
	Short description.
	Detailed description
	*/
	typedef ImplElements< EdgeData::HalfEdge, Edge_ > HalfEdgeEdgeElements;


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
