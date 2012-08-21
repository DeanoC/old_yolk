/** \file LWOLoader.cpp
   Light wave 7 Object Loader implementation.
   A generic template C++ files
   (c) 2002 Deano Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "lightwaveloader.h"
#include <stack>
#include "LWNode.h"
#include "LWObject.h"
#include "LWBone.h"
#include "DynamicsMasterPlugin.h"
#include "LWSceneLoader.h"

#if !defined( UNREFERENCED_PARAMETER )
#define UNREFERENCED_PARAMETER(x) (void)x
#endif

using namespace LightWave;

namespace
{
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

SceneLoader::KEY_DATA MainKeyArray[] = {
	// Objects
	{ "LWSC",				&SceneLoader::LWSCKey,	&SceneLoader::LWSCKey,				&SceneLoader::LWSCKey },
	{ "LoadObjectLayer",	&SceneLoader::skipKey,	&SceneLoader::LoadObjectLayerKey,	&SceneLoader::LoadObjectLayerKeyV5 },
	{ "AddNullObject",		&SceneLoader::skipKey,	&SceneLoader::AddNullObjectKey,		&SceneLoader::AddNullObjectKeyV5 },
	{ "AddBone",			&SceneLoader::skipKey,	&SceneLoader::AddBoneKey,			&SceneLoader::AddBoneKeyV5 },
	{ "AddCamera",			&SceneLoader::skipKey,	&SceneLoader::skipKey,				&SceneLoader::skipKey },
	{ "AddLight",			&SceneLoader::skipKey,	&SceneLoader::skipKey,				&SceneLoader::skipKey },
	{ "Plugin",				&SceneLoader::skipKey,	&SceneLoader::PluginKey,			&SceneLoader::PluginKey },
	{ "EndPlugin",			&SceneLoader::skipKey,	&SceneLoader::skipKey,				&SceneLoader::EndPluginKey },

	// end of key array
	{ "",				0,							0 } 
};

SceneLoader::KEY_DATA EndSubKeyArray[] = {
	// Objects
	{ "LWSC",				},
	{ "LoadObjectLayer",	},
	{ "AddNullObject",		},
	{ "AddBone",			},
	{ "AddCamera",			},
	{ "AddLight",			},

	// end of key array
	{ "", } 
};


SceneLoader::KEY_DATA ObjectKey[] = {
	// Objects
	{ "ShowObject",			&SceneLoader::skipKey,	&SceneLoader::skipKey,					&SceneLoader::skipKey },
	{ "ObjectMotion",		&SceneLoader::skipKey,	&SceneLoader::ObjectMotionKey,			&SceneLoader::ObjectMotionKey },
	{ "NumChannels",		&SceneLoader::skipKey,	&SceneLoader::NumChannelsKey,			&SceneLoader::NumChannelsKey },
	{ "Channel",			&SceneLoader::skipKey,	&SceneLoader::ChannelKey,				&SceneLoader::ChannelKey },
	{ "LockedChannels",		&SceneLoader::skipKey,	&SceneLoader::skipKey,					&SceneLoader::skipKey },
	{ "ShadowOptions",		&SceneLoader::skipKey,	&SceneLoader::skipKey,					&SceneLoader::skipKey },
	{ "ParentItem",			&SceneLoader::skipKey,	&SceneLoader::ParentItemKey,			&SceneLoader::ParentItemKey },
	{ "BoneFalloffType",	&SceneLoader::skipKey,	&SceneLoader::BoneFalloffTypeKey,		&SceneLoader::BoneFalloffTypeKey },
	{ "Plugin",				&SceneLoader::skipKey,	&SceneLoader::PluginKey,				&SceneLoader::PluginKey },

	// end of key array
	{ "", 0, 0, 0 } 
};

SceneLoader::KEY_DATA BoneKey[] = {
	// Bones
	{ "ShowBone",			&SceneLoader::skipKey,	&SceneLoader::skipKey,					&SceneLoader::skipKey },
	{ "BoneMotion",			&SceneLoader::skipKey,	&SceneLoader::BoneMotionKey,			&SceneLoader::BoneMotionKey },
	{ "NumChannels",		&SceneLoader::skipKey,	&SceneLoader::NumChannelsKey,			&SceneLoader::NumChannelsKey },
	{ "Channel",			&SceneLoader::skipKey,	&SceneLoader::ChannelKey,				&SceneLoader::ChannelKey },
	{ "LockedChannels",		&SceneLoader::skipKey,	&SceneLoader::skipKey,					&SceneLoader::skipKey },
	{ "ParentItem",			&SceneLoader::skipKey,	&SceneLoader::ParentItemKey,			&SceneLoader::ParentItemKey },
	{ "BoneName",			&SceneLoader::skipKey,	&SceneLoader::BoneNameKey,				&SceneLoader::BoneNameKey},
	{ "BoneActive",			&SceneLoader::skipKey,	&SceneLoader::BoneActiveKey,			&SceneLoader::BoneActiveKey },
	{ "BoneRestPosition",	&SceneLoader::skipKey,	&SceneLoader::BoneRestPositionKey,		&SceneLoader::BoneRestPositionKey },
	{ "BoneRestDirection",	&SceneLoader::skipKey,	&SceneLoader::BoneRestDirectionKey,		&SceneLoader::BoneRestDirectionKey },
	{ "BoneRestLength",		&SceneLoader::skipKey,	&SceneLoader::BoneRestLengthKey,		&SceneLoader::BoneRestLengthKey },
	{ "BoneWeightMapName",	&SceneLoader::skipKey,	&SceneLoader::BoneWeightMapNameKey,		&SceneLoader::BoneWeightMapNameKey },
	{ "BoneWeightMapOnly",	&SceneLoader::skipKey,	&SceneLoader::BoneWeightMapOnlyKey,		&SceneLoader::BoneWeightMapOnlyKey },
	{ "BoneNormalization",	&SceneLoader::skipKey,	&SceneLoader::BoneNormalizationKey,		&SceneLoader::BoneNormalizationKey },
	{ "BoneStrength",		&SceneLoader::skipKey,	&SceneLoader::BoneStrengthKey,			&SceneLoader::BoneStrengthKey },
	{ "ScaleBoneStrength",	&SceneLoader::skipKey,	&SceneLoader::BoneScaleBoneStrengthKey,	&SceneLoader::BoneScaleBoneStrengthKey },

	// end of key array
	{ "", 0, 0, 0 } 
};

} // end empty namespace

//---------------------------------------------------------------------------
// Local Classes and structures
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Local Prototypes & Functions
//---------------------------------------------------------------------------

/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
SceneLoader::SceneLoader() :
	version(1),
	curObject(0),
	ungetLastLine(false),
	curBone(0)
{
	registerMasterPlugin( ".DynamicsMaster", &SceneLoader::DynamicsMasterPluginCallback );
	registerItemMotionPlugin( ".DynamicsMotion", &SceneLoader::DynamicsItemPluginCallback );
}

SceneLoader::~SceneLoader() {
	ObjectList::iterator obIt = objects.begin();
	while( obIt != objects.end() ) {
		delete (*obIt);
		++obIt;
	}

	BoneList::iterator bnIt = bones.begin();
	while( bnIt != bones.end() ) {
		delete (*bnIt);
		++bnIt;
	}

	PluginDataList::iterator plIt = masterPluginData.begin();
	while( plIt != masterPluginData.end() ) {
		delete (*plIt);
		++plIt;
	}


}

void SceneLoader::loadFile( const std::string& filename )
{
	FILE* f;

	f = fopen(filename.c_str(),"rb");
	if (f == NULL)
		CoreThrowException(FileNotFoundError, filename );

	keyReader( f );

	// link up node in hierachy order
	linkNodes();
}

/**
returns the chunk index if available.
scans the chunk list and return the index if found
@param Id chunk Identifier
@return index in chunk table or -1 if not found
*/
int SceneLoader::findKey(const std::string& text, const KEY_DATA* keyArray) { 
	int i = 0;
	const KEY_DATA* key = &keyArray[0];

	while( !std::string(key->text).empty() ) {
		if(std::string(key->text) == text)
			return i;

		i++;
		key++;
	}

	return -1;
}

void SceneLoader::readLine(FILE* f) {
	if(ungetLastLine == true) {
		strcpy(lineBuffer, lastLineBuffer);
		ungetLastLine = false;
		return;
	}

	lineBuffer[0] = 0;
	fgets(lineBuffer,255,f);
	strcpy(lastLineBuffer, lineBuffer);
	lineNum++;
}

char* SceneLoader::skipWhiteSpace() {
	// skips some spaces at the front of line buffer
	char* ptr = lineBuffer;

	while( *ptr == ' ' )
		ptr++;

	return ptr;
}

//------------------------------------------------------------------------------------------
//	NAME			: KeyReader
//	INPUTS			: f - pointer to FILE structure
//					  KeyData - pointer to array
//	OUTPUTS			: None
//	NOTES			: Default destructor
//------------------------------------------------------------------------------------------

void SceneLoader::keyReader(FILE *f) { 
	lineNum = 1;

	while (!feof(f) ) {

		do {
			readLine(f);
		} while( lineBuffer[0] == 0xA );

		// split key and value
		char *keyText = strtok(lineBuffer," \t\xA\xD");
		char *valueText = strtok(0,"\xA\xD");

		if (keyText != 0) {
			// find handler

			int keyIndex = findKey( keyText, MainKeyArray );
			if ( keyIndex != -1 ) {
				const KEY_DATA& key = MainKeyArray[keyIndex];
				if(version == 1) {
					(this->*key.FuncV1)( f, valueText );
				}
				else if(version == 3) {
					(this->*key.FuncV3)( f, valueText );
				}
				else if( version >= 5) {
					(this->*key.FuncV5)( f, valueText );
				}
			}
		}
	}
}

void SceneLoader::subKeyReader(FILE *f, const KEY_DATA* subKeyArray, const KEY_DATA* exitKeyArray) { 
	char* keyText; 
	// loop until we find another main key!
	do {
		do {
			readLine(f);
		} while( lineBuffer[0] == 0xA );

		// split key and value
		keyText = strtok(lineBuffer," \t\xA\xD");

		if (keyText != 0) {
			// some keys now have a { NAME signifying start of an object
			if( keyText[0] == '{' ) {
				keyText = strtok( 0, " \t\xA\xD" );
			}

			char* valueText = strtok(0,"\xA\xD");

			// end this subkey if we get any exit key
			if( findKey(keyText, exitKeyArray) != -1)
				break;

			// find handler

			int keyIndex = findKey( keyText, subKeyArray );
			if ( keyIndex != -1 ) {
				const KEY_DATA& key = subKeyArray[keyIndex];
				if(version == 1) {
					(this->*key.FuncV1)( f, valueText );
				} else if(version == 3) {
					(this->*key.FuncV3)( f, valueText);
				} else if( version >= 5) {
					(this->*key.FuncV5)( f, valueText );
				}
			}
		}
	} while ( !feof(f) && (keyText == 0 || findKey(keyText, exitKeyArray) == -1 ) ); 

	ungetLastLine = true;
}


void SceneLoader::registerMasterPlugin( const std::string& name, PluginCallback pPlugin ) {
	assert( masterPluginRegistry.find( name ) == masterPluginRegistry.end() );
	masterPluginRegistry[ name ] = pPlugin;
}

void SceneLoader::registerItemMotionPlugin( const std::string& name, PluginCallback pPlugin ) {
	assert( itemMotionPluginRegistry.find( name ) == itemMotionPluginRegistry.end() );
	itemMotionPluginRegistry[ name ] = pPlugin;
}


void SceneLoader::linkNodes()
{
	#if 0 // TODO bones again
	// link bones
	BoneList::iterator boneIt = m_Bones.begin();
	while( boneIt != m_Bones.end() )
	{
		// if no parentNum leave parent as null
		unsigned int parentType = ((*boneIt)->m_channels.m_parentNum & 0xF0000000) >> 28;
		switch( parentType )
		{
		case 0:
		case Node::CAMERA: 
		case Node::LIGHT:	
			// bones must be parent to either objects or other bones
			assert(false); break;
		case Node::OBJECT:
			{
				unsigned int parentNum = ((*boneIt)->m_channels.m_parentNum & ~0xF0000000);
				assert( parentNum < m_Objects.size() );
				Object* obj = m_Objects[parentNum];
				(*boneIt)->m_parent = obj;
				obj->m_bones.push_back( (*boneIt) );
				(*boneIt)->m_object = obj;
			} break;
		case Node::BONE:
			{
				unsigned int boneNum = ((*boneIt)->m_channels.m_parentNum & 0x0FFF0000) >> 16;
				unsigned int objNum = ((*boneIt)->m_channels.m_parentNum & 0x0000FFFF) ;
				assert( objNum < m_Objects.size() );
				Object* obj = m_Objects[objNum];
				obj->m_bones.push_back( (*boneIt) );
				(*boneIt)->m_object = obj;
				assert( boneNum < obj->m_bones.size() );
				(*boneIt)->m_parent = obj->m_bones[boneNum];
			} break;
		}

		++boneIt;
	}
	#endif

	// link objects
	ObjectList::iterator objIt = objects.begin();
	while( objIt != objects.end() ) {
		assert( (*objIt)->parent == 0 );
		
		// if no parentNum leave parent as null
		unsigned int parentType = ((*objIt)->channels.parentNum & 0xF0000000) >> 28;
		switch( parentType )
		{
		case 0:
			// no parent;
			(*objIt)->parent = 0;
			break;
		case Node::CAMERA: 	assert(false); break;
		case Node::LIGHT:	assert(false); break;
		case Node::OBJECT:
			{
				unsigned int parentNum = ((*objIt)->channels.parentNum & ~0xF0000000);
				assert( parentNum < objects.size() );
				(*objIt)->parent = objects[ parentNum ];
			} break;
		case Node::BONE:
			{
/*				unsigned int boneNum = ((*objIt)->channels.parentNum & 0x0FFF0000) >> 16;
				unsigned int objNum = ((*objIt)->channels.parentNum & 0x0000FFFF) ;
				assert( objNum < objects.size() );
				Object* obj = objects[objNum];
				assert( boneNum < obj->bones.size() );
				(*objIt)->parent = obj->bones[boneNum];*/
			} break;
		}

		++objIt;
	}
	#if 0 // TODO bones again
	// for each bone work out its rest matrix
	// we have to walk up there hierachy and the descend building the matrix
	BoneList::iterator restIt = m_Bones.begin();
	while( restIt != m_Bones.end() )
	{
		std::stack< Node* > objects;

		// get parent 
		Node* parent = (*restIt)->m_parent;
		assert(parent != 0);

		// walk to top of hierachy 
		while(parent != 0 && parent != (*restIt)->m_object )
		{
			objects.push( parent );
			parent = parent->m_parent;
		}

		Math::Matrix4x4 transformMatrix = Math::IdentityMatrix();

		// now descend building transformation matrix
		while( !objects.empty() )
		{
			Node* obj = objects.top();
			assert(obj != 0);

			if(obj->m_type == Node::BONE)
			{ 
				Bone* bone = static_cast<Bone*>(obj);
				Math::Matrix4x4 restMatrix;
				Node::evaluateHPBAngles(	-Math::degree_to_radian<float>() * bone->m_restDirection[0], 
											-Math::degree_to_radian<float>() * bone->m_restDirection[1],
											-Math::degree_to_radian<float>() * bone->m_restDirection[2], restMatrix );
				restMatrix = restMatrix * Math::CreateTranslationMatrix(bone->m_restPosition);
				transformMatrix = restMatrix * transformMatrix;
			} else
			{
				float		objParams[9];
				Math::Matrix4x4	objMatrix;

				obj->m_channels.evaluate(0, objParams );
				Node::evaluateLWChannelGroup( 9, objParams, objMatrix );

				transformMatrix = objMatrix * transformMatrix;
			}

			objects.pop();
		}

		Math::Vector3 vec;
		Math::Matrix4x4 restMatrix;

		Node::evaluateHPBAngles(	-Math::degree_to_radian<float>() * (*restIt)->m_restDirection[0], 
									-Math::degree_to_radian<float>() * (*restIt)->m_restDirection[1],
									-Math::degree_to_radian<float>() * (*restIt)->m_restDirection[2], restMatrix );
		restMatrix = restMatrix * Math::CreateTranslationMatrix((*restIt)->m_restPosition);

		restMatrix = restMatrix * transformMatrix;
		(*restIt)->m_worldRestPos = Math::GetTranslation( restMatrix );
		(*restIt)->m_worldRestDir = Math::GetZAxis( restMatrix );
		(*restIt)->m_restMatrix = restMatrix;

		++restIt;
	}
	#endif
}

//------------------------------------------------------------------------------------------
//-- KEYS ------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/**
skip this key.
Doesn't work for multi line segments, but they should still be ignored
@param f filehandle
@param ValueText text of the 2 param on this line (the value of this key)
*/
void SceneLoader::skipKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( ValueText );
	// DO Nothing just skip the key
}

/**
skip this key.
Doesn't work for multi line segments, but they should still be ignored
@param f filehandle
@param ValueText text of the 2 param on this line (the value of this key)
*/
void SceneLoader::LWSCKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( ValueText );

	// next line should be a version number
	readLine(f);
	version = atol(lineBuffer);

	if( version > 5 ) {
		LOG(INFO) << "LWS greater than we know about, will attempt to load with V5 loader\n";
	}

}

/**
skip this key.
Doesn't work for multi line segments, but they should still be ignored
@param f filehandle
@param ValueText text of the 2 param on this line (the value of this key)
*/
void SceneLoader::LoadObjectLayerKey(FILE* f, const char* ValueText)
{

	// ValueText has a number and the filename
	char* numberText = strtok(const_cast<char*>(ValueText)," \xA\xD");
	char* fileName = strtok(0,"\xA\xD");

	// this is the layer number in the object file!
	unsigned int number = atol( numberText );
	// layer num appear to be 1 indexed?
	if( number != 0 )
		number--;

	assert(curObject == 0);

	objects.push_back( new Object() );
	curObject = objects.back();
	curObject->name = fileName;

	curObject->layer = number;
	curObject->type = Object::OBJECT;
	curChannelGroup = &curObject->channels;

	subKeyReader( f, ObjectKey, EndSubKeyArray );

	curObject->loader = CORE_NEW LightWave::LWO_Loader( curObject->name );

	curChannelGroup = 0;
	curObject = 0;
}

void SceneLoader::LoadObjectLayerKeyV5(FILE* f, const char* ValueText)
{

	// ValueText has a layer number, another number and then the filename
	char* layerText = strtok(const_cast<char*>(ValueText)," \xA\xD");
	char* numberText = strtok(0," \xA\xD");
	char* fileName = strtok(0,"\xA\xD");

	// this is the layer number in the object file!
	unsigned int layer = atol( layerText );
	// layer num appear to be 1 indexed?
	if( layer != 0 )
		layer--;

	assert(curObject == 0);

	objects.push_back( new Object() );
	curObject = objects.back();
	curObject->name = fileName;

	curObject->layer = layer;
	curObject->type = Object::OBJECT;
	curChannelGroup = &curObject->channels;

	subKeyReader( f, ObjectKey, EndSubKeyArray );

	curObject->loader = CORE_NEW LightWave::LWO_Loader( (std::string("../") + std::string(curObject->name)).c_str() );

	curChannelGroup = 0;
	curObject = 0;
}

void SceneLoader::AddNullObjectKey(FILE* f, const char* ValueText)
{

	// ValueText just has a name
	char* name = strtok(const_cast<char*>(ValueText),"\xA\xD");

	assert(curObject == 0);

	objects.push_back( new Object() );
	curObject = objects.back();

	curObject->name = name;
	curObject->type = Object::NULL_OBJECT;
	curChannelGroup = &curObject->channels;

	subKeyReader( f, ObjectKey, EndSubKeyArray );

	curChannelGroup = 0;
	curObject = 0;
}

void SceneLoader::AddNullObjectKeyV5(FILE* f, const char* ValueText) {

	// ValueText just has a number and a name
	char* numberText = strtok(const_cast<char*>(ValueText)," \xA\xD");
	char* name = strtok(0,"\xA\xD");

	assert(curObject == 0);

	objects.push_back( new Object() );
	curObject = objects.back();

	curObject->name = name;
	curObject->type = Object::NULL_OBJECT;
	curChannelGroup = &curObject->channels;

	subKeyReader( f, ObjectKey, EndSubKeyArray );

	curChannelGroup = 0;
	curObject = 0;
}

void SceneLoader::AddBoneKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER(ValueText);

	assert(curBone == 0);

	bones.push_back( new Bone() );
	curBone = bones.back();
	curChannelGroup = &curBone->channels;

	subKeyReader( f, BoneKey, EndSubKeyArray );

	curChannelGroup = 0;
	curBone = 0;
}

void SceneLoader::AddBoneKeyV5(FILE* f, const char* ValueText)
{
	// ValueText just has a number
	char* numberText = strtok(const_cast<char*>(ValueText)," \xA\xD");
	assert(curBone == 0);

	bones.push_back( new Bone() );
	curBone = bones.back();
	curChannelGroup = &curBone->channels;

	subKeyReader( f, BoneKey, EndSubKeyArray );

	curChannelGroup = 0;
	curBone = 0;
}

void SceneLoader::ObjectMotionKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( ValueText );
}

void SceneLoader::BoneMotionKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( ValueText );
}

void SceneLoader::NumChannelsKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );
	assert( curChannelGroup != 0);

	curChannelGroup->numChannels = atol(ValueText);
	curChannelGroup->channels.resize(curChannelGroup->numChannels);
	curChannelCount = 0;
}
void SceneLoader::ChannelKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );
	assert( curChannelGroup != 0);

	if( curChannelCount >= curChannelGroup->numChannels)
		return;

	curChannelCount++;
	unsigned int channel = atol( ValueText );

	// skip { Envelope
	readLine(f);
	char* envText = strtok(lineBuffer,"\xA\xD");

	UNREFERENCED_PARAMETER( envText );
	assert( std::string(envText) == "{ Envelope" );

	// read number of key line
	readLine(f);
	char* numKeyText = skipWhiteSpace();
	unsigned int numKeys = atol(numKeyText);
	if(numKeys == 0)
		numKeys = 1;

	curChannelGroup->channels[channel].numKeys = numKeys;
	curChannelGroup->channels[channel].keys.resize( numKeys );


	// for each key read the data in
	for(unsigned int i=0;i < numKeys;i++) {
		// read key line
		readLine(f);
		
		char* keyText = strtok(lineBuffer," \xA\xD");
		
		UNREFERENCED_PARAMETER( keyText );
		assert(std::string(keyText) == "Key" );

		// float value 
		char* valueText = strtok(0," \xA\xD");
		curChannelGroup->channels[channel].keys[i].value = (float) atof(valueText);

		// time value 
		char* timeText = strtok(0," \xA\xD");
		curChannelGroup->channels[channel].keys[i].time = (float) atof(timeText) * 1000.f;

		// spantype value 
		char* spantypeText = strtok(0," \xA\xD");
		curChannelGroup->channels[channel].keys[i].spantype = 
											(Envelope::Key::SPAN_TYPE) atoi(spantypeText);

		float p1,p2,p3,p4,p5,p6;

		// p1 value 
		char* p1Text = strtok(0," \xA\xD");
		p1 = (float) atof(p1Text);
		// p2 value 
		char* p2Text = strtok(0," \xA\xD");
		p2 = (float) atof(p2Text);
		// p3 value 
		char* p3Text = strtok(0," \xA\xD");
		p3 = (float) atof(p3Text);
		// p4 value 
		char* p4Text = strtok(0," \xA\xD");
		p4 = (float) atof(p4Text);
		// p5 value 
		char* p5Text = strtok(0," \xA\xD");
		p5 = (float) atof(p5Text);
		// p6 value 
		char* p6Text = strtok(0," \xA\xD");
		p6 = (float) atof(p6Text);

		switch( curChannelGroup->channels[channel].keys[i].spantype )
		{
		case Envelope::Key::TCB:
		case Envelope::Key::BEZIER1:
		case Envelope::Key::HERMITE:
			curChannelGroup->channels[channel].keys[i].tension = p1;
			curChannelGroup->channels[channel].keys[i].continuity = p2;
			curChannelGroup->channels[channel].keys[i].bias = p3;
			curChannelGroup->channels[channel].keys[i].p0 = p4;
			curChannelGroup->channels[channel].keys[i].p1 = p5;
			curChannelGroup->channels[channel].keys[i].p2 = p6;
			curChannelGroup->channels[channel].keys[i].p3 = 0.f;
			break;
		case Envelope::Key::BEZIER2:
			curChannelGroup->channels[channel].keys[i].tension = 0.f;
			curChannelGroup->channels[channel].keys[i].continuity = 0.f;
			curChannelGroup->channels[channel].keys[i].bias = 0.f;
			curChannelGroup->channels[channel].keys[i].p0 = p1;
			curChannelGroup->channels[channel].keys[i].p1 = p2;
			curChannelGroup->channels[channel].keys[i].p2 = p3;
			curChannelGroup->channels[channel].keys[i].p3 = p4;
			break;
		default:
			curChannelGroup->channels[channel].keys[i].tension = 0.f;
			curChannelGroup->channels[channel].keys[i].continuity = 0.f;
			curChannelGroup->channels[channel].keys[i].bias = 0.f;
			curChannelGroup->channels[channel].keys[i].p0 = 0.f;
			curChannelGroup->channels[channel].keys[i].p1 = 0.f;
			curChannelGroup->channels[channel].keys[i].p2 = 0.f;
			curChannelGroup->channels[channel].keys[i].p3 = 0.f;
			break;
		}
	}

	// read behaviours line
	readLine(f);

	char* behText = strtok(lineBuffer," \xA\xD");

	UNREFERENCED_PARAMETER( behText );
	assert( std::string(behText) == "Behaviors" );

	// pre value 
	char* preText = strtok(0," \xA\xD");
	curChannelGroup->channels[channel].pre = (Envelope::BEHAVIOUR_TYPE) atoi(preText);
	// post value 
	char* postText = strtok(0," \xA\xD");
	curChannelGroup->channels[channel].post = (Envelope::BEHAVIOUR_TYPE) atoi(postText);
}

void SceneLoader::ParentItemKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );

	// base 16(hexadecimal) conversion
	unsigned long item = strtoul( ValueText, 0, 16);

	unsigned long parent = item & 0xFFFFFFFF; // needs to be decoded later

	curChannelGroup->parentNum = parent;
}

void SceneLoader::BoneNameKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );

	assert( curBone != 0 );

	curBone->name = std::string( ValueText );
}
void SceneLoader::BoneActiveKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );

	assert( curBone != 0 );

	unsigned long item = atol( ValueText);

	curBone->active = (item != 0) ? true : false;
}
void SceneLoader::BoneRestPositionKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );
	
	assert( curBone != 0 );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");
	float x = (float) atof( val0 );
	char* val1  = strtok(0," \xA\xD");
	float y = (float) atof( val1 );
	char* val2  = strtok(0," \xA\xD");
	float z = (float) atof( val2 );

	curBone->restPosition = Math::Vector3(x,y,z);
}
void SceneLoader::BoneRestDirectionKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );
	
	assert( curBone != 0 );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");
	float x = (float) atof( val0 );
	char* val1  = strtok(0," \xA\xD");
	float y = (float) atof( val1 );
	char* val2  = strtok(0," \xA\xD");
	float z = (float) atof( val2 );

	curBone->restDirection = Math::Vector3(x,y,z);
}
void SceneLoader::BoneRestLengthKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );
	
	assert( curBone != 0 );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");
	float x = (float) atof( val0 );

	curBone->restLength = x;
}
void SceneLoader::BoneWeightMapNameKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );
	
	assert( curBone != 0 );

	curBone->weightMapName = std::string(ValueText);
}
void SceneLoader::BoneWeightMapOnlyKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );

	assert( curBone != 0 );

	unsigned long item = atol( ValueText);

	curBone->weightMapOnly = (item != 0) ? true : false;
}
void SceneLoader::BoneNormalizationKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );

	assert( curBone != 0 );

	unsigned long item = atol( ValueText);

	curBone->normalisation = (item != 0) ? true : false;
}
void SceneLoader::BoneStrengthKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );
	
	assert( curBone != 0 );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");
	float x = (float) atof( val0 );

	curBone->strength = x;
}

void SceneLoader::BoneScaleBoneStrengthKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );

	assert( curBone != 0 );

	unsigned long item = atol( ValueText);

	curBone->scaleStrength = (item != 0) ? true : false;
}

void SceneLoader::BoneFalloffTypeKey(FILE* f, const char* ValueText)
{
	// this an object key?????????????
	UNREFERENCED_PARAMETER( f );

	assert( curObject != 0 );

	unsigned long item = atol( ValueText )-1;

	assert( item >= Object::INVERSE_DISTANCE );
	assert( item <= Object::INVERSE_DISTANCE_POW128 );

	curObject->fallOffType = (Object::BONE_FALLOFF) item;
}

void SceneLoader::PluginKey(FILE* f, const char* ValueText)
{
	UNREFERENCED_PARAMETER( f );


	char* type = strtok(const_cast<char*>(ValueText)," \xA\xD");
	char* num = strtok(0," \xA\xD");
	char* name = strtok(0,"\xA\xD"); // name can have spaces in

	UNREFERENCED_PARAMETER( num );

	if( strcmp( type, "MasterHandler" ) == 0 ) {
		// global scene plugins
		PlugInMap::const_iterator plIt = masterPluginRegistry.find( name );
		if( plIt != masterPluginRegistry.end() ) {
			auto func = plIt->second;
			(this->*func)( f );
		}
		curNodePlugin = nullptr;
	} else if( strcmp( type, "ItemMotionHandler" ) == 0 ) {
		assert( curObject != 0 );

		// per object motion plugins
		PlugInMap::const_iterator plIt = itemMotionPluginRegistry.find( name );
		if( plIt != itemMotionPluginRegistry.end() ) {
			auto func = plIt->second;
			(this->*func)( f );
		}
		curNodePlugin = nullptr;
	}


}
void SceneLoader::EndPluginKey(FILE* f, const char* ValueText) {
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( ValueText );

}

/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
