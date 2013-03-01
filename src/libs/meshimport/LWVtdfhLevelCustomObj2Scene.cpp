#include "meshimport.h"

#include "LWScene2GO.h"
#include "lightwaveloader/vtdfhLevelCustomObjPlugin.h"
#include "meshmod/property.h"
#include "meshmod/scenenode.h"
#include "meshops/basicmeshops.h"
#include "core/aabb.h"

#include "localworld/properties.h" // usually meshmod doesn't use localworld files but this just makes it easier to keep everything shared

void VtdfhLevelCustomObj2Scene( LightWave::VtdfhLevelCustomObjPlugin* dip, MeshMod::ScenePtr scnNode ) {
	using namespace MeshMod;
	scnNode->properties.push_back( std::make_shared<Property>( PROP_LEVEL_NAME, dip->levelName ) ); 
	scnNode->properties.push_back( std::make_shared<Property>( PROP_LEVEL_DESC, dip->levelDesc ) ); 
}