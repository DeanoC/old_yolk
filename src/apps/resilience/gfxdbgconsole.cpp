
#include "core/core.h"
#include "core/vector_math.h"
#include "localworld/sceneworld.h"
#include "scene/rendercontext.h"

#include "gfxdbgconsole.h"

GfxDbgConsole::GfxDbgConsole( SceneWorldPtr _world ) :
	world( _world ),
	curLine( 0 )
{
	namespace arg = std::placeholders;

	auto r2d = std::bind( &GfxDbgConsole::renderable2DCallback, this, arg::_1, arg::_2 );
	world->addRenderable2D( std::make_shared< Scene::Renderable2DCallbackType >( r2d ) );	

	for( int i = 0; i < Y_LINE_COUNT; ++i ) {
		textRows[i].clear();
	}
	for( int i = 0; i < VAR_COUNT; ++i ) {
		textVars[i].clear();
		xVars[i] = 0;
		yVars[i] = 0;
	}

	font.reset(Scene::FontHandle::load("arial") );

}

GfxDbgConsole::~GfxDbgConsole() {	
}

void GfxDbgConsole::scrollUp() {
	for(unsigned int i=1;i < Y_LINE_COUNT;i++) {
		textRows[i-1] = textRows[i];
	}
}

void GfxDbgConsole::scrollDown() {
	for(unsigned int i=0;i < Y_LINE_COUNT-1;i++) {
		textRows[i+1] = textRows[i];
	}
	textRows[0].clear();
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
		textRows[i].clear();
	}
	curLine = 0;
}

void GfxDbgConsole::print( const char* _text ) {
	textRows[curLine]= _text;
	setCursor( curLine+1 );
}

void GfxDbgConsole::printVar(	const int _index, 
								const float _x, 
								const float _y, 
								const char* _text ) {
	textVars[ _index ] = _text;
	xVars[ _index ] = _x;
	yVars[ _index ] = _y;

}


void GfxDbgConsole::renderable2DCallback( const Scene::ScreenPtr& _scr, Scene::RenderContext* _ctx ) {
	Math::Vector2 pos(-0.8f, -0.8f); // TODO proper x and y adjust
	Core::RGBAColour col(1.f,1.f,1.f,1.f);
	const float pt = 17.0f;

	float lineGap = 0.0f;

	for (int i = 0; i < Y_LINE_COUNT; ++i) {
		pos.x = -0.8f;
		for (const char ch : textRows[i]) {
			Math::Vector2 advance = _scr->getComposer()->putChar( font.get(), 
																  ch, 
																  Scene::ImageComposer::PM_OVER, 
																  pos, 
																  pt, 
																  col, 
																  15);
			pos.x += advance.x;
			lineGap = std::max( lineGap, advance.y);
		}
		pos.y += lineGap + (_scr->getOneVerticalPixInNDC() * 3);
	}

	for (int i = 0; i < VAR_COUNT; ++i) {
		if (!textVars[i].empty()) {
			pos.x = xVars[i];
			pos.y = yVars[i];
			for (const char ch : textVars[i]) {
				Math::Vector2 advance = _scr->getComposer()->putChar(font.get(),
					ch,
					Scene::ImageComposer::PM_OVER,
					pos,
					pt,
					col,
					15);
				pos.x += advance.x;
			}
		}
	}
}
