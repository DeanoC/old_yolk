/** \file MeshModVertex.cpp
   MeshMod Vertex implementation.
   (c) 2002 Deano Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshmod.h"
#include <algorithm>

#include "vertex.h"
#include "mesh.h"

//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------
namespace
{
	unsigned int dummy_UINTFloatMarker = 0xFFFFFFFF;
};

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
namespace MeshMod
{
const float s_epsilon1e_5 = 1e-5f;
const float s_floatMarker = *(reinterpret_cast<float*>( &dummy_UINTFloatMarker ));

/**
Clones the input vertex.
An exact copy of the input vertex is created and its index returned, point reps are kept upto date
@param vertexToCopy the index of the vertex to clone
@return index of the new vertex
*/
unsigned int Mesh::cloneVertex( const VertexIndex vertexToCopy ) {
	const VertexIndex newIndex = (const VertexIndex) vertexContainer.cloneElement( vertexToCopy );

	// add to the point rep list (increased via vertex container)
	const VertexIndex temp = (*pointRep)[vertexToCopy].next;
	(*pointRep)[vertexToCopy] = VertexData::PointRep(newIndex);
	(*pointRep)[newIndex] = VertexData::PointRep(temp);

	return newIndex;
}

/**
Adds a new position.
Adds a new position (which has the effect of creating a new vertex), updates the point rep
@param x pos.x
@param y pos.y
@param z pos.z
@return index of new vertex
*/
VertexIndex Mesh::addPosition( const float x, const float y, const float z ) {
	const VertexIndex oldSize = (const VertexIndex) vertexContainer.size();

	// default to linking to itself
	const VertexIndex link = oldSize;

	vertexContainer.resize( oldSize + 1 );
	// add position
	(*position)[oldSize] = VertexData::Position(x,y,z);

	// add point rep
	(*pointRep)[oldSize] = VertexData::PointRep(link);

	return oldSize;
}

/**
Removes a vertex position.
This is only partially implemented, its only used to remove additional similar vertices.
If you try and delete the final similar vertex, it will currently assert, in future
it should update the face and edge lists properly.
Also note vertices aren't physical removed they will have a point rep of MeshModM_INVALID_INDEX
this is to keep the potentially expensive reindex operation to a minimum
@param index the index to remove
*/
void Mesh::removeVertex( const VertexIndex index ) {
	// decide wether we have a similar vertex to move to 
	// or wether this is the last vertex at this position and as such
	// we cause faces to lose a vertex (triangles becoming lines etc)
	VertexIndex link = (*pointRep)[index].next;

	// a point rep set to MeshModM_INVALID_INDEX is 'deleted'
	if( link == MM_INVALID_INDEX ) {
		return;
	}

	EdgeIndexContainer edgeList;
	getVertexEdges( index, edgeList );

	VertexIndex moveTo = MM_INVALID_INDEX;
	if( link == index ) {
		// easy case is a free vertex (no attached edges)
		if( edgeList.size() == 0 ) {
			(*pointRep)[index] = MM_INVALID_INDEX;
			return;
		} else {
			TODO_ASSERT( false ); // TODO edge contracts so far but doesn't remove the invalid edge from face yet
			// we surrogate the vertex along the first edge for all edges
			// which collapses faces
			// This isn't the pretties replacement for that, we need a Quadric rating 
			// edge/vertex contractor
			//EdgeData::HalfEdge& fedge = (*m_halfEdgeEdge)[edgeList.begin()];
			//moveTo = fedge.m_endVertexIndex;			
		}  
	} else {
		// just move all edges using the position to the link
		moveTo = link;
	}
	
	// we use a set to ensure no duplicates
	// the edge set we are moving vertices to
	std::unordered_set<EdgeIndex> edgeSet;
	std::copy(	(*halfEdge)[moveTo].edgeIndexList.begin(),
				(*halfEdge)[moveTo].edgeIndexList.end(),
				std::inserter( edgeSet, edgeSet.end() ) );
	// clear the pair edges
	(*halfEdge)[index].edgeIndexList.clear(); 

	EdgeIndexContainer::iterator edIt = edgeList.begin();
	while( edIt != edgeList.end() ) {
		EdgeData::HalfEdge& edge = (*halfEdgeEdge)[(*edIt)];

		if( edge.startVertexIndex == index ) {
			if( edge.endVertexIndex != moveTo ) {
				edge.startVertexIndex = moveTo;
				edgeSet.insert((*edIt)); 
			}
		}
		if( edge.endVertexIndex == index ) {
			if( edge.startVertexIndex != moveTo ) {
				edge.endVertexIndex = moveTo;
				edgeSet.insert((*edIt)); 
			}
		}

		EdgeIndexContainer::iterator pedIt = edge.pairEdges.begin();
		while( pedIt != edge.pairEdges.end() ) {
			EdgeData::HalfEdge& pedge = (*halfEdgeEdge)[(*pedIt)];

			if( pedge.startVertexIndex == index ) {
				if( pedge.endVertexIndex != moveTo ) {
					pedge.startVertexIndex = link;
					edgeSet.insert((*pedIt)); 
				}
			}
			if( pedge.endVertexIndex == index ) {
				if( edge.startVertexIndex != moveTo ) {
					pedge.endVertexIndex = link;
					edgeSet.insert((*pedIt)); 
				}
			}
			++pedIt;
		}
		++edIt;
	}
	// now move the set back into the list
	(*halfEdge)[moveTo].edgeIndexList.clear();
	std::copy(	edgeSet.begin(),
				edgeSet.end(),
				std::inserter( (*halfEdge)[moveTo].edgeIndexList, (*halfEdge)[moveTo].edgeIndexList.end() ) );

	if( link != index ) {
		// loop to find the previous link in the point rep chain
		VertexIndex i = (*pointRep)[index].next;
		do {
			i = (*pointRep)[i].next;
		}
		while( (*pointRep)[i].next != index );

		(*pointRep)[i].next = (*pointRep)[index].next;
	}
	(*pointRep)[index] = MM_INVALID_INDEX;

}

bool Mesh::isValidVertex( const VertexIndex index ) const {
	if( (*pointRep)[index].next == MM_INVALID_INDEX ) {
		return false;
	} else {
		return true;
	}
}

/**
Collapse all similar position to a single one, ignores all other data similarity is just for position
Generally do before you add any other data as it will potionally remove that other data
@param fEpsilon how similar is similar?
*/
void Mesh::removeAllSimilarPositions( float fEpsilon ) {
	// NOTE : I'm not sure I should do these 3 operations...
	// but then this is a extremely destructive operations so
	// meh if you don't know what your doing you've got bigger problems

	// first stop is to use a point rep to accelerate the op
	// this should be fairly fast due to a 1D sort
	createPointRep( fEpsilon );

	// remove all the pair edges???
	breakAllPairEdges();
	// connect pair edges? 
	connectPairEdges();

	// walk the position array
	std::vector<VertexData::Position>::iterator posIt = position->elements.begin();
	std::vector<VertexIndex> vertList;
	while( posIt != position->elements.end() ) {
		const VertexIndex vertNum = (VertexIndex) std::distance( position->elements.begin(), posIt );

		// skip deleted one
		if( !isValidVertex( vertNum ) ) {
			++posIt;
			continue;
		}

		// using the points list we can now quickly find our similar brethren
		vertList.clear();
		getSimilarVertices( vertNum, vertList );
		
		// now remove all those who are similar to the original
		std::vector<VertexIndex>::iterator vlIt = vertList.begin();
		VertexIndex i = (*pointRep)[vertNum].next;
		while( i != vertNum ) {
			VertexIndex del = i;
			i = (*pointRep)[i].next;
			removeVertex( del );
		}

		++posIt;
	}
}

/**
Do the 2 vertices have the same position?.
Uses the pointrep to check if the 2 input vertices have the same position
@param i0 first vertex index
@param i1 second vertex index
@return boolean result
*/
bool Mesh::similarPosition( const VertexIndex i0, const VertexIndex i1) const {
	// the obvious really quick check :-)
	if( i0 == i1 ) {
		return true;
	}

	// deleted vertex never match the same
	if( (*pointRep)[i0].next == MM_INVALID_INDEX ||
		(*pointRep)[i1].next == MM_INVALID_INDEX ) {
		return false;
	}

	// walk the point rep, similar vertices match the samePosition test
	unsigned int i = i0;
	do {
		if( (*pointRep)[i].next == i1 )
			return true;

		i = (*pointRep)[i].next;
	} while( (*pointRep)[i].next != i0 );

	// nope, we rely on point rep so we are not the same
	// NOTE : in future this might not be good enough...
	// and provide a more extensive version for when you really really
	// want an exhaustive samePosition (i.e blendshapes may have
	// same position but they might not be similar...)
	return false;
}

/**
Get a list of all vertex that share the input vertex position.
Use the point rep to produce a list of vertex that share the same(ish) position
@param x pos.x
@param y pos.y
@param z pos.z
@return index of new vertex
*/
void Mesh::getSimilarVertices( const VertexIndex vertNum, VertexIndexContainer& vertList ) const {
	VertexIndex i = vertNum;

	// a deleted vertex has no 'similar' vertices
	if( (*pointRep)[i].next == MM_INVALID_INDEX ) {
		return;
	}

	do {
		vertList.push_back( i );
		i = (*pointRep)[i].next;
	} while( (*pointRep)[i].next != vertNum );
}

/**
Is this vertex similar to a vertex in the list of vertices provided
@return index of first similar vertex 
*/
VertexIndex Mesh::hasPosition( const VertexIndex i0, const VertexIndexContainer& vertexList ) const {
	VertexIndexContainer::const_iterator vIt = vertexList.begin();
	while( vIt != vertexList.end() ) {
		if( similarPosition( i0, (*vIt) ) == true ) {
			return (*vIt);
		}
		++vIt;
	}
	return MM_INVALID_INDEX;
}

}

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
