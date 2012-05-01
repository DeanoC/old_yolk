/*
 * dwm.h
 *
 *  Created on: 15 Apr 2012
 *      Author: deanoc
 */

#ifndef YOLK_DWM_DWM_H_
#define YOLK_DWM_DWM_H_

#include "core/core.h"
#include "core/file_path.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "core/fileio.h"
#include "riak/core_types.hxx"

class VMThread;

// todo create the two seperate libs, for now trust everyone
#define DWM_TRUSTED

#if defined( DWM_TRUSTED )
typedef std::shared_ptr<riak::object> RiakObjPtr;
#endif

class Dwm {
public:
	Dwm();
   ~Dwm();

   bool openCommChans( std::shared_ptr<boost::asio::io_service> _io, const std::string& hostname );

	void bootstrapLocal();

	llvm::Module* loadBitCode( const Core::FilePath& filepath );
   llvm::Module* loadBitCode( Core::InOutInterface& inny );

   llvm::LLVMContext& getContext() const { return context; };

private:
   void checkSysInfoVersion( const std::string& str );

#if defined( DWM_TRUSTED )
   std::string                                           riakAddr;
   int                                                   riakPort;
#endif
   std::string                                           dwmChanAddr;
   int                                                   dwmChanPort;
   std::shared_ptr<boost::asio::ip::tcp::socket>         dwmChanSock;
   std::shared_ptr<boost::asio::io_service>              io;
	Core::vector<Core::shared_ptr<VMThread>>				   vmThreads;
	llvm::LLVMContext&										      context;
	Core::unordered_map< Core::FilePath, llvm::Module* >	modules;
};


#endif /* DWM_H_ */
