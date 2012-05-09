/*
 * /Z/Projects/Cyberspace/yolk_repo/src/libs/dwm/bitcoder.h
 *
 * (c) Copywrite Cloud Pixies Ltd.
 */
#pragma once

#ifndef YOLK_DWM_BITCODER_H_
#define YOLK_DWM_BITCODER_H_

#include "llvm/Target/TargetMachine.h"
#include "llvm/Module.h"
#include "core/singleton.h"
#include "llvm/ADT/Triple.h"

class BitCoder : public Core::Singleton<BitCoder> {
public:
	friend class Core::Singleton<BitCoder>;
	void addLibrary( std::shared_ptr<llvm::Module> lib );
	void removeLibrary( std::shared_ptr<llvm::Module> lib );

	std::string make( std::shared_ptr<llvm::Module> bc );

protected:
	BitCoder();
	
private:
	llvm::Triple 								triple;
	std::shared_ptr<llvm::TargetMachine>		tm;
	std::list< std::shared_ptr<llvm::Module> > 	libraries;
};

#endif
