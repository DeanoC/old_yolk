// 
//  SwfPlayer.cpp
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "gui/SwfParser/Parser.h"
#include "gui/SwfParser/SwfRect.h"
#include "core/file_path.h"
#include "SwfMovieClip.h"
#include "SwfRuntimeBuilder.h"
#include "SwfGradientTextureManager.h"
#include "SwfRuntimeUtils.h"
/*
#if PLATFORM == IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif PLATFORM == APPLE_MAC
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif
*/
#if defined( USE_ACTIONSCRIPT )
#	include "AsVM.h"
#endif
#include <zlib.h>
#include "SwfPlayer.h"

namespace Swf
{
	SwfPlayer::SwfPlayer() {
#if defined( USE_ACTIONSCRIPT )
		virtualMachine = NULL;
#endif		
	}

	void SwfPlayer::create(const std::string& _path, float _xscale, float _yscale ){
		paused = false;
		
#if defined( USE_ACTIONSCRIPT )
		SwfFrameItem::s_asPropertyStringMap.clear();
		SwfFrameItem::s_asPropertyStringMap["_x"] = 0;
		SwfFrameItem::s_asPropertyStringMap["_y"] = 1;
		SwfFrameItem::s_asPropertyStringMap["_X"] = 0; 
		SwfFrameItem::s_asPropertyStringMap["_Y"] = 1;
		SwfFrameItem::s_asPropertyStringMap["_xscale"] = 2; 
		SwfFrameItem::s_asPropertyStringMap["_yscale"] = 3;
		SwfFrameItem::s_asPropertyStringMap["_currentframe"] = 4;
		SwfFrameItem::s_asPropertyStringMap["_totalframes"] = 5; 
		SwfFrameItem::s_asPropertyStringMap["_alpha"] = 6; 
		SwfFrameItem::s_asPropertyStringMap["_visible"] = 7; 
		SwfFrameItem::s_asPropertyStringMap["_width"] = 8; 
		SwfFrameItem::s_asPropertyStringMap["_height"] = 9; 
		SwfFrameItem::s_asPropertyStringMap["_rotation"] = 10; 
		SwfFrameItem::s_asPropertyStringMap["_target"] = 11; 
		SwfFrameItem::s_asPropertyStringMap["_framesloaded"] = 12;
		SwfFrameItem::s_asPropertyStringMap["_name"] = 13;
		SwfFrameItem::s_asPropertyStringMap["_droptarget"] = 14;
		SwfFrameItem::s_asPropertyStringMap["_url"] = 15;
		SwfFrameItem::s_asPropertyStringMap["_highquality"] = 16; 
		SwfFrameItem::s_asPropertyStringMap["_focusrect"] = 17;
		SwfFrameItem::s_asPropertyStringMap["_soundbuftime"] = 18;
		SwfFrameItem::s_asPropertyStringMap["_quality"] = 19;
		SwfFrameItem::s_asPropertyStringMap["_xmouse"] = 20;
		SwfFrameItem::s_asPropertyStringMap["_ymouse"] = 21;
		
		Core::FilePath path(_path);
		virtualMachine = CORE_NEW AsVM( path.BaseName().RemoveExtension().value() );		
#endif

		parser = CORE_NEW Parser();
		parser->Parse(_path);
		
		rootClip = SwfMovieClip::CreateRoot(this, parser->frameList );
		gradientTextureManager = CORE_NEW SwfGradientTextureManager();
		frameRateInMs = 1000.0f / parser->frameRate;
		
		scaleX = _xscale;
		scaleY = _yscale;
		
		unsigned int stockCol[] = 
		{
			0xFFFFFFFF,
			0x000000FF
		};
		/*
		glGenTextures(2, stockTextures);
		CALL_GL(glBindTexture( GL_TEXTURE_2D, stockTextures[0] ));
		CALL_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &stockCol[0]));
		CALL_GL(glBindTexture( GL_TEXTURE_2D, stockTextures[1] ));
		CALL_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &stockCol[1]));
		CALL_GL(glBindTexture( GL_TEXTURE_2D, 0 ));
	    */
	    TODO_ASSERT( false && "GL Code" );
		SwfRuntimeBuilder runtimeBuilder(this, parser);

		runtimeBuilder.BuildBitmaps(runtimeBitmaps);
		runtimeBuilder.BuildFonts(runtimeFonts);
		runtimeBuilder.BuildCharacterPaths(characterPaths);
	}
	
	SwfPlayer::~SwfPlayer() {
#if defined( USE_ACTIONSCRIPT )
		if( virtualMachine != NULL) {
			CORE_DELETE( virtualMachine );
		}
#endif
	}
	
	void SwfPlayer::process() {
		rootClip->Process();
	}
	
	void SwfPlayer::advance(float _elapsedMs){
		if( paused == false ){
			rootClip->Advance(_elapsedMs);
		}
	}
	void SwfPlayer::togglePause(){
		paused ^= true;
	}
	void SwfPlayer::display(int _backingWidth, int _backingHeight ){
		float sx = (float)(parser->headerRect->maxX - parser->headerRect->minX);
		float sy = (float)(parser->headerRect->maxY - parser->headerRect->minY);
		
		twipToNdx = Math::IdentityMatrix();
		twipToNdx = Math::CreateTranslationMatrix(-(sx / 2.0f), -(sy / 2.0f), 0.0f);
		twipToNdx = Math::MultiplyMatrix(twipToNdx, Math::CreateScaleMatrix((2.0f*scaleX) / sx, -(2.0f*scaleY) / sy, 1.0f));
		twipToPixels = Math::MultiplyMatrix(twipToNdx, Math::CreateScaleMatrix( _backingWidth*0.5f, _backingHeight*0.5f, 1.0f ) );
/*		CALL_GL(glMatrixMode(GL_PROJECTION));
		CALL_GL(glLoadMatrixf((float*)twipToNdx));

#if PLATFORM == IPHONE		
		// TODO move into driver program
		int backingWidth, backingHeight;
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
#endif
		int scisWidth = _backingWidth * scaleX;
		int scisHeight = _backingHeight * scaleY;
		CALL_GL(glEnable(GL_SCISSOR_TEST));
		CALL_GL(glScissor( (_backingWidth - scisWidth)/2, (_backingHeight - scisHeight)/2,scisWidth, scisHeight));
		rootClip->Display(this);
		CALL_GL(glDisable(GL_SCISSOR_TEST));
*/
		TODO_ASSERT( false && "GL code" );		
		//TestImage(19, 1.0f, 1.0f);
	}

	const void* SwfPlayer::internalPreCreate( const char* name, const SwfPlayer::CreationInfo *creation ) {
		SwfPlayer* player = CORE_NEW SwfPlayer();
		player->create( name, 1.0f, 1.0f );
		return player;
	}

} /* Swf */
