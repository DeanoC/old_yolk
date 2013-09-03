
#include "core/core.h"
#include "core/vector_math.h"
#include "gui/swfruntime/ActionScript/AsObject.h"
#include "gui/swfruntime/player.h"
#include "localworld/sceneworld.h"
#include "gui/swfruntime/movieclip.h"
#include "scene/rendercontext.h"
#include "gui/swfruntime/swfman.h"

#include "gfxdbgconsole.h"

namespace {
	static const char* s_RowNames[GfxDbgConsole::Y_LINE_COUNT] = {
		"TextRow1",
		"TextRow2",
		"TextRow3",
		"TextRow4",
		"TextRow5",
		"TextRow6",
		"TextRow7",
		"TextRow8",
		"TextRow9",
		"TextRow10",
		"TextRow11",
		"TextRow12",
		"TextRow13",
		"TextRow14",
		"TextRow15",
		"TextRow16",
		"TextRow17",
		"TextRow18",
		"TextRow19",
		"TextRow20",
	};
	static const char* s_VarNames[GfxDbgConsole::VAR_COUNT] = {
		"TextFloaty1",
		"TextFloaty2",
		"TextFloaty3",
		"TextFloaty4",
	};
}

GfxDbgConsole::GfxDbgConsole( SceneWorldPtr _world ) :
	world( _world ),
	curLine( 0 )
{
	if( Swf::SwfMan::exists() == false ) {
		Swf::SwfMan::init();
	}

	namespace arg = std::placeholders;

	flashConsole.reset( Swf::PlayerHandle::load( "dbg_text_console" ) );
	auto r2d = std::bind( &GfxDbgConsole::renderable2DCallback, this, arg::_1, arg::_2 );
	world->addRenderable2D( std::make_shared< Scene::Renderable2DCallbackType >( r2d ) );	

	auto fl = flashConsole.acquire();
	if( fl ) {
		for( int i = 0; i < Y_LINE_COUNT; ++i ) {
			textRows[i] = CORE_NEW Swf::AsObjectString("");
			fl->getRoot()->setProperty( s_RowNames[i], textRows[i] );
		}
		for( int i = 0; i < VAR_COUNT; ++i ) {
			textVars[i] = CORE_NEW Swf::AsObjectString("");
			xVars[i] = CORE_NEW Swf::AsObjectNumber(0);
			yVars[i] = CORE_NEW Swf::AsObjectNumber(0);
			fl->getRoot()->setProperty( s_VarNames[i], textVars[i] );
			auto lc = fl->getRoot()->findLocalTarget( s_VarNames[i] );
			lc->setProperty( 0, xVars[i]);
			lc->setProperty( 1, yVars[i] );
		}
	}
}

GfxDbgConsole::~GfxDbgConsole() {	
	for( int i = 0; i < VAR_COUNT; ++i ) {
		CORE_DELETE( textVars[i] );
		CORE_DELETE( yVars[i] );
		CORE_DELETE( xVars[i] );
	}

	for( int i = 0; i < Y_LINE_COUNT; ++i ) {
		CORE_DELETE( textRows[i] );
	}

	// need to refernce cound singleton
	Swf::SwfMan::shutdown();	
}

void GfxDbgConsole::scrollUp() {
	for(unsigned int i=1;i < Y_LINE_COUNT;i++) {
		textRows[i-1]->value = textRows[i]->value;
	}
}

void GfxDbgConsole::scrollDown() {
	for(unsigned int i=0;i < Y_LINE_COUNT-1;i++) {
		textRows[i+1]->value = textRows[i]->value;
	}
	textRows[0]->value.clear();
}

void GfxDbgConsole::setCursor(unsigned int line) {
	while( line >= Y_LINE_COUNT ) {
		scrollUp();
		line--;
	}

	curLine = line;
}

void GfxDbgConsole::clear() {
	for(unsigned int i=0;i < Y_LINE_COUNT;i++) {
		textRows[i]->value.clear();
	}
	curLine = 0;
}

void GfxDbgConsole::print( const char* _text ) {
	textRows[curLine]->value = _text;

	setCursor( curLine+1 );
}

void GfxDbgConsole::printVar( const int _index, const float _x, const float _y, const char* _text ) {
	textVars[ _index ]->value = _text;

	auto fl = flashConsole.acquire();
	if( fl ) {
		auto lc = fl->getRoot()->findLocalTarget( s_VarNames[ _index ] );
		Math::Vector2 v;
		v = Math::Vector2( _x, _y );
		v = Math::TransformAndDropZ( v, fl->getNdxToTwip() );
		xVars[ _index ]->value = v.x;
		yVars[ _index ]->value = v.y;

		lc->setProperty( 0, xVars[ _index ] );
		lc->setProperty( 1, yVars[ _index ] );
	}

}


void GfxDbgConsole::renderable2DCallback( const Scene::ScreenPtr& , Scene::RenderContext* _ctx ) {
	auto fl = flashConsole.tryAcquire();
	if( fl ) {
		fl->display( _ctx );
	}
}
