#include "localworld.h"
#include "thingid.h"
#include <random>

std::mt19937 *eng = nullptr;
ThingId NewThingId() {
	// leave first N free 
	if( eng == nullptr ) {
		eng = new std::mt19937();
	}
    std::uniform_int_distribution<int> unif( 10000 );

	//return ThingId { 0, unif(*eng) };
	ThingId id;
	id.a = 0;
	id.b = unif(*eng);
	return id;
}
