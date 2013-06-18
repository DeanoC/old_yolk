#include "core/core.h"
#include "voxtree.h"

void VoxTreeTest() {
	Core::AABB bound( Math::Vector3(-128,-128,-128), Math::Vector3(128,128,128) );
	Vox::Tree test( bound );

	Vox::Brick* brick;
	uint32_t brickIndex = test.allocateBrick( &brick );
	brick->splitable = false;
	for( float i = -4; i < 4; ++i ) {
		test.insertPoint( Math::Vector3(i,i,i), brickIndex );
	}
	test.pack();
}