/**
 @file	dwmman.cpp

 Implments the dwmman class.
 */
#include "core/core.h"
#include "dwmman.h"
#include "heartbeat.h"

DWMMan::DWMMan() {
	Core::unique_lock< Core::shared_mutex > writerLock( dwmMutex );
//	inactiveDWMs.reserve( 1000 );

}

void DWMMan::addNewDWM( boost::asio::ip::address addr ) { 
	Core::unique_lock< Core::shared_mutex > writerLock( dwmMutex );
	inactiveDWMs.push_back( addr );
}

bool DWMMan::isAreaActive( int area ) { 
	Core::shared_lock< Core::shared_mutex > writerLock( dwmMutex );
	return activeDWMsByArea.find( area) != activeDWMsByArea.end(); 
}
const boost::asio::ip::address DWMMan::activateDWMForArea( int area ) {
	Core::unique_lock< Core::shared_mutex > writerLock( dwmMutex );
	assert( activeDWMsByArea.find( area ) == activeDWMsByArea.end() );

	boost::asio::ip::address noAddr = boost::asio::ip::address();
	boost::asio::ip::address dwmAddy;
	do {
		dwmAddy = inactiveDWMs.empty() ? noAddr : inactiveDWMs.back();
		if( dwmAddy != noAddr ) {
			inactiveDWMs.pop_back();
		}
	} while( dwmAddy != noAddr && HeartBeat::get()->checkAlive( dwmAddy ) == false );

	if( dwmAddy != noAddr ) {
		activateArea( dwmAddy, area );
	} 
	return dwmAddy;
}
void DWMMan::activateArea( const boost::asio::ip::address& addr, const int area ) {
	// NO lock as called from activateDWMForArea which holds it
	auto dwm = std::make_shared<DWMChan>( addr, area );
	activeDWMsByArea[ area ] = dwm;
	activeDWMsByAddr[ addr ] = dwm;
	dwm->useBackPassage();

	LOG(INFO) << "Area " << area << " activated\n";
}

void DWMMan::accept( boost::asio::io_service& io_service, const std::string& addr ) {
	ioService.reset( &io_service );

	using namespace boost::asio::ip;
	std::stringstream ss;
	ss << DWM_CHAN_PORT;

	tcp::resolver resolver( io_service );
	tcp::resolver::query query( addr, ss.str() );
	acceptor = std::make_shared<AcceptorPtr::element_type>( io_service, *resolver.resolve( query ) );

	// enough for 1 accept per HW thread (thou will be 99.9% a sleep)
	for (std::size_t i = 0; i < Core::thread::hardware_concurrency(); ++i) {
		accept();
	}
}

void DWMMan::accept() {

	SocketPtr socket = std::make_shared<SocketPtr::element_type>( *ioService );
	acceptor->async_accept( *socket, 
		[this, socket]( const boost::system::error_code& error ) {
			if (!error ) {
				auto dwmIt = activeDWMsByAddr.find( socket->remote_endpoint().address() ); 
				if(  dwmIt != activeDWMsByAddr.end() ) {
					auto dwm = *dwmIt;
					dwm.second->accept( socket );
				} else {
					LOG(INFO) << "accept recv'ed but not in the last of active DWMs... " << socket->remote_endpoint().address().to_string() << "\n";
				}
			}
			this->accept();
		}
	);
}