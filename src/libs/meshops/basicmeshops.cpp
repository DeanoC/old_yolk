/** \file meshsorter.cpp
   Geometry Optimiser Mesh sorting operations.
   (c) 2012 Deano Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshops.h"
#include <cassert>
#include <algorithm>
#include <set>
#include <meshmod/mesh.h>
#include <meshmod/positionvertex.h>
#include <meshmod/halfedgevertex.h>
#include <meshmod/pointrepvertex.h>
#include <meshmod/halfedge.h>
#include <core/aabb.h>
#include "basicmeshops.h"

namespace MeshOps {

DECLARE_EXCEPTION( BasicMeshOp, "Cannot process mesh with this op" );


BasicMeshOps::BasicMeshOps( const MeshMod::MeshPtr& _mesh ) :
	mesh( _mesh )
{
}

/**
Compute and stores triangle plane equations.
Add a face element with each faces plane equation, will work for polygons but non-planar 
polygons may produce incorrect plane equations.

If any lines or point faces are in the mesh the planeequation for that face will be the default
*/
void BasicMeshOps::computeFacePlaneEquations( bool replaceExisting, bool zeroBad, bool fixBad ) {
	using namespace MeshMod;
	using namespace Math;

	VertexElementsContainer& vertCon = mesh->getVertexContainer();
	FaceElementsContainer& faceCon = mesh->getFaceContainer();

	// create plane equation face data if nessecary
	PlaneEquationFaceElements* peEle = faceCon.getElements<PlaneEquationFaceElements>();
	if( peEle != 0 && replaceExisting == false ) {
		return; // we already have normal and don't want to overwrite existing so just return
	} else {
		peEle = faceCon.getOrAddElements<PlaneEquationFaceElements>();
	}

	PositionVertexElements* posEle = vertCon.getElements<PositionVertexElements>();

	PlaneEquationFaceElements::iterator faceIt = peEle->elements.begin();
	while( faceIt != peEle->elements.end() ) {
		const FaceIndex faceNum = (FaceIndex) std::distance( peEle->elements.begin(), faceIt );

		VertexIndexContainer faceVert;
		mesh->getFaceVertices( faceNum, faceVert );

		// only makes sense for triangles or polygons (TODO should use newell method for polygons)
		if( faceVert.size() >= 3) {
			Vector3 a( (*posEle)[faceVert[0]].x, (*posEle)[faceVert[0]].y, (*posEle)[faceVert[0]].z );
			Vector3 b( (*posEle)[faceVert[1]].x, (*posEle)[faceVert[1]].y, (*posEle)[faceVert[1]].z );
			Vector3 c( (*posEle)[faceVert[2]].x, (*posEle)[faceVert[2]].y, (*posEle)[faceVert[2]].z );

			Vector3 dba = b - a;
			Vector3 dbc = b - c;

			Vector3 cross = Cross( dba, dbc );
			Vector3 nc = Normalise( cross );

			if( IsFinite(nc) ) {
				// d = distance along the plane normal to a vertex (all are on the plane if planar)
				float d = Dot(nc,b);
				(*peEle)[ faceNum ].planeEq = Promote(nc,-d);
			} else {
				if( zeroBad ) {
					(*peEle)[ faceNum ].planeEq = Vector4(0,0,0,0);
				} else if( fixBad ) {
					// polygon has degenerated to a line or point
					// therefore fake a normal (any will do) and project a vertex
					// it will give a plane going throught the line or point (best we can do)
					nc = Vector3(1,0,0); // any normal would do, randome would be better tbh...
					float d = Dot(nc,b);
					(*peEle)[ faceNum ].planeEq = Promote(nc,-d);
				} else {
					CoreThrowException( BasicMeshOp, "Bad plane equation" );
				}
			}
		} else {
			if( zeroBad ) {
				(*peEle)[ faceNum ].planeEq = Vector4(0,0,0,0);
			} else if( fixBad && faceVert.size() >= 1 ) {
				Vector3 a( (*posEle)[faceVert[0]].x, (*posEle)[faceVert[0]].y, (*posEle)[faceVert[0]].z );
				// line or point cannot have a plane equation
				// therefore fake a normal (any will do) and project a vertex
				// it will give a plane going throught the line or point (best we can do)
				Vector3 nc(1,0,0); // any normal would do, randome would be better tbh...
				float d = Dot(nc,a);
				(*peEle)[ faceNum ].planeEq = Promote(nc,-d);
			}
		}

		++faceIt;
	}
}

/**
Computes and store per vertex normals.
If the object already has vertex normals they will be kept if replaceExising == false.
Simple average lighting normal.
*/
void BasicMeshOps::computeVertexNormals( bool replaceExisting ) {
	using namespace MeshMod;
	VertexElementsContainer& vertCon = mesh->getVertexContainer();
	FaceElementsContainer& faceCon = mesh->getFaceContainer();
	
	NormalVertexElements* normEle = vertCon.getElements<NormalVertexElements>();
	if( normEle != 0 && replaceExisting == false ) {
		return; // we already have normal and don't want to overwrite existing so just return
	} else {
		normEle = vertCon.getOrAddElements<NormalVertexElements>();
	}

	// clear normals
	std::fill( normEle->begin(), normEle->end(), VertexData::Normal(0,0,0) );

	computeFacePlaneEquations( replaceExisting );

	PlaneEquationFaceElements* planeEle = faceCon.getElements<PlaneEquationFaceElements>();
	assert( planeEle != 0 );

	// add the poly equation to every vertex in every face
	PlaneEquationFaceElements::const_iterator faceIt = planeEle->cbegin();
	while( faceIt != planeEle->cend() ) {
		const FaceIndex faceNum = planeEle->distance<FaceIndex>( faceIt );

		VertexIndexContainer vertList;
		mesh->getFaceVertices( faceNum, vertList );
		VertexIndexContainer::const_iterator vtIt = vertList.cbegin();
		while( vtIt != vertList.cend() ) {
			VertexIndexContainer simVertList;
			mesh->getSimilarVertices( *vtIt, simVertList );			
			VertexIndexContainer::iterator svtIt = simVertList.begin();
			while( svtIt != simVertList.end() ) {
				(*normEle)[*svtIt].x += (*faceIt).planeEq[0];
				(*normEle)[*svtIt].y += (*faceIt).planeEq[1];
				(*normEle)[*svtIt].z += (*faceIt).planeEq[2];
				++svtIt;
			}
			++vtIt;
		}
		++faceIt;
	}

	// now normalise all normals
	NormalVertexElements::iterator normIt = normEle->begin();
	while( normIt != normEle->end() ) {
		const VertexIndex vertNum = normEle->distance<VertexIndex>( normIt );
		if( !mesh->isValidVertex(vertNum) ) {
			// invalid vertex so set normal to NAN
			(*normIt).x = s_floatMarker;
			(*normIt).y = s_floatMarker;
			(*normIt).z = s_floatMarker;
		} else
		{
			//valid vertex (not deleted)
			float norm = ((*normIt).x * (*normIt).x) +
						((*normIt).y * (*normIt).y) +
						((*normIt).z * (*normIt).z);
			norm = sqrtf(norm);

			(*normIt).x = -(*normIt).x / norm;
			(*normIt).y = -(*normIt).y / norm;
			(*normIt).z = -(*normIt).z / norm;
		}
		++normIt;
	}

}
/**
Computes and returns the axis aligned box from the meshes position
*/
void BasicMeshOps::computeAABB( Core::AABB& aabb ) {
	using namespace MeshMod;
	const VertexElementsContainer& vertCon = mesh->getVertexContainer();
	const PositionVertexElements* posEle = vertCon.getElements<PositionVertexElements>();

	aabb = Core::AABB(); // reset aabb
	// now normalise all normals
	PositionVertexElements::const_iterator posIt = posEle->cbegin();
	while( posIt != posEle->cend() ) {
		aabb.expandBy( Math::Vector3( (*posIt).x,  (*posIt).y,  (*posIt).z ) );
		++posIt;
	}
}

/**
Triangulates all faces into triangles. points and lines will be untouched
All faces must be simple convex polygons (no complex polygons yet), post this call all faces will have <= 3 vertices
*/
void BasicMeshOps::triangulate() {
	using namespace MeshMod;
	FaceElementsContainer& faceCon = mesh->getFaceContainer();
	EdgeElementsContainer& edgeCon = mesh->getEdgeContainer();

	FaceFaceElements* faceEle = faceCon.getElements<FaceFaceElements>();
	HalfEdgeEdgeElements* halfEdgeEle = edgeCon.getElements<HalfEdgeEdgeElements>();

	const unsigned int origFaceCount = (unsigned int) faceEle->elements.size();

	for(FaceIndex faceNum = 0; faceNum < origFaceCount;faceNum++) {
		EdgeIndexContainer faceEdges;

		mesh->getFaceEdges( faceNum, faceEdges );

		if( faceEdges.size() > 3 ) {
			// first 2 edges from face edges
			EdgeIndex e0 = faceEdges[0];
			EdgeIndex e1 = faceEdges[1];
			EdgeIndex e2;

			unsigned int origEdgeNum = 2;

			for(unsigned int i=0;i < faceEdges.size() - 2;i++) {
				FaceIndex triNum;

				if( i != 0) {
					triNum = faceCon.cloneElement( faceNum ); // clone original
				} else {
					// reuse original poly if first triangle
					triNum = faceNum;
				}

				// start edge
				if ( i > 0 ) {
					// insert a new half edge (reverse the last tri last edge)
					e0 = mesh->addHalfEdge(	(*halfEdgeEle)[e0].endVertexIndex,
											(*halfEdgeEle)[e0].startVertexIndex,
											triNum );
					// check
					assert( (*halfEdgeEle)[e0].endVertexIndex ==
							(*halfEdgeEle)[e1].startVertexIndex );
				} else {
					// just check
					assert( (*halfEdgeEle)[e0].endVertexIndex ==
							(*halfEdgeEle)[e1].startVertexIndex );
				}

				// end edge (do we need a new one or can we use the original polys edge)
				if ( i != faceEdges.size() - 3 ) {
					// insert a new edge
					e2 = mesh->addHalfEdge(	(*halfEdgeEle)[e1].endVertexIndex,
											(*halfEdgeEle)[e0].startVertexIndex,
											triNum );
				} else {
					e2 = faceEdges[ faceEdges.size() - 1 ];
					assert( (*halfEdgeEle)[e1].endVertexIndex ==
							(*halfEdgeEle)[e2].startVertexIndex );
				}

				(*faceEle)[triNum].anyHalfEdge = e0; // repoint first edge
				// make edge point to correct triangle
				(*halfEdgeEle)[e0].faceIndex = triNum;
				(*halfEdgeEle)[e1].faceIndex = triNum;
				(*halfEdgeEle)[e2].faceIndex = triNum;

				// connect edges (complete the cycle list)
				(*halfEdgeEle)[e0].nextEdge = e1;
				(*halfEdgeEle)[e1].nextEdge = e2;
				(*halfEdgeEle)[e2].nextEdge = e0;

				// update edge indices (next edge)
				e0 = e2;
				e1 = faceEdges[origEdgeNum++];
			}
		}
	}
}

// post will have no n-gons, points, lines, triangles and quads will remain
// TODO restitch planar triangle pairs to quads
// TODO non-planar quads decompose to triangles
void BasicMeshOps::quadOrTriangulate() {
	using namespace MeshMod;
	FaceElementsContainer& faceCon = mesh->getFaceContainer();
	EdgeElementsContainer& edgeCon = mesh->getEdgeContainer();

	FaceFaceElements* faceEle = faceCon.getElements<FaceFaceElements>();
	HalfEdgeEdgeElements* halfEdgeEle = edgeCon.getElements<HalfEdgeEdgeElements>();

	const unsigned int origFaceCount = (unsigned int) faceEle->elements.size();

	for(FaceIndex faceNum = 0; faceNum < origFaceCount;faceNum++) {
		EdgeIndexContainer faceEdges;

		mesh->getFaceEdges( faceNum, faceEdges );

		if( faceEdges.size() > 4 ) {
			// first 2 edges from face edges
			EdgeIndex e0 = faceEdges[0];
			EdgeIndex e1 = faceEdges[1];
			EdgeIndex e2;

			unsigned int origEdgeNum = 2;

			for(unsigned int i=0;i < faceEdges.size() - 2;i++) {
				FaceIndex triNum;

				if( i != 0) {
					triNum = faceCon.cloneElement( faceNum ); // clone original
				} else {
					// reuse original poly if first triangle
					triNum = faceNum;
				}

				// start edge
				if ( i > 0 ) {
					// insert a new half edge (reverse the last tri last edge)
					e0 = mesh->addHalfEdge(	(*halfEdgeEle)[e0].endVertexIndex,
											(*halfEdgeEle)[e0].startVertexIndex,
											triNum );
					// check
					assert( (*halfEdgeEle)[e0].endVertexIndex ==
							(*halfEdgeEle)[e1].startVertexIndex );
				} else {
					// just check
					assert( (*halfEdgeEle)[e0].endVertexIndex ==
							(*halfEdgeEle)[e1].startVertexIndex );
				}

				// end edge (do we need a new one or can we use the original polys edge)
				if ( i != faceEdges.size() - 3 ) {
					// insert a new edge
					e2 = mesh->addHalfEdge(	(*halfEdgeEle)[e1].endVertexIndex,
											(*halfEdgeEle)[e0].startVertexIndex,
											triNum );
				} else {
					e2 = faceEdges[ faceEdges.size() - 1 ];
					assert( (*halfEdgeEle)[e1].endVertexIndex ==
							(*halfEdgeEle)[e2].startVertexIndex );
				}

				(*faceEle)[triNum].anyHalfEdge = e0; // repoint first edge
				// make edge point to correct triangle
				(*halfEdgeEle)[e0].faceIndex = triNum;
				(*halfEdgeEle)[e1].faceIndex = triNum;
				(*halfEdgeEle)[e2].faceIndex = triNum;

				// connect edges (complete the cycle list)
				(*halfEdgeEle)[e0].nextEdge = e1;
				(*halfEdgeEle)[e1].nextEdge = e2;
				(*halfEdgeEle)[e2].nextEdge = e0;

				// update edge indices (next edge)
				e0 = e2;
				e1 = faceEdges[origEdgeNum++];
			}
		}
	}
}

/**
Computes and store per vertex normals.
Based on mesh libs version with fixing and handling of slivers
*/
void BasicMeshOps::computeVertexNormalsEx( bool replaceExisting, bool zeroBad, bool fixBad ) {
	assert( false );
	if( zeroBad && fixBad ) {
		CoreThrowException( BasicMeshOp, "Cannot both zero and fix bad normals" );
	}

	using namespace MeshMod;
	using namespace Math;

	VertexElementsContainer& vertCon = mesh->getVertexContainer();
	FaceElementsContainer& faceCon = mesh->getFaceContainer();
	EdgeElementsContainer& edgeCon = mesh->getEdgeContainer();

	NormalVertexElements* normEle = vertCon.getElements<NormalVertexElements>();
	if( normEle != 0 && replaceExisting == false ) {
		return; // we already have normal and don't want to overwrite existing so just return
	} else {
		normEle = vertCon.getOrAddElements<NormalVertexElements>();
	}

	// clear normals
	std::fill( normEle->elements.begin(), normEle->elements.end(), VertexData::Normal(0,0,0) );
	computeFacePlaneEquations( replaceExisting, zeroBad, fixBad );

	const HalfEdgeEdgeElements*		heEle = edgeCon.getElements<HalfEdgeEdgeElements>();
	const PlaneEquationFaceElements* planeEle = faceCon.getElements<PlaneEquationFaceElements>();
	assert( planeEle != 0 );

	EdgeIndexContainer edgeContainer;
	edgeContainer.reserve( 10 );

	std::vector<VertexData::Normal>::iterator normIt = normEle->elements.begin();
	// each normal try to generate a fair normal (TODO smoothing group faces)
	// fix or zero bad normals, fix try using simplier plane equation generator
	// TODO if this fix fails try mesh libs average of existing normals?
	while( normIt != normEle->elements.end() ) {

		// get the vertex and edges conected to this vertex
		const VertexIndex vertexIndex = (VertexIndex) std::distance( normEle->elements.begin(), normIt );
		edgeContainer.clear();
		mesh->getVertexEdges( vertexIndex, edgeContainer );
		EdgeIndexContainer::const_iterator edgeIt = edgeContainer.begin();

		Math::Vector3 vertexNormal(0,0,0);

		// TODO smoothing groups
		while( edgeIt != edgeContainer.end() ) {

			const EdgeData::HalfEdge& he = (*heEle)[ *edgeIt ];
			const FaceData::PlaneEquation& pe = (*planeEle)[ he.faceIndex ];
			Math::Vector3 localNormal = Math::Vector3( pe.planeEq.x, pe.planeEq.y, pe.planeEq.z );

			// get opposing indices
			const EdgeIndex i1 = ( (*edgeIt) + 1 ) % (EdgeIndex) edgeContainer.size();
			const EdgeIndex i2 = ( (*edgeIt) + 2 ) % (EdgeIndex) edgeContainer.size();
			const EdgeData::HalfEdge& he1 = (*heEle)[ i1 ];
			const EdgeData::HalfEdge& he2 = (*heEle)[ i2 ];
			const FaceData::PlaneEquation& pe1 = (*planeEle)[ he1.faceIndex ];
			const FaceData::PlaneEquation& pe2 = (*planeEle)[ he2.faceIndex ];

			Math::Vector3 e1 = Normalise( Math::Vector3( pe1.planeEq.x, pe1.planeEq.y, pe1.planeEq.z ) );
			Math::Vector3 e2 = Normalise( Math::Vector3( pe2.planeEq.x, pe2.planeEq.y, pe2.planeEq.z ) );

			// compute the angle and only accumulate at non-sliver angles
			const float angle = std::acos( -Dot( e1, e2 ) / ( Math::Length(e1)* Math::Length(e2) ) );
			if( Core::isfinite( angle ) ) {
				// accumulate the normal
				vertexNormal += localNormal * angle;
			}

			++edgeIt;
		}

		// normalise it
		vertexNormal = Math::Normalise( vertexNormal );

		// check it's finite
		if( !IsFinite( vertexNormal ) ){
			// either fix it using simplier plane equation average, zero or ignore
			if( zeroBad ) {
				vertexNormal = Math::Vector3(0,0,0);
			} else if( fixBad ) {
				EdgeIndexContainer::const_iterator edgeIt = edgeContainer.begin();
				vertexNormal = Math::Vector3(0,0,0);
				while( edgeIt != edgeContainer.end() ) {
					const EdgeData::HalfEdge& he = (*heEle)[ *edgeIt ];
					const FaceData::PlaneEquation& pe = (*planeEle)[ he.faceIndex ];
					vertexNormal += Math::Vector3( pe.planeEq.x, pe.planeEq.y, pe.planeEq.z );
					++edgeIt;
				}
				// normalise it
				vertexNormal = Math::Normalise( vertexNormal );
			}
		}

		// TODO remove denormals here at source, to ensure none enter the pipe at source
		// check for denormal components
//		for( mt_uint component = 0; component < 3; ++component ) {
//			if( !std::isnormal( normal[component] ) )
//				normal[component] = mt_float( 0 );			
//		}

		(*normIt).x += vertexNormal.x;
		(*normIt).y += vertexNormal.y;
		(*normIt).z += vertexNormal.z;

		++normIt;
	}
}

} // end namespace