#include "lightwaveloader.h"

#include "LWSceneLoader.h"
#include "boost/lexical_cast.hpp"
#include "vtdfhEnemyCustomObjPlugin.h"

namespace
{

using namespace LightWave;
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------


#define SK &SceneLoader::skipKey

static SceneLoader::KEY_DATA VTDFHEnemy_KeyArray[] = {
	{ "Enemy",			SK,	SK,	&SceneLoader::VtdfhEnemySubKey },
	{ "EnemyType",			SK,	SK,	&SceneLoader::VtdfhEnemyTypeSubKey },
	
	// end of key array
	{ "",0,	0, 0 } 
};

static SceneLoader::KEY_DATA VTDFHEnemy_EndKeyArray[] = {
	{ "EndPlugin",			SK,	SK,	&SceneLoader::EndPluginKey },
	{ "", 0, 0, 0 } 
};

}

namespace LightWave {

const std::string VtdfhEnemyCustomObjPlugin::PluginName = "VtdfhEnemy";

void SceneLoader::VtdfhEnemyPluginCallback( FILE* f ) {
	subKeyReader( f, VTDFHEnemy_KeyArray, VTDFHEnemy_EndKeyArray );
}

void SceneLoader::VtdfhEnemySubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin == 0 );
	assert( curObject != 0 );
	curNodePlugin =  new VtdfhEnemyCustomObjPlugin();
	curObject->plugins.push_back( curNodePlugin );
}

void SceneLoader::VtdfhEnemyTypeSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == VtdfhEnemyCustomObjPlugin::PluginName );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");

	((VtdfhEnemyCustomObjPlugin*)curNodePlugin)->type = boost::lexical_cast<int>(val0);
}

}