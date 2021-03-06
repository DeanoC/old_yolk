// 
//  SwfPlayer.cpp
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "core/file_path.h"
#include "scene/vertexinput.h"
#include "scene/rendercontext.h"
#include <zlib.h>
#include "gui/swfparser/parser.h"
#include "gui/swfparser/SwfRect.h"
#include "ActionScript/AsVM.h"
#include "swfman.h"
#include "builder.h"
#include "gradienttexturemanager.h"
#include "movieclip.h"
#include "utils.h"
#include "bitmap.h"
#include "font.h"
#include "character.h"
#include "player.h"

namespace Swf {
	Player::Player() {
		virtualMachine = NULL;
	}

	void Player::create( const std::string& _path, float _xscale, float _yscale ){
		paused = false;
		Core::FilePath path(_path);
		
		virtualMachine = CORE_NEW AsVM( path.BaseName().RemoveExtension().value() );		

		parser = CORE_NEW SwfParser();
		parser->Parse( path.ReplaceExtension( ".swf" ).value() );
		
		rootClip = MovieClip::createRoot( this, parser->frameList );
		frameRateInMs = 1000.0f / parser->frameRate;

		scaleX = _xscale;
		scaleY = _yscale;

		float sx = (float)(parser->headerRect->maxX - parser->headerRect->minX);
		float sy = (float)(parser->headerRect->maxY - parser->headerRect->minY);
		CORE_ASSERT( parser->fileVersion <= Swf::SwfParser::MAX_VERSION_SUPPORTED );	
		twipToNdx = Math::IdentityMatrix();
		twipToNdx = Math::CreateTranslationMatrix(-(sx / 2.0f), -(sy / 2.0f), 0.0f);
		twipToNdx = Math::MultiplyMatrix(twipToNdx, Math::CreateScaleMatrix((2.0f*scaleX) / sx, -(2.0f*scaleY) / sy, 1.0f));
		ndxToTwip = Math::InverseMatrix( twipToNdx );
		
		Builder runtimeBuilder(this, parser);

		runtimeBuilder.buildBitmaps(runtimeBitmaps);
		runtimeBuilder.buildFonts(runtimeFonts);
		runtimeBuilder.buildCharacterPaths(characterPaths);

		rootClip->process();
	}
	
	Player::~Player() {
		if( virtualMachine != NULL) {
			CORE_DELETE( virtualMachine );
		}
		// kill the root frame list

		CORE_DELETE( parser );
		CORE_DELETE( rootClip );
		for( auto i : runtimeBitmaps ) {
			CORE_DELETE( i.second );
		}
		for( auto i : runtimeFonts ) {
			CORE_DELETE( i.second );
		}
		for( auto i : characterPaths ) {
			CORE_DELETE( i.second );
		}
	}
	
	void Player::process() {
		rootClip->process();
	}
	
	void Player::advance(float _elapsedMs){
		if( paused == false ){
			rootClip->advance(_elapsedMs);

			for( auto i : codeToRun ) {
				virtualMachine->processByteCode( rootClip, i );	
			}
			codeToRun.resize( 0 );
		}
	}
	void Player::togglePause(){
		paused ^= true;
	}
	void Player::display( Scene::RenderContext* _ctx ){
		_ctx->getConstantCache().setViewMatrix();
		_ctx->getConstantCache().setProjectionMatrix( twipToNdx );

/*		twipToPixels = Math::MultiplyMatrix(twipToNdx, Math::CreateScaleMatrix( _backingWidth*0.5f, _backingHeight*0.5f, 1.0f ) );
		int scisWidth = _backingWidth * scaleX;
		int scisHeight = _backingHeight * scaleY;
		CALL_GL(glEnable(GL_SCISSOR_TEST));
		CALL_GL(glScissor( (_backingWidth - scisWidth)/2, (_backingHeight - scisHeight)/2,scisWidth, scisHeight)); */

		SwfMan::getr().bind( _ctx );

		rootClip->display( this, _ctx );

		//		CALL_GL(glDisable(GL_SCISSOR_TEST));
		//TestImage(19, 1.0f, 1.0f);

	}

	const void* Player::internalPreCreate( const char* name, const Player::CreationInfo *creation ) {
		Player* player = CORE_NEW Player();
		player->create( name, 1.0f, 1.0f );
		return player;
	}
	void Player::scheduleActionByteCodeRun( SwfActionByteCode* byteCode ) {
		codeToRun.push_back( byteCode );
	}

} /* Swf */
