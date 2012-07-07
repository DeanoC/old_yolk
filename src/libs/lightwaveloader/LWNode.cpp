/** \file LWOLayer.cpp
   Light wave 7 Layer implementation.
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
#include "LWNode.h"
#include "LWNodePlugin.h"

using namespace LightWave;

//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
using namespace LightWave;

/**
calculates a rotation matrix from LW style heading, pitch and bank angles
*/
void Node::evaluateHPBAngles( const float heading , const float pitch, const float bank,
								 Math::Matrix4x4& out )
{
	Math::Matrix4x4 headingMat = Math::CreateYRotationMatrix( heading );
	Math::Matrix4x4 pitchMat = Math::CreateXRotationMatrix( pitch );
	Math::Matrix4x4 bankMat = Math::CreateZRotationMatrix( bank );

	out = (bankMat * pitchMat) * headingMat;
}
/*
calculates a Quaternion from LW style heading, pitch and bank angles
*/
void Node::evaluateHPBAngles( const float heading , const float pitch, const float bank,
								 Math::Quaternion& out ) {
	Math::Quaternion headingQuat = Math::CreateRotationQuat( Math::Vector3(0,1,0), heading );
	Math::Quaternion pitchQuat = Math::CreateRotationQuat( Math::Vector3(1,0,0), pitch );
	Math::Quaternion bankQuat = Math::CreateRotationQuat( Math::Vector3(0,0,1), bank );

	out = (bankQuat * pitchQuat) * headingQuat;
}

void Node::evaluateLWChannelGroup( const unsigned int numChannels, const float* channels, 
									Math::Matrix4x4& out )
{
	Math::Matrix4x4 posMat, rotMat, scaleMat;
	// 1st 3 channels are position
	if(numChannels >= 3)
	{
		posMat = Math::CreateTranslationMatrix( channels[0], channels[1], channels[2] );
	}
	// next 3 channels are euler rotations
	if(numChannels >= 6)
	{
		evaluateHPBAngles(channels[3], channels[4], channels[5], rotMat);

		// next 3 channels are scale
		if(numChannels >= 9)
		{
			scaleMat = Math::CreateScaleMatrix( channels[6], channels[7], channels[8] );

			out = (rotMat * scaleMat) * posMat;
		} else
		{
			out = rotMat * posMat;
		}
	} else 
	{
		out = posMat;
	}
}

void Node::evaluateLWChannelGroup( const unsigned int numChannels, const float* channels, 
									Math::Vector3& outPos, Math::Quaternion& outQuat, Math::Vector3& outScale )
{
	outPos = Math::Vector3( 0, 0, 0 );
	outQuat = Math::IdentityQuat();
	outScale = Math::Vector3(1,1,1);

	// 1st 3 channels are position
	if(numChannels >= 3) {
		outPos = Math::Vector3( channels[0], channels[1], channels[2] );
	}
	// next 3 channels are euler rotations
	if(numChannels >= 6) {
		evaluateHPBAngles(channels[3], channels[4], channels[5], outQuat);
	}
	// next 3 channels are scale
	if(numChannels >= 9) {
		outScale = Math::Vector3( channels[6], channels[7], channels[8] );
	}
}

/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
