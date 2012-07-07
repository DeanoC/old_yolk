/** \file basicmeshops.h
	
   (c) 2012 Dean Calver
 */

#if !defined( GOOPERATIONS_BASICMESHOPS_H_ )
#define GOOPERATIONS_BASICMESHOPS_H_

namespace MeshOps {
	/**
		Basic Mesh Ops performs a bunch of common simple operations a mesh
	*/
	class BasicMeshOps {
	public:
		BasicMeshOps( const MeshMod::MeshPtr& _mesh );

		//! compute a per-face plane equation
		void computeFacePlaneEquations( bool replaceExisting = true, bool zeroBad = false, bool fixBad = true );

		//! generates a basic vertex normal set. optionally replace any existing normals
		void computeVertexNormals( bool replaceExisting = true );

		//!
		void computeVertexNormalsEx( bool replaceExisting = true, bool zeroBad = false, bool fixBad = true );

		//! runs a simple fan based triangulation algorithm on the mesh.
		void triangulate();
	
		//! runs a simple n-gon to triangles, leaving others (inlcuding quads) alone
		void quadOrTriangulate();

	private:
		MeshMod::MeshPtr	mesh;
	};
};


#endif