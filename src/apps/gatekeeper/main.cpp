#include "core/core.h"
#include "boost/program_options.hpp"
#include "json_spirit/json_spirit_reader.h"


void readConfig( std::string& hostname, int& port ) {
   std::ifstream is( "./config.json" );
   if( is.bad() )
      return;

   json_spirit::Value value;

   json_spirit::read( is, value );
   if( value.is_null() )
      return;

   const json_spirit::Array& configArray = value.get_array();
   // array of json objects
   for( unsigned int i = 0; i < configArray.size(); ++i ) {
      auto obj = configArray[i].get_obj();
      for( auto val = obj.cbegin(); val != obj.cend(); ++val ) {
         if( val->name_ == "hostname" ) {
            hostname = val->value_.get_str();
         } else if( val->name_ == "port" ) {
            port = val->value_.get_int();
         }
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

      std::string hostname( "192.168.254.95" );
      int port( 8081 );

      readConfig( hostname, port );
   } 
   CoreCatchAllOurExceptions {
      LogException( err );
      return 1;
   }
   CoreCatchAll {
      return 1;
   }
	return 0;
}