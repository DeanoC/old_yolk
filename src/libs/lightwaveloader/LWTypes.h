/** \file LWTypes.h
   Lightwave types.
   Some types defined in the file format help file
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_LOADER_LWTYPES_H_ )
#define LIGHTWAVE_LOADER_LWTYPES_H_

#if !defined(WEIRD_CORE_VECTOR_MATH_H)
#include <core/vector_math.h>
#endif

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{
	// typedef for LWO2 types
	typedef uint8_t						U1;
	typedef uint16_t					U2;
	typedef uint32_t					U4;
	typedef int8_t						S1;
	typedef int16_t						S2;
	typedef int32_t						S4;
	typedef Math::Vector3				VEC12;
	typedef VEC12						COL12;
	typedef std::string					S0;
	typedef struct{ char data[4]; }		ID4;
	typedef uint32_t					VX;
	typedef float						FP4;

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------

	// Endian convertion macros
	inline U4 GetInt32EC( U4* i )
	{
		unsigned int a = *( ((unsigned char*)i) + 3);
		unsigned int b = *( ((unsigned char*)i) + 2);
		unsigned int c = *( ((unsigned char*)i) + 1);
		unsigned int d = *( ((unsigned char*)i) + 0);

		return (U4)( (a <<  0) | (b <<  8) | (c << 16) | (d << 24) );
	}
	inline S4 GetInt32EC( S4* i )
	{
		unsigned int a = *( ((unsigned char*)i) + 3);
		unsigned int b = *( ((unsigned char*)i) + 2);
		unsigned int c = *( ((unsigned char*)i) + 1);
		unsigned int d = *( ((unsigned char*)i) + 0);

		return (S4)( (a <<  0) | (b <<  8) | (c << 16) | (d << 24) );
	}

	inline U2 GetInt16EC( U2* i )
	{
		unsigned char a = *( ((unsigned char*)i) + 1);
		unsigned char b = *( ((unsigned char*)i) + 0);

		return U2(	((unsigned int)(a & 0xff) <<  0) |
					((unsigned int)(b & 0xff) <<  8) );
	}
	inline S2 GetInt16EC( S2* i )
	{
		unsigned char a = *( ((unsigned char*)i) + 1);
		unsigned char b = *( ((unsigned char*)i) + 0);

		return S2(	((unsigned int)(a & 0xff) <<  0) |
					((unsigned int)(b & 0xff) <<  8) );
	}

	// readers for LWO2 types
	inline U1 read( FILE *f, U1& val )
	{
		fread( &val, sizeof(U1), 1, f);
		return val;
	}
	inline U2 read( FILE *f, U2& val )
	{
		fread( &val, sizeof(U2), 1, f);
		val = (U2) GetInt16EC( &val );
		return val;
	}
	inline U4 read( FILE *f, U4& val )
	{
		fread( &val, sizeof(U4), 1, f);
		val = GetInt32EC( &val );
		return val;
	}
	inline S1 read( FILE *f, S1& val )
	{
		fread( &val, sizeof(S1), 1, f);
		return val;
	}
	inline S2 read( FILE *f, S2& val )
	{
		fread( &val, sizeof(S2), 1, f);
		val = (S2) GetInt16EC( &val );
		return val;
	}
	inline S4 read( FILE *f, S4& val )
	{
		fread( &val, sizeof(S4), 1, f);
		val = GetInt32EC( &val );
		return val;
	}
	inline FP4 read( FILE *f, FP4& val )
	{
		S4 temp;
		fread( &temp, sizeof(S4), 1, f);
		temp = GetInt32EC( &temp );
		val = *(reinterpret_cast<float*>(&temp));

		return val;
	}

	inline ID4 read( FILE *f, ID4& val )
	{
		fread( &val, sizeof(ID4), 1, f);
		return val;
	}

	inline VEC12 read( FILE *f, VEC12& val )
	{
		U4 v0,v1,v2;
		fread( &v0, sizeof(U4), 1, f);
		fread( &v1, sizeof(U4), 1, f);
		fread( &v2, sizeof(U4), 1, f);
		v0 = GetInt32EC( &v0 );
		v1 = GetInt32EC( &v1 );
		v2 = GetInt32EC( &v2 );

		val[0] = *(reinterpret_cast<float*>( &v0 ));
		val[1] = *(reinterpret_cast<float*>( &v1 ));
		val[2] = *(reinterpret_cast<float*>( &v2 ));

		return val;
	}

	inline void read( FILE *f, S0& val )
	{
		char c;
		unsigned int count = 0;
		do
		{
			fread( &c, sizeof(char), 1, f);
			count++;
			if(c != 0)
				val += c;
		} while( c != 0);

		// account for odd length string
		if( count & 0x1 )
			fread( &c, sizeof(char), 1, f);
	}

	inline VX readVX( FILE* f, VX& vx)
	{
		unsigned char c;
		VX ret;

		c = fgetc( f );
		// 2 or 4 byte VX?
		if(c == 0xFF) {
			unsigned int temp;
			// 4 byte VX use the next 3 bytes only
			c = fgetc( f );
			temp = static_cast<VX>(c) << 16;
			c = fgetc( f );
			temp |= static_cast<VX>(c) << 8;
			c = fgetc( f );
			temp |= static_cast<VX>(c);
			ret = temp;
		} else {
			// 2 byte VX use marker and next byte
			unsigned short temp = (unsigned short)(c) << 8;
			c = fgetc( f );
			temp |= static_cast<unsigned short>(c);
			ret = temp;
		}

		vx = ret;
		return vx;
	}

/**
Short description.
Detailed description
*/

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

} // end namespace

//---------------------------------------------------------------------------
// Externals
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// End Header file
//---------------------------------------------------------------------------
#endif
