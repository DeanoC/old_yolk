/** \file Heightfield.cpp
   (c) 2012 Deano Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshmod.h"
#include <algorithm>
#include <set>
#include "heightfield.h"
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
using namespace MeshMod;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

// protected copy ctor
Heightfield::Heightfield( const Heightfield& rhs ) :
	SceneObject( "Heightfield", rhs.getName() )
{
}
/**
defualt ctor
*/
Heightfield::Heightfield( const std::string& name ) :
	SceneObject( "Heightfield", name )
{
}

/**
default dtor
*/
Heightfield::~Heightfield()
{
}

Heightfield* Heightfield::clone() const {
	return CORE_NEW Heightfield( *this );
}

/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
