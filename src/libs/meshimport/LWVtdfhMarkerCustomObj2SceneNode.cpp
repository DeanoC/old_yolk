#include "meshimport.h"

#include "LWScene2GO.h"
#include "lightwaveloader/vtdfhMarkerCustomObjPlugin.h"
#include "meshmod/property.h"
#include "meshmod/scenenode.h"
#include "meshops/basicmeshops.h"
#include "core/aabb.h"

void VtdfhMarkerCustomObj2SceneNode( LightWave::VtdfhMarkerCustomObjPlugin* dip, MeshMod::SceneNodePtr scnNode ) {
	using namespace MeshMod;
//	scnNode->properties.push_back( std::make_shared<Property>( "LevelName", dip->levelName ) ); 
//	scnNode->properties.push_back( std::make_shared<Property>( "LevelDesc", dip->levelDesc ) ); 
}