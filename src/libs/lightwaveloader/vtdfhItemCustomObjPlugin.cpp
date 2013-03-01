#include "lightwaveloader.h"

#include "LWSceneLoader.h"
#include "boost/lexical_cast.hpp"
#include "vtdfhItemCustomObjPlugin.h"

namespace
{

using namespace LightWave;
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------


#define SK &SceneLoader::skipKey

static SceneLoader::KEY_DATA VTDFHItem_KeyArray[] = {
	{ "Item",			SK,	SK,	&SceneLoader::VtdfhItemSubKey },
	{ "ItemType",			SK,	SK,	&SceneLoader::VtdfhItemTypeSubKey },
	
	// end of key array
	{ "",0,	0, 0 } 
};

static SceneLoader::KEY_DATA VTDFHItem_EndKeyArray[] = {
	{ "EndPlugin",			SK,	SK,	&SceneLoader::EndPluginKey },
	{ "", 0, 0, 0 } 
};

}

namespace LightWave {

const std::string VtdfhItemCustomObjPlugin::PluginName = "VtdfhItem";

void SceneLoader::VtdfhItemPluginCallback( FILE* f ) {
	subKeyReader( f, VTDFHItem_KeyArray, VTDFHItem_EndKeyArray );
}

void SceneLoader::VtdfhItemSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin == 0 );
	assert( curObject != 0 );
	curNodePlugin =  new VtdfhItemCustomObjPlugin();
	curObject->plugins.push_back( curNodePlugin );
}

void SceneLoader::VtdfhItemTypeSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == VtdfhItemCustomObjPlugin::PluginName );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");

	((VtdfhItemCustomObjPlugin*)curNodePlugin)->type = boost::lexical_cast<int>(val0);
}

}