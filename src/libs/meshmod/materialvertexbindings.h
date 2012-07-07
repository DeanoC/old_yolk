#pragma once
/** \file MeshModMaterialVertexBindings.h
	A list of vertex types and sub types this material requires

(c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_MATERIAL_VERTEX_BINDINGS_H_ )
#define MESH_MOD_MATERIAL_VERTEX_BINDINGS_H_ 

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
		//! a collection parameters that can be attached to a particular material
		struct VertexBindings
		{
			static const std::string getName() { return "VertexBindings"; };

			VertexBindings() :
				numVertexBindings(0)
			{
			}

			// currently have a max of 16 vertex bindings... should be 
			// enough for now i think, easy to pump up if a problem

			static const unsigned int MAX_VERTEX_BINDINGS = 16;
			unsigned int numVertexBindings;

			struct VertexBinding
			{
				std::string		name;
				std::string		subName;
			} vertexBindings[ MAX_VERTEX_BINDINGS ];

			void AddBinding( const std::string& name, const std::string& subname = std::string() )
			{
				CORE_ASSERT( numVertexBindings < MAX_VERTEX_BINDINGS );
				vertexBindings[ numVertexBindings ].name = name;
				vertexBindings[ numVertexBindings ].subName = subname;
				++numVertexBindings;
			}
		};
	}
	//! material parameters elements to be attached to a mesh 
	typedef ImplElements< MaterialData::VertexBindings, Mesh_ > VertexBindingsElements;


} // end MeshMod

#endif // MESH_MOD_NORMAL_VERTEX_H_
