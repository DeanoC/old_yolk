///-------------------------------------------------------------------------------------------------
/// \file	meshops\floattexture.h
///
/// \brief	Declares the floattexture class.
///
/// \details	
///		floattexture description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
///
/// \todo	Fill in detailed file description.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef floattexture_h__
#define floattexture_h__

///-------------------------------------------------------------------------------------------------
/// \namespace	MeshOps
///
/// \brief	.
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace MeshOps {

///-------------------------------------------------------------------------------------------------
/// \class	FloatLayer
///
/// \brief	A named layer, size is defined by its owning FloatTexture
///
/// \todo	Fill in detailed class description.
////////////////////////////////////////////////////////////////////////////////////////////////////
class FloatLayer {
	friend class FloatTexture;

public:
	const std::string& getName() { return name; }
	const float* getData() const { return &data[0]; }
	float* getData() { return &data[0]; }

	///-------------------------------------------------------------------------------------------------
	/// \fn	unsigned int FloatLayer::getComponentCount() const
	///
	/// \brief	Gets the component per pixel count.
	///
	/// \return	The component count.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	unsigned int getComponentCount() const { return componentCount; }

private:
	FloatLayer( const class FloatTexture& owner, const std::string& _name, unsigned int _componentCount );
	std::string name;
	std::vector<float> data;
	unsigned int componentCount;		//!< Number of components per pixel
};


///-------------------------------------------------------------------------------------------------
/// \class	FloatTexture
///
/// \brief	Float texture. Holds layers of the same size image (components per pixel and dimension)
/// 		Each layer is named, allowing complex multi-layer textures to be passed around
///
/// \todo	Fill in detailed class description.
////////////////////////////////////////////////////////////////////////////////////////////////////
class FloatTexture {
public:
	///-------------------------------------------------------------------------------------------------
	/// \fn
	/// FloatTexture::FloatTexture( unsigned int _width, unsigned int _height,
	/// unsigned int _componentCount );
	///
	/// \brief	Constructor.
	///
	/// \param	_width		   	The width. 
	/// \param	_height		   	The height. 
	/// \param	_componentCount	Number of components per pixel. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	FloatTexture( unsigned int _width, unsigned int _height );

	///-------------------------------------------------------------------------------------------------
	/// \fn	unsigned int FloatTexture::getWidth() const
	///
	/// \brief	Gets the width.
	///
	/// \return	The width.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	unsigned int getWidth() const { return width; }
	
	///-------------------------------------------------------------------------------------------------
	/// \fn	unsigned int FloatTexture::getHeight() const
	///
	/// \brief	Gets the height.
	///
	/// \return	The height.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	unsigned int getHeight() const { return height; }
	
	
	///-------------------------------------------------------------------------------------------------
	/// \fn	unsigned int FloatTexture::getLayerCount() const
	///
	/// \brief	Gets the layer count.
	///
	/// \todo	Fill in detailed method description.
	///
	/// \return	The layer count.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	unsigned int getLayerCount() const { return layerCount; }

	///-------------------------------------------------------------------------------------------------
	/// \fn	const FloatLayer& FloatTexture::getLayer( const Core::string* name ) const
	///
	/// \brief	Gets a layer.
	///
	/// \todo	Fill in detailed method description.
	///
	/// \param	name	The name. 
	///
	/// \return	The layer.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const FloatLayer& getLayer( const std::string& name ) const { return (*layers.find(name)).second; }

	///-------------------------------------------------------------------------------------------------
	/// \fn	FloatLayer& FloatTexture::getLayer( const Core::string* name )
	///
	/// \brief	Gets a layer.
	///
	/// \todo	Fill in detailed method description.
	///
	/// \param	name	The name. 
	///
	/// \return	The layer.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	FloatLayer& getLayer( const std::string& name ) { return (*layers.find(name)).second; }

	///-------------------------------------------------------------------------------------------------
	/// \fn
	/// FloatLayer* FloatTexture::addLayer( const Core::string& name,
	/// unsigned int _componentCount )
	///
	/// \brief	Adds a layer.
	///
	/// \param	name		   	The name. 
	/// \param	_componentCount	Number of per pixel components. 
	///
	/// \return	null if it fails, else.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	FloatLayer& addLayer( const std::string& name, unsigned int _componentCount ) {
		LayerMap::iterator it = layers.insert( layers.begin(), 
						LayerMap::value_type(name, FloatLayer( *this, name, _componentCount ) ) );
		return (*it).second;
	}

private:
	unsigned int width;					//!< The width
	unsigned int height;				//!< The height
	unsigned int layerCount;			//!< Number of layers
	typedef std::map<std::string,FloatLayer> LayerMap;
	LayerMap layers;	//!< The layers
};

}

#endif // floattexture_h__
