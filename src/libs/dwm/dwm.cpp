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
#include "bitcoder.h"
#include "mmu.h"

#include "vmthread.h"

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

Dwm::Dwm() :
	context( llvm::getGlobalContext() ) {

   auto libcbc = loadBitCode( Core::FilePath( "./libc.a" ) );
   BitCoder::get()->addLibrary( libcbc );
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

std::shared_ptr<llvm::Module> Dwm::loadBitCode( const Core::FilePath& filepath ) {
	using namespace Core;
	using namespace llvm;

	File bcFile;
	
	if( bcFile.open( filepath.value().c_str() ) == false ) {
		// file not found
		return nullptr;
	}
   return loadBitCode( bcFile );
}

std::shared_ptr<llvm::Module> Dwm::loadBitCode( Core::InOutInterface& inny ) {
	using namespace Core;
	using namespace llvm;

	uint64_t bcLen = inny.bytesLeft();

	// note on 32 bit system only load max 32 bit file size (no harm)
	MemoryBuffer *bcBuffer = MemoryBuffer::getNewMemBuffer( (size_t) bcLen );
	inny.read( (uint8_t*) bcBuffer->getBuffer().data(), (size_t) bcLen );
	llvm::Module* mod = llvm::ParseBitcodeFile( bcBuffer, context );

	return std::shared_ptr<llvm::Module>(mod);
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

void Dwm::bootstrapLocal() {
   using namespace Core;
   using namespace llvm;

   auto hwThreads = Core::thread::hardware_concurrency();
   /* ust testing 
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
   // load initial bitcode modules
//   auto initbc = loadBitCode( MEMFILE_INEXEBITCODE( bootstrap ) );
   auto initbc = loadBitCode( Core::FilePath("./hello_world") );
   initbc->setModuleIdentifier( "bootstrap" );
   auto prg = BitCoder::get()->make( initbc );
   // init thread0 into llvm execution environment
   auto thread0 = Core::shared_ptr<VMThread>( new VMThread( *this ) );
   vmThreads.push_back( thread0 );

   thread0->getEngine()->process( prg );

//   std::vector<GenericValue> args;
   //   args.push_back( debugFnGV );
   //   thread0->getEngine()->exec->addGlobalMapping( debugFnGV, &DebugOutFn );

   // lets start booting
 //  thread0->getEngine()->run( "bootstrap0", args );

}