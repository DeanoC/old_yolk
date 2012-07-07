#ifndef _YOLK_DWN_WORLD_H_
#define _YOLK_DWN_WORLD_H_ 1

#include "scene/renderworld.h"
class World : public Scene::RenderWorld {

};

typedef std::shared_ptr<World> WorldPtr;

#endif