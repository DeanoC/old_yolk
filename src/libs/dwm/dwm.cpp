/*
 * dwm.cpp
 *
 *  Created on: 15 Apr 2012
 *      Author: deanoc
 */
#include "dwm.h"
#include "core/fileio.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "riak/client.hxx"
#include "riak/transport.hxx"
#include "riak/transports/single-serial-socket.hxx"

#include "vmthread.h"

#define DECL_INEXEBITCODE( NAME ) extern const unsigned char* binary_data_vm_code_##NAME##_cpp; extern size_t binary_data_vm_code_##NAME##_cpp_sizeof;
#define MEMFILE_INEXEBITCODE( NAME )  MemFile( (uint8_t*) binary_data_vm_code_##NAME##_cpp, binary_data_vm_code_##NAME##_cpp_sizeof )

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
	context( llvm::getGlobalContext() ) {

   #ifndef NDEBUG
      llvm::DebugFlag = false;
   #endif
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

void Dwm::bootstrapLocal() {
	using namespace Core;
   using namespace llvm;
	
	auto hwThreads = Core::thread::hardware_concurrency();

   boost::asio::io_service ios;

   riak::transport::delivery_provider connection;
   CoreTry {
      connection = riak::make_single_socket_transport("192.168.254.95", 8081, ios);
   } CoreCatch( boost::system::system_error& e ) {
      Log << e.what() << Logger::endl;
      return;
   }

   auto my_store = riak::make_client(connection, &no_sibling_resolution, ios);
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



