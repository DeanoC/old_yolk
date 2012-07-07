/** \file template.h
   Template Header.
   A generic template C++ header files
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_LOADER_LWBLOK_H_ )
#define LIGHTWAVE_LOADER_LWBLOK_H_

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

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------

	/**
	Short description.
	Detailed description
	*/
	struct LWBlok
	{
		// general blok params
		enum {
			IMAP,
			PROC,
			GRAD,
			SHDR
		} type;

		enum {
			COLR,
			DIFF,
			LUMI,
			SPEC,
			GLOS,
			REFL,
			TRAN,
			RIND,
			TRNL,
			BUMP
		} channel;

		bool enable;

		enum {
			OP_ADDITIVE,
			OP_SUBTRACTIVE,
			OP_DIFFERENCE,
			OP_MULTIPLY,
			OP_DIVIDE,
			OP_ALPHA,
			OP_TEXTURE_DISPLACEMENT
		} opacityType;

		float opacity;

		// tmap params
		VEC12	center;
		VEC12	size;
		VEC12	rotation;
		S0		refObjectName;
		bool	objectSpace; // if false world space

		// imap params
		enum PROJECTION
		{
			PLANAR,
			CYLINDRICAL,
			SPHERICAL,
			CUBIC,
			FRONT_PROJECTION,
			UV
		} projectionMode;

		enum AXIS
		{
			X_AXIS,
			Y_AXIS,
			Z_AXIS
		} axis;

		VX imageIndex;

		enum WRAP_OPTION {
			RESET,
			REPEAT,
			MIRROR,
			EDGE
		};
		
		WRAP_OPTION widthWrap;
		WRAP_OPTION heightWrap;

		FP4 widthWrapAmount;
		FP4 heightWrapAmount;

		S0	vmapName;

		U2	aastFlags;
		FP4 aastStrength;

		U2	pixelBlendFlags;

		FP4 stck;
		FP4 textureAmplitude;
		S0	ordinal;

		LWBlok() :
			enable(true),
			opacityType(OP_ADDITIVE),
			opacity(1.0f),
			center( 0,0,0 ),
			rotation( 0,0,0 ),
			objectSpace(false),
			widthWrap( REPEAT ),
			heightWrap( REPEAT ),
			stck(0.f),
			textureAmplitude( 1.f )
			{};

		// used with std::sort to sort ordinal
		static bool OrdinalSortFunctor( const LWBlok* a, const LWBlok* b )
		{
			return (strcmp( a->ordinal.c_str(), b->ordinal.c_str() ) < 0);
		}
	};
}
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
