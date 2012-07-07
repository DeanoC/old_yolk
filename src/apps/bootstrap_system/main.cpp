#include "core/core.h"
#include "riak/client.hxx"
#include "riak/transport.hxx"
#include "riak/transports/single-serial-socket.hxx"
#include "json_spirit/json_spirit_reader.h"
#include "json_spirit/json_spirit_writer.h"
#include "boost/program_options.hpp"

DECLARE_EXCEPTION(RiakSystemExists, Riak DB already has a system);
DECLARE_EXCEPTION(RiakHardNetwork, Hard Riak error likely unable to find DB check host and port );


class SystemBootStrapper {
public:
   SystemBootStrapper() :
      hostname( "127.0.0.1" ),
      port( 8087 ),
      deleteSystem( true )
   {
   }
   void Go() {
      using namespace boost;

      readConfig();
      CoreTry {
         connection = riak::make_single_socket_transport(hostname, port, ios);
         store = riak::make_client(connection, boost::bind(&SystemBootStrapper::noSiblingResolution, this, _1), ios);
         store->get_object( "sys", "info", boost::bind(&SystemBootStrapper::isRiakSetup, this, _1, _2, _3) );
         ios.run();
      } CoreCatch( boost::system::system_error& e ) {
         LOG(INFO) << e.what() << Core::Logger::endl;
         CoreReThrow;
      } CoreCatchAll {
         CoreReThrow;
      }

   }
   void allowDelete( bool enable ) {
      deleteSystem = enable;
   }
private:
   std::string                         hostname;
   int                                 port;
   bool                                deleteSystem;
   boost::asio::io_service             ios;
   riak::transport::delivery_provider  connection;
   std::shared_ptr<riak::client>      store;

   void readConfig() {
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

   std::shared_ptr<riak::object> noSiblingResolution (const riak::siblings&) {
      // TODO
      auto garbage = std::make_shared<riak::object>();
      garbage->set_value("<result of sibling resolution>");
      garbage->set_content_type("application/json");
      return garbage;
   }

   void isRiakSetup (
           const std::error_code& error,
           std::shared_ptr<riak::object> object,
           riak::value_updater& updater ) {
      using namespace boost;
      if (!error) {
         if (!! object) {
//            LOG(INFO) << str(std::format("Fetch succeeded! Value is: %1%") % object->value()) << Core::Logger::endl;
            // Riak already exist, bail from set it up and wiping stuff
            // TODO force deletion option
            if( deleteSystem ) {
               LOG(INFO) << "Overwriting existing Riak system backend..." << Core::Logger::endl;
               setupRiakVTSystem( object, updater );
            } else {
               LOG(INFO) << "Riak backend exists but already has a valid VT system on it, bailing to ensure no damge, use --delete if you really want to delete it" << Core::Logger::endl;
               CoreThrowException( RiakSystemExists, "" );
            }
         } else {
            LOG(INFO) << "Riak backend is ready to recieve a base system..." << Core::Logger::endl;
            setupRiakVTSystem( object, updater );
         }

      } else {
         LOG(INFO) << "Could not receive the object from Riak due to a hard error. Bailing" << Core::Logger::endl;
         CoreThrowException( RiakHardNetwork, error.message().c_str() );
      }
   }
   void setupRiakVTSystem( std::shared_ptr<riak::object> sysInfoObj, riak::value_updater& updater ) {
      json_spirit::Object jsonSysInfoObj;
      jsonSysInfoObj.push_back( json_spirit::Pair( "version_major", 0 ) );
      jsonSysInfoObj.push_back( json_spirit::Pair( "version_minor", 0 ) );
      jsonSysInfoObj.push_back( json_spirit::Pair( "version_rev", 0 ) );

      std::stringstream os;
      json_spirit::write_formatted( jsonSysInfoObj, os );
      LOG(INFO) << os.str();

      auto newObj = std::make_shared<riak::object>();
      newObj->set_value( os.str() );
      newObj->set_content_type("application/json");

      riak::put_response_handler fn0 = [] (const std::error_code& error) {
            if (!error) { LOG(INFO) << "sys/info put succeeded!" << Core::Logger::endl; }
            else { CoreThrowException( RiakHardNetwork, error.message().c_str() ); } 
         };

      updater(newObj, fn0 );
      store->get_object( "sys", "motd", []( const std::error_code& error, std::shared_ptr<riak::object> object, riak::value_updater& updater ) {
         if (!error) {
            json_spirit::Object jsonSysMotdObj;
            std::stringstream os;

            jsonSysMotdObj.push_back( json_spirit::Pair( "page", "The first Motd from VT" ) );
            json_spirit::write_formatted( jsonSysMotdObj, os );
            LOG(INFO) << os.str();

            auto newObj = std::make_shared<riak::object>();
            newObj->set_value( os.str() );
            newObj->set_content_type("application/json");
            riak::put_response_handler fn1 = [] (const std::error_code& error) {
                  if (!error) { LOG(INFO) << "sys/mtod put succeeded!" << Core::Logger::endl; }
                  else { CoreThrowException( RiakHardNetwork, error.message().c_str() ); } 
               };
            updater(newObj, fn1 );
         } else {
            LOG(INFO) << "Could not receive the object from Riak due to a hard error. Bailing" << Core::Logger::endl;
            CoreThrowException( RiakHardNetwork, error.message().c_str() );
         }
      });
   }

};

int Main() {
   namespace po = boost::program_options;

   // Declare the supported options.
   po::options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("delete", "delete any previous system discovered, WARNING")
   ;
   CoreTry {
      SystemBootStrapper strapper;
      po::variables_map vm;
      po::store(po::parse_command_line(Core::g_argc, Core::g_argv, desc), vm);
      po::notify(vm);    

      if (vm.count("help")) {
          LOG(INFO) << desc << "\n";
          return 1;
      }
      strapper.allowDelete( !!vm.count("delete") );

      strapper.Go();
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
