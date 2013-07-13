#include "core/core.h"
#include "voxtree.h"

Vox::Tree* VoxTreeTest() {
	Core::AABB bound( Math::Vector3(-128,-128,-128), Math::Vector3(128,128,128) );
	Vox::Tree* test = CORE_NEW Vox::Tree( bound );

	Vox::Brick* brick;
	uint32_t brickIndex = test->allocateBrick( &brick );
	brick->splitable = false;
	for( float k = 0; k < 32; ++k ) {
		for( float j = 0; j < 32; ++j ) {
			for( float i = 0; i < 32; ++i ) {
				if( i == 0 && j == 0 && k == 0 ) continue;
				test->insertPoint( Math::Vector3(i,j,k), brickIndex );
			}
		}
	}
	// floor
	for( float k = -32; k < 0; ++k ) {
		for( float i = -32; i < 0; ++i ) {
			test->insertPoint( Math::Vector3(i,-1, k), brickIndex );
		}
	}

	test->pack();

	// make a test collision rep
	test->visitLeaves( 
		// cull func
		[]( const Vox::Tree::VisitHelper& _helper, const Vox::Node& _node, const Core::AABB& _aabb ) -> bool {
			return false; // no culling for static collision rep
		}, 
		// leaf visit func
		[]( const Vox::Tree::VisitHelper& _helper, const Vox::Node& _node, const Core::AABB& _aabb ) -> void {
		}
	);
	return test;
}