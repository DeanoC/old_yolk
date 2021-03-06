#include "core/core.h"
#include "voxtree.h"
#include "proceduraltextureutils.h"
#include "procvoxtree.h"
/*
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

	return test;
}
*/


Vox::ProcVoxTree* VoxTreeTest() {
	Core::AABB bound( Math::Vector3(-12800,-12800,-12800), Math::Vector3(12800,12800,12800) );
	Vox::ProcVoxTree* test = CORE_NEW Vox::ProcVoxTree( bound );
/*
	uint32_t brickIndex = 1;
	for( float k = 0; k < 16; ++k ) {
		for( float j = 0; j < 16; ++j ) {
			for( float i = 0; i < 16; ++i ) {

				float val = FBm( Math::Vector3(i,j,k) * (1.f/16.f), 2.3f, 2 );

				if( val > 0.999f ) {
					test->insertPoint( Math::Vector3(i,j,k), brickIndex );
				}
			}
		}
	}

	test->pack();
*/
	return test;
}
