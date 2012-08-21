#include "scene.h"
#include "core/resourceman.h"

#include "environment.h"

namespace Scene {

Environment::Environment( Core::BinPropertyResourceHandlePtr handle ) :
	properties( handle ) {
		
	// TODO efficient search (properties in binary order or hash or ?)
	res = properties.acquire();
	for( int i = 0; i < res->getNumProperties(); ++i ) {
		auto name = res->getName( i );
		if( name == "Gravity" || name == "gravity" ) {
			CORE_ASSERT( res->getType(i) == Core::BinProperty::BPT_FLOAT );
			CORE_ASSERT( res->getCount(i) == 3 );
			gravityPtr = res->getAs<float>( i );
		}
	}
}

Environment::~Environment() {
}

Math::Vector3 Environment::getGravity() const {
	if( gravityPtr == nullptr ) {
		return Math::Vector3(0,-9.8f,0);
	} else {
		return Math::Vector3( gravityPtr );
	}
}


}