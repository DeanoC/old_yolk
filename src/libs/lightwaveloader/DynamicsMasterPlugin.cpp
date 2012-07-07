#include "lightwaveloader.h"

#include "LWSceneLoader.h"
#include "DynamicsMasterPlugin.h"

namespace
{

using namespace LightWave;
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

SceneLoader::KEY_DATA DMP_KeyArray[] = {
	// Objects
	{ "DynamicsMaster",		&SceneLoader::skipKey,	&SceneLoader::skipKey,	&SceneLoader::DynamicsMasterSubKey },
	{ "Version",			&SceneLoader::skipKey,	&SceneLoader::skipKey,	&SceneLoader::DynamicsMasterVersionSubKey },
	{ "DrawShapes",			&SceneLoader::skipKey,	&SceneLoader::skipKey,	&SceneLoader::DynamicsMasterDrawShapesSubKey },
	{ "Enabled",			&SceneLoader::skipKey,	&SceneLoader::skipKey,	&SceneLoader::DynamicsMasterEnabledSubKey },
	{ "StepsPerSecond",		&SceneLoader::skipKey,	&SceneLoader::skipKey,	&SceneLoader::DynamicsMasterStepsPerSecondSubKey },
	{ "TimeScale",			&SceneLoader::skipKey,	&SceneLoader::skipKey,	&SceneLoader::DynamicsMasterTimeScaleSubKey },
	{ "Gravity",			&SceneLoader::skipKey,	&SceneLoader::skipKey,	&SceneLoader::DynamicsMasterGravitySubKey },

	// end of key array
	{ "",0,	0, 0 } 
};

SceneLoader::KEY_DATA DMP_EndKeyArray[] = {
	{ "EndPlugin",			&SceneLoader::skipKey,	&SceneLoader::skipKey,	&SceneLoader::EndPluginKey },
	{ "", 0, 0, 0 } 
};

}

namespace LightWave {

const std::string DynamicsMasterPlugin::PluginName = "DynamicsMaster";

void SceneLoader::DynamicsMasterPluginCallback( FILE* f )
{
	subKeyReader( f, DMP_KeyArray, DMP_EndKeyArray );
}

void SceneLoader::DynamicsMasterSubKey( FILE* f, const char* ValueText ) {
	curNodePlugin =  new DynamicsMasterPlugin();
	masterPluginData.push_back( curNodePlugin );
}

void SceneLoader::DynamicsMasterVersionSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsMasterPlugin*)curNodePlugin)->version = x;
}

void SceneLoader::DynamicsMasterDrawShapesSubKey( FILE* f, const char* ValueText ) {
	
	assert( curNodePlugin != 0 );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsMasterPlugin*)curNodePlugin)->drawShapes = !!x;
}

void SceneLoader::DynamicsMasterEnabledSubKey( FILE* f, const char* ValueText ) {
	
	assert( curNodePlugin != 0 );

	readLine(f);
	char* val0 = strtok(lineBuffer,"\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsMasterPlugin*)curNodePlugin)->enabled = !!x;
}

void SceneLoader::DynamicsMasterStepsPerSecondSubKey( FILE* f, const char* ValueText ) {
	
	assert( curNodePlugin != 0 );

	readLine(f);
	char* val0 = strtok(lineBuffer,"\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsMasterPlugin*)curNodePlugin)->stepsPerSecond = x;
}

void SceneLoader::DynamicsMasterTimeScaleSubKey( FILE* f, const char* ValueText ) {
	
	assert( curNodePlugin != 0 );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer,"\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsMasterPlugin*)curNodePlugin)->timeScale = x;
}

void SceneLoader::DynamicsMasterGravitySubKey( FILE* f, const char* ValueText ) {
	
	assert( curNodePlugin != 0 );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	char* val1 = strtok(0," \t\xA\xD");
	char* val2 = strtok(0," \t\xA\xD");
	float x = (float) atof( val0 );
	float y = (float) atof( val1 );
	float z = (float) atof( val2 );

	((DynamicsMasterPlugin*)curNodePlugin)->gravity = Math::Vector3(x,y,z);
}

}