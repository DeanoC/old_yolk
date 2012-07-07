/**
 @file	gatekeeperfsm.h

 Declares the gatekeeperfsm
 */
#pragma once
#ifndef GATEKEEPER_GATEKEEPERFSM_H_
#define GATEKEEPER_GATEKEEPERFSM_H_

#include "fsmevents.h"
#include "heartbeat.h"
#include "protocols/handshake.proto.pb.h"
#include "dwmman.h"
class Connection;

// don't usually do this but makes the statement defs much shorter!
using namespace boost::msm::front;
using namespace FSMEvents;
///-------------------------------------------------------------------------------------------------
/// \struct	GatekeeperFSM
///
/// \brief	Gatekeeper finite state machine. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
struct GatekeeperFSM : public state_machine_def<GatekeeperFSM> {
	GatekeeperFSM( Connection* servr ) :
		server( servr ) {}

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
			boost::asio::async_write( *fsm.server->getSocket(), boost::asio::buffer( fsm.buffer.data(), fc.ByteSize()+sizeof(uint32_t) ), *fsm.server->tmpServer );
		}
	};

	struct GeneralRecv : public state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			LOG(INFO) << "GeneralRecv";
			namespace asio = boost::asio;
			asio::async_read( *fsm.server->getSocket(), asio::buffer(fsm.buffer), 
				[&fsm](const boost::system::error_code& error, std::size_t bytes_transferred ) -> size_t {
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
			if( fr.service() == Messages::FirstResponse::CLIENT && fr.has_id()) {
				auto clientEvent = FSMEvents::WantClientService();
				clientEvent.id = fr.id();
				fsm.process_event( clientEvent );
			}
			(*fsm.server->tmpServer)( boost::system::error_code(), fr.service() );
		}
	};

	struct ClientService : public state<> {
		template <class EVT,class FSM> void on_entry(EVT const& evt,FSM& fsm) {
			LOG(INFO) << "Client Service";
			if( evt.id == 0 ) {
				// special dummy client id
				int area = 1;
				if( DWMMan::get()->isAreaActive( area ) ) {
				} else {
					// TODO get this party started
					DWMMan::get()->activateDWMForArea( area );
				}
			} else {
				assert( false ); // TODO 
			}

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
				boost::asio::async_write( *fsm.server->getSocket(), boost::asio::buffer( fsm.buffer.data(), req.ByteSize()+sizeof(uint32_t) ), *fsm.server->tmpServer );
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

			// now send a defib message
			Messages::RemoteDataRequest req;
			req.set_request( Messages::RemoteDataRequest::DEFIB );
			req.set_port( HB_PORT );
			req.set_rate( HB_SECONDS );
			req.SerializeToArray( fsm.buffer.data()+sizeof(uint32_t), fsm.buffer.size()-sizeof(uint32_t) );
			*((uint32_t*)fsm.buffer.data()) = req.ByteSize();
			boost::asio::async_write( *fsm.server->getSocket(), boost::asio::buffer( fsm.buffer.data(), req.ByteSize()+sizeof(uint32_t) ), *fsm.server->tmpServer );
			// and add to the unused dwm list (riak? TODO)
			DWMMan::get()->addNewDWM( fsm.server->getSocket()->remote_endpoint().address() );
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

	// the initial state of the FSM. Must be defined for each main states
	typedef boost::mpl::vector< Empty, AllOk > initial_state;

	// Transition table for gatekeeper
	struct transition_table : boost::mpl::vector <
// +----------------+-------------------+-------------------+-----------+--------+
// |    State		|      Event		|      Next			|  Action   | Guard  |
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
	Connection* server;
	std::array<uint8_t, 8192> buffer;

};

class GatekeeperFSMHelper : public boost::msm::back::state_machine<GatekeeperFSM> {
public:
	GatekeeperFSMHelper( Connection* server ) :
		boost::msm::back::state_machine<GatekeeperFSM>( server ) {}
};


#endif