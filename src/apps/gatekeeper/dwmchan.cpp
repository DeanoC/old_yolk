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
/*
	// use the heartbeat back channel to get the dwm server to open up a TCP channel
	bpEndpoint = boost::asio::ip::udp::endpoint( addr, HeartBeat::getReturnPort(addr) );
	backPassage = std::make_shared<boost::asio::ip::udp::socket>( *DWMMan::Get()->getIoService() );
	backPassage->open( boost::asio::ip::udp::v4() );

	// TODO improve semi busy loop
	bool recved = false;
	do {
		bpBuf[0] = BP_RET_TCP_CHAN;
		backPassage->async_send_to( boost::asio::buffer(bpBuf,4), bpEndpoint, [](  const boost::system::error_code& error, size_t ) {			
		});
		backPassage->async_receive_from( boost::asio::buffer(bpBuf,4), bpEndpoint,	[&recved](  const boost::system::error_code& error, size_t ) {
			if( !error ) {
				recved = true;
			} else {
				LOG(INFO) << error.message() << "\n";
			}
		});
		LOG(INFO) << "Sending BP_RET_TCP_CHAN over the back passage to " << bpEndpoint.address().to_string() << ":" << bpEndpoint.port() << "\n";
		Core::this_thread::sleep( boost::posix_time::milliseconds(50) );
	} while( recved == false );*/
}
