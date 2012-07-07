/** \file LWSurface.h
Lightwave surface defination.
(c) 2002 Dean Calver
*/

#if !defined( LIGHTWAVE_LOADER_LWSURFACE_H_ )
#define LIGHTWAVE_LOADER_LWSURFACE_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LWTypes.h"
#include "LWBlok.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{
	struct Surface
	{
		Surface() :
			baseColour( 0,0,0 ),
			diffuse(0),
			specular(0),
			luminosity(0),
			transparency(0),
			translunency(0),
			glossiness(0.4f),
			doubleSided(false),
			maxSmoothingAngle( -1.f ),
			curBlock(0)
		{};

		std::string name;
		unsigned int tagID;
		COL12	baseColour;
		FP4		diffuse;
		FP4		specular;
		FP4		luminosity;
		FP4		reflection;
		FP4		transparency;
		FP4		translunency;
		FP4		glossiness;
		bool	doubleSided;
		FP4		maxSmoothingAngle; // <=0 not smoothed (in radians)


		std::vector<LWBlok> blocks;
		LWBlok* curBlock;

		// colour channel blocks sorted by ordinal
		std::vector<LWBlok*> colourBlocks;
	};
}
//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------

/**
Short description.
Detailed description
*/

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
