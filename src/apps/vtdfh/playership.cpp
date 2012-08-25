#include "vtdfh.h"
#include "playership.h"


PlayerShip::PlayerShip( SceneWorldPtr _world ) :
	world( _world ) {

	ship = std::make_shared<Thing>( std::make_shared<Scene::Hier>( "basic" ) );
	world->add( ship );
}

PlayerShip::~PlayerShip() {
	world->remove( ship );
}