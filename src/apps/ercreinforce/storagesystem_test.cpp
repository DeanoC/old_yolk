#include <boost/test/unit_test.hpp>

#include "basestoragesystem.h"

struct F {
	F() : storageSystem( nullptr ) {}

	std::unique_ptr< BaseStorageSystem > storageSystem;
};

BOOST_FIXTURE_TEST_SUITE( StorageSystem_Test, F )

BOOST_AUTO_TEST_CASE( constructor_test ) {
	storageSystem.reset( new BaseStorageSystem() );
	BOOST_CHECK( storageSystem );
}

BOOST_AUTO_TEST_SUITE_END()
