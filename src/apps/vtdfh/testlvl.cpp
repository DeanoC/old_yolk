#include "vtdfh.h"
#include "localworld/properties.h"
#include "playership.h"
#include "hovertank.h"
#include "indigoweakspider.h"
#include "testlvl.h"

TestLvl::TestLvl( SceneWorldPtr _world ) :
	world( _world ) {
    namespace arg = std::placeholders;

	land.reset(  ThingFactory::createLevelFromHier( std::make_shared<Scene::Hier>( "begin_luck" ), NewThingId(), 
						std::bind( &TestLvl::decodeLevelProperties, this, arg::_1, arg::_2, arg::_3, arg::_4 ) ) );
	world->add( land ); // render object
}

TestLvl::~TestLvl() {
	player = nullptr;
	world->remove( land );
}

void TestLvl::update( float timeMS ) {
}

void TestLvl::spawnPlayer( int _localPlayerNum ) {
	Core::TransformNode* spawnPnt = land->getTransform();
	// TODO proper spawn point select
	if( !playerStarts.empty() ) {
		spawnPnt = playerStarts[0].transform;
	}

	player = std::make_shared<HoverTank>( world, _localPlayerNum, spawnPnt );
}

void TestLvl::decodeLevelProperties( LevelThing* thing, std::unordered_map<std::string, int>& nameMap, 
										const Core::BinPropertyResourcePtr& binProp, Core::TransformNode * transformNode ) {
	decodeMarkerProperties( thing, nameMap, binProp, transformNode );
	decodeEnemyProperties( thing, nameMap, binProp, transformNode );
	decodeItemProperties( thing, nameMap, binProp, transformNode );
}

void TestLvl::decodeMarkerProperties( LevelThing* thing, std::unordered_map<std::string, int>& nameMap, 
										const Core::BinPropertyResourcePtr& binProp, Core::TransformNode * transformNode ) {
	if( nameMap.find( PROP_MARKER_TYPE ) != nameMap.cend() ) {
		if(*binProp->getAs<int32_t>( nameMap[ PROP_MARKER_TYPE ] ) == MT_PlayerStart ) {
			playerStarts.push_back( PlayerStartMarker( transformNode ) );
		} else if(*binProp->getAs<int32_t>( nameMap[ PROP_MARKER_TYPE ] ) == MT_Generic ) {
			genericMarkers.push_back( GenericMarker() );
		}
	}
}

void TestLvl::decodeEnemyProperties( LevelThing* thing, std::unordered_map<std::string, int>& nameMap, 
										const Core::BinPropertyResourcePtr& binProp, Core::TransformNode * transformNode ) {
	if( nameMap.find( PROP_ENEMY_TYPE ) != nameMap.cend() ) {
		if(*binProp->getAs<int32_t>( nameMap[ PROP_ENEMY_TYPE ] ) == ET_IndigoWeakSpider ) {
			ownedEnemies.push_back( std::make_shared<IndigoWeakSpider>( world, transformNode ) );
		}
	}
}

void TestLvl::decodeItemProperties( LevelThing* thing, std::unordered_map<std::string, int>& nameMap, 
										const Core::BinPropertyResourcePtr& binProp, Core::TransformNode * transformNode ) {
	if( nameMap.find( PROP_ITEM_TYPE ) != nameMap.cend() ) {
		if(*binProp->getAs<int32_t>( nameMap[ PROP_ITEM_TYPE ] ) == IT_EnergyCrystal ) {
			levelItems.push_back( std::make_shared<EnergyCrystal>( world, transformNode ) );
		}
	}
}
