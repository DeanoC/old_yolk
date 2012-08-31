//!-----------------------------------------------------
//!
//! \file thing.cpp
//!
//!-----------------------------------------------------
#include "localworld.h"
#include "sceneworld.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "scene/hier.h"
#include "scene/dynamicsproperties.h"
#include "scene/meshcolshape.h"
#include "scene/spherecolshape.h"
#include "scene/boxcolshape.h"
#include "scene/cylindercolshape.h"
#include "scene/dynamicphysical.h"
#include "properties.h"
#include "thing.h"

Thing::~Thing() {
	preps.clear();
	vreps.clear();
}

size_t Thing::addPhysical( Scene::PhysicalPtr prep ) {
	preps.push_back( prep );
	return preps.size() - 1;
}
size_t Thing::addRenderable( Scene::RenderablePtr vrep ) {
	vreps.push_back( vrep );
	return preps.size() - 1;
}
