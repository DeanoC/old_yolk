/**
 @file	Z:\Projects\wierd\source\gl\fbo.h

 @brief	Declares the fbo class.
 */
#pragma once
#if !defined( WIERD_GL_FBO_H__ )
#define WIERD_GL_FBO_H__

#include "memory.h"
#include "texture.h"

namespace Gl {
	enum FBO_ATTACHMENT_POINT {
		FAP_COLOUR0 = GL_COLOR_ATTACHMENT0,
		FAP_COLOUR1 = GL_COLOR_ATTACHMENT1,
		FAP_COLOUR2 = GL_COLOR_ATTACHMENT2,
		FAP_COLOUR3 = GL_COLOR_ATTACHMENT3,
		FAP_COLOUR4 = GL_COLOR_ATTACHMENT4,
		FAP_COLOUR5 = GL_COLOR_ATTACHMENT5,
		FAP_COLOUR6 = GL_COLOR_ATTACHMENT6,
		FAP_COLOUR7 = GL_COLOR_ATTACHMENT7,
		FAP_DEPTH = GL_DEPTH_STENCIL_ATTACHMENT, // TODO pure depth??

		FAP_MAX_ATTACHMENT_POINTS = 9,
	};
	enum FBO_FRAMEBUFFER_SD {
		FFSD_SOURCE = GL_READ_FRAMEBUFFER,
		FFSD_DEST = GL_DRAW_FRAMEBUFFER,
		FFSD_BOTH = GL_FRAMEBUFFER,
	};

	class Fbo : public Memory { 
	public:
		Fbo();
		virtual ~Fbo();

		void bind( FBO_FRAMEBUFFER_SD target = FFSD_BOTH );
		void unbind( FBO_FRAMEBUFFER_SD target = FFSD_BOTH );

		void attach( FBO_ATTACHMENT_POINT pnt, const Scene::TexturePtr& target );
		void detach( FBO_ATTACHMENT_POINT pnt );

		void detachAll();
	protected:
		bool				attachType[ FAP_MAX_ATTACHMENT_POINTS ];
		unsigned int		colourCount;
	};
}

#endif