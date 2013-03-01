/*
 * dwm.cpp
 *
 *  Created on: 15 Apr 2012
 *      Author: deanoc
 */

#include <sys/stat.h>
#include "core/fileio.h"
#include "riak/client.hxx"
#include "riak/transport.hxx"
#include "riak/transports/single-serial-socket.hxx"
#include "json_spirit/json_spirit_reader.h"
#include "bitcoder.h"

#include "mmu.h"
#include "world.h"
#include "vmthreads.h"
#include "dwm.h"

DECLARE_EXCEPTION( DBBackEndHard, "Failure to contact backend DB" );

#define DECL_INEXEBITCODE( NAME ) extern const unsigned char* binary_data_vm_code_##NAME##_cpp; extern size_t binary_data_vm_code_##NAME##_cpp_sizeof;
#define MEMFILE_INEXEBITCODE( NAME ) Core::MemFile( (uint8_t*) binary_data_vm_code_##NAME##_cpp, binary_data_vm_code_##NAME##_cpp_sizeof ).inOut()

std::shared_ptr<riak::object> no_sibling_resolution (const ::riak::siblings&)
{
    auto garbage = std::make_shared<riak::object>();
    garbage->set_value("<result of sibling resolution>");
    garbage->set_content_type("text/plain");
    return garbage;
}

Dwm::Dwm() {
   switcherElf = BitCoder::get()->assemble( BitCoder::TRUSTED, Core::FilePath("./switcher.S") );
   world = std::make_shared<World>();
}

Dwm::Dwm( WorldPtr _world ) {
   switcherElf = BitCoder::get()->assemble( BitCoder::TRUSTED, Core::FilePath("./switcher.S") );
   world = _world;
}

Dwm::~Dwm() {
}

bool Dwm::openCommChans( std::shared_ptr<boost::asio::io_service> _io, const std::string& hostname ) {
   io = _io;

   // TODO get address and ports from remote
   // hardcoded from base hostname for now
#if defined( DWM_TRUSTED )
   riakAddr = "192.168.254.95";//hostname;
   riakPort = 8081;//8087;
#endif
   dwmChanAddr = hostname;
   dwmChanPort = 5002;

#if defined( DWM_TRUSTED )
   // TODO fallback IPs, better error handling etc.
   CoreTry {
      riakConn = riak::make_single_socket_transport(riakAddr, riakPort, *io);
   } CoreCatch( boost::system::system_error& e ) {
      LOG(WARNING) << e.what() << "\n";
      return false;
   }
#endif

   namespace asio = boost::asio;
   using namespace asio::ip;

   std::stringstream ss;
   ss << dwmChanPort;

   dwmChanSock = std::make_shared<tcp::socket>( *io );

   // resolve the address and port into possible socket endpoints
   tcp::resolver netResolver( *io );
   tcp::resolver::query endPoint( dwmChanAddr, ss.str() );
   tcp::resolver::iterator epIter = netResolver.resolve( endPoint );
   // find the first valid endpoint that wants to communicate with us
   const tcp::resolver::iterator endPointEnd;
   boost::system::error_code err = asio::error::host_not_found;
   while (err && epIter != endPointEnd ) {
      dwmChanSock->close();
      dwmChanSock->connect(*epIter, err);
      ++epIter;
   }

   if( err ) {
      LOG(WARNING) << "Err " << err.message() << "\n";
      return false;
   }
   bool ret = true;
   // just a ping to wake t'other side
   std::array< uint8_t, 1> buffer;
   buffer[0] = 1;
   asio::write( *dwmChanSock, asio::buffer(buffer) );

   return true;
}

void Dwm::checkSysInfoVersion( const std::string& str ) {
   int version_major;
   int version_minor;
   int version_rev;

   json_spirit::Value value;
   json_spirit::read( str, value );
   if( value.is_null() )
      return;
   auto obj = value.get_obj();

   // decode json oject
   for( auto val = obj.cbegin(); val != obj.cend(); ++val ) {
      if( val->name_ == "version_major" ) {
         version_major = val->value_.get_int();
      } else if( val->name_ == "version_minor" ) {
         version_minor = val->value_.get_int();
      } else if( val->name_ == "version_rev" ) {
         version_rev = val->value_.get_int();
      }
   }
   // TODO Reject any server side where major.minor > version we are compiled with
   LOG(INFO) << "System Version: " << version_major << "." <<
                                       version_minor << "." <<
                                       version_rev << "\n"; 
}

bool checkFileTimestamps( const Core::FilePath& a, const Core::FilePath& b) {
   struct stat     aStatbuf;
   struct stat     bStatbuf;
   int aRet = stat( a.value().c_str(), &aStatbuf );
   int bRet = stat( b.value().c_str(), &bStatbuf );
   if( aRet != 0 || bRet != 0 ) {
      return false;
   }
   if( aStatbuf.st_mtime >= bStatbuf.st_mtime ) {
      return false;
   } else {
      return true;
   }
}

std::string cacheElf( const Core::FilePath& a, const Core::FilePath& b ) {
   Core::File aFile;
   Core::File bFile;

   bool cacheOk = true;

   // if either doesn't exist then not upto date obviously...
   if( aFile.open( a.value().c_str() ) == false ) {
      cacheOk = false;
   }
   if( bFile.open( b.value().c_str() ) == false ) {
      cacheOk = false;
   }

   if( (cacheOk == true) && 
       checkFileTimestamps( a, b ) ) {
      std::string ret;
      uint64_t bcLen = bFile.bytesLeft();
      ret.resize( bcLen );
      bFile.read( (uint8_t*) &ret[0], (size_t) bcLen );
      return ret;
   } else {
      // cache is not okay
      aFile.close();
      bFile.close();

      auto initbc = BitCoder::get()->loadBitCode( a );
      initbc->setModuleIdentifier( "bootstrap" );
      auto prg = BitCoder::get()->make( BitCoder::UNTRUSTED, initbc );
      bFile.createNew( b.value().c_str() );
      bFile.write( (uint8_t*) &prg[0], prg.size() );
      return prg;
   }


}

void Dwm::bootstrapLocal() {
   using namespace Core;
   using namespace llvm;

   /* whilst testing 
   auto store = riak::make_client(riakConn, &no_sibling_resolution, *io);
   store->get_object( "sys", "info", [&](const std::error_code& err, RiakObjPtr obj, riak::value_updater&) {
      if(!err) {
         if( obj->has_value() ) {
            checkSysInfoVersion( obj->value() );            
            return;
         }
      } 
      CoreThrowException( DBBackEndHard, "" );
   });
   */

   auto prg = cacheElf( Core::FilePath("./hello_world"), Core::FilePath("./cache/hello_world.elf") );

   // init thread into llvm execution environment
   auto thread = std::make_shared<VMThreads>( *this );
   vmThreads.push_back( thread );

   thread->run( prg );

}

