#ifndef YOLK_LOCALWORLD_LEVEL_THING_H_
#define YOLK_LOCALWORLD_LEVEL_THING_H_ 1

#include "thing.h"

// level things don't have a lot todo, as the SceneWorld, actually has the stuff in it 
// (and all things have a pointer to that, still stuff can go here as nessecary
// TODO maybe this is more a component...
class LevelThing : public Thing {
public:
	friend class ThingFactory;
protected:
	LevelThing( const ThingId _id ) : Thing( TBC_WORLD, _id ) {}

	
};

typedef std::shared_ptr<LevelThing> LevelThingPtr;

#endif