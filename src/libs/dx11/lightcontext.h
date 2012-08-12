#pragma once
/// \file	lightcontext.h
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
/// a light context is a virtual interface that
/// is used to describe how many and what kinds of lights
/// a particular renderable wants
#if !defined(WIERD_GRAPHICS_LIGHTCONTEXT_H)
#define WIERD_GRAPHICS_LIGHTCONTEXT_H

#include "light.h"

namespace Graphics {

	/// Abstract interface for a light context. 
	class LightContext {
	public:
		/// Destructor.
		virtual ~LightContext() {};
		/// Gets the number lights.
		/// \return	The number lights.
		virtual int getNumLights() const = 0;
		/// Gets a light.
		/// \param	iIndex	Zero-based index of the.
		/// \return	The light.
		virtual Light& getLight( unsigned int iIndex ) = 0;
		/// Gets a light.
		/// \param	iIndex	Zero-based index of the.
		/// \return	The light.
		virtual const Light& getLight( unsigned int iIndex ) const = 0;
		/// Adds a light. 
		/// \param	light	The light.
		/// \return	.
		virtual int addLight( const LightPtr light ) = 0;
		/// Removes the light described by iIndex.
		/// \param	iIndex	Zero-based index of the.
		virtual void removeLight( int iIndex ) = 0;
		/// Updates the gpu via the context.
		/// \param	context	The context.
		virtual void updateGPU( RenderContext *const context ) = 0;
	};

	

	/// concrete LightContext for when Dx11 path.
	/// \sa	LightContext
	class LightContextD3D11 : public LightContext {
	public:
		/// Default constructor.
		LightContextD3D11();
		/// Destructor.
		~LightContextD3D11();
		/// Gets the number lights.
		/// \return	The number lights.
		virtual int getNumLights() const;
		/// Gets a light.
		/// \param	iIndex	Zero-based index of the.
		/// \return	The light.
		virtual Light& getLight( unsigned int iIndex );
		/// Gets a light.
		/// \param	iIndex	Zero-based index of the.
		/// \return	The light.
		virtual const Light& getLight( unsigned int iIndex ) const;
		/// Adds a light. 
		/// \param	light	The light.
		/// \return	.
		virtual int addLight( const LightPtr light );
		/// Removes the light described by iIndex.
		/// \param	iIndex	Zero-based index of the.
		virtual void removeLight( int iIndex );
		/// Updates the gpu described by context.
		/// \param	context	The context.
		virtual void updateGPU( RenderContext *const context );
		/// Gets the tbuffer.
		/// \return	The tbuffer.
		TexturePtr	getTbuffer() const {
			return tbuffer;
		}
	protected:
		Core::vector<LightPtr> lights; ///< The lights
		TexturePtr			tbuffer;	///< The tbuffer
		TextureHandlePtr	tbufferHandle;  ///< Handle of the tbuffer
	};

}; // end namespace Graphics

#endif //WIERD_GRAPHICS_LIGHTCONTEXT_H