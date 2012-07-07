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
#include <meshmod/edge.h>
#include <meshmod/halfedge.h>
#include <meshmod/materialface.h>
#include "meshsorter.h"

namespace MeshOps {

MeshSorter::MeshSorter( const MeshMod::MeshPtr& _mesh ) :
	mesh( _mesh )
{
}

namespace {
class  MatSort_Local
{
public:
	MeshMod::MaterialFaceElements* surfElement;

	/// function to sort surface by index
	bool operator() ( const MeshMod::FaceData::SortMapper& a, const MeshMod::FaceData::SortMapper& b) const {
		return ( (*surfElement)[a.index].surfaceIndex < (*surfElement)[b.index].surfaceIndex );
	}
};

//! sorts things based on the float value, you can then
//! get the original version via the index
struct AxisSortCompare {
	MeshMod::PositionVertexElements* posElement;
	MeshMod::VertexData::Position::AXIS axis;

	/// function to sort by position[axis]
	bool operator() ( const MeshMod::VertexData::SortMapper& a, const MeshMod::VertexData::SortMapper& b) const {
		return ( (*posElement)[a.index].get(axis) < (*posElement)[b.index].get(axis) );
	}
};

}
/**
Sorts all face by face material index.
Adds a material index field and sorts it into material ordered list
*/
MeshMod::SortMapperFaceElements* MeshSorter::sortFacesByMaterialIndex() {
	using namespace MeshMod;
	// we create a sort mapper, that which face would be here if sort by
	// indicated type
	// i.e. Sorted.face0 = Face[ SortMapper[0] ]

	FaceElementsContainer& faceCon = mesh->getFaceContainer();
	SortMapperFaceElements* sortEle = faceCon.getOrAddElements<SortMapperFaceElements>( "MaterialIndex" );

	// fill sort mapper with identity mapping
	SortMapperFaceElements::iterator idenIt = sortEle->elements.begin();
	while( idenIt != sortEle->elements.end() ) {
		(*idenIt).index = (unsigned int) std::distance(sortEle->elements.begin(), idenIt);
		++idenIt;
	}

	MatSort_Local sorter;
	sorter.surfElement = faceCon.getElements<MaterialFaceElements>();

	// if we have no surface materinal indices ther is no sorting to do
	if( sorter.surfElement != 0) {
		std::sort( sortEle->elements.begin(), sortEle->elements.end(), sorter );
	}

	return sortEle;
}

/**
*/
MeshMod::SortMapperVertexElements* MeshSorter::sortVerticesByAxis( MeshMod::VertexData::Position::AXIS axis ) {
	using namespace MeshMod;
	// we create a sort mapper, that which vertex would be here if sort by
	// indicated type
	// i.e. Sorted.vertex0 = Vertex[ SortMapper[0] ]

	static const char* axisNames[] = { "X axis", "Y axis", "Z axis" };

	VertexElementsContainer& vertCon = mesh->getVertexContainer();
	SortMapperVertexElements* sortEle = vertCon.getOrAddElements<SortMapperVertexElements>( axisNames[axis] );

	// fill sort mapper with identity mapping
	SortMapperVertexElements::iterator idenIt = sortEle->elements.begin();
	while( idenIt != sortEle->elements.end() ) {
		(*idenIt).index = (unsigned int) std::distance(sortEle->elements.begin(), idenIt);
		++idenIt;
	}

	AxisSortCompare sorter;
	sorter.posElement = vertCon.getElements<PositionVertexElements>();
	sorter.axis = axis;
	std::sort( sortEle->elements.begin(), sortEle->elements.end(), sorter );

	return sortEle;
}


}