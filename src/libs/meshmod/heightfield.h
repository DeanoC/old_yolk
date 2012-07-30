#pragma once
/** \file heightfield.h
   (c) 2012 Dean Calver
 */

#if !defined( MESH_MOD_HEIGHTFIELD_H_ )
#define MESH_MOD_HEIGHTFIELD_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "sceneobject.h"
//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod
{

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------

	class Heightfield : public SceneObject
	{
	public:
		//! ctor.
		Heightfield( const std::string& name );
		//! dtor.
		~Heightfield();

		Heightfield* clone() const;

		void setWidth( int _width ) { width = _width; }
		void setHeight( int _height ) { height = _height; }

		void setHeightField( const std::vector<float>& _heights ) { 
			heightfield = _heights;
		}

	protected:
		// for clone only
		Heightfield( const Heightfield& rhs );

		int					width;
		int 				height;
		std::vector<float>	heightfield;
	};

	typedef std::shared_ptr<Heightfield> HeightfieldPtr;

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
