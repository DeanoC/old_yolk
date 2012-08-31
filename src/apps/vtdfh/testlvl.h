#ifndef YOLK_TESTLVL_H_
#define YOLK_TESTLVL_H_ 1

#include "localworld/levelthing.h"
#include "indigoweakspider.h"

class PlayerStartMarker {
public:
	PlayerStartMarker( Core::TransformNode* _node ) : transform( _node ){}
	Core::TransformNode* transform;
};
class GenericMarker {};
class HoverTank;

class TestLvl {
public:
	TestLvl( SceneWorldPtr _world );
	~TestLvl();
	
	void update( float timeS );

	void spawnPlayer( int _localPlayerNum );
private:
	void decodeLevelProperties( LevelThing* thing, std::unordered_map<std::string, int>& nameMap, 
										const Core::BinPropertyResourcePtr& binProp, Core::TransformNode * transformNode );
	void decodeMarkerProperties( LevelThing* thing, std::unordered_map<std::string, int>& nameMap, 
										const Core::BinPropertyResourcePtr& binProp, Core::TransformNode * transformNode );
	void decodeEnemyProperties( LevelThing* thing, std::unordered_map<std::string, int>& nameMap, 
										const Core::BinPropertyResourcePtr& binProp, Core::TransformNode * transformNode );

	SceneWorldPtr 	world;
	LevelThingPtr	land;

	typedef std::vector<PlayerStartMarker> 	PlayerStartVector;
	typedef std::vector<GenericMarker> 		GenericMarkerVector;
	typedef std::vector<IndigoWeakSpiderPtr>	EnemyVector; // TODO generise to more than weak spiders

	PlayerStartVector 						playerStarts;
	GenericMarkerVector 					genericMarkers;
	EnemyVector								ownedEnemies;
	std::shared_ptr<HoverTank>				player;


};

#endif