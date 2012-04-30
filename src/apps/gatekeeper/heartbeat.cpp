/**
 @file	gatekeeper/heartbeat.cpp

 Implements the heartbeat class.
 */
#include "core/core.h"
#include "heartbeat.h"

void HeartBeat::setAddress( boost::asio::io_service& io_service, const std::string& addr )
{
	ioService = &io_service;

	using namespace boost::asio::ip;
	std::stringstream ss;
	ss << HB_PORT;

	timer = std::make_shared<DeadLineTimerPtr::element_type>( io_service );

	tcp::resolver resolver( io_service );
	tcp::resolver::query query( addr, ss.str() );
	acceptor = std::make_shared<AcceptorPtr::element_type>( io_service, *resolver.resolve( query ) );

	tick( boost::system::error_code() );

	// upto 2 heartbeat recvs, as low priority don't want any more threads (even though
	// the time spend not waitign should be vanishingly small but does involve a mutex so...)
	recv();
	recv();
}

void HeartBeat::tick( const boost::system::error_code& error ) {
	beatCount++;
	timer->expires_from_now( boost::posix_time::seconds( HB_SECONDS ) );
	timer->async_wait( boost::bind(&HeartBeat::tick, this, _1) );
}

void HeartBeat::recv() {
	SocketPtr socket = std::make_shared<SocketPtr::element_type>( *ioService );
	acceptor->async_accept( *socket, 
			[this, socket]( const boost::system::error_code& error ){
				if (!error ) {
					Core::unique_lock< Core::shared_mutex > writerLock( mapMutex );
					ip2Alive[ socket->remote_endpoint().address() ] = 
										std::pair<int, SocketPtr>(beatCount, socket);
					LOG(INFO) << "Ping from " << socket->remote_endpoint().address().to_string() << ":" 
															<< socket->remote_endpoint().port() << "\n";
				}
				this->recv();
			}
	);

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

HeartBeat::SocketPtr HeartBeat::getBeatingHeart( const boost::asio::ip::address& addr ) {
	if( checkAlive( addr ) ) {
		Core::shared_lock< Core::shared_mutex > readerLock( mapMutex );
		auto ipPair = ip2Alive.find( addr );
		if( ipPair == ip2Alive.end() ) {
			return nullptr;
		}
		return ipPair->second.second;
	} else {
		return nullptr;
	}
}
