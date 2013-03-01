#ifndef _YOLK_DWN_WORLD_H_
#define _YOLK_DWN_WORLD_H_ 1

#include "scene/renderworld.h"

class TrustedRegion;

typedef void (*ApiSetupFunction)( TrustedRegion* );

class World : public Scene::RenderWorld {
public:
	virtual ApiSetupFunction getApiSetupFunction() { return &InstallApiFuncs; }

	static void InstallApiFuncs( TrustedRegion* trustedRegion );
};

typedef std::shared_ptr<World> WorldPtr;

#endif