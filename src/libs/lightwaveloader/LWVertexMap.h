/** \file LWVertexMap.h
   A Lightwave object V2 vertex map subclassed to the various.
   Vertex map types, weight maps, uv maps etc
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_LOADER_LWVERTEXMAP_H_ )
#define LIGHTWAVE_LOADER_LWVERTEXMAP_H_

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
	class VertexMap
	{
	public:
		enum TYPE {
			PICK = 0,
			WGHT,
			MNVW,
			TXUV,
			RGB,
			RGBA,
			MORF,
			SPOT
		};

		virtual ~VertexMap() {}
		virtual TYPE getType() = 0;
		virtual unsigned int getDimension() = 0;

		S0		name;
	};

	class PickMap : public VertexMap
	{
	public:
		PickMap() {};
		TYPE getType() { return PICK; }
		unsigned int getDimension() { return 0; }

		std::vector<VX>	picks;
	};

	class WeightMap : public VertexMap
	{
	public:
		WeightMap() {};
		TYPE getType() { return WGHT; }
		unsigned int getDimension() { return 1; }

		struct Weight
		{
			VX		index;
			float	value;
			Weight( VX in, float val ) :
				index(in), value(val) {};
		};

		std::vector<Weight>	weights;
	};

	class UVMap : public VertexMap
	{
	public:
		UVMap() {};
		TYPE getType() { return TXUV; }
		unsigned int getDimension() { return 2; }

		struct UV
		{
			VX		index;
			float	u;
			float	v;
			UV( VX in, float val0, float val1 ) :
				index(in), u(val0), v(val1) {};
		};
		std::vector<UV> uvs;
	};

	class RGBMap : public VertexMap
	{
	public:
		RGBMap() {};

		TYPE getType() { return RGB; }
		unsigned int getDimension() { return 3; }

		struct RGBFloat
		{
			VX		index;
			float	r;
			float	g;
			float	b;
			RGBFloat( VX in, float val0, float val1, float val2 ) :
				index(in), r(val0), g(val1), b(val2) {};
		};
		std::vector<RGBFloat> rgbs;
	};

	class RGBAMap : public VertexMap
	{
	public:
		RGBAMap() {};
		TYPE getType() { return RGBA; }
		unsigned int getDimension() { return 4; }

		struct RGBAFloat
		{
			VX		index;
			float	r;
			float	g;
			float	b;
			float	a;
			RGBAFloat( VX in, float val0, float val1, float val2, float val3 ) :
				index(in), r(val0), g(val1), b(val2),a(val3) {};
		};
		std::vector<RGBAFloat> rgbas;
	};

	class MorfMap : public VertexMap
	{
	public:
		MorfMap() {};
		TYPE getType() { return MORF; }
		unsigned int getDimension() { return 3; }

		struct VertexDelta
		{
			VX index;
			float x;
			float y;
			float z;
			VertexDelta( VX in, float val0, float val1, float val2 ) :
				index(in), x(val0), y(val1), z(val2) {};
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

