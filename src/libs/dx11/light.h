/// \file	light.h
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
/// \brief	Declares the light class.
#pragma once
#ifndef WIERD_GRAPHICS_LIGHT_H__
#define WIERD_GRAPHICS_LIGHT_H__

namespace Graphics {

/// Basic Light. 
class Light {
public:
	/// Default constructor.
	Light() : 
		worldSpace( Math::IdentityMatrix() ),
		colour(0,0,0,1) {};
	/// Destructor.
	virtual ~Light(){}
	/// Query if this light is active.
	/// \return	true if active, false if not.
	bool isActive() { return (colour[3] > 0.0f); }

	/// Gets the direction. \return	The direction.
	Math::Vector3 getDirection() const { return Math::GetZAxis( worldSpace ); }
	/// Gets the position. \return	The position.
	Math::Vector3 getPosition() const { return Math::GetTranslation( worldSpace ); }
	/// Gets the colour. \return	The colour.
	const Core::RGBAColour& getColour(void) const	{ return(colour);	};
	void setColour( const Core::RGBAColour& _colour ) { colour = _colour; }
	/// Gets the gpu buffer size required for this light.
	/// \return	The gpu buffer size in floats.
	virtual uint32_t getGpuSize() const = 0;
	/// Transfer this light to gpu buffer pointed to by dest.
	/// \param [out]	dest	
	virtual void writeToGpuBuffer( float* dest ) const = 0;

protected:
	Math::Matrix4x4		worldSpace;  //!< The world space matrix of this light (z = direction)
	Core::RGBAColour	colour;		//!< Colour of the light, Alpha = Intensity
};
/// Defines an alias representing a shared light pointer.
typedef Core::shared_ptr<Light> LightPtr;


/// Simple Directional light, mostly for debugging. 
/// \sa	Light
class DirectionalLight : public Light {
public:
	void setDirection( const Math::Vector3& dir ) {
		Math::Vector3 yxis(0,1,0);
		if( fabsf(Math::Dot(dir, yxis)-1.0f) < 1e-2f ) {
			yxis = Math::Vector3(1,0,0);
		}
		Math::SetXAxis( worldSpace, Math::Cross(yxis, dir) );
		Math::SetYAxis( worldSpace, yxis );
		Math::SetZAxis( worldSpace, dir );
	}
	virtual uint32_t getGpuSize() const { 
		return sizeof(Math::Matrix4x4) + sizeof(Math::Vector4); 
	}

	virtual void writeToGpuBuffer( float* dest ) const {
		memcpy( dest, (worldSpace), sizeof(worldSpace) ); 
		dest += (sizeof(worldSpace)/4);
		memcpy( dest, (const float*)colour.getAsVector(), sizeof(Math::Vector4) );
	}

};
}

#endif // end _GRAPHICS_LIGHT_H__