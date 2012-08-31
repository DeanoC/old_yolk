#include "meshimport.h"

#include "LWScene2GO.h"
#include "lightwaveloader/vtdfhMarkerCustomObjPlugin.h"
#include "meshmod/property.h"
#include "meshmod/scenenode.h"
#include "meshops/basicmeshops.h"
#include "core/aabb.h"

#include "localworld/properties.h" // usually meshmod doesn't use localworld files but this just makes it easier to keep everything shared

void VtdfhMarkerCustomObj2SceneNode( LightWave::VtdfhMarkerCustomObjPlugin* dip, MeshMod::SceneNodePtr scnNode ) {
	using namespace MeshMod;
	scnNode->properties.push_back( std::make_shared<Property>( PROP_MARKER_TYPE, dip->type ) ); 
}