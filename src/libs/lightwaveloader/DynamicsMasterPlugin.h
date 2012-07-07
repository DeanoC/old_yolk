/** \file DynamicsMasterPlugin.h
   (c) 2012 Dean Calver
 */

#if !defined( LIGHTWAVE_DYNAMICS_MASTER_PLUGIN_H_ )
#define LIGHTWAVE_DYNAMICS_MASTER_PLUGIN_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
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
	struct DynamicsMasterPlugin : public LightWave::NodePlugin {

		unsigned int version;
		bool drawShapes;
		bool enabled;
		unsigned int stepsPerSecond;
		float timeScale;
		Math::Vector3 gravity;

		static const std::string PluginName;

		DynamicsMasterPlugin() : 
			NodePlugin( PluginName ),
			version(0),
			drawShapes(false),
			enabled(false),
			stepsPerSecond( 180 ),
			timeScale( 1.0f ),
			gravity( 0, -9.8f, 0 )
		{
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
