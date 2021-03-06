#ifndef YOLK_3DSHELL_H_
#define YOLK_3DSHELL_H_ 1

#include "shell3d.h"
#include "scene/screen.h"

namespace Scene {
	class Renderer;
};

class Shell3D {
public:
	Shell3D();
	~Shell3D();

	void start();
	void run();
	void end();

	SceneWorldPtr getSceneWorld() const { return world; }
protected:
	SceneWorldPtr						world;
	Scene::Renderer*					renderer;
	Scene::ScreenPtr					screen;
};


#endif