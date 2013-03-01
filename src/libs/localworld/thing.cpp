//!-----------------------------------------------------
//!
//! \file thing.cpp
//!
//!-----------------------------------------------------
#include "localworld.h"
#include "thing.h"

Thing::~Thing() {
	preps.clear();
	prepColMasks.clear();
	sreps.clear();
	srepColMasks.clear();
	vreps.clear();
}
size_t Thing::add( Scene::RenderablePtr vrep ) {
	vreps.push_back( vrep );
	return vreps.size() - 1;
}
size_t Thing::add( Scene::PhysicalPtr prep, uint32_t _collisionMask ) {
	preps.push_back( prep );
	prepColMasks.push_back( _collisionMask );
	return preps.size() - 1;
}
size_t Thing::add( Scene::PhysicSensorPtr srep, uint32_t _collisionMask ) {
	sreps.push_back( srep );
	srepColMasks.push_back( _collisionMask );
	return sreps.size() - 1;
}