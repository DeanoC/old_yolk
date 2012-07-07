#pragma once
/** \file MeshModPositionVertex.h
   A vertex holding position.
   Position vertex data is fundemental to the operation of 
   MeshMod, Its a 3-tuple of float data
   (c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_POSITION_VERTEX_H_ )
#define MESH_MOD_POSITION_VERTEX_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined( MESH_MOD_TYPES_H_ )
#	include "types.h"
#endif
#if !defined( MESH_MOD_INDEX_TYPES_H_ )
#	include "indextypes.h"
#endif
#if !defined( MESH_MOD_VARICONTAINER_H_ )
#	include "varicontainer.h"
#endif

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{
	namespace VertexData
	{
		//! a position is a 3-tuple representing a physical position, its
		//! a float per coordinate which should be enough for most applications
		//! As position is a 'special' vertex attribute, if you require more 
		//! accuracy its probably a good idea to have a seperate more accurate
		//! position attribute as well as the float position, cos currently the
		//! internal system will often truncate to float anyway.
		//! \todo make accuracy a compile time option
		struct Position {
			enum AXIS {
				X = 0,
				Y = 1,
				Z = 2,
			};
			//! each coordinate
			float x, y ,z;

			//! default ctor, marks the position as invalid
			Position() :
				x(s_floatMarker),y(s_floatMarker), z(s_floatMarker)
			{};
			//! ctor that takes 3 floats
			Position( const float x_, const float y_, const float z_) :
				x(x_), y(y_), z(z_)
			{}
			const float& get( AXIS _axis ) const {
				switch( _axis ) {
				default: 
				case X: return x;
				case Y: return y;
				case Z: return z;
				}
			}
			float& get( AXIS _axis ) {
				switch( _axis ) {
				default: 
				case X: return x;
				case Y: return y;
				case Z: return z;
				}
			}
			Math::Vector3 getVector3() const {
				return Math::Vector3( x, y, z );
			}
			Position interpolate( const Position& b, const float t ) const {
				const float one_minus_t = 1.0f - t;
				return Position(	(x * t) + (b.x * one_minus_t),
					(y * t) + (b.y * one_minus_t),
					(z * t) + (b.z * one_minus_t) );
			}
			Position interpolate( const Position& b, const Position& c, const float u, const float v ) const {
				const float w = 1.0f - v - u;
				return Position(	(x * u) + (b.x * v) + (c.x * w),
					(y * u) + (b.y * v) + (c.y * w),
					(z * u) + (b.z * v) + (c.z * w) );
			}
			//! is the position equal to the data passed in, using an epsilon parameter to decide
			bool equal(const Position& other, const float epsilon = s_epsilon1e_5) const 
			{

				if( (fabsf(x - other.x) < epsilon) &&
					(fabsf(y - other.y) < epsilon) &&
					(fabsf(z - other.z) < epsilon) )
					return true;
				else return false;
			}

			//! is the position equal to the data passed in, using an epsilon parameter to decide
			bool equal(const float x_, const float y_, const float z_, const float epsilon = s_epsilon1e_5) const 
			{
				return equal( Position(x_, y_, z_), epsilon );
			}

			//! is this position valid (has real vertex data)
			bool isValid() const
			{
				// for float NAN we must use interger compares
				const unsigned int im = *(reinterpret_cast<const unsigned int *>(&s_floatMarker));

				const unsigned int ix = *(reinterpret_cast<const unsigned int *>(&x));
				const unsigned int iy = *(reinterpret_cast<const unsigned int *>(&y));
				const unsigned int iz = *(reinterpret_cast<const unsigned int *>(&z));

				// either our marker NAN return false;
				if( ix == im || iy == im || iz == im)
					return false;
				else return true;
			}

			//! name used to get this kind of data
			static const std::string getName() { return "Position"; };
		};
	}

	//! position vertex Element (x,y,z)
	typedef ImplElements< VertexData::Position, Vertex_ > PositionVertexElements;

} // end MeshMod
#endif // MESH_MOD_POSITION_VERTEX_H_
