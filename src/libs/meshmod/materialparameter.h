#pragma once
/** \file MeshModMaterialParameter.h
	Material parameter containers attached a variable
	sized and typed list to a material
   (c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_MATERIAL_PARAMETER_H_ )
#define MESH_MOD_MATERIAL_PARAMETER_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined( MESH_MOD_TYPES_H_ )
#	include "types.h"
#endif

#if !defined( MESH_MOD_VARICONTAINER_H_ )
#	include "varicontainer.h"
#endif

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{

	namespace MaterialData
	{
		//! we now have a varicontainer for material parameters
		typedef VariContainer< Elements<MaterialParameters_> > ParameterContainer;

		//! a collection parameters that can be attached to a particular material
		struct Parameters {
			static const std::string getName() { return "Parameters"; };

			ParameterContainer			parameters;
		};
	}
	//! material parameters elements to be attached to a mesh 
	typedef ImplElements< MaterialData::Parameters, Mesh_ > MaterialParameterElements;


} // end MeshMod

#endif // MESH_MOD_NORMAL_VERTEX_H_
