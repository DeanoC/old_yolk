#include "meshimport.h"

#include "LWScene2GO.h"
#include "lightwaveloader/vtdfhLevelCustomObjPlugin.h"
#include "meshmod/property.h"
#include "meshmod/scenenode.h"
#include "meshops/basicmeshops.h"
#include "core/aabb.h"

void VtdfhLevelCustomObj2Scene( LightWave::VtdfhLevelCustomObjPlugin* dip, MeshMod::ScenePtr scnNode ) {
	using namespace MeshMod;
	scnNode->properties.push_back( std::make_shared<Property>( "LevelName", dip->levelName ) ); 
	scnNode->properties.push_back( std::make_shared<Property>( "LevelDesc", dip->levelDesc ) ); 
}