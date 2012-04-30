/**
 @file	gatekeeper/heartbeat.cpp

 Implements the heartbeat class.
 */
#include "core/core.h"
#include "heartbeat.h"

int HeartBeat::beatCount = 0;
Core::shared_mutex							HeartBeat::mapMutex;
std::map< boost::asio::ip::address, std::pair<int,int> >	HeartBeat::ip2Alive;

HeartBeat::HeartBeat( boost::asio::io_service& io_service ) :
	beatSock( std::make_shared<boost::asio::ip::udp::socket>( io_service,  boost::asio::ip::udp::endpoint( boost::asio::ip::address_v4::any(), HB_PORT ) ) ),
	beatTimer( std::make_shared<boost::asio::deadline_timer>( io_service ) )
{
	beat( boost::system::error_code() );
	tick( boost::system::error_code() );
}

void HeartBeat::tick( const boost::system::error_code& error ) {
	beatCount++;
	beatTimer->expires_from_now( boost::posix_time::seconds( HB_SECONDS ) );
	beatTimer->async_wait( boost::bind(&HeartBeat::tick, this, _1) );
}
void HeartBeat::beat( const boost::system::error_code& error ) {
	if (!error || error == boost::asio::error::message_size) {
		// port 0 is an internal call
		if( beatRemoteEndpoint.port() != 0 ) {
			Core::unique_lock< Core::shared_mutex > writerLock( mapMutex );
			ip2Alive[ beatRemoteEndpoint.address() ] = std::pair<int,int>(beatCount,HB_PORT);
//			ip2Alive[ beatRemoteEndpoint.address() ] = std::pair<int,int>(beatCount,beatRemoteEndpoint.port());
//			LOG(INFO) << "Ping from " << beatRemoteEndpoint.address().to_string() << ":" << beatRemoteEndpoint.port() << "\n";
		} 
	} else {
		LOG(INFO) << "beat error : " << error.message() << "\n";
	}

	beatSock->async_receive_from(
				boost::asio::buffer(beatBuffer), beatRemoteEndpoint,
				boost::bind(&HeartBeat::beat, this, _1));
}

bool HeartBeat::checkAlive( const boost::asio::ip::address& addr ) {
	Core::shared_lock< Core::shared_mutex > readerLock( mapMutex );
	auto ipPair = ip2Alive.find( addr );
	if( ipPair == ip2Alive.end() ) {
		return false;
	}
	if( beatCount - ipPair->second.first > HB_ITS_DEAD_JIM ) {
		readerLock.unlock();
		// also delete whilst we are here as it dead
		Core::unique_lock< Core::shared_mutex > writerLock( mapMutex );
		ip2Alive.erase( ipPair );
		return false;
	}
	return true;
}

int HeartBeat::getReturnPort( const boost::asio::ip::address& addr ) {
	Core::shared_lock< Core::shared_mutex > readerLock( mapMutex );
	auto ipPair = ip2Alive.find( addr );
	if( ipPair == ip2Alive.end() ) {
		return 0;
	} else {
		return ipPair->second.second;
	}
}
