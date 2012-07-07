/** \file LWLayer.h
   A Lightwave object V2 layer.
   Also contains geometry etc
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_CHANNELGROUP_H_ )
#define LIGHTWAVE_CHANNELGROUP_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LWEnvelope.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{
	/// channel groups are groups of envelope representing a splined transformation
	struct ChannelGroup
	{
		/// number of channels (usually 9, 3 pos, 3 rot, 3 scale)
		unsigned int			numChannels;
		/// the envelopes the make up this channel group.ALWAYs numChannels size
		std::vector<LightWave::Envelope>	channels;

		/// evaluate the channel group at timt into the array of floats
		void evaluate( const float time, float* rbt ) const;

		unsigned int			parentNum;	///< ignore parentNum of the channel group (used in the LW loader)

		ChannelGroup() :		///< default ctor
			parentNum(0),
			numChannels(0) {}
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
