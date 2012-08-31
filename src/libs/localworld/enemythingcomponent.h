#ifndef YOLK_LOCALWORLD_ENEMY_THING_COMPONENT_H_
#define YOLK_LOCALWORLD_ENEMY_THING_COMPONENT_H_ 1

#include "localworld/thingcomponent.h"

class EnemyThingComponent : public ThingComponent {
public:
	static ThingComponentId COMPONENT_ID = THING_COMP_ID('E','N','M','Y');;

	const ThingComponentFlags 	getComponentFlags() const override { return TCF_NONE; }
	const std::string 			getComponentName() const override { return "Enemy Thing Component"; }
	const ThingComponentId 		getComponentId() const override { return COMPONENT_ID; }

};

#endif