///-------------------------------------------------------------------------------------------------
/// \file	meshops\floattexture.cpp
///
/// \brief	Implements the floattexture class.
///
/// \details	
///		floattexture description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
/// \todo	Fill in detailed file description.
////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshops.h"
#include "floattexture.h"

namespace MeshOps {

FloatLayer::FloatLayer( const class FloatTexture& owner, const std::string& _name, unsigned int _componentCount ) :
	name( _name ),
	componentCount( _componentCount ) {
	data.resize( owner.getWidth() * owner.getHeight() * getComponentCount() );
}

FloatTexture::FloatTexture( unsigned int _width, unsigned int _height ) :
	width( _width ),
	height( _height ) {
}


}
