#include "core/core.h"
#include "boost/program_options.hpp"
#include "json_spirit/json_spirit_reader.h"
#include "heartbeat.h"
#include "tcpserver.h"
#include "dwmman.h"

void readConfig( std::string& hostname, int& port ) {
   std::ifstream is( "./config.json" );
   if( is.bad() || !is.is_open() )
      return;
   
   json_spirit::Value value;

   json_spirit::read( is, value );
   if( value.is_null() )
      return;

   auto obj = value.get_obj();
   for( auto val = obj.cbegin(); val != obj.cend(); ++val ) {
      if( val->name_ == "hostname" ) {
         hostname = val->value_.get_str();
      } else if( val->name_ == "port" ) {
         port = val->value_.get_int();
      }
   }
}


int Main() {
	using namespace Core;
	namespace po = boost::program_options;
	int ret  = 0;

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
	;
	CoreTry {
		po::variables_map vm;
		po::store(po::parse_command_line(Core::g_argc, Core::g_argv, desc), vm);
		po::notify(vm);    

		if (vm.count("help")) {
			LOG(INFO) << desc << "\n";
			return 1;
		}

		std::string hostname( "192.168.254.95" );
		int port( 8081 );

		readConfig( hostname, port );


		std::shared_ptr<boost::asio::io_service> ioService = std::make_shared<boost::asio::io_service>();
		std::shared_ptr<boost::asio::io_service::work> work = std::make_shared<boost::asio::io_service::work>(*ioService);
		DWMMan::Init();
		HeartBeat::Init();

		DWMMan::Get()->accept( *ioService, hostname );
		HeartBeat::Get()->accept( *ioService, hostname );

		// Launch the initial gatekeeper co-routine server.
		// once gatekeeper has decided it likes the incoming socket it hands it off 
		auto server = TcpServer( *ioService , hostname, port );
		server();

		// Wait for signals indicating time to shut down.
		boost::asio::signal_set signals( *ioService );
		signals.add(SIGINT);
		signals.add(SIGTERM);
#if defined(SIGQUIT)
		signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
		signals.async_wait( [&work, ioService](const boost::system::error_code&, int signal ) { 
			LOG(INFO) << "Signal Recv'ed, exiting\n";
			// handle signals, for now exit as soon as work queues are empty
			work.reset(); 
			// TODO handle exit now signal
			if( signal == 9 ) {
				LOG(INFO) << "Shutting down NOW!\n";
				ioService->stop();
			}
		});
		
		// Create a pool of threads to run all of the io_services.
		std::vector<std::shared_ptr<Core::thread> > threads;
		for (std::size_t i = 0; i < Core::thread::hardware_concurrency(); ++i) {
			threads.push_back( std::make_shared<Core::thread>( boost::bind( &boost::asio::io_service::run, &(*ioService) ) ) );
			threads[i]->join();
		}
		
		// Run the server.
		ioService->run();
	} 
	CoreCatchAllOurExceptions {
		LogException( err );
		ret = 1;
	}
	CoreCatchAllStdExceptions {
		LOG(ERROR) << err.what();
		ret = 1;
	}
	CoreCatchAll {
		ret = 1;
	}
	HeartBeat::Shutdown();
	DWMMan::Shutdown();
	return ret;
}