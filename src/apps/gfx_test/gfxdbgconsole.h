#pragma once
#ifndef YOLK_GFXDBGCONSOLE_H_
#define YOLK_GFXDBGCONSOLE_H_ 1

#include "core/vector_math.h"
#include "gui/swfruntime/ActionScript/AsObject.h"
#include "gui/swfruntime/player.h"
#include "localworld/sceneworld.h"

class GfxDbgConsole {
public:
	static const int Y_LINE_COUNT = 20;
	static const int VAR_COUNT = 4;

	GfxDbgConsole( SceneWorldPtr _world );
	~GfxDbgConsole();

	void scrollUp();
	void scrollDown();
	void setCursor(unsigned int line);
	void clear();
	void print( const char* _text );

	void printVar( const int _index, const float _x, const float _y, const char* _text );

private:
	void renderable2DCallback( const Scene::ScreenPtr&, Scene::RenderContext* _ctx );

	SceneWorldPtr										world;

	Core::ScopedResourceHandle<Swf::PlayerHandle> 		flashConsole;
	Swf::AsObjectString*								textRows[Y_LINE_COUNT];
	Swf::AsObjectString*								textVars[VAR_COUNT];
	Swf::AsObjectNumber*								xVars[VAR_COUNT];
	Swf::AsObjectNumber*								yVars[VAR_COUNT];
	int													curLine;
};

#endif