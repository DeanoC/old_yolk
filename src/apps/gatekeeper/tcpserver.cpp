/**
 @file	gatekeeper/tcpserver.cpp

 Implements the tcpserver class.
 */
#include "core/core.h"
#include "tcpserver.h"
#include "protocols/handshake.proto.pb.h"

TcpServer::TcpServer( boost::asio::io_service& io_service,
		    const std::string& address, const int& port ) {
	boost::asio::ip::tcp::resolver resolver(io_service);
	std::stringstream ss;
	ss << port;
	boost::asio::ip::tcp::resolver::query query(address, ss.str() );
	acceptor.reset( CORE_NEW boost::asio::ip::tcp::acceptor(io_service, *resolver.resolve(query)));
}

// don't usually do this but makes the statement defs much shorter!
using namespace Core::msm::front;
///-------------------------------------------------------------------------------------------------
/// \struct	ServerHandshakeFSM
///
/// \brief	handshake finite state machine. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
struct ServerHandshakeFSM : public state_machine_def<ServerHandshakeFSM> {
	ServerHandshakeFSM( ClientConnection* servr ) :
		server( servr ) {}

	// list of events
	struct Contact {};
	struct GetResponse {};
	struct ServiceRecv {};
	struct ErrorEvent {};
	struct WantClientService {};	
	struct WantDWMService {};	
	struct HWCapacityRecv {};

	// The list of FSM states  
	struct Empty : public state<> {};

	struct FirstContact : public state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			LOG(INFO) << "First Contact";
			Messages::FirstContact fc;
			fc.set_magicid( 0xDEADDEAD );
			fc.set_clientstring( "Gatekeeper" );
			// write the protobuf into the buffer, leaving space for the size at the start
			fc.SerializeToArray( fsm.buffer.data()+sizeof(uint32_t), fsm.buffer.size()-sizeof(uint32_t) );
			*((uint32_t*)fsm.buffer.data()) = fc.ByteSize();
			Core::asio::async_write( *fsm.server->getSocket(), Core::asio::buffer( fsm.buffer.data(), fc.ByteSize()+sizeof(uint32_t) ), *fsm.server->tmpServer );
		}
	};

	struct GeneralRecv : public state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			LOG(INFO) << "GeneralRecv";
			namespace asio = boost::asio;
			Core::asio::async_read( *fsm.server->getSocket(), asio::buffer(fsm.buffer), 
				[&](const boost::system::error_code& error, std::size_t bytes_transferred ) -> size_t {
					if( bytes_transferred >= sizeof(uint32_t) ) {
						// first 4 bytes is size
						return std::max( (size_t)(*((uint32_t*)fsm.buffer.data()) + sizeof(uint32_t)) - bytes_transferred, (size_t)0 );
					} else {
						return sizeof(uint32_t);
					}
				},
				*fsm.server->tmpServer);
		}
	};

	struct SelectService : public state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			LOG(INFO) << "Select Service";
			Messages::FirstResponse fr;
			fr.ParseFromArray( fsm.buffer.data() + sizeof(uint32_t), *((uint32_t*)fsm.buffer.data()) );
			(*fsm.server->tmpServer)( boost::system::error_code(), fr.service() );
		}
	};

	struct ClientService : public state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			LOG(INFO) << "Client Service";
		}
	};
	struct DWMService : public state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			LOG(INFO) << "DWM Service";
			auto remoteEndpoint = fsm.server->getSocket()->remote_endpoint();
			// TODO decide where trusted or untrusted based on safe IP range
			bool trusted = true;
			if( trusted ) {
				Messages::RemoteDataRequest req;
				req.set_request( Messages::RemoteDataRequest::HW_CAPACITY );
				req.SerializeToArray( fsm.buffer.data()+sizeof(uint32_t), fsm.buffer.size()-sizeof(uint32_t) );
				*((uint32_t*)fsm.buffer.data()) = req.ByteSize();
				Core::asio::async_write( *fsm.server->getSocket(), Core::asio::buffer( fsm.buffer.data(), req.ByteSize()+sizeof(uint32_t) ), *fsm.server->tmpServer );
			} else {
				// TODO untrusted DWM server insertation
				// reject it an exit communication llop
				(*fsm.server->tmpServer)( boost::system::error_code(), 0 );
			}
		}
	};

	struct GotHWCapacity : public state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			LOG(INFO) << "GotHWCapacity";
			Messages::HWCapacity hc;
			hc.ParseFromArray( fsm.buffer.data() + sizeof(uint32_t), *((uint32_t*)fsm.buffer.data()) );
			LOG(INFO) << "New DWM Server with " << hc.numhwthreads() << " HW Threads\n";
		}
	};	

	// the two main states, ok or error, any exception switches to error which is a terminition condition of the fsm
	struct AllOk : public state<> {};
	struct ErrorMode : public terminate_state<> {}; 	///< this state is also made terminal so that all the events are blocked

	/// development error, state can't handle the event thats been fired
	template <class Fsm,class Event> void no_transition(Event const& e, Fsm& ,int state) {
		LOG(INFO) << "no transition from state " << state << " on event " << typeid(e).name() << "\n";
	}

	/// exchange an exception into an Error event
	template <class Fsm,class Event> void exception_caught(Event const& e,Fsm& fsm, std::exception&) {
		fsm.process_event( ErrorEvent() );
	}

	// the initial state of the SM. Must be defined
	// two regions the standard state (0) and the error state (1)
	typedef Core::mpl::vector< Empty, AllOk > initial_state;

	// Transition table for player
	struct transition_table : Core::mpl::vector <
// +----------------+-------------------+-------------------+-----------+--------+
// |    Start		|      Event		|      Next			|  Action   | Guard  |
// +----------------+-------------------+-------------------+-----------+--------+
Row< Empty			, Contact			, FirstContact		, none      , none   >, 
Row< FirstContact	, GetResponse		, GeneralRecv		, none      , none   >,
Row< GeneralRecv	, ServiceRecv		, SelectService		, none      , none   >,
Row< GeneralRecv	, HWCapacityRecv	, GotHWCapacity		, none      , none   >,
Row< SelectService	, WantClientService	, ClientService		, none      , none   >,
Row< SelectService	, WantDWMService	, DWMService		, none      , none   >,
Row< DWMService		, GetResponse		, GeneralRecv		, none      , none   >,
// +----------------+-------------------+-------------------+-----------+--------+
Row< AllOk			, ErrorEvent		, ErrorMode			, none      , none   > 
// +----------------+-------------------+-------------------+-----------+--------+
> {};
	ClientConnection* server;
	std::array<uint8_t, 8192> buffer;

};

class HandShakeFSM : public Core::msm::back::state_machine<ServerHandshakeFSM> {
public:
	HandShakeFSM( ClientConnection* server ) :
		Core::msm::back::state_machine<ServerHandshakeFSM>( server ) {}
};

ClientConnection::ClientConnection( boost::asio::io_service& io_service ) {
   socket = std::make_shared<boost::asio::ip::tcp::socket>( io_service );
   fsm = CORE_NEW HandShakeFSM( this );
}

ClientConnection::~ClientConnection() {
	boost::system::error_code ec;
	socket->shutdown( boost::asio::ip::tcp::socket::shutdown_both, ec );
	socket.reset();
	CORE_DELETE( fsm );
}
template<class Event>
void ClientConnection::process_event(TcpServer* server, Event const& evt) {
	tmpServer = server;
	fsm->process_event( evt );
}

/**
 Functor called to handle client.
 Calls to this become a co-operative fibered based multi-tasker.
 Parent are spawned by the io_service when an event happens, accept the socket
 and connection and fork a child before existing the function.
 Children handle the connections, (for single true thread when the parents exits)
 at yield points it can switch to other co-op thread, when the child is done 
 it close the socket its been handling and exits the function.
 Multiple threads can attach to the io_service (by run()) to allow pre-emptive 
 threading to occur on top. This ensures HW threads are used in consumption of 
 network input.
 NOTE: parent threads should not use the socket they create in anyway after the
 child fork without mutexs. Currently there is no guarentee of hw thread pinning
 so each yield/reenter may transfer onto a different HW thread, this should only
 be a problem in terms of caches (performance on snooped caches + explicit flush
 on DIY cache snooping platforms).
 */
#include "core/yield.h"
// when called from io_service param == length when called directly can be whatever
// you like
void TcpServer::operator()( boost::system::error_code ec, std::size_t param ) {
   if(!ec) {
		reenter(this) {
			do {
				// TODO socket arena/pool
				connection.reset( CORE_NEW ClientConnection( acceptor->get_io_service() ) );

				yield acceptor->async_accept( *connection->getSocket(), *this );

				// kick off a child which will pass the which and execute
				// the handling code below
				fork TcpServer(*this)();

			} while( is_parent() );

			// child thread handles stuff here
			yield connection->process_event( this, ServerHandshakeFSM::Contact() );
         	yield connection->process_event( this, ServerHandshakeFSM::GetResponse() );
         	yield connection->process_event( this, ServerHandshakeFSM::ServiceRecv() );
			if( param == Messages::FirstResponse::CLIENT ) {
       			/*yield */connection->process_event( this, ServerHandshakeFSM::WantClientService() );
			} else if( param == Messages::FirstResponse::DWM ) {
	         	yield connection->process_event( this, ServerHandshakeFSM::WantDWMService() );
				if( param != 0 ) {
	         		yield connection->process_event( this, ServerHandshakeFSM::GetResponse() );
	         		yield connection->process_event( this, ServerHandshakeFSM::HWCapacityRecv() );
				}
			}

			// finished talking, close the socket
			connection.reset();
		}
	}
}
#include "core/unyield.h"
