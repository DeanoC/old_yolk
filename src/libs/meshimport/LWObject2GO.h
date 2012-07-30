/** \file LWObject2Go.h
   (c) 2002 Dean Calver
 */

#if !defined( GOIMPORT_LWOBJECT2GO_H_ )
#define GOIMPORT_LWOBJECT2GO_H_
#pragma once

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

// forward decl
namespace LightWave
{
	class Object;
};

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshmod/mesh.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshImport
{

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Externals
//---------------------------------------------------------------------------
extern MeshMod::MeshPtr LightWaveObject2GoMesh(	const LightWave::Object& in, 
										MeshMod::MeshPtr out = MeshMod::MeshPtr(),
										const unsigned int layer = -1,
										const Math::Matrix4x4& transform =  Math::IdentityMatrix() );
class LWOImp  : public ImportInterface {
public:
	LWOImp( const std::string& filename );
	virtual ~LWOImp();

	virtual bool loadedOk() const override;
	virtual MeshMod::ScenePtr toMeshMod() override;

private:
	MeshMod::MeshPtr 	mesh;
	MeshMod::ScenePtr 	scene;
};

} // end namespace

//---------------------------------------------------------------------------
// End Header file
//---------------------------------------------------------------------------
#endif
