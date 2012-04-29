#include "core/core.h"
#include "protocols/backpassage.h"
#include "heart.h"

Heart::Heart( boost::asio::io_service& _io_service ) :
	io_service( _io_service ) {
}

void Heart::start( const std::string& addr, int port, int _rate ) {
	namespace asio = boost::asio;
	using namespace asio::ip;
	rate = _rate;

	std::stringstream ss;
	ss << port;
	udp::resolver resolver( io_service );
	udp::resolver::query query( udp::v4(), addr, ss.str() );
	target = *resolver.resolve( query );
 
	beatSock = std::make_shared<udp::socket>( io_service );
	beatSock->open( boost::asio::ip::udp::v4() );
	beatTimer = std::make_shared<asio::deadline_timer>( io_service );

	// open up the back passage (Gotse ftw!)
	beatPassageBuffer[0] = BP_NO_MESSAGE;
	backmsg( boost::system::error_code() );
	tick( boost::system::error_code() );
}

void Heart::tick ( const boost::system::error_code& error ) {
	namespace asio = boost::asio;
	if( !error ) {
//		LOG(INFO) << "Sending heart beat to " << target.address().to_string() << ":" << target.port() << "\n";
		beatSock->async_send_to( asio::buffer(beatBuffer), target, 
			[](const boost::system::error_code&, size_t ) {
				// Do nothing at the moment, just send
		}); 
	}

	beatTimer->expires_from_now( boost::posix_time::seconds( rate ) );
	beatTimer->async_wait( boost::bind(&Heart::tick, this, _1) );
}

void Heart::backmsg( const boost::system::error_code& error ) {
	namespace asio = boost::asio;

	uint8_t msg = beatPassageBuffer[0];
	// process msg
	switch( msg ) {
		case BP_RET_TCP_CHAN: {
			LOG(INFO) << "Back passage : BP_RET_TCP_CHAN recv'd"; 
			// send back anything as ack
			beatSock->async_send_to( asio::buffer(beatPassageBuffer), target, [](const boost::system::error_code&, size_t ) {});
		}
		case BP_NO_MESSAGE:
		default:
			break;
	}

	LOG(INFO) << "Waiting on the back passage from " << target.address().to_string() << ":" << target.port() << "\n";
	// wait for next backpassage msg
	beatSock->async_receive_from( asio::buffer(beatPassageBuffer), target, boost::bind(&Heart::backmsg, this, _1) );
}