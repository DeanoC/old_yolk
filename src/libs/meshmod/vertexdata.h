#pragma once
/** \file MeshModVertex.h
   A vertex system.
   Holds  a number of named vertex elements, 
   contains a list of similar vertices (same position, different elements (uv's, normals)
   contains a list of polyMeshModns each vertex is attached to
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_VERTEXDATA_H_ )
#define MESH_MOD_VERTEXDATA_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#if !defined( MESH_MOD_TYPES_H_ )
#include "types.h"
#endif

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod {

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------
	/**
		A namespace just to keep vertex data seperate from the reset of MeshMod.
		Custom vertex data obviously doesn't require it, but its not a bad
		idea...
	*/
	namespace VertexData {

		// boolean marker adds a boolean per vertex for marking visitation etc
		struct BoolMarker {
			bool marker;

			BoolMarker(){};
			BoolMarker( const bool mark_ ) : marker(mark_) {};

			BoolMarker interpolate( const BoolMarker& b, const float t ) const {
				if( t > 0.5f ) {
					return BoolMarker( b.marker );
				} else {
					return BoolMarker( marker );
				}
			}

			BoolMarker interpolate( const BoolMarker& b, const BoolMarker& c, const float _u, const float _v ) const {
				const float w = 1.0f - _v - _u;
				const float mark = (marker * _u) + (b.marker * _v) + (c.marker * w);
				return BoolMarker( mark > 0.5f ? true : false );
			}

			static const std::string getName() { return "BoolMarker"; };
		};

		struct Float1Tuple {
			float data;

			Float1Tuple() : data(0.f) {};
			Float1Tuple( const float data_ ) : data(data_) {};

			Float1Tuple interpolate( const Float1Tuple& b, const float t ) const {
				const float one_minus_t = 1.0f - t;
				return Float1Tuple(	(data * t) + (b.data * one_minus_t) );
			}

			Float1Tuple interpolate( const Float1Tuple& b, const Float1Tuple& c, const float _u, const float _v ) const {
				const float w = 1.0f - _v - _u;
				return Float1Tuple(	(data * _u) + (b.data * _v) + (c.data * w) );
			}

			static const std::string getName() { return "Float1Tuple"; };
		};

		struct UnsignedInt1Tuple {
			unsigned int data;
			UnsignedInt1Tuple(){};
			UnsignedInt1Tuple( const unsigned int data_) : data(data_) {};

			UnsignedInt1Tuple interpolate( const Float1Tuple& b, const float ft ) const {
				// doubles needed to ensure all ints 32 bit ints are covered
				const double t = (double) ft;
				const double one_minus_t = 1.0 - t;
				return UnsignedInt1Tuple( (unsigned int) ((data * t) + (b.data * one_minus_t)) );
			}

			UnsignedInt1Tuple interpolate( const UnsignedInt1Tuple& b, const UnsignedInt1Tuple& c, const float _u, const float _v ) const {
				const double u = (double) _u;
				const double v = (double) _v;
				const double w = 1.0 - v - u;
				return UnsignedInt1Tuple( (unsigned int)((data * _u) + (b.data * _v) + (c.data * w) ) );
			}

			static const std::string getName() { return "UnsignedInt1Tuple"; };
		};

		struct BoneWeights {
			struct Weight {
				unsigned int	index;
				float			weight;

				Weight(){};
				Weight( const unsigned int in_, const float wt_ ) :
					index(in_), weight(wt_) {};

				// useful for sorting by weight
				bool operator<( const Weight& rhs ) const {
					return weight < rhs.weight;
				}
			};
			std::vector<Weight> boneData;

			BoneWeights interpolate( const BoneWeights& b, const float t ) const {
				const float one_minus_t = 1.0f - t;
				// bone weights are hard to interpolate, if they have different indexes
				// or number of weights per vertex. if both are the same we interpolate
				// the actually weight else we just copy our version
				// TODO when copying the index/weight using the magnitude of t to select 
				// TODO the source would be better I think
				if( boneData.size() == b.boneData.size() ) {
					BoneWeights nbw;
					nbw.boneData.resize( boneData.size() );
					for( unsigned int i = 0; i < boneData.size(); ++i ) {
						if( boneData[i].index == b.boneData[i].index) {
							nbw.boneData[i].index = boneData[i].index;
							nbw.boneData[i].weight = boneData[i].weight * t + 
												b.boneData[i].weight * one_minus_t; 					
						} else {
							nbw.boneData[i].index = boneData[i].index;
							nbw.boneData[i].weight = boneData[i].weight;
						}
					}

					return nbw;					
				} else {					
					return *this;
				}
			}
			BoneWeights interpolate( const BoneWeights& b, const BoneWeights& c, const float u, const float v ) const {
				// bone weights are hard to interpolate, if they have different indexes
				// or number of weights per vertex. if both are the same we interpolate
				// the actually weight else we just copy our version
				// TODO when copying the index/weight using the magnitude of uvw to select 
				// TODO the source would be better I think
				if( boneData.size() == b.boneData.size() && 
					boneData.size() == c.boneData.size() ) {
					BoneWeights nbw;
					nbw.boneData.resize( boneData.size() );
					const float w = 1.0f - v - u;
					for( unsigned int i = 0; i < boneData.size(); ++i ) {
						if( boneData[i].index == b.boneData[i].index &&
							boneData[i].index == c.boneData[i].index ) {
							nbw.boneData[i].index = boneData[i].index;
							nbw.boneData[i].weight = boneData[i].weight * u + 
								b.boneData[i].weight * v + 
								b.boneData[i].weight * w;
						} else {
							nbw.boneData[i].index = boneData[i].index;
							nbw.boneData[i].weight = boneData[i].weight;
						}
					}

					return nbw;					
				} else {					
					return *this;
				}
			}


			static const std::string getName() { return "BoneWeights"; };
		};

		struct PositionDelta {
			float dx;
			float dy;
			float dz;

			PositionDelta() :
				dx(s_floatMarker),dy(s_floatMarker), dz(s_floatMarker)
			{};
			PositionDelta( const float x_, const float y_, const float z_ ) :
					dx(x_), dy(y_), dz(z_) {};

			PositionDelta interpolate( const PositionDelta& b, const float t ) const {
				const float one_minus_t = 1.0f - t;
				return PositionDelta(	(dx * t) + (b.dx * one_minus_t),
					(dy * t) + (b.dy * one_minus_t),
					(dz * t) + (b.dz * one_minus_t) );
			}
			PositionDelta interpolate( const PositionDelta& b, const PositionDelta& c, const float u, const float v ) const {
				const float w = 1.0f - v - u;
				return PositionDelta(	(dx * u) + (b.dx * v) + (c.dx * w),
					(dy * u) + (b.dy * v) + (c.dy * w),
					(dz * u) + (b.dz * v) + (c.dz * w) );
			}
	
			static const std::string getName() { return "PositionDelta"; };
		};

		struct SortMapper {
			unsigned int index;

			SortMapper(){};
			SortMapper( const unsigned int in_ ) :
				index(in_)
			{}
			static const std::string getName() { return "SortMapper"; };
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
