#ifndef YOLK_LOCALWORLD_THING_FACTORY_H_
#define YOLK_LOCALWORLD_THING_FACTORY_H_ 1

#include "thing.h"
#include "levelthing.h"

#define thingDecodeFuncPtr void (Thing*, std::unordered_map<std::string, int>&, const Core::BinPropertyResourcePtr&, Core::TransformNode *)
#define levelThingDecodeFuncPtr void (LevelThing*, std::unordered_map<std::string, int>&, const Core::BinPropertyResourcePtr&, Core::TransformNode *)

class ThingFactory {
public:
	static Thing* createEmptyThing( const THING_BROAD_CATEGORIES _bc, const ThingId _id = NewThingId() );
	static Thing* createThingFromHier( Scene::HierPtr hier, const THING_BROAD_CATEGORIES _bc, const ThingId _id = NewThingId(), boost::function< thingDecodeFuncPtr > callback = &decodePhysicProperties );
	static LevelThing* createLevelFromHier( Scene::HierPtr hier, const ThingId _id = NewThingId(), boost::function< levelThingDecodeFuncPtr > callback = nullptr );
private:
	static void decodePhysicProperties( Thing* thing, std::unordered_map<std::string, int>& nameMap, 
										const Core::BinPropertyResourcePtr& binProp, Core::TransformNode * transformNode );
	static void decodeLevelProperties( LevelThing* thing, std::unordered_map<std::string, int>& nameMap, 
										const Core::BinPropertyResourcePtr& binProp, Core::TransformNode * transformNode );

};

#endif