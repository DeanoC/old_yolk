/** \file LWScene2GO.cpp
	Lightwave Scene to GO rep and node representation
   (c) 2002 Deano Calver
   (c) 2012 Cloud Pixies Ltd.
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshimport.h"
#include "core/file_path.h"
#include <boost/lexical_cast.hpp>
#include "LWScene2GO.h"
#include "LWObject2GO.h"
#include "lightwaveloader/LWObject.h"
#include "lightwaveloader/LWSceneLoader.h"
#include "lightwaveloader/DynamicsMasterPlugin.h"
#include "lightwaveloader/DynamicsItemPlugin.h"
#include "lightwaveloader/VtdfhLevelCustomObjPlugin.h"
#include "lightwaveloader/VtdfhEnemyCustomObjPlugin.h"
#include "lightwaveloader/VtdfhMarkerCustomObjPlugin.h"
#include "lightwaveloader/VtdfhItemCustomObjPlugin.h"
#include "meshmod/mesh.h"
#include "meshmod/scene.h"
#include "meshmod/sceneobject.h"


//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
using namespace MeshMod;

extern void LWDynamicsItem2SceneNode( LightWave::DynamicsItemPlugin* dip, MeshMod::SceneNodePtr scnNode );
extern void VtdfhLevelCustomObj2Scene( LightWave::VtdfhLevelCustomObjPlugin* dip, MeshMod::ScenePtr scnNode );
extern void VtdfhEnemyCustomObj2SceneNode( LightWave::VtdfhEnemyCustomObjPlugin* dip, MeshMod::SceneNodePtr scnNode );
extern void VtdfhMarkerCustomObj2SceneNode( LightWave::VtdfhMarkerCustomObjPlugin* dip, MeshMod::SceneNodePtr scnNode );
extern void VtdfhItemCustomObj2SceneNode( LightWave::VtdfhItemCustomObjPlugin* dip, MeshMod::SceneNodePtr scnNode );


//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
namespace MeshImport
{
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

struct TempNode {
	bool					dynamic;
	Math::Matrix4x4			matrix;
	MeshMod::MeshPtr		mesh;
	TempNode*				parent;
	std::list<TempNode*>	children;
	const LightWave::Node*	LWNode;
	MeshMod::SceneNodePtr	sceneNode;

	TempNode() :	mesh(0), 
					parent(0), 
					matrix( Math::IdentityMatrix() ),
					dynamic( false ) ,
					LWNode(0),
					sceneNode(0)
		{};
	void UnlinkNode() {
		std::list<TempNode*>::iterator childIt = children.begin();
		while( childIt != children.end() ) {
			(*childIt)->parent = parent;
			++childIt;
		}
		children.clear();
	}
};

LWSImp::LWSImp( const std::string& filename ) {
	scene = new LightWave::SceneLoader();
	scene->loadFile( filename );
}

LWSImp::~LWSImp() {
	delete scene;
}


MeshMod::ScenePtr LWSImp::toMeshMod() {
	return lightWaveScene2MeshMod( *scene );
}



/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
ScenePtr LWSImp::lightWaveScene2MeshMod( const LightWave::SceneLoader& in )
{
	nodeMap.clear();

	// objects
	auto obIt = in.objects.cbegin();
	while( obIt != in.objects.cend() ) {
		// first build map
		nodeMap[ *obIt ] = new TempNode;
		++obIt;
	}
	#if 0 // TODO bones
	// bones
	auto bnIt = in.m_Bones.cbegin();
	while( bnIt != in.m_Bones.cend() ) {
		// first build map
		nodeMap[ *bnIt ] = new TempNode;
		++bnIt;
	}
	#endif

	std::stack< TempNode* > nodeStack;
	std::stack< Math::Matrix4x4 > matrixStack;
	// build hierachy info
	std::map<const LightWave::Node*, TempNode*>::iterator mapIt = nodeMap.begin();
	while( mapIt != nodeMap.end() )
	{
		mapIt->second->dynamic = LWNodeHasAnimation( mapIt->first );
		mapIt->second->LWNode = mapIt->first;

		// make sure any node that is a bone's object is not removed
		if( mapIt->second->LWNode->type == LightWave::Node::BONE )
		{
			const LightWave::Bone* bone = (const LightWave::Bone*) mapIt->second->LWNode;
			nodeMap[bone->object]->dynamic = true;
		}

		// if we are not the ultimate parent
		if( mapIt->first->parent != 0)
		{
			TempNode* parent = nodeMap[ mapIt->first->parent ];
			parent->children.push_back( mapIt->second );
			mapIt->second->parent = parent;
		} else
		{
			nodeStack.push( mapIt->second);
			matrixStack.push( Math::IdentityMatrix() );
		}
		++mapIt;
	}
	std::list<TempNode*> activeNodes;

	// remove unused nodes and build go meshes for object
	while( !nodeStack.empty() )
	{
		TempNode* node = nodeStack.top();
		Math::Matrix4x4 matrix = matrixStack.top();
		nodeStack.pop();
		matrixStack.pop();

		float		objParams[9];
		Math::Matrix4x4	objMatrix;
		node->LWNode->channels.evaluate(0, objParams );
		LightWave::Node::evaluateLWChannelGroup( 9, objParams, objMatrix );
		matrix = objMatrix * matrix;

		// so far all lw nodes are objects
		// add children with the correct matrix
		std::list<TempNode*>::iterator childIt = node->children.begin();
		while( childIt != node->children.end() ) {
			nodeStack.push( (*childIt) );
			if( node->dynamic == true ) {
				// children are local to this matrix 
				matrixStack.push( Math::IdentityMatrix() );
			} else {
				// this node will be collapse into its parent
				// but children need its matrix to be passed down
				matrixStack.push( matrix );
			}
			++childIt;
		}
		TempNode* parent = node->parent;
/*		if( node->dynamic == false && parent != 0) {
			node->UnlinkNode();
			if( node->LWNode->type == LightWave::Node::OBJECT ) {
				LightWave::Object* obj = (LightWave::Object*)( node->LWNode );
				if( obj->loader == NULL ) {
					if( lwoLoaderCache.find( obj->name ) == lwoLoaderCache.end() ) {
						obj->loader = CORE_NEW LightWave::LWO_Loader( obj->name );
						lwoLoaderCache[ obj->name ] = obj->loader;
					} else {
						obj->loader = lwoLoaderCache[ obj->name ];
					}
				}
				parent->mesh = LightWaveObject2GoMesh( *obj, parent->mesh, obj->layer, matrix );
			}
		} else */ {
			if( node->LWNode->type == LightWave::Node::OBJECT ) {
				LightWave::Object* obj = (LightWave::Object*)( node->LWNode );
				if( obj->loader == NULL ) {
					if( lwoLoaderCache.find( obj->name ) == lwoLoaderCache.end() ) {
						obj->loader = CORE_NEW LightWave::LWO_Loader( obj->name );
						lwoLoaderCache[ obj->name ] = obj->loader;
					} else {
						obj->loader = lwoLoaderCache[ obj->name ];
					}
				}
				node->mesh = LightWaveObject2GoMesh( *obj, 0, obj->layer );
			}
			activeNodes.push_back(node);
		}
	}

	// build game scene nodes
	std::list<TempNode*>::iterator actIt = activeNodes.begin();
	while( actIt != activeNodes.end() ) {
		SceneNodePtr sceneNode = std::make_shared<SceneNode>();
		(*actIt)->sceneNode = sceneNode;
		sceneNode->name = (*actIt)->LWNode->name;

		switch( (*actIt)->LWNode->type ) {
			case LightWave::Node::NULL_OBJECT: {
				int a = 0;
											   }
			case LightWave::Node::OBJECT: {
				LightWave::Object* obj = (LightWave::Object*)( (*actIt)->LWNode );
				if( (*actIt)->mesh ) {
					sceneNode->addObject( (*actIt)->mesh );
				}
				sceneNode->name = Core::FilePath( (*actIt)->LWNode->name ).BaseName().RemoveExtension().value() + 
															boost::lexical_cast<std::string>( obj->objNum & 0x0FFFFFFF);
				break;
			}
			case LightWave::Node::BONE:
			case LightWave::Node::LIGHT:
			case LightWave::Node::CAMERA:
			default:
				break;

		}
		std::replace( sceneNode->name.begin(), sceneNode->name.end(), ' ', '_' );
		std::replace( sceneNode->name.begin(), sceneNode->name.end(), '\"', '_' );
		std::replace( sceneNode->name.begin(), sceneNode->name.end(), '.', '_' );
		std::replace( sceneNode->name.begin(), sceneNode->name.end(), '-', '_' );
		++actIt;
	}

	ScenePtr returner = std::make_shared<Scene>();
	std::list<SceneNodePtr> ultimateParents;

	// second pass build game herichay and make game envelopes
	std::list<TempNode*>::iterator sceIt = activeNodes.begin();
	while( sceIt != activeNodes.end() )
	{
		assert( (*sceIt)->sceneNode != 0 );

		// ultimate parent stuff
		if( (*sceIt)->parent == 0 ) {
			ultimateParents.push_back( (*sceIt)->sceneNode );
		}

		// transfer children
		std::list<TempNode*>::iterator childIt = (*sceIt)->children.begin();
		while( childIt != (*sceIt)->children.end() ) {
			assert( (*childIt)->sceneNode != 0 );
			(*sceIt)->sceneNode->addChild( (*childIt)->sceneNode );
			++childIt;
		}

		if( !LWNodeHasAnimation( (*sceIt)->LWNode ) ) {
			// no animations so just transform 0th time envelope into transform
			// is this wrong for single non 0th time keys? I think the evaluate handles at frame 0
			// however this shold be classed as an animation so not appear here...

			float objParams[9];
			Transform& trans = (*sceIt)->sceneNode->transform;
			(*sceIt)->LWNode->channels.evaluate(0, objParams );
			LightWave::Node::evaluateLWChannelGroup( 9, objParams, trans.position, trans.orientation, trans.scale );

		} else {
			// TODO animation envelopes
		}

//		ConvertChannelGroup( (*sceIt)->m_LWNode, (*sceIt)->m_sceneNode );

/*		if( (*sceIt)->m_LWNode->m_type == LightWave::Node::OBJECT )
		{
			ModelNode* object = (ModelNode*) (*sceIt)->m_sceneNode;
			const LightWave::Object* obj = (const LightWave::Object*) (*sceIt)->m_LWNode;
			std::vector<LightWave::Bone*>::const_iterator bnIt = obj->m_bones.begin();
			while( bnIt != obj->m_bones.end() )
			{
				const LightWave::Bone* bone = (*bnIt);
				BoneNode* bnNode = (BoneNode*) nodeMap[ bone ]->m_sceneNode;
				assert( bnNode != 0 );

				object->addBone( bnNode );
				++bnIt;
			}
		}*/

		// add LW node plugins that we understand as Mesh Mod node properties
		auto npropIt = (*sceIt)->LWNode->plugins.cbegin();
		auto scnNode = (*sceIt)->sceneNode;

		while( npropIt != (*sceIt)->LWNode->plugins.cend() ) {
			if( (*npropIt)->pluginName == LightWave::DynamicsItemPlugin::PluginName ) {
				// Dynamics Item hold node physics parameters
				auto dip = (LightWave::DynamicsItemPlugin*) *npropIt;
				if( dip->enabled ) {
					LWDynamicsItem2SceneNode( dip, scnNode );
				}
			} else if( (*npropIt)->pluginName == LightWave::VtdfhLevelCustomObjPlugin::PluginName ) {
				// whilst this is a node plugin is should probably be a master plugin, move the data to the 
				// global property pool
				auto dip = (LightWave::VtdfhLevelCustomObjPlugin*) *npropIt;
				VtdfhLevelCustomObj2Scene( dip, returner );
			} else if( (*npropIt)->pluginName == LightWave::VtdfhEnemyCustomObjPlugin::PluginName ) {
				auto dip = (LightWave::VtdfhEnemyCustomObjPlugin*) *npropIt;
				VtdfhEnemyCustomObj2SceneNode( dip, scnNode );
			} else if( (*npropIt)->pluginName == LightWave::VtdfhMarkerCustomObjPlugin::PluginName ) {
				auto dip = (LightWave::VtdfhMarkerCustomObjPlugin*) *npropIt;
				VtdfhMarkerCustomObj2SceneNode( dip, scnNode );
			} else if( (*npropIt)->pluginName == LightWave::VtdfhItemCustomObjPlugin::PluginName ) {
				auto dip = (LightWave::VtdfhItemCustomObjPlugin*) *npropIt;
				VtdfhItemCustomObj2SceneNode( dip, scnNode );
			}


			++npropIt;
		}

		++sceIt;
	}
	returner->sceneNodes.insert( returner->sceneNodes.end(), ultimateParents.begin(), ultimateParents.end() );

	// add LW master plugins that we understand as Mesh Mod scene properties
	auto mpropIt = in.masterPluginData.cbegin();
	while( mpropIt != in.masterPluginData.cend() ) {
		if( (*mpropIt)->pluginName == LightWave::DynamicsMasterPlugin::PluginName ) {
			// Dynamics Master hold global physics parameters
			auto dmp = (LightWave::DynamicsMasterPlugin*) *mpropIt;
			if( dmp->enabled ) {
				returner->properties.push_back( std::make_shared<Property>( "gravity" , dmp->gravity ) );
				returner->properties.push_back( std::make_shared<Property>( "stepsPerSecond", dmp->stepsPerSecond ) );
			}
		}

		++mpropIt;
	}

	// clean up
	std::map<const LightWave::Node*, TempNode*>::iterator clnIt = nodeMap.begin();
	while( clnIt != nodeMap.end() ) {
		delete clnIt->second;
		++clnIt;
	}
	return returner;	
}
/*
// this is currently a bit silly as they are almost the same but in future they probably
// won't be
void LWScene2GO_Local::ConvertChannelGroup( const LightWave::Node* in, SceneNodePtr out )
{
	const LightWave::ChannelGroup& inGrp = in->m_channels;
	SceneNode::ChannelGroup& outGrp = out->getChannelGroup();

	outGrp.m_numChannels = inGrp.m_numChannels;
	outGrp.m_Channels.resize( outGrp.m_numChannels );
	for(unsigned int i=0;i < inGrp.m_numChannels;i++)
	{
		::Envelope& outEnv = outGrp.m_Channels[i];
		const LightWave::Envelope& inEnv = inGrp.m_Channels[i];
		outEnv.m_pre = (::Envelope::BEHAVIOUR_TYPE) inEnv.m_pre;
		outEnv.m_post = (::Envelope::BEHAVIOUR_TYPE) inEnv.m_post;
		outEnv.m_numKeys = inEnv.m_numKeys;
		outEnv.m_keys.resize( outEnv.m_numKeys );
		for(unsigned int j=0;j < inEnv.m_numKeys;j++)
		{
			::Envelope::Key& outEKey = outEnv.m_keys[j];
			const LightWave::Envelope::Key& inEKey = inEnv.m_keys[j];
			outEKey.m_spantype = (::Envelope::Key::SPAN_TYPE) inEKey.m_spantype;
			outEKey.m_value = inEKey.m_value;
			outEKey.m_time = inEKey.m_time;
			outEKey.m_tension = inEKey.m_tension;
			outEKey.m_continuity = inEKey.m_continuity;
			outEKey.m_bias = inEKey.m_bias;
			outEKey.m_p0 = inEKey.m_p0;
			outEKey.m_p1 = inEKey.m_p1;
			outEKey.m_p2 = inEKey.m_p2;
			outEKey.m_p3 = inEKey.m_p3;
		}
	}
}
*/

bool LWSImp::LWNodeHasAnimation( const LightWave::Node* node ) {
	const LightWave::ChannelGroup& chGrp = node->channels;

	for( unsigned int i = 0; i < chGrp.numChannels; i++) {
		if( chGrp.channels[i].numKeys > 1 )
			return true;
	}

	return false;
}


}