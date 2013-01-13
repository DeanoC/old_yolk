#include "cyberhackerspace.h"
#include "localworld/properties.h"
#include "player/player.h"
#include "baselvl.h"

BaseLvl::BaseLvl( SceneWorldPtr _world ) :
	world( _world ) {
    namespace arg = std::placeholders;

	loadTiles();

	environment.reset( ThingFactory::createEmptyLevel() );

	genFace( 3 );

	world->add( environment );
}

BaseLvl::~BaseLvl() {
	player = nullptr;
	world->remove( environment );
	environment.reset();
}

void BaseLvl::update( float timeMS ) {
}

void BaseLvl::loadTiles() {
	tiles.push_back( std::make_shared<Scene::Hier>( "blank_tile" ) );
}

void BaseLvl::genFace( int dim ) {

	levelGrid.resize( dim * dim );

	for( int y = 0; y < dim; ++y ) {
		for( int x = 0; x < dim; ++x ) {
			levelGrid[ (y * dim) + x ].reset( ThingFactory::createThingFromHier( tiles[0], TBC_WORLD ) );
		}
	}

}
