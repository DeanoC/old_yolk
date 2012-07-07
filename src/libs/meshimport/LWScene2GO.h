/** \file LWScene2Go.h
	Converts a scene into a renderable version using GO converter
   (c) 2002 Dean Calver
 */

#if !defined( GOIMPORT_LWSCENE2GO_H_ )
#define GOIMPORT_LWSCENE2GO_H_
#pragma once

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

#include "meshmod/scene.h"

namespace LightWave {
	class SceneLoader;
	class Node;
}

namespace MeshImport
{
	//---------------------------------------------------------------------------
	// Classes and structures
	//---------------------------------------------------------------------------
	DECLARE_EXCEPTION( LWSImportFileError, A error occured loading a LightWave Scene );

	/**
	Short description.
	Detailed description
	*/
	class LWSImp  : public ImportInterface {
	public:
		LWSImp( const std::string& filename );
		virtual ~LWSImp();

		virtual bool loadedOk() const override { return scene != NULL; }
		virtual MeshMod::ScenePtr toMeshMod() override;

	private:
		MeshMod::ScenePtr lightWaveScene2MeshMod( const LightWave::SceneLoader& in );

		bool LWNodeHasAnimation( const LightWave::Node* node );
		void ConvertChannelGroup( const LightWave::Node* in, MeshMod::SceneNodePtr out );

		std::map<const LightWave::Node*, class TempNode*> nodeMap;

		LightWave::SceneLoader*	scene;
	};


}
//---------------------------------------------------------------------------
// End Header file
//---------------------------------------------------------------------------
#endif

