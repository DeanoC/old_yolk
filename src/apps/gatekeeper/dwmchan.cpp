/**
 @file	dwmchan.cpp

 Implements the dwmchan class.
 */

#include "core/core.h"
#include "heartbeat.h"
#include "dwmman.h"
#include "protocols/backpassage.h"
#include "dwmchan.h"

DWMChan::DWMChan( const boost::asio::ip::address& _addr, const int _area ) :
	addr( _addr ),
	area( _area )
{
}

void DWMChan::useBackPassage() {
	auto backPassage = HeartBeat::Get()->getBeatingHeart( addr );
	std::array< uint8_t, 1>	bpBuf;

	namespace asio = Core::asio;
	// TODO improve semi busy loop
	bool recved = false;
	do {
		bpBuf[0] = BP_RET_TCP_CHAN;
		asio::write( *backPassage, boost::asio::buffer(bpBuf,1) );
		asio::async_read( *backPassage, boost::asio::buffer(bpBuf,1), 
			[&recved]( const boost::system::error_code& error, size_t ) {
				if( !error ) {
					recved = true;
				} else {
					LOG(INFO) << error.message() << "\n";
				}
			}
		);

		LOG(INFO) << "Sending BP_RET_TCP_CHAN over the back passage to " << backPassage->remote_endpoint().address().to_string() << ":" << backPassage->remote_endpoint().port() << "\n";
		backPassage->get_io_service().poll();
		Core::this_thread::sleep( boost::posix_time::milliseconds(50) );
	} while( recved == false );
}

void DWMChan::accept( std::shared_ptr<boost::asio::ip::tcp::socket> incoming ) {
	LOG(INFO) << "DWM Channel open\n";
	socket = incoming;

	// DWMChan can now send and recieve info i.e player join, untrusted db access etc. 
}