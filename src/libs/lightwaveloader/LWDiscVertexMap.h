/** \file LWDiscVertexMap.h
   A Lightwave object V2 discontinuous vertex map subclassed to the various.
   Vertex map types, weight maps, uv maps etc
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_LOADER_LWDISCVERTEXMAP_H_ )
#define LIGHTWAVE_LOADER_LWDISCVERTEXMAP_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LWTypes.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{
	class DiscVertexMap
	{
	public:
		enum TYPE {
			PICK = 0,	// not supported
			WGHT,
			MNVW,
			TXUV,
			RGB,
			RGBA,
			MORF,		// not supported
			SPOT		// not supported
		};

	
		virtual TYPE getType() = 0;
		virtual unsigned int getDimension() = 0;

		S0		name;
	};

	class WeightDMap : public DiscVertexMap
	{
	public:
		WeightDMap() {};
		TYPE getType() { return WGHT; }
		unsigned int getDimension() { return 1; }

		struct Weight
		{
			VX		vIndex;
			VX		pIndex;
			float	value;
			Weight( VX vin, VX pin, float val ) :
				vIndex(vin), pIndex(pin), value(val) {};
		};

		std::vector<Weight>	weights;
	};

	class UVDMap : public DiscVertexMap
	{
	public:
		UVDMap() {};
		TYPE getType() { return TXUV; }
		unsigned int getDimension() { return 2; }

		struct UV
		{
			VX		vIndex;
			VX		pIndex;
			float	u;
			float	v;
			UV( VX vin, VX pin, float val0, float val1 ) :
				vIndex(vin), pIndex(pin), u(val0), v(val1) {};
		};
		std::vector<UV> uvs;
	};

	class RGBDMap : public DiscVertexMap
	{
	public:
		RGBDMap() {};

		TYPE getType() { return RGB; }
		unsigned int getDimension() { return 3; }

		struct RGBFloat
		{
			VX		vIndex;
			VX		pIndex;
			float	r;
			float	g;
			float	b;
			RGBFloat( VX vin, VX pin, float val0, float val1, float val2 ) :
				vIndex(vin), pIndex(pin), r(val0), g(val1), b(val2) {};
		};
		std::vector<RGBFloat> rgbs;
	};

	class RGBADMap : public DiscVertexMap
	{
	public:
		RGBADMap() {};
		TYPE getType() { return RGBA; }
		unsigned int getDimension() { return 4; }

		struct RGBAFloat
		{
			VX		vIndex;
			VX		pIndex;
			float	r;
			float	g;
			float	b;
			float	a;
			RGBAFloat( VX vin, VX pin, float val0, float val1, float val2, float val3 ) :
				vIndex(vin), pIndex(pin), r(val0), g(val1), b(val2),a(val3) {};
		};
		std::vector<RGBAFloat> rgbas;
	};

	class MorfDMap : public DiscVertexMap
	{
	public:
		MorfDMap() {};
		TYPE getType() { return MORF; }
		unsigned int getDimension() { return 3; }

		struct VertexDelta
		{
			VX		vIndex;
			VX		pIndex;
			float	x;
			float	y;
			float	z;
			VertexDelta( VX vin, VX pin, float val0, float val1, float val2 ) :
				vIndex(vin), pIndex(pin), x(val0), y(val1), z(val2) {};
		};
		std::vector<VertexDelta> deltas;
	};

}
//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------

/**
Short description.
Detailed description
*/

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

