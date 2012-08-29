#include "lightwaveloader.h"

#include "LWSceneLoader.h"
#include "vtdfhLevelCustomObjPlugin.h"

namespace
{

using namespace LightWave;
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------


#define SK &SceneLoader::skipKey

SceneLoader::KEY_DATA VTDFH_KeyArray[] = {
	{ "LevelName",			SK,	SK,	&SceneLoader::VtdfhLevelNameSubKey },
	{ "LevelDesc",			SK,	SK,	&SceneLoader::VtdfhLevelDescSubKey },	
	
	// end of key array
	{ "",0,	0, 0 } 
};

SceneLoader::KEY_DATA VTDFH_EndKeyArray[] = {
	{ "EndPlugin",			SK,	SK,	&SceneLoader::EndPluginKey },
	{ "", 0, 0, 0 } 
};

}

namespace LightWave {

const std::string VtdfhLevelCustomObjPlugin::PluginName = "VtdfhLevel";

void SceneLoader::VtdfhLevelPluginCallback( FILE* f ) {
	assert( curNodePlugin == 0 );
	assert( curObject != 0 );
	curNodePlugin =  new VtdfhLevelCustomObjPlugin();
	curObject->plugins.push_back( curNodePlugin );
	subKeyReader( f, VTDFH_KeyArray, VTDFH_EndKeyArray );
}

void SceneLoader::VtdfhLevelNameSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == VtdfhLevelCustomObjPlugin::PluginName );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");

	((VtdfhLevelCustomObjPlugin*)curNodePlugin)->levelName = val0;
}

void SceneLoader::VtdfhLevelDescSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == VtdfhLevelCustomObjPlugin::PluginName );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");

	((VtdfhLevelCustomObjPlugin*)curNodePlugin)->levelDesc = val0;
}

}