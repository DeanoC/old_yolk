///-------------------------------------------------------------------------------------------------
/// \file	meshmod\edge.cpp
///
/// \brief	Implements the edge class.
///
/// \details	
///		edge description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
/// \todo	Fill in detailed file description.
////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshmod.h"
#include <algorithm>

#include "edge.h"
#include "mesh.h"

//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
namespace MeshMod {

/**
Adds a half edge to the mesh.
Handles all the bookkeeping the half edge's correct, doesn't have a next edge index at this stage
@param svIndex start vertex index
@param evIndex end vertex index
@param poIndex polygon index the edge is attached to
@return index of half edge
*/
EdgeIndex Mesh::addHalfEdge( const VertexIndex svIndex,
	const VertexIndex evIndex,
	const FaceIndex poIndex )
{
	// add half edge
	EdgeData::HalfEdge e0;
	e0.startVertexIndex	= svIndex;
	e0.endVertexIndex		= evIndex;
	e0.faceIndex			= poIndex;
	e0.nextEdge			= MM_INVALID_INDEX;
	edgeContainer.resize( edgeContainer.size() + 1 );
	const EdgeIndex edgeIndex = (EdgeIndex) halfEdgeEdge->size() - 1;
	(*halfEdgeEdge)[edgeIndex] = e0;

	// check for duplicates
	if( std::find(	(*halfEdge)[svIndex].edgeIndexList.begin(), 
		(*halfEdge)[svIndex].edgeIndexList.end(),
		edgeIndex ) == (*halfEdge)[svIndex].edgeIndexList.end() ) {
			(*halfEdge)[svIndex].edgeIndexList.push_back( edgeIndex );
	}
	if( std::find(	(*halfEdge)[evIndex].edgeIndexList.begin(), 
		(*halfEdge)[evIndex].edgeIndexList.end(),
		edgeIndex ) == (*halfEdge)[evIndex].edgeIndexList.end() ) {
			(*halfEdge)[evIndex].edgeIndexList.push_back( edgeIndex );
	}

	return edgeIndex;
}

#if 0
void Mesh::removeEdge( const EdgeIndex index ) {
	EdgeData::HalfEdge& hedge = (*m_halfEdgeEdge)[ index ];

	// foreach pair edge
	Core::vector<EdgeIndex>::iterator peIt = hedge.m_pairEdges.begin();
	while( peIt != hedge.m_pairEdges.end() ) {
		Edge::HalfEdge& pedge = (*m_halfEdgeEdge)[ *peIt ];
		// loop to find the previous link in the face edge chain
		EdgeIndex i = (*m_face)[ pedge.m_faceIndex ].anyHalfEdge; 
		if( i == *peIt ) {
			(*m_face)[ pedge.m_faceIndex ].anyHalfEdge = pedge.next;
		}

		do {
			i = (*m_halfEdgeEdge)[i].next;
		} while( (*m_halfEdgeEdge)[i].next != *peIt );

		(*m_halfEdgeEdge)[i].next = pedge.next;
		++peIt;
	}

	// loop to find the previous link in the face edge chain
	EdgeIndex i = (*m_face)[ hedge.m_faceIndex ].anyHalfEdge; 
	if( i == *peIt ) {
		(*m_face)[ hedge.m_faceIndex ].anyHalfEdge = hedge.next;
	}

	do {
		i = (*m_halfEdgeEdge)[i].next;
	} while( (*m_halfEdgeEdge)[i].next != index );

	(*m_halfEdgeEdge)[i].next = hedge.next;

}
#endif
/**
Breaks all edge pairs for the entire mesh.
Break each half edge from its other half
*/
void Mesh::breakAllPairEdges()
{
	std::vector<EdgeData::HalfEdge>::iterator edgeIt = halfEdgeEdge->elements.begin();
	while( edgeIt != halfEdgeEdge->elements.end() )
	{
		(*edgeIt).pairEdges.clear();
		++edgeIt;
	}
}

/**
Connects Half edge to the other half edges for the entire mesh.
Builds the half edge to half edge data structures
*/
void Mesh::connectPairEdges() {
	// foreach face
	std::vector<FaceData::Face>::iterator triIt = face->elements.begin();
	while( triIt != face->elements.end() )
	{
		const EdgeIndex firstEdge = (*triIt).anyHalfEdge;
		EdgeIndex edgeNum = firstEdge;

		do {
			// scan around edges attached to the start vertex of this edge
			// pair up any edge with the same start and end
			VertexIndex startVert = (*halfEdgeEdge)[edgeNum].startVertexIndex;
			VertexIndex endVert = (*halfEdgeEdge)[edgeNum].endVertexIndex;

			// swap indices for consistent ordering
			if( startVert > endVert) {
				std::swap( startVert, endVert );
			}

			//iterate through the edges connect to the start vertex
			EdgeIndexContainer::const_iterator vedgeIt = (*halfEdge)[ startVert ].edgeIndexList.begin();
			while( vedgeIt != (*halfEdge)[ startVert ].edgeIndexList.end() ) {
				// check that we are not working on our own edge
				if( (*vedgeIt) != edgeNum ) {
					// does this edge have the same start and end vertex indices, if so its a pair
					EdgeData::HalfEdge& e0 = (*halfEdgeEdge)[ (*vedgeIt) ];
					if( ( (e0.startVertexIndex == startVert) &&
						(e0.endVertexIndex == endVert) ) ||
						( (e0.startVertexIndex == endVert) &&
						(e0.endVertexIndex == startVert) ) ) {
							e0.pairEdges.push_back( edgeNum );
							// rely on the other find to add the other side
					}
				}
				++vedgeIt;
			}

			edgeNum = (*halfEdgeEdge)[edgeNum].nextEdge;
		} while( edgeNum != firstEdge );

		++triIt;
	}
}

} // end