/** \file LWOLayer.cpp
   Light wave 7 Layer implementation.
   A generic template C++ files
   (c) 2002 Deano Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "lightwaveloader.h"
#include "LWOLoader.h"

//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
using namespace LightWave;

/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
Layer::~Layer()
{
	std::vector<VertexMap*>::iterator vmIt = vertexMaps.begin();
	while( vmIt != vertexMaps.end() ) {
		delete *vmIt;
		++vmIt;
	}

	vertexMaps.clear();

	std::vector<DiscVertexMap*>::iterator vdmIt = discVertexMaps.begin();
	while( vdmIt != discVertexMaps.end() ) {
		delete *vdmIt;
		++vdmIt;
	}

	discVertexMaps.clear();
}

