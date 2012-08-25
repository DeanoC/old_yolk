#ifndef YOLK_LOCALWORLD_THINGID_H_
#define YOLK_LOCALWORLD_THINGID_H_ 1

struct ThingId {
	uint64_t	a;
	uint64_t	b;
};

ThingId NewThingId();

#endif