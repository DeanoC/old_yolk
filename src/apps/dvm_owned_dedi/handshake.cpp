#include "core/core.h"
#include "core/tcp.h"
#include "heart.h"

#include "protocols/handshake.proto.pb.h"

extern std::shared_ptr<Heart>			g_heart;

//namespace {
using namespace boost::msm; // for front::state (can't use without front as ambigious)
using namespace boost::msm::front; // for Row

// front-end: define the FSM structure 
struct HandshakeStateMachine : public state_machine_def<HandshakeStateMachine>, private boost::noncopyable {
	HandshakeStateMachine( boost::asio::io_service* _io, const std::string& addr, const std::string& port ) :
		 io_service( *_io ), serverAddr( addr ), serverPort( port ) {}

	// list of events
	struct Start {};
	struct Resolved {};
	struct ServiceReply {};
	struct DispatcherEvent {};
	struct SendHWCapacity {};

	struct ErrorEvent {};

	// The list of FSM states
	struct Empty : public front::state<> {};

	struct ResolvingHost : public front::state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			LOG(INFO) << "Resolving " << fsm.serverAddr << ":" << fsm.serverPort << "\n";
			// resolve the address and port into possible socket endpoints
			boost::asio::ip::tcp::resolver netResolver( fsm.io_service );
			boost::asio::ip::tcp::resolver::query endPoint( fsm.serverAddr, fsm.serverPort );
			boost::asio::ip::tcp::resolver::iterator epIter = netResolver.resolve( endPoint );

			// find the first valid endpoint that wants to communicate with us
			Core::TcpConnection::pointer connection = Core::TcpConnection::create( fsm.io_service );
			const boost::asio::ip::tcp::resolver::iterator endPointEnd;
			boost::system::error_code err = boost::asio::error::host_not_found;
			while (err && epIter != endPointEnd )
			{
				connection->socket().close();
				connection->socket().connect(*epIter, err);
				++epIter;
			}
			// nobody wanted to talk to us, so handshake fail
			if (err) {
				LOG(INFO) << "Resolve error : " << err.message();
				fsm.process_event( ErrorEvent() );
			} else {
				fsm.connection = connection;
				fsm.process_event( Resolved() );
			}
		}
	};

	struct Hello : public front::state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm ) {
			size_t readSize = fsm.connection->syncRead( fsm.buffer.data(), fsm.buffer.size() );
			Messages::FirstContact fc;
			fc.ParseFromArray( fsm.buffer.data(), (int) readSize );
			CORE_ASSERT( fc.has_clientstring() );
			CORE_ASSERT( fc.has_magicid() );
			CORE_ASSERT( fc.magicid() == 0xDEADDEAD );
			LOG(INFO) << "Contact made with Server : " << fc.clientstring();
			fsm.process_event( ServiceReply() );
		}
	};

	struct DWMDediRequest : public front::state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			Messages::FirstResponse fr;
			fr.set_service( Messages::FirstResponse::DWM );
			fr.SerializeToArray( fsm.buffer.data(), fsm.buffer.size() );
			fsm.connection->syncWrite( fsm.buffer.data(), fr.ByteSize() );
			fsm.process_event( DispatcherEvent() );
		}
	};
	struct Dispatcher : public front::state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			size_t readSize = fsm.connection->syncRead( fsm.buffer.data(), fsm.buffer.size() );
			Messages::RemoteDataRequest rdr;
			rdr.ParseFromArray( fsm.buffer.data(), readSize );

			switch( rdr.request() ) {
			case Messages::RemoteDataRequest::HW_CAPACITY:
				//TODO another addresses + regular updates
				assert( rdr.has_port() == false );
				assert( rdr.has_ip4() == false );
				assert( rdr.has_ip6() == false );
				assert( rdr.has_rate() == false );
				fsm.process_event( SendHWCapacity() );
				break;
			case Messages::RemoteDataRequest::DEFIB: {
				std::string addr( fsm.serverAddr );
				int port;
				int rate = 15;
				if( rdr.has_ip4() ) {
					addr = rdr.ip4();
				}
				if( rdr.has_ip6() ) {
					addr = rdr.ip6();
				}
				if( rdr.has_port() ) {
					port = rdr.port();
				} else {
					port = atoi( fsm.serverPort.c_str() );
				} 
				if( rdr.has_rate() ) {
					rate = rdr.rate();
				}

				g_heart->start( addr, port, rate );
				break;
				}
			default:
				break;
			}
		}
	};

	struct SendingHWCapacity : public front::state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			Messages::HWCapacity hc;
			hc.set_numhwthreads( Core::thread::hardware_concurrency() );
			hc.set_numcores( Core::thread::hardware_concurrency() ); // TODO physical core count not logical
			hc.set_dwmmemory( 1024 * 1024 * 1024 ); // TODO fix at 1 GiB for now

			hc.SerializeToArray( fsm.buffer.data(), (int) fsm.buffer.size() );
			fsm.connection->syncWrite( fsm.buffer.data(), hc.ByteSize() );
			fsm.process_event( DispatcherEvent() );
		}
	};

	// the two main states, ok or error, any exception switches to error which is a terminition condition of the fsm
	struct AllOk : public front::state<> {};
	struct ErrorMode : public terminate_state<> {}; 	///< this state is also made terminal so that all the events are blocked


	/// development error, state can't handle the event thats been fired
	template <class Fsm,class Event> void no_transition(Event const& e, Fsm& ,int state) {
		LOG(INFO) << "no transition from state " << state << " on event " << typeid(e).name() << "\n";
	}

	/// exchange an exception into an Error event
	template <class Fsm,class Event> void exception_caught(Event const& e,Fsm& fsm, std::exception&) {
		fsm.process_event( ErrorEvent() );
	}

	// the initial state of the FSM. Must be defined for each main states
	typedef boost::mpl::vector< Empty, AllOk > initial_state;

	// Transition table for DWM
	struct transition_table : boost::mpl::vector <
// +--------------------+-------------------+-------------------+---------------+-----------+
// |    State			|  Event			|     Next			|     Action    |  Guard    |
// +--------------------+-------------------+-------------------+---------------+-----------+
Row< Empty				, Start				, ResolvingHost		, none			, none		>, 
Row< ResolvingHost		, Resolved			, Hello				, none			, none		>,
Row< Hello				, ServiceReply		, DWMDediRequest	, none			, none		>, 
Row< DWMDediRequest		, DispatcherEvent	, Dispatcher		, none			, none		>, 
Row< Dispatcher			, SendHWCapacity	, SendingHWCapacity	, none			, none		>, 
Row< SendingHWCapacity	, DispatcherEvent	, Dispatcher		, none			, none		>, 
// +--------------+-----------------+---------------+---------------+-----------+
Row< AllOk		  , ErrorEvent		, ErrorMode		, none			, none		> 
// +--------------+-----------------+---------------+---------------+-----------+
> {};

	boost::asio::io_service&		io_service;	//!< The i/o service object
	std::string						serverAddr;	//!< The server address string
	std::string						serverPort;	//!< The server port string
	Core::TcpConnection::pointer	connection;	//!< Connection to the server once resolved
	std::array<uint8_t, 1024*8>		buffer;
};

//} // end namespace 

///-------------------------------------------------------------------------------------------------
/// \fn	bool GameServer::Utils::HandshakeWithMcp()
/// \return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Handshake( boost::asio::io_service& io_service, const std::string& addr, int port ) {

	std::stringstream ss;
	ss << port;
	boost::msm::back::state_machine<HandshakeStateMachine> sm( &io_service, addr, ss.str() );

	// needed to start the highest-level SM. This will call on_entry and mark the start of the SM
	sm.start();
	sm.process_event( HandshakeStateMachine::Start() );

	return ( !sm.is_flag_active<boost::msm::TerminateFlag>() );
}
