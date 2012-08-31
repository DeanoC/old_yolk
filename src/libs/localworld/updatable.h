#ifndef YOLK_LOCALWORLD_UPDATABLE_H_
#define YOLK_LOCALWORLD_UPDATABLE_H_ 1

#include "thingcomponent.h"

class Updatable : public ThingComponent {
public:
	static ThingComponentId COMPONENT_ID = THING_COMP_ID('U','P','D','T');

	const ThingComponentFlags 	getComponentFlags() const override { return TCF_NONE; }
	const std::string 			getComponentName() const override { return "Updatable Component"; }
	const ThingComponentId 		getComponentId() const override { return COMPONENT_ID; }

	boost::function< void (float) > updateCallback;
};

typedef std::shared_ptr<Updatable> UpdatablePtr;
#endif