#include "core/core.h"
#include "voxtree.h"

Vox::Tree* VoxTreeTest() {
	Core::AABB bound( Math::Vector3(-128,-128,-128), Math::Vector3(128,128,128) );
	Vox::Tree* test = CORE_NEW Vox::Tree( bound );

	Vox::Brick* brick;
	uint32_t brickIndex = test->allocateBrick( &brick );
	brick->splitable = false;
	for( float k = 0; k < 64; ++k ) {
		for( float j = 0; j < 64; ++j ) {
			for( float i = 0; i < 64; ++i ) {
				if( i == 0 && j == 0 && k == 0 ) continue;
				test->insertPoint( Math::Vector3(i,j,k), brickIndex );
			}
		}
	}
	// floor
	for( float k = -32; k < 32; ++k ) {
		for( float i = -32; i < 32; ++i ) {
			test->insertPoint( Math::Vector3(i,-1, k), brickIndex );
		}
	}

	test->pack();
	return test;
}