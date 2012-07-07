/** \file LWChannelGroup.cpp
   (c) 2002 Deano Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "lightwaveloader.h"
#include "LWChannelGroup.h"

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
Short description.
Detailed description
@param param description
@return description
@exception description
*/
void LightWave::ChannelGroup::evaluate( const float time, float* rbt ) const
{
	for(unsigned int i=0;i < numChannels;i++)
	{
		rbt[i] = channels[i].evaluate(time);
	}
}