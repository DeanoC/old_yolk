#include "core/core.h"
#include "voxtree.h"

Vox::Tree* VoxTreeTest() {
	Core::AABB bound( Math::Vector3(-128,-128,-128), Math::Vector3(128,128,128) );
	Vox::Tree* test = CORE_NEW Vox::Tree( bound );

	Vox::Brick* brick;
	uint32_t brickIndex = test->allocateBrick( &brick );
	brick->splitable = false;
	for( float k = 1; k < 21; ++k ) {
		for( float j = 0; j < 8; ++j ) {
			for( float i = 0; i < 20; ++i ) {
				test->insertPoint( Math::Vector3(i,j,k), brickIndex );
			}
		}
	}
	test->pack();
	return test;
}