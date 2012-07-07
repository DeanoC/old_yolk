/** \file meshsorter.h
	
   (c) 2012 Dean Calver
 */

#if !defined( GOOPERATIONS_MESHSORTER_H_ )
#define GOOPERATIONS_MESHSORTER_H_


namespace MeshOps {
	/**
		MeshSorter performs a variety of non destructive sorts on a mesh
	*/
	class MeshSorter {
	public:

		MeshSorter( const MeshMod::MeshPtr& _mesh );

		MeshMod::SortMapperFaceElements* sortFacesByMaterialIndex();
		MeshMod::SortMapperVertexElements* sortVerticesByAxis( MeshMod::VertexData::Position::AXIS axis );

	private:
		const MeshMod::MeshPtr	mesh;
	};
};

#endif