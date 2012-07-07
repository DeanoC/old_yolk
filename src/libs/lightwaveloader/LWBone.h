/** \file LWBone.h
   A Lightwave bone.
   lightwave bone (a subclass of a lightwave node)
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_BONE_H_ )
#define LIGHTWAVE_BONE_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LWNode.h"

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
	class Bone : public Node
	{
	public:

		std::string		name;
		bool			active;
		Math::Vector3	restPosition;
		Math::Vector3	restDirection;
		float			restLength;
		std::string		weightMapName;
		bool			weightMapOnly;
		bool			normalisation;
		float			strength;
		bool			scaleStrength;

		bool			limitedRange;
		float			innerLimit;
		float			outerLimit;

		Math::Vector3	worldRestPos;
		Math::Vector3	worldRestDir; // HPB angles misnamed by Lightwave
		Math::Matrix4x4	restMatrix;
		Node*			object;

		Bone() :
			active(false),
			restPosition( 0,0,0 ),
			restDirection( 0,0,0 ),
			restLength( 0.f ),
			weightMapOnly( false ),
			normalisation( false ),
			strength( 0.f ),
			scaleStrength(false),
			limitedRange(false),
			innerLimit( 0.f ),
			outerLimit( 0.f ),
			worldRestPos( 0,0,0 ),
			worldRestDir( 0,0,0 ),
			object( 0 )
		{
			type = Node::BONE;
		}
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
