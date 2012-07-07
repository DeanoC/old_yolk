#pragma once
/** \file MeshModRGBMaterialParameter.h
An RGB material parameter
   (c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_SCALAR_MATERIAL_PARAMETER_H_ )
#define MESH_MOD_SCALAR_MATERIAL_PARAMETER_H_

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
		//! single float scalar
		struct FloatScalar : public ParameterBase
		{
			float x;
				
			//! default ctor, marks this scalar as invalid
			FloatScalar() : 
				x(s_floatMarker)
			{};
			//! 1 float ctor
			FloatScalar( const float x_) :
				x(x_)
			{}
			//! 1 double ctor (truncate)
			FloatScalar( const double x_) :
				x(static_cast<float>(x_))
			{}
			//! is the position equal to the data passed in, using an epsilon parameter to decide
			bool equal(const FloatScalar& other, const float epsilon = s_epsilon1e_5) const 
			{

				if( (fabsf(x - other.x) < epsilon) )
					return true;
				else return false;
			}

			//! is the position equal to the data passed in, using an epsilon parameter to decide
			bool equal(const float x_, const float epsilon = s_epsilon1e_5) const 
			{
				return equal( FloatScalar(x_), epsilon );
			}

			//! is this normal valid
			bool isValid() const
			{
				// for float NAN we must use interger compares
				const unsigned int im = *(reinterpret_cast<const unsigned int *>(&s_floatMarker));

				const unsigned int ix = *(reinterpret_cast<const unsigned int *>(&x));

				// either our marker NAN return false;
				if( ix == im )
					return false;
				else return true;
			}
			//! name is used to get this data
			static const std::string getName() { return "FloatScalar"; };
		};
		//! Material Parameters RGB data
		typedef ImplElements< MaterialData::FloatScalar, MaterialParameters_ > FloatScalarElements;
	}
} // end MeshMod

#endif // MESH_MOD_SCALAR_MATERIAL_PARAMETER_H_
