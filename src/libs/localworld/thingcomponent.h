#ifndef YOLK_LOCALWORLD_THING_COMPONENT_H_
#define YOLK_LOCALWORLD_THING_COMPONENT_H_ 1

#ifndef YOLK_CORE_UTILS_H_
#	include "core/core_utils.h"
#endif

typedef const uint32_t ThingComponentId;
#define THING_COMP_ID(x,y,z,w) Core::GenerateID<x,y,z,w>::value

typedef uint32_t ThingComponentFlags;

enum ThingComponentFlagsBits {
	TCF_NONE = 0,
	TCF_NEEDS_UPDATE = BIT(0),	
};

class Thing;

class ThingComponent {
public:
	virtual const ThingComponentFlags 	getComponentFlags() const = 0 ;
	virtual const std::string			getComponentName() const = 0;
	virtual const ThingComponentId 		getComponentId() const = 0;
};

#endif