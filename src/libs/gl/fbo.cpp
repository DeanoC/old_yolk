/**
 @file	Z:\Projects\wierd\source\gl\fbo.cpp

 @brief	Implements the fbo class.
 */
#include "gl.h"
#include "fbo.h"

namespace {
GLenum drawBufferArray[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6,
		GL_COLOR_ATTACHMENT7,
		GL_COLOR_ATTACHMENT8,
	};
}

namespace Gl {

Fbo::Fbo() {
	generateName( MNT_FRAME_BUFFER );
	for( int i = 0;i < FAP_MAX_ATTACHMENT_POINTS; ++i ) {
		attachType[i] = false;
	}
	colourCount = 0;
}

Fbo::~Fbo() {
	unbind();
}

void Fbo::bind( FBO_FRAMEBUFFER_SD target ) {
	glFramebufferDrawBuffersEXT( name, colourCount, drawBufferArray );
	GL_CHECK
	auto nchk = glCheckNamedFramebufferStatusEXT( name, target );
	switch( nchk ) {
		case GL_FRAMEBUFFER_COMPLETE: break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: LOG(ERROR) << "named GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: LOG(ERROR) << "named GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: LOG(ERROR) << "named GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: LOG(ERROR) << "named GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED: LOG(ERROR) << "named GL_FRAMEBUFFER_UNSUPPORTED\n"; break;
		default: LOG(INFO) << "named glCheckFramebufferStatus ERR " << nchk << "\n"; break;
	}

	glBindFramebuffer( target, name );
	GL_CHECK
	auto chk = glCheckFramebufferStatus( target );
	switch( chk ) {
		case GL_FRAMEBUFFER_COMPLETE: break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED: LOG(ERROR) << "GL_FRAMEBUFFER_UNSUPPORTED\n"; break;
		default: LOG(INFO) << "glCheckFramebufferStatus ERR " << chk << "\n"; break;
	}
}

void Fbo::unbind( FBO_FRAMEBUFFER_SD target ) {
	glBindFramebuffer( target, 0 );
	GL_CHECK
}

void Fbo::attach( FBO_ATTACHMENT_POINT pnt, const TexturePtr& target ) {
	bool renderBufferType = false;

	CORE_ASSERT( target );

	if( target->getType() == MNT_RENDER_BUFFER ) {
		glNamedFramebufferRenderbufferEXT( name, pnt, GL_RENDERBUFFER, target->getName() );
		GL_CHECK
		renderBufferType = true;
	} else {
		CORE_ASSERT( target->getType() == MNT_TEXTURE_OBJECT );
		if( target->getHeight() > 1 ) {
			if( target->getDepth() <= 1 ) {
				if( target->getSampleCount() > 1 ) {
					glNamedFramebufferTexture2DEXT( name, pnt, GL_TEXTURE_2D_MULTISAMPLE, target->getName(), 0 );
				} else {
					glNamedFramebufferTexture2DEXT( name, pnt, GL_TEXTURE_2D, target->getName(), 0 );
				}
			} else {
				glNamedFramebufferTexture3DEXT( name, pnt, GL_TEXTURE_3D, target->getName(), 0, 0 );
			}
		} else {
			glNamedFramebufferTexture1DEXT( name, pnt, GL_TEXTURE_1D, target->getName(), 0 );
		}
		GL_CHECK
	}

	if( pnt == FAP_DEPTH ) {
		attachType[ FAP_MAX_ATTACHMENT_POINTS-1 ] = renderBufferType;
	} else {
		attachType[ (int)pnt - (int)FAP_COLOUR0 ] = renderBufferType;
		colourCount++;
	}
}

void Fbo::detach( FBO_ATTACHMENT_POINT pnt ) {
	bool renderBufferType = false;
	if( pnt == FAP_DEPTH ) {
		renderBufferType = attachType[ FAP_MAX_ATTACHMENT_POINTS - 1 ];
	} else {
		renderBufferType = attachType[ (int)pnt - (int)FAP_COLOUR0 ];
		colourCount--;
	}

	if( renderBufferType ) {
		glNamedFramebufferRenderbufferEXT( name, pnt, GL_RENDERBUFFER, 0 );
		GL_CHECK
	} else {
		glNamedFramebufferTexture2DEXT( name, pnt, GL_TEXTURE_2D, 0, 0 );
		GL_CHECK
	}
}

void Fbo::detachAll() {
	for( int i = 0; i < colourCount; ++i ) {
		detach( (FBO_ATTACHMENT_POINT) ((int)FAP_COLOUR0 + i) );
	}
	detach( FAP_DEPTH );
}

}
