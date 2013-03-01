#include "lightwaveloader.h"

#include "LWSceneLoader.h"
#include "boost/lexical_cast.hpp"
#include "vtdfhMarkerCustomObjPlugin.h"

namespace
{

using namespace LightWave;
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

#define SK &SceneLoader::skipKey

static SceneLoader::KEY_DATA VTDFHMarker_KeyArray[] = {
	{ "Marker",			SK,	SK,	&SceneLoader::VtdfhMarkerSubKey },
	{ "MarkerType",			SK,	SK,	&SceneLoader::VtdfhMarkerTypeSubKey },
	
	// end of key array
	{ "",0,	0, 0 } 
};

static SceneLoader::KEY_DATA VTDFHMarker_EndKeyArray[] = {
	{ "EndPlugin",			SK,	SK,	&SceneLoader::EndPluginKey },
	{ "", 0, 0, 0 } 
};

}

namespace LightWave {

const std::string VtdfhMarkerCustomObjPlugin::PluginName = "VtdfhMarker";

void SceneLoader::VtdfhMarkerPluginCallback( FILE* f ) {
	subKeyReader( f, VTDFHMarker_KeyArray, VTDFHMarker_EndKeyArray );
}

void SceneLoader::VtdfhMarkerSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin == 0 );
	assert( curObject != 0 );
	curNodePlugin =  new VtdfhMarkerCustomObjPlugin();
	curObject->plugins.push_back( curNodePlugin );
}

void SceneLoader::VtdfhMarkerTypeSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == VtdfhMarkerCustomObjPlugin::PluginName );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");
	((VtdfhMarkerCustomObjPlugin*)curNodePlugin)->type = boost::lexical_cast<int>(val0);
}

}