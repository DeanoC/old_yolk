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
#include "llvm/Type.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "core/fileio.h"

class VMThread;

class Dwm {
public:
	Dwm();
   ~Dwm();
	void bootstrapLocal();

	llvm::Module* loadBitCode( const Core::FilePath& filepath );
   llvm::Module* loadBitCode( Core::InOutInterface& inny );

   llvm::LLVMContext& getContext() const { return context; };

private:
	Core::vector<Core::shared_ptr<VMThread>>				   vmThreads;
	llvm::LLVMContext&										      context;
	Core::unordered_map< Core::FilePath, llvm::Module* >	modules;
};


#endif /* DWM_H_ */
