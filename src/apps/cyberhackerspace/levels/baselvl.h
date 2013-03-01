#ifndef YOLK_TESTLVL_H_
#define YOLK_TESTLVL_H_ 1

#include "scene/hier.h"
#include "localworld/levelthing.h"

class GenericMarker {};
class Player;

class BaseLvl {
public:
	BaseLvl( SceneWorldPtr _world );
	~BaseLvl();
	
	void update( float timeS );

private:
	void loadTiles();
	void genFace( int dim );

	SceneWorldPtr 	world;
	LevelThingPtr	environment;

	typedef std::vector<GenericMarker> 		GenericMarkerVector;

	GenericMarkerVector 					genericMarkers;
	std::shared_ptr<Player>					player;

	std::vector<Scene::HierPtr>				tiles;

	std::vector< ThingPtr >					levelGrid;
};

#endif