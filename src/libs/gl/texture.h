/**
 @file	Z:\Projects\wierd\source\gl\texture.h

 @brief	Declares the texture class.
 */
#pragma once
#if !defined( YOLK_GL_TEXTURE_H_ )
#define YOLK_GL_TEXTURE_H_

#include "memory.h"
#include "scene/texture.h"

namespace Gl {

	class Texture : public Memory, public Scene::Texture {
	public:
		static Texture* internalLoad( const Core::ResourceHandleBase* baseHandle, 
												const char* pTextureFileName, 
												bool preload );

		static Texture* internalCreate( const CreationStruct* pStruct );

		GLenum getGlFormat() const { return glformat; }
		bool isRenderBuffer() const { return renderBuffer; }

	protected:
		Texture();
		GLenum 	glformat;				//!< gl format of this texture
		bool	renderBuffer;			//!< true if a render buffer object
	};
}

#endif