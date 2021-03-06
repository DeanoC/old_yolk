#pragma once
/** \file MeshModVertex.h
   A vertex system.
   Holds  a number of named vertex elements, 
   contains a list of similar vertices (same position, different elements (uv's, normals)
   contains a list of polyMeshModns each vertex is attached to
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_MATERIALDATA_H_ )
#define MESH_MOD_MATERIALDATA_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------
	namespace MaterialData {
		struct Name {
			std::string matName;

			Name(){};
			Name( const std::string& name_) :
				matName(name_)
			{}
			
			static const std::string getName() { return "Name"; };
		};

		struct Shader {
			std::string shaderName;

			Shader(){};
			Shader( const std::string& name_) :
				shaderName(name_)
			{}
			
			static const std::string getName() { return "Shader"; };
		};


		struct LightParams {
			float baseColour[3];
			float Kd; // diffuse
			float Ks; // specular
			float Kl; // luminosity
			float specular_exponent;	// spec exp
			float transparency;			// 1 = solid 0 = full see through
			float translucency;			// as transp but for lights
			float Kr;					// reflection 

			static const std::string getName() { return "LightParams"; };
		};

		struct BackFace {
			enum {
				FRONT	= 0,
				BACK	= 1,
				NONE	= 2,
			} facing;

			static const std::string getName() { return "BackFace"; };
		};

		struct Texture {
			std::string fileName;
			std::string uvChannel;

			static const std::string getName() { return "Texture"; };
		};
	}
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
