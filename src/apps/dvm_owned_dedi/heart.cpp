#include "core/core.h"
#include "protocols/backpassage.h"
#include "heart.h"

DECLARE_EXCEPTION( HeartCantStart, "Heart can't start up properly" );

extern void DWMMain( std::shared_ptr<Core::thread> leash );

Heart::Heart( boost::asio::io_service& _io_service ) :
	io_service( _io_service ) {
}

void Heart::start( const std::string& addr, int port, int _rate ) {
	namespace asio = boost::asio;
	using namespace asio::ip;
	rate = _rate;

	std::stringstream ss;
	ss << port;

	beatSock = std::make_shared<tcp::socket>( io_service );

	// resolve the address and port into possible socket endpoints
	tcp::resolver netResolver( io_service );
	tcp::resolver::query endPoint( addr, ss.str() );
	tcp::resolver::iterator epIter = netResolver.resolve( endPoint );
	// find the first valid endpoint that wants to communicate with us
	const tcp::resolver::iterator endPointEnd;
	boost::system::error_code err = asio::error::host_not_found;
	while (err && epIter != endPointEnd ) {
		beatSock->close();
		beatSock->connect(*epIter, err);
		++epIter;
	}
	if( err ) {
		CoreThrowException( HeartCantStart, err.message() );
	}

	beatTimer = std::make_shared<asio::deadline_timer>( io_service );

	// open up the back passage (Gotse ftw!)
	beatPassageBuffer[0] = BP_NO_MESSAGE;
	backmsg( boost::system::error_code() );
	tick( boost::system::error_code() );
}

void Heart::tick ( const boost::system::error_code& error ) {
	namespace asio = boost::asio;
	if( !error ) {
//		LOG(INFO) << "Sending heart beat to " << beatSock->remote_endpoint().address().to_string() << ":" << beatSock->remote_endpoint().port() << "\n";
		beatSock->async_send( asio::buffer(beatBuffer), 
			[](const boost::system::error_code& err, size_t len) {
				if( !!err ) {
					LOG(INFO) << "Err " << err.message() << "\n";
				} else {
//					LOG(INFO) << "Heart Sent " << len << " byte\n";
				}
			} ); 
	}

	beatTimer->expires_from_now( boost::posix_time::seconds( rate ) );
	beatTimer->async_wait( boost::bind(&Heart::tick, this, _1) );
}

void Heart::backmsg( const boost::system::error_code& error ) {
	if( error ) {
		LOG(INFO) << "Backmsg err : " << error.message() << "\n";
	}
	namespace asio = boost::asio;

	uint8_t msg = beatPassageBuffer[0];
	// process msg
	switch( msg ) {
		case BP_RET_TCP_CHAN: {
//			LOG(INFO) << "Back passage : BP_RET_TCP_CHAN recv'd"; 
			// send back anything as ack
			beatSock->async_send( asio::buffer(beatPassageBuffer), [](const boost::system::error_code&, size_t ) {});

			// needs to be idempotant 
			Core::unique_lock<Core::shared_mutex> mu(dwmMutex);
			Core::shared_ptr<Core::thread> sp = dwm.lock();
			// TODO use atomics, this won't work on non ordered memory semantics
			bool dwmupdated = false;
			bool waiter = true;
			if( !sp ) {
				sp.reset( CORE_NEW Core::thread( [&dwmupdated, &sp, &waiter]() {
						// wait for weak ptr ownership semantics to be taken over, before
						// taking a copy of sp for the thread to keep
						while( dwmupdated == false ){ Core::this_thread::sleep( boost::posix_time::milliseconds(5) ); };

						Core::shared_ptr<Core::thread> sp_copy = sp;
						waiter = false;
						DWMMain(sp_copy);
					}) 
				);
				dwm = sp;
				dwmupdated = true;
				while( waiter ) { Core::this_thread::sleep( boost::posix_time::milliseconds(5) ); };
			}
		}; break;
		case BP_NO_MESSAGE:
		default:
//			LOG(INFO) << "Back passage : BP_NO_MESSAGE recv'd"; 
			break;
	}

//	LOG(INFO) << "Waiting on the back passage from " << target.address().to_string() << ":" << target.port() << "\n";
	// wait for next backpassage msg
	beatSock->async_receive( asio::buffer(beatPassageBuffer,1), boost::bind(&Heart::backmsg, this, _1) );
}