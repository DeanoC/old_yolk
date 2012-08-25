#include "vtdfh.h"
#include "testlvl.h"

TestLvl::TestLvl( SceneWorldPtr _world ) :
	world( _world ) {

	land = std::make_shared<Thing>( std::make_shared<Scene::Hier>( "basic" ) );
	world->add( land );
}

TestLvl::~TestLvl() {
	world->remove( land );
}