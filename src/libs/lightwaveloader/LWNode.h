/** \file LWNode.h
   A Lightwave node.
   Base of all lightwave object (a dummy node)
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_NODE_H_ )
#define LIGHTWAVE_NODE_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LWChannelGroup.h"
#include "LWNodePlugin.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------
	/**
	Short description.
	Detailed description
	*/
	struct Node {
		enum {
			NULL_OBJECT		= 0,
			OBJECT			= 1,
			LIGHT			= 2,
			CAMERA			= 3,
			BONE			= 4,
			UNKNOWN			= 0xFFFFFFFF,
		}							type;

		/// if m_parent == 0 then link to ultimate parent
		Node*						parent;
		/// channel group of this node
		ChannelGroup				channels;
		/// list of any plugins attached to this node
		std::vector<NodePlugin*>	plugins;
		/// Node name or filename
		std::string			name;


		Node() :	parent(0),
					type(Node::UNKNOWN)
		{};

		~Node() {
			std::vector<NodePlugin*>::iterator plIt = plugins.begin();
			while( plIt != plugins.end() ) {
				delete (*plIt);
				++plIt;
			}
		};

		static void evaluateHPBAngles( const float heading , const float pitch, const float bank,
										Math::Matrix4x4& out );
		static void evaluateHPBAngles( const float heading , const float pitch, const float bank,
										Math::Quaternion& out );

		static void evaluateLWChannelGroup( const unsigned int numChannels, const float* channels, 
										Math::Matrix4x4& out );
		static void evaluateLWChannelGroup( const unsigned int numChannels, const float* channels, 
										Math::Vector3& outPos, Math::Quaternion& outQuat, Math::Vector3& outScale );
	};

} // end namespace
//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Externals
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// End Header file
//---------------------------------------------------------------------------
#endif
