#include "core/core.h"
#include "riak/client.hxx"
#include "riak/transport.hxx"
#include "riak/transports/single-serial-socket.hxx"
#include "json_spirit/json_spirit_reader.h"
#include "json_spirit/json_spirit_writer.h"

DECLARE_EXCEPTION(RiakSystemExists, Riak DB already has a system);
DECLARE_EXCEPTION(RiakHardNetwork, Hard Riak error likely unable to find DB check host and port );


class SystemBootStrapper {
public:
   SystemBootStrapper() :
      hostname( "192.168.254.95" ),
      port( 8081 )
   {
   }
   void Go() {
      readConfig();
      CoreTry {
         connection = riak::make_single_socket_transport(hostname, port, ios);
      } CoreCatch( boost::system::system_error& e ) {
         LOG(INFO) << e.what() << Core::Logger::endl;
         CoreReThrow;
      }

      store = riak::make_client(connection, Core::bind(&SystemBootStrapper::noSiblingResolution, this, Core::_1), ios);
      store->get_object( "sys", "info", Core::bind(&SystemBootStrapper::isRiakSetup, this, Core::_1, Core::_2, Core::_3) );
      ios.run();

   }
private:
   std::string hostname;
   int         port ;
   boost::asio::io_service ios;
   riak::transport::delivery_provider connection;
   Core::shared_ptr<riak::client> store;

   void readConfig() {
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

   std::shared_ptr<riak::object> noSiblingResolution (const riak::siblings&) {
       auto garbage = std::make_shared<riak::object>();
       garbage->set_value("<result of sibling resolution>");
       garbage->set_content_type("text/plain");
       return garbage;
   }

   void isRiakSetup (
           const std::error_code& error,
           std::shared_ptr<riak::object> object,
           riak::value_updater& updater ) {
      using namespace boost;
      if (!error) {
         if (!! object) {
            LOG(INFO) << str(format("Fetch succeeded! Value is: %1%") % object->value()) << Core::Logger::endl;
            // Riak already exist, bail from set it up and wiping stuff
            // TODO force deletion
            LOG(INFO) << "Riak backend exists but already has a valid VT system on it, bailing to ensure no damge, delete it first if you really want too!" << Core::Logger::endl;
            CoreThrowException( RiakSystemExists, "" );
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

      Core::stringstream os;
      json_spirit::write_formatted( jsonSysInfoObj, os );
      LOG(INFO) << os.str();

      auto newObj = std::make_shared<riak::object>();
      newObj->set_value( os.str() );
      newObj->set_content_type("text/plain");

      riak::put_response_handler putHandler = Core::bind( &SystemBootStrapper::putSysInfoResult, this, Core::_1);
      updater(newObj, putHandler );
   }
   void putSysInfoResult (const std::error_code& error)
   {
       if (!error) {
           LOG(INFO) << "Put succeeded!" << Core::Logger::endl;
       }
       else {
           LOG(ERROR) << "Put succeeded!" << Core::Logger::endl;
       }
   }

};

int Main() {

   CoreTry {
      SystemBootStrapper strapper;
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
