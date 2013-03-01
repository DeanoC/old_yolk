#include "core/core.h"
#include "boost/program_options.hpp"
#include "json_spirit/json_spirit_reader.h"
#include "handshake.h"

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

		std::string hostname( "127.0.0.1" );
		int port( 2045 );

		boost::asio::io_service io;

		// Wait for signals indicating time to shut down.
		boost::asio::signal_set signals(io);
		signals.add(SIGINT);
		signals.add(SIGTERM);
#if defined(SIGQUIT)
		signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
		signals.async_wait(boost::bind(&boost::asio::io_service::stop, &io));

		// Create a pool of threads to run all of the io_services.
		std::vector<std::shared_ptr<Core::thread> > threads;
		for (std::size_t i = 0; i < Core::thread::hardware_concurrency(); ++i) {
			threads.push_back( std::make_shared<Core::thread>( boost::bind( &boost::asio::io_service::run, &io ) ) );
		}

		readConfig( hostname, port );
		if( Handshake( io, hostname, port ) == true ) {

			// Wait for all threads in the pool to exit.
			for (std::size_t i = 0; i < threads.size(); ++i)
				threads[i]->join();
		}
	} 
	CoreCatchAllOurExceptions {
		LogException( err );
		return 1;
	}
	CoreCatchAllStdExceptions {
		LOG(ERROR) << err.what();
		return 1;
	}
	CoreCatchAll {
		return 1;
	}
	return 0;


	return 0;
}