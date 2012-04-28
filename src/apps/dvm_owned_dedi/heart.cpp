#include "core/core.h"
#include "heart.h"

Heart::Heart( boost::asio::io_service& _io_service ) :
	io_service( _io_service ) {
}

void Heart::start( const std::string& addr, int port, int _rate ) {
	using namespace boost::asio::ip;
	rate = _rate;

	std::stringstream ss;
	ss << port;
	udp::resolver resolver( io_service );
	udp::resolver::query query( udp::v4(), addr, ss.str() );
	target = *resolver.resolve( query );
 
	beatSock = std::make_shared<udp::socket>( io_service );
	beatSock->open( boost::asio::ip::udp::v4() );
	beatTimer = std::make_shared<boost::asio::deadline_timer>( io_service );
	tick( boost::system::error_code() );
}

void Heart::tick ( const boost::system::error_code& error ) {
	namespace asio = boost::asio;
	if( !error ) {
//		LOG(INFO) << "Sending heart beat to " << target.address().to_string() << ":" << target.port() << "\n";
		beatSock->async_send_to( asio::buffer(beatBuffer, 4), target, 
			[](const boost::system::error_code&, size_t ) {
				// Do nothing at the moment, just send
		}); 
	}

	beatTimer->expires_from_now( boost::posix_time::seconds( rate ) );
	beatTimer->async_wait( boost::bind(&Heart::tick, this, _1) );
}

