/** \file LWLayer.h
   A Lightwave object V2 layer.
   Also contains geometry etc
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_LWLAYER_H_ )
#define LIGHTWAVE_LWLAYER_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LWTypes.h"
#include "LWFace.h"
#include "LWPolyTag.h"
#include "LWVertexMap.h"
#include "LWDiscVertexMap.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{
	struct Layer {
		enum { NO_PARENT = 0xFFFF };

		U2				number;		///< Layer ID number
		enum
		{	NO_FLAGS = 0x0,
			HIDDEN = 0x1
		}				flags;		///< flags (hidden or not)
		VEC12			pivot;		///< Pivot point of layer
		S0				name;			///< ID name
		U2				parent;		///< parent layer or NO_PARENT

		Layer() :
			number( 0 ),
			flags( NO_FLAGS ),
			pivot( 0,0,0 ),
			parent( NO_PARENT )
		{};
		~Layer();

		std::vector<VEC12>		points;	///< point list relative to m_pivot
		std::vector<Face>		faces;	///< face list

		std::vector<PolyTag> surfaceTag2Poly;	///< surface tag 2 poly map
		std::vector<PolyTag> partTag2Poly;	///< part tag 2 poly map -
		std::vector<PolyTag> smgpTag2Poly;	///< smooth group tag 2 poly map

		std::vector<VertexMap*>			vertexMaps;		///< vertex map pointer table (needs cleaning in dtor)
		std::vector<DiscVertexMap*>		discVertexMaps;	///< discontinous vertex map pointer table (needs cleaning in dtor)
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
