#include "vtdfh.h"
#include "testlvl.h"

TestLvl::TestLvl( SceneWorldPtr _world ) :
	world( _world ) {

	land = std::make_shared<Thing>( std::make_shared<Scene::Hier>( "basic" ) );
	world->add( land ); // render object
	world->add( static_cast<Updatable*>( this ) ); // updateble interface
}

TestLvl::~TestLvl() {
	world->remove( static_cast<Updatable*>( this ) );
	world->remove( land );
}
void TestLvl::update( float timeMS ) {
}