#include <boost/test/unit_test.hpp>

#include "memoryblockstoragedevice.h"

BOOST_AUTO_TEST_SUITE( MemoryBlockStoreDevice_Test )

BOOST_AUTO_TEST_CASE( constructor_test ) {
	std::unique_ptr< MemoryStorageDevice > ts( new MemoryStorageDevice( 10000, 4096 ) );
	BOOST_CHECK( ts );
	BOOST_CHECK_EQUAL( ts->getBlockSize(),  4096 );
	BOOST_CHECK_EQUAL( ts->getBlockCount(), 2 );
}

BOOST_AUTO_TEST_CASE( read_write_test ) {
	static const int BLOCK_SIZE = 4096;
	std::unique_ptr< MemoryStorageDevice > ts( new MemoryStorageDevice( 10000, BLOCK_SIZE ) );
	uint8_t empty[ BLOCK_SIZE ];
	memset( empty, 0, BLOCK_SIZE );
	ts->writeBlock( 0, empty );
	auto out = ts->readBlock( 0 );
	BOOST_CHECK_EQUAL( memcmp( empty, out, BLOCK_SIZE ), 0 );
	uint8_t vals[ BLOCK_SIZE ];
	for( int i = 0; i < BLOCK_SIZE; ++i ) {
		vals[i] = i;
	}
	auto out2 = ts->readBlock( 0 );
	BOOST_CHECK_NE( memcmp( vals, out2, BLOCK_SIZE ), 0 );
	ts->writeBlock( 0, vals );
	auto out3 = ts->readBlock( 0 );
	BOOST_CHECK_EQUAL( memcmp( vals, out3, BLOCK_SIZE ), 0 );
	ts->writeBlock( 1, vals );
	auto out4 = ts->readBlock( 1 );
	BOOST_CHECK_EQUAL( memcmp( vals, out4, BLOCK_SIZE ), 0 );
	BOOST_CHECK_EQUAL( memcmp( out3, out4, BLOCK_SIZE ), 0 );

}

BOOST_AUTO_TEST_SUITE_END()
