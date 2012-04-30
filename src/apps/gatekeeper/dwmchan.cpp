/**
 @file	dwmchan.cpp

 Implements the dwmchan class.
 */

#include "core/core.h"
#include "heartbeat.h"
#include "dwmman.h"
#include "protocols/backpassage.h"
#include "dwmchan.h"

DWMChan::DWMChan( const boost::asio::ip::address& addr, const int area ) {
	auto backPassage = HeartBeat::Get()->getBeatingHeart( addr );

	namespace asio = Core::asio;
	// TODO improve semi busy loop
	bool recved = false;
	do {
		bpBuf[0] = BP_RET_TCP_CHAN;
		asio::async_write( *backPassage, boost::asio::buffer(bpBuf,1), 
			[]( const boost::system::error_code&, size_t ) {} );
		asio::async_read( *backPassage, boost::asio::buffer(bpBuf,1), 
			[&recved]( const boost::system::error_code& error, size_t ) {
				if( !error ) {
					recved = true;
				} else {
					LOG(INFO) << error.message() << "\n";
				}
		});

		LOG(INFO) << "Sending BP_RET_TCP_CHAN over the back passage to " << backPassage->remote_endpoint().address().to_string() << ":" << backPassage->remote_endpoint().port() << "\n";
		Core::this_thread::sleep( boost::posix_time::milliseconds(50) );
	} while( recved == false );
}
