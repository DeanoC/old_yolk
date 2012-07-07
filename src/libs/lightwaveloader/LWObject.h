/** \file LWObject.h
   A Lightwave object.
   lightwave object (a subclass of a lightwave node)
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_OBJECT_H_ )
#define LIGHTWAVE_OBJECT_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LWNode.h"
#include "LWBone.h"
#include "LWOLoader.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{
	class Object : public Node
	{
	public:
		enum BONE_FALLOFF {
			INVERSE_DISTANCE		= 0,
			INVERSE_DISTANCE_POW2	= 1,
			INVERSE_DISTANCE_POW4	= 2, ///< Default if no falloff key
			INVERSE_DISTANCE_POW8	= 3,
			INVERSE_DISTANCE_POW16	= 4,
			INVERSE_DISTANCE_POW32	= 5,
			INVERSE_DISTANCE_POW64	= 6,
			INVERSE_DISTANCE_POW128	= 7,
		};

		static float getExponent( BONE_FALLOFF falloff ) {
			return (float)(1 << (unsigned int)(falloff) );
		}

		unsigned int		layer;
		std::vector<Bone*>	bones;
		BONE_FALLOFF		fallOffType;
					
		LWO_Loader*			loader;

		Object() :
			fallOffType(INVERSE_DISTANCE_POW4),
			loader( 0 ) {}
		~Object() {
			//delete m_loader;
		}

	};

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------

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
