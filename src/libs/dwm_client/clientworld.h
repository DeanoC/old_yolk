#ifndef _YOLK_DWM_CLIENT_CLIENT_WORLD_H_
#define _YOLK_DWM_CLIENT_CLIENT_WORLD_H_ 1

#include "dwm/world.h"

class ClientWorld : public World {
public:
};

typedef std::shared_ptr<ClientWorld> ClientWorldPtr;

#endif