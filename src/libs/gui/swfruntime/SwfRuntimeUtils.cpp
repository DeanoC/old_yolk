// 
//  SwfRuntimeUtils.cpp
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "gui/SwfParser/SwfMatrix.h"
/*
#if PLATFORM == IPHONE
#include <OpenGLES/ES1/gl.h>
#elif PLATFORM == APPLE_MAC
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif
*/
#include "SwfRuntimeUtils.h"

namespace Swf {
	Math::Matrix4x4 Convert(SwfMatrix* _matrix) {
		return Math::Matrix4x4( _matrix->scaleX, _matrix->rotateSkew0, 	  	0, 0,
        						_matrix->rotateSkew1, _matrix->scaleY, 	  	0, 0,
								0,						0,			   	  				1, 0,
								_matrix->translateX, _matrix->translateY, 	0, 1 );
	}
	
	std::string ToLowerIfReq( const std::string& _name, bool _isCaseSensitive ) {
		if( _isCaseSensitive == false ) {
			std::string str = _name;
			std::transform(str.begin(), str.end(), str.begin(), tolower);
			return str;
		} else {
			return _name;
		}
	}
	
/*
	void CheckGL(const char* _call) {
		int err;
		do{
			err = glGetError();
			if(err != GL_NO_ERROR){
				LOG(INFO) << "GL_ERROR : " << _call << " : ";
				switch(err){
					case GL_INVALID_ENUM: LOG(INFO) << "INVALID_ENUM"; break;
					case GL_INVALID_VALUE: LOG(INFO) << "INVALID_VALUE"; break;
					case GL_INVALID_OPERATION: LOG(INFO) << "INVALID_OPERATION"; break;
					case GL_STACK_OVERFLOW: LOG(INFO) << "STACK_OVERFLOW"; break;
					case GL_STACK_UNDERFLOW: LOG(INFO) << "STACK_UNDERFLOW"; break;
					case GL_OUT_OF_MEMORY: LOG(INFO) << "OUT_OF_MEMORY"; break;
				};
				LOG(INFO) << "\n";
			}
		} while(err != GL_NO_ERROR);
	}
	void TestImage( int textNum, float xScale, float yScale ){
		
		const GLfloat squareVertices[] = {
			-0.5f, -0.5f,
			0.5f, -0.5f,
			-0.5f, 0.5f,
			0.5f, 0.5f,			
		};
		const GLfloat squareUVs[] = {
			0, 0,
			1.0f * xScale, 0,
			0, 1 * yScale,
			1.0f * xScale, 1.0f * yScale,			
		};
		
		CALL_GL(glMatrixMode(GL_PROJECTION));
		CALL_GL(glPushMatrix());
		CALL_GL(glLoadIdentity());
#if PLATFORM == IPHONE		
		CALL_GL(glOrthof(-1.0f, 1.0f, -1.5f, 1.5f, -1.0f, 1.0f));
#else
		CALL_GL(glOrtho(-1.0f, 1.0f, -1.5f, 1.5f, -1.0f, 1.0f));
#endif 
		CALL_GL(glMatrixMode(GL_MODELVIEW));
		CALL_GL( glLoadIdentity() );
				
		CALL_GL(glVertexPointer(2, GL_FLOAT, 0, squareVertices));
		CALL_GL(glEnableClientState(GL_VERTEX_ARRAY));
		CALL_GL(glTexCoordPointer(2, GL_FLOAT, 0, squareUVs));
		CALL_GL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
		CALL_GL(glClientActiveTexture(GL_TEXTURE0));

		CALL_GL( glEnable(GL_TEXTURE_2D) );
		CALL_GL( glActiveTexture(GL_TEXTURE0) );
		CALL_GL( glMatrixMode(GL_TEXTURE) );
		CALL_GL( glLoadIdentity() );
		CALL_GL( glBindTexture(GL_TEXTURE_2D, textNum) );
		CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE) );
		CALL_GL( glTexEnvf(GL_TEXTURE_ENV,GL_RGB_SCALE, 1) );
		CALL_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		CALL_GL(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
		CALL_GL(glDisableClientState(GL_VERTEX_ARRAY));
		CALL_GL(glDisable(GL_TEXTURE_2D));		
	}
	*/
} /* Swf */
