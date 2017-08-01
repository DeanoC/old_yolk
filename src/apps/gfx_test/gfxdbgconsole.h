#pragma once
#ifndef YOLK_GFXDBGCONSOLE_H_
#define YOLK_GFXDBGCONSOLE_H_ 1

#include "core/vector_math.h"
#include "localworld/sceneworld.h"
#include "scene/font.h"

class GfxDbgConsole {
public:
	static const int Y_LINE_COUNT = 20;
	static const int VAR_COUNT = 4;

	GfxDbgConsole(SceneWorldPtr _world);
	~GfxDbgConsole();

	void scrollUp();
	void scrollDown();
	void setCursor(unsigned int line);
	void clear();
	void print(const char* _text);

	void printVar(const int _index, const float _x, const float _y, const char* _text);

private:
	void renderable2DCallback(const Scene::ScreenPtr&, Scene::RenderContext* _ctx);

	SceneWorldPtr										world;

	std::string											textRows[Y_LINE_COUNT];
	std::string											textVars[VAR_COUNT];
	float												xVars[VAR_COUNT];
	float												yVars[VAR_COUNT];
	int													curLine;
	Core::ScopedResourceHandle<Scene::FontHandle>		font;
};

#endif