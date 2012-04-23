/*
 * dwm.cpp
 *
 *  Created on: 15 Apr 2012
 *      Author: deanoc
 */
#include "dwm.h"
#include "core/fileio.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "riak/client.hxx"
#include "riak/transport.hxx"
#include "riak/transports/single-serial-socket.hxx"
#include "json_spirit/json_spirit_reader.h"

#include "vmthread.h"

DECLARE_EXCEPTION( DBBackEndHard, "Failure to contact backend DB" );

#define DECL_INEXEBITCODE( NAME ) extern const unsigned char* binary_data_vm_code_##NAME##_cpp; extern size_t binary_data_vm_code_##NAME##_cpp_sizeof;
#define MEMFILE_INEXEBITCODE( NAME ) Core::MemFile( (uint8_t*) binary_data_vm_code_##NAME##_cpp, binary_data_vm_code_##NAME##_cpp_sizeof ).inOut()

DECL_INEXEBITCODE( helloworld );
DECL_INEXEBITCODE( bootstrap );

std::shared_ptr<riak::object> no_sibling_resolution (const ::riak::siblings&)
{
    auto garbage = std::make_shared<riak::object>();
    garbage->set_value("<result of sibling resolution>");
    garbage->set_content_type("text/plain");
    return garbage;
}

Dwm::Dwm() :
	context( llvm::getGlobalContext() ),
   riakAddr( "127.0.0.1" ),
   riakPort( 8087 ) {
}

Dwm::~Dwm() {
}

llvm::Module* Dwm::loadBitCode( const Core::FilePath& filepath ) {
	using namespace Core;
	using namespace llvm;

	File bcFile;
	
	if( bcFile.open( filepath.value().c_str() ) == false ) {
		// file not found
		return nullptr;
	}
   return loadBitCode( bcFile );
}

llvm::Module* Dwm::loadBitCode( Core::InOutInterface& inny ) {
	using namespace Core;
	using namespace llvm;

	uint64_t bcLen = inny.bytesLeft();

	// note on 32 bit system only load max 32 bit file size (no harm)
	MemoryBuffer *bcBuffer = MemoryBuffer::getNewMemBuffer( (size_t) bcLen );
	inny.read( (uint8_t*) bcBuffer->getBuffer().data(), (size_t) bcLen );
	llvm::Module* mod = llvm::ParseBitcodeFile( bcBuffer, context );

	return mod;
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

}

void Dwm::bootstrapLocal() {
	using namespace Core;
   using namespace llvm;

	auto hwThreads = Core::thread::hardware_concurrency();

   boost::asio::io_service ios;

   // TODO fallback IPs, better error handling etc.
   riak::transport::delivery_provider connection;
   CoreTry {
      connection = riak::make_single_socket_transport(riakAddr, riakPort, ios);
   } CoreCatch( boost::system::system_error& e ) {
      LOG(ERROR) << e.what() << Logger::endl;
      return;
   }

   auto store = riak::make_client(connection, &no_sibling_resolution, ios);
   store->get_object( "sys", "info", [&](const std::error_code& err, RiakObjPtr obj, riak::value_updater&) {
      if(!err) {
         if( obj->has_value() ) {
            checkSysInfoVersion( obj->value() );            
            return;
         }
      } 
      CoreThrowException( DBBackEndHard, "" );
   });
   ios.run();

   // load initial bitcode modules
   auto initbc = loadBitCode( MEMFILE_INEXEBITCODE( bootstrap ) );

	// init thread0 into llvm execution environment
	auto thread0 = Core::shared_ptr<VMThread>( new VMThread( *this, initbc ) );
	vmThreads.push_back( thread0 );

   std::vector<GenericValue> args;
//   args.push_back( debugFnGV );
//   thread0->getEngine()->exec->addGlobalMapping( debugFnGV, &DebugOutFn );

   // lets start booting
   thread0->getEngine()->run( "bootstrap0", args );

}



