/**
 @file	Z:\Projects\wierd\source\gl\databuffer.h

 @brief	Declares the databuffer class.
 */
#pragma once
#if !defined( YOLK_GL_VAO_H_ )
#define YOLK_GL_VAO_H_

#include "memory.h"
#include "scene/vertexinput.h"

namespace Gl {
	class Vao :	public Memory, public Scene::VertexInput {
	public:
		static Vao* internalCreate(	const Core::ResourceHandleBase* baseHandle, 
									const char* pName, const CreationStruct* creation );

		static GLenum getElementGlType( const Scene::VinElement element );
	protected:
		Vao();
	};


}

#endif