/** \file MeshModMesh.cpp
   Geometry Optimiser Mesh implementation.
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
#include <set>
#include "mesh.h"
#include "positionvertex.h"
#include "halfedgevertex.h"
#include "pointrepvertex.h"
#include "halfedge.h"
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
using namespace MeshMod;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

// protected copy ctor
Mesh::Mesh( const Mesh& rhs ) :
	SceneObject( "Mesh", rhs.getName() )
{
	rhs.edgeContainer.cloneTo( edgeContainer );	
	rhs.vertexContainer.cloneTo( vertexContainer );	
	rhs.faceContainer.cloneTo( faceContainer );	
	rhs.materialContainer.cloneTo( materialContainer );	

	position = vertexContainer.getElements<PositionVertexElements>();
	halfEdge = vertexContainer.getElements<HalfEdgeVertexElements>();
	pointRep = vertexContainer.getElements<PointRepVertexElements>();
	face = faceContainer.getElements<FaceFaceElements>();
	halfEdgeEdge = edgeContainer.getElements<HalfEdgeEdgeElements>();
}
/**
defualt ctor
*/
Mesh::Mesh( const std::string& name ) :
	SceneObject( "Mesh", name )
{
	// helps the auto copier and diagnostics

	// default vertex elements
	position = vertexContainer.addElements<PositionVertexElements>();
	halfEdge = vertexContainer.addElements<HalfEdgeVertexElements>();
	pointRep = vertexContainer.addElements<PointRepVertexElements>();

	// default face elements
	face = faceContainer.addElements<FaceFaceElements>();

	// default edge elements
	halfEdgeEdge = edgeContainer.addElements<HalfEdgeEdgeElements>();
}

/**
default dtor
*/
Mesh::~Mesh()
{
}

namespace
{

//! links a float (X) to the vertex index
typedef std::pair<float, VertexIndex> sortItem;

//! sorts things based on the float value, you can then
//! get the original version via the index
struct PointRepSortCompare
{
	bool operator()( const sortItem& a, const sortItem& b )
	{
		return( a.first < b.first );
	}
};
}

/**
Create a point rep, based on the idea that any thing close (fEpsilon) to the 
same position, is the 'similar'
Uses a 1D sort to speed this up, assuming that most things will be reasonable
distributed along the X. Obviously worse case (A line constant in X) could be
very slow. but I'm betting that in practice this will be fast enough for most
data sets.
@param fEpsilon how similar is similar?
*/
void Mesh::createPointRep( float fEpsilon ) {
	// temp storage for the sort
	std::vector< sortItem > sortPos;

	// its can't be bigger than the position array so reserve that much ram
	sortPos.reserve( position->size() );

	// spin through all the position adding any valid ones to a
	// X and index vector, we then sort by X to quickly find which position
	// are close to each other
	std::vector<VertexData::Position>::const_iterator firstIt = position->elements.begin();
	std::vector<VertexData::Position>::const_iterator posIt = firstIt;
	while( posIt != position->elements.end() ) {
		const VertexIndex vertNum = (VertexIndex) std::distance(firstIt, posIt );
		if( isValidVertex(vertNum) ) {
			sortPos.push_back( sortItem( (*posIt).x, vertNum ) );
		}
		++posIt;
	}
	// now do the sort so we have the indices of things close in the X 
	std::sort( sortPos.begin(), sortPos.end(), PointRepSortCompare() );

	// clear existing point representation
	PointRepVertexElements::iterator prIt = pointRep->elements.begin();
	while( prIt != pointRep->elements.end() ) {
		const VertexIndex prNum = (VertexIndex) std::distance(pointRep->elements.begin(), prIt );
		// a point rep set to MM_INVALID_INDEX is 'deleted' if its deleted leave it deleted
		if( (*prIt).next != MM_INVALID_INDEX ) {
			(*prIt).next = prNum;
		}
		++prIt;
	}

	std::vector< sortItem >::iterator sposIt = sortPos.begin() + 1;
	while( sposIt != sortPos.end() )
	{
		const unsigned int vertNum = (*sposIt).second;
		// ignore deleted vertices
		if( !isValidVertex( vertNum ) ) {
			++sposIt;
			continue;
		}

		std::vector< sortItem >::iterator siIt = sposIt - 1;
		float x = position->elements[ vertNum ].x;
		float y = position->elements[ vertNum ].y;
		float z = position->elements[ vertNum ].z;

		// need to back up until we are further than epsilon (with a nice fudge factor)
		// away, test everything in this range and add it to our point rep if in range
		// we never scan forward as the next johnny will search backwards
		while(	siIt != sortPos.begin() &&
				(fabsf((*sposIt).first - (*siIt).first) < fEpsilon*5)  ) {
			const VertexIndex siNum = (*siIt).second;
			// ignore deleted vertices
			if( !isValidVertex( siNum ) ) {
				--siIt;
				continue;
			}

			// ignore deleted vertices
			if( (*position)[ siNum ].equal( x, y, z, fEpsilon ) ) {
				const VertexIndex link = (*pointRep)[ siNum ].next;
				(*pointRep)[ siNum ].next = vertNum;
				(*pointRep)[ vertNum ].next = link;
				break;
			}
			--siIt;
		}
		++sposIt;
	}
}


/**
Adds a polygon to the mesh.
Adds a poly, creates edge/half edge data structures 
@param indices - vertex indices the polygon is composed off
@param baseVertex - all vertex indices are based off this number
@return index of new polyMeshModn
*/
FaceIndex Mesh::addPolygon(		const VertexIndexContainer& indices, 
								const VertexIndex baseVertex )
{
	// doesn't triangulate added polygons directly (triangulation is done later, if you require)
	// should always be convex simple polygons!!

	const FaceIndex polyNum = face->size();

	FaceData::Face t0;
	// index of this polygon first edge
	EdgeIndex firstEdge = (EdgeIndex) halfEdgeEdge->size();
	// if the input face data has 0 vertices
	if( indices.size() == 0 ) {
		t0.anyHalfEdge = MM_INVALID_INDEX;
	} else {
		t0.anyHalfEdge = firstEdge;
	}
	faceContainer.resize( polyNum + 1 );
	face->getElement(polyNum) = t0;

	// insert polygon at the end of the polygons list
	for(VertexIndex i=0;i < indices.size(); i++) {
		const VertexIndex v0Index = indices[i] + baseVertex;
		const VertexIndex v1Index = (VertexIndex) indices[ (i+1) % indices.size() ] + baseVertex;

		CORE_ASSERT( isValidVertex(v0Index) && isValidVertex(v1Index) );

		const EdgeIndex e0 = addHalfEdge( v0Index, v1Index, polyNum );
		(*halfEdgeEdge)[e0].nextEdge = firstEdge + ((i+1) % indices.size());
	}

	return polyNum;
}


/**
Gets all the vertices attached to the face.
Uses the half edge pointers to get all vertices that make up this face
@param faceNum - face/polygon to get vertices from
@param vertexList - return list of vertex
*/
void Mesh::getFaceVertices( const FaceIndex faceNum, VertexIndexContainer& vertexList ) const
{
	const EdgeIndex firstEdge = (*face)[faceNum].anyHalfEdge;
	EdgeIndex edgeNum = firstEdge;

	// safely handly 0 vertex faces...
	if( edgeNum == MM_INVALID_INDEX )
		return;

	// gather vertices for polygons
	do {
		CORE_ASSERT( isValidVertex((*halfEdgeEdge)[edgeNum].startVertexIndex) );

		vertexList.push_back( (*halfEdgeEdge)[edgeNum].startVertexIndex );
		edgeNum = (*halfEdgeEdge)[edgeNum].nextEdge;
	} while( edgeNum != firstEdge );
}


/**
get all the edge attached to this vertex.
Uses the edge set to quickly get all edge this vertex is attached to
@param vertNum vertex to get edge from
@param edgeList return edge list
*/
void Mesh::getVertexEdges( const VertexIndex vertNum, EdgeIndexContainer& edgeList ) const
{
	// a deleted vertex has no edges
	if( !isValidVertex(vertNum) )
		return;

	std::set<EdgeIndex> edgeSet;

	VertexIndex curVert = vertNum;
	do {
		CORE_ASSERT( isValidVertex(curVert) );

		edgeSet.insert( (*halfEdge)[curVert].edgeIndexList.begin(),
			(*halfEdge)[curVert].edgeIndexList.end() );

		curVert = (*pointRep)[curVert].next;
	} while (vertNum != curVert );

	std::copy(	edgeSet.cbegin(), edgeSet.cend(),
				std::inserter( edgeList, edgeList.end() ) );

}

/**
Gets all the edges attached to the face.
Uses the half edge next pointers to get all edges
@param faceNum - face/polygon to get edges from
@param edgeList - return list of edges 
*/
void Mesh::getFaceEdges( const FaceIndex faceNum, EdgeIndexContainer& edgeList ) const {
	const EdgeIndex firstEdge = (*face)[faceNum].anyHalfEdge;
	EdgeIndex edgeNum = firstEdge;

	// safely handly 0 vertex faces...
	if( edgeNum == MM_INVALID_INDEX )
		return;

	// gather vertices for triangle
	do {
		edgeList.push_back( edgeNum );
		edgeNum = (*halfEdgeEdge)[edgeNum].nextEdge;
	} while( edgeNum != firstEdge );
}

/**
Gets all the faces surrounding (sharing edges) to specified face.
Uses the half edge pointers pair edges to get all faces surrounding
@param faceNum - face/polygon to get vertices from
@param faceList - return list of face
*/
void Mesh::getSurroundingFaces( const FaceIndex faceNum, FaceIndexContainer& faceList ) const {
	const EdgeIndex firstEdge = (*face)[faceNum].anyHalfEdge;
	EdgeIndex edgeNum = firstEdge;

	// gather edges for face
	do {
		EdgeIndexContainer::iterator edIt = (*halfEdgeEdge)[edgeNum].pairEdges.begin();
		while( edIt != (*halfEdgeEdge)[edgeNum].pairEdges.end() )
		{
			CORE_ASSERT( faceNum != (*halfEdgeEdge)[ (*edIt) ].faceIndex );

			faceList.push_back( (*halfEdgeEdge)[ (*edIt) ].faceIndex );
			++edIt;
		}
		edgeNum = (*halfEdgeEdge)[edgeNum].nextEdge;
	} while( edgeNum != firstEdge );
}

/**
finds a material by name. currently linear search if that proves slow (i.e. lots of materials)
a map could easily be implemented.
*/
MaterialIndex Mesh::findMaterial( const char* pName ) const {
	const MaterialElementsContainer& matCon = getMaterialContainer();
	// no names, then must be invalid material
	const NameMaterialElements* nameEle = matCon.getElements<const NameMaterialElements>();
	if( nameEle == 0 )
		return MM_INVALID_INDEX;

	// search for this name
	NameMaterialElements::const_iterator idenIt = nameEle->elements.begin();
	while( idenIt != nameEle->elements.end() ) {
		if( idenIt->matName == pName ) {
			return (MaterialIndex) (std::distance( nameEle->elements.begin(), idenIt ));
		}
		++idenIt;
	}

	// can't find it
	return MM_INVALID_INDEX;
}

Mesh* Mesh::clone() const {
	return CORE_NEW Mesh( *this );
}

/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
