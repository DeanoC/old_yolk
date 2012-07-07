#pragma once
/** \file MeshModVertex.h
   A vertex system.
   Holds  a number of named vertex elements, 
   contains a list of similar vertices (same position, different elements (uv's, normals)
   contains a list of polyMeshModns each vertex is attached to
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_MATERIAL_H_ )
#define MESH_MOD_MATERIAL_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined( MESH_MOD_MATERIALDATA_H_ )
#include "materialdata.h"
#endif
//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------


	//! material bits and peices (usually just name, shader and parameter now...) attached to a mesh
	typedef VariContainer< Elements<Mesh_> > MaterialElementsContainer;

	typedef ImplElements< MaterialData::Name, Mesh_ > NameMaterialElements;
	typedef ImplElements< MaterialData::Shader, Mesh_ > ShaderMaterialElements;

	// these 3 are the old system, these days use the parameter system
	typedef ImplElements< MaterialData::LightParams, Mesh_ > LightParamsMaterialElements;
	typedef ImplElements< MaterialData::BackFace, Mesh_ > BackFaceMaterialElements;
	typedef ImplElements< MaterialData::Texture, Mesh_ > TextureMaterialElements;


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
