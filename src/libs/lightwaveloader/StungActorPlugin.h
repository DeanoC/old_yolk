/** \file LWNode.h
   A Lightwave node.
   Base of all lightwave object (a dummy node)
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_STUNGACTORPLUGIN_H_ )
#define LIGHTWAVE_STUNGACTORPLUGIN_H_

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
	struct StungActorPlugin : public LightWave::NodePlugin
	{
		enum ACTOR_TYPE
		{
			AT_ENEMY			=	0,
			AT_HEALTH			=	1,
			AT_WEAPON			=	2,
		} m_type;
		
		enum ENEMY_TYPE
		{
			ET_ANT				=	0,
			ET_MOSY				=	1,
		} m_enemyType;
		
		enum WEAPON_TYPE
		{
			WT_PISTOL			=	0,
			WT_SHOTGUN			=	1,
			WT_ASSAULT_RIFLE	=	2,
			WT_UZI				=	3,
		} m_weaponType;

		unsigned int m_health;

		static const std::string PluginName;

		StungActorPlugin() : 
			NodePlugin( PluginName ),
			m_type(AT_HEALTH),
			m_enemyType(ET_ANT),
			m_health(100),
			m_weaponType(WT_PISTOL)
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
