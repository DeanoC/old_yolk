#pragma once
/** \file MeshModRGBMaterialParameter.h
An RGB material parameter
   (c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_TEXTURE_MATERIAL_PARAMETER_H_ )
#define MESH_MOD_TEXTURE_MATERIAL_PARAMETER_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined( MESH_MOD_VARICONTAINER_H_ )
#	include "varicontainer.h"
#endif
#if !defined( MESH_MOD_MATERIAL_PARAMETER_BASE_H_ )
#	include "materialparameterbase.h"
#endif

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{
	namespace MaterialData 
	{

		//! Texture Parameter 
		struct TextureParameter : public ParameterBase {
			enum TextureType {
				TT_CUBE = 0,
				TT_1D=1,
				TT_2D=2,
				TT_3D=3,
			};
			TextureType	type;
			std::string textureName;
				
			//! default ctor, marks this textureName as invalid
			TextureParameter() {};
			//! type and texture name
			TextureParameter( TextureType _type, const std::string& _name ) :
				type(_type), textureName(_name) {}

				//! is the position equal to the data passed in, using an epsilon parameter to decide
			bool equal(const TextureParameter& other ) const {
				return ( type == other.type && textureName == other.textureName );
			}

			//! is this Texture valid
			bool isValid() const {
				return !textureName.empty();
			}


			//! name is used to get this data
			static const std::string getName() { return "Texture"; };
		};
		//! Material Parameters Texture data
		typedef ImplElements< TextureParameter, MaterialParameters_ > TextureParameterElements;

	}

} // end MeshMod

#endif // MESH_MOD_RGB_MATERIAL_PARAMETER_H_
