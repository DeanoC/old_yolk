#include "meshimport.h"

#include "LWScene2GO.h"
#include "lightwaveloader/vtdfhItemCustomObjPlugin.h"
#include "meshmod/property.h"
#include "meshmod/scenenode.h"
#include "meshops/basicmeshops.h"
#include "core/aabb.h"

#include "localworld/properties.h" // usually meshmod doesn't use localworld files but this just makes it easier to keep everything shared

void VtdfhItemCustomObj2SceneNode( LightWave::VtdfhItemCustomObjPlugin* dip, MeshMod::SceneNodePtr scnNode ) {
	using namespace MeshMod;
	scnNode->properties.push_back( std::make_shared<Property>( PROP_ITEM_TYPE, dip->type ) ); 
//	scnNode->properties.push_back( std::make_shared<Property>( "LevelDesc", dip->levelDesc ) ); 
}