#pragma once
/** \file MeshModMesh.h
   A MeshMod Mesh.
   Holds vertices, faces, materials
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_MESH_H_ )
#define MESH_MOD_MESH_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined(MESH_MOD_INDEX_TYPES_H_)
#include "indextypes.h"
#endif
#if !defined(MESH_MOD_SCENE_OBJECT_H_)
#include "sceneobject.h"
#endif
#if !defined(MESH_MOD_POSITION_VERTEX_H_)
#include "positionvertex.h"
#endif
#if !defined(MESH_MOD_NORMAL_VERTEX_H_)
#include "normalvertex.h"
#endif
#if !defined(MESH_MOD_HALF_EDGE_VERTEX_H_)
#include "halfedgevertex.h"
#endif
#if !defined(MESH_MOD_POINT_REP_VERTEX_H_)
#include "pointrepvertex.h"
#endif
#if !defined(MESH_MOD_VERTEX_H_)
#include "vertex.h"
#endif
#if !defined(MESH_MOD_TRIANGLE_H_)
#include "triangle.h"
#endif
#if !defined(MESH_MOD_HALFEDGE_H_)
#include "halfedge.h"
#endif
#if !defined(MESH_MOD_FACE_H_)
#include "face.h"
#endif
#if !defined(MESH_MOD_MATERIAL_H_)
#include "material.h"
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
	Mesh presents a geometry mesh.
	The Mesh is main object used by the object system, a mesh has a list
	of vertices, edges, faces and materials that represent any 3D object.
	Once inside the mesh, indices are used to reference the source item,
	The source element arrays can be retrieved to get at the actual data.

	A quick overview of terminology
	Position -	Position is an attribute that every vertex MUST have,
				a number of vertices can share a 'similar' position, this
				means they are the same physical 3D space but with 
				different attributes (UV, normals).
	Vertex -	A vertex is an unique N-D space point, each vertex has a 
				point rep list that tell you of 'similar' vertices (share
				the same position).
				Vertices's can have there own list of elements.
	Edge -		A edge connects 2 vertices, at the moment it is always an
				edge of a polygon. A full edge is made of 2 half-edges,
				half edges are connected to one other half edge, each half
				edge is linked to a particular polygon.
				Edges can have there own list of elements.
	Half Edge -	Half edges connect polygons together and allow fast 
				topology ops, A Mesh only tracks half-edges and not edges.
				An half-edge by itself marks a not connected polygon.
				Currently support for more than 2 manifold surfaces is 
				unsupported at best and probably breaks things a lot.
	Polygon -	A Mesh supports polygons fully, but some of the algorithms
				require simplex convex polygons and triangles are your best
				bet for pulling out renderer data
	Triangle -	A polygon with 3 faces, always linear, simple and hardware
				friendly.
	Face -		Another name for polygon, 
				Faces have the own list of elements
	Material -	Possible to be renamed but essentially a list of elements
				that are generally indexed by face attributes. Currently
				used for materials but possible there are other useful 
				types.

	The implementation is spread over several files 
		mesh.cpp, vertex.cpp, edge.cpp
	*/
	class Mesh : public SceneObject
	{
	public:
		//! ctor.
		Mesh( const std::string& name );
		//! dtor.
		~Mesh();

		Mesh* clone() const;

		//-----------------------------------
		// Mesh creation/alteration functions
		//----------------------------------

		//! adds a position (does not check for duplicates), returns the index.
		VertexIndex addPosition( const float x, const float y, const float z );

		//! removes the indicated vertex (position + extra data at index ).
		void removeVertex(			const VertexIndex index );

		//! has the vertex been deleted, return false if deleted.
		bool isValidVertex(			const VertexIndex index ) const;

		//! clones a vertex and returns the clones vertex index.
		VertexIndex cloneVertex(	const VertexIndex vertexToCopy );

		//! add a polygon made of the specified indices, optional base vertex to offset indices.
		FaceIndex addPolygon(		const VertexIndexContainer& indices, 
									const VertexIndex baseVertex = 0);

		//! add a half edge between the 2 vertex indices (XvIndex) of the polygon (fIndex).
		EdgeIndex addHalfEdge(		const VertexIndex svIndex,
									const VertexIndex evIndex,
									const FaceIndex fIndex );

		//! build a point rep representation using fEpsilon to decide if position are 'similar'
		void createPointRep( float fEpsilon = s_epsilon1e_5 );

		//! breaks any paired edges among on all polygons.
		void breakAllPairEdges();

		//! finds and hooks up any unpaired edges that should be paired.
		void connectPairEdges();

		template< class T >
		VertexIndex  addVertexAttributeToFace(	const VertexIndex vIndex, 
												const FaceIndex fIndex, 
												ImplElements< T, Vertex_ >& eleContainer,
												const T& data );


		//-----------------------------------
		// Get access to the data held by this mesh
		//----------------------------------

		//! get read/write access to the meshes edge container
		EdgeElementsContainer& getEdgeContainer() { return edgeContainer; };
		//! get read only access to the meshes edge container
		const EdgeElementsContainer& getEdgeContainer() const { return edgeContainer; };
		//! get read/write access to the meshes vertex container
		VertexElementsContainer& getVertexContainer() { return vertexContainer; };
		//! get read only access to the meshes vertex container
		const VertexElementsContainer& getVertexContainer() const { return vertexContainer; };
		//! get read/write access to the meshes face container
		FaceElementsContainer& getFaceContainer() { return faceContainer; };
		//! get read only access to the meshes face container
		const FaceElementsContainer& getFaceContainer() const { return faceContainer; };
		//! get read write access meshes material container
		MaterialElementsContainer& getMaterialContainer() {	return materialContainer; };
		//! get read only access meshes material container
		const MaterialElementsContainer& getMaterialContainer() const {	return materialContainer; };

		//-----------------------------------
		// Topology access functions
		//----------------------------------
		
		//! retrieves all the edges making up a face.
		void getFaceEdges( const FaceIndex faceNum, EdgeIndexContainer& edgeList ) const;
		//! retrieves all the vertices making up a face.
		void getFaceVertices( const FaceIndex faceNum, VertexIndexContainer& vertexList ) const;
		//! retrieves the face indices of all faces that surround (touch) the input face.
		void getSurroundingFaces( const FaceIndex faceNum, FaceIndexContainer& faceList ) const;
		//! retrieves all the edges that connect to/from the specified vertex.
		void getVertexEdges( const VertexIndex vertNum, EdgeIndexContainer& edgeList ) const;
		//! get all vertices at same position as vertNum (using pointrep for speed).
		void getSimilarVertices( const VertexIndex vertNum, VertexIndexContainer& vertexList ) const;
		//! are the two position indices at the same position (are 'similar').
		bool similarPosition( const VertexIndex i0, const VertexIndex i1) const;
		//! scan the vertex index list to see if have a samePosition return the index if we have else MeshModM_INVALID_INDEX
		VertexIndex hasPosition( const VertexIndex i0, const VertexIndexContainer& vertexList ) const;

		//! find a particular material by name (note currently linear search assumes few materials...)
		MaterialIndex findMaterial( const char* pName ) const;

		//-----------------------------------
		// Simple little algortihms that operate on a mesh
		// Most operations live in the MeshOps library
		//----------------------------------

		//! collapse all 'similar' position to a single (unspecified) position.
		//! WARNING Dangerous as the non-position data will be lost
		void removeAllSimilarPositions( float fEpsilon );

	protected:
		// for clone only
		Mesh( const Mesh& rhs );
		//--------- Containers ----------------//
		//! half edge elements container.
		EdgeElementsContainer		edgeContainer;
		//! vertex element container.
		VertexElementsContainer		vertexContainer;
		//! face elements container.
		FaceElementsContainer		faceContainer;
		//! material elements container.
		MaterialElementsContainer	materialContainer;

		//--------- Predefined Face elements pointer ----------------//
		//! face edge indices
		FaceFaceElements*			face;

		//--------- Predefined Edge elements pointer ----------------//
		//! edge half edge
		HalfEdgeEdgeElements*		halfEdgeEdge;

		//--------- Predefined Vertex elements pointer ----------------//
		//! position there is ALWAYS a 1 to 1 ratio with ALL other elements,
		//! if no data is available fill with s_floatMarker.
		PositionVertexElements*		position;
		//! if position is the same but has been duplicated PointRep contain a list of indices
		//! last one points to itself.
		PointRepVertexElements*		pointRep;
		//! vertex element half edges index.
		HalfEdgeVertexElements*		halfEdge;
	};

	typedef std::shared_ptr<Mesh> MeshPtr;

	template< class T >
	inline VertexIndex Mesh::addVertexAttributeToFace(	const VertexIndex vPosIndex, 
														const FaceIndex fIndex, 
														ImplElements< T, Vertex_ >& eleContainer,
														const T& data ) {
		// first scan the face for the position we have an index for
		VertexIndexContainer faceVertices;
		getFaceVertices( fIndex, faceVertices );
		VertexIndex vfPosIndex = hasPosition( vPosIndex, faceVertices );

		// if this fires the position passed in doesn't belong to the face
		// passed it, which makes this whole thing useless...
		CORE_ASSERT( vfPosIndex != MM_INVALID_INDEX );

		// does this face vertex already have the attribute data
		if( eleContainer[ vfPosIndex ].equal( data ) == true ) {
			// it does, so nothing to do just return the index
			return vfPosIndex;
		} else {
			// nope, if it has a valid index 
			// clone the old vertex and insert the data
			// else just make it a valid index by sticking 
			// that data in there
			if( eleContainer[ vfPosIndex ].isValid() ) {
				VertexIndex vIndex = cloneVertex( vfPosIndex );
				eleContainer[ vIndex ] = data;
				return vIndex;
			} else {
				eleContainer[ vfPosIndex ] = data;
				return vfPosIndex;
			}
		}
	}
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
