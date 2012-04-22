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

typedef std::shared_ptr<riak::object> RiakObjPtr;

class Dwm {
public:
	Dwm();
   ~Dwm();
	void bootstrapLocal();

	llvm::Module* loadBitCode( const Core::FilePath& filepath );
   llvm::Module* loadBitCode( Core::InOutInterface& inny );

   llvm::LLVMContext& getContext() const { return context; };

   void setRiakAddress ( const std::string& addr ) { riakAddr = addr; }
   void setRiakPort( const int port ) { riakPort = port; }

private:
   void checkSysInfoVersion( const std::string& str );

   std::string                                           riakAddr;
   int                                                   riakPort;

	Core::vector<Core::shared_ptr<VMThread>>				   vmThreads;
	llvm::LLVMContext&										      context;
	Core::unordered_map< Core::FilePath, llvm::Module* >	modules;
};


#endif /* DWM_H_ */
