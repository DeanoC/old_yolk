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
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCRegisterInfo.h"

namespace Core { class InOutInterface; class FilePath; }

class BitCoder : public Core::Singleton<BitCoder> {
public:
	friend class Core::Singleton<BitCoder>;
	void addLibrary( std::shared_ptr<llvm::Module> lib );
	void removeLibrary( std::shared_ptr<llvm::Module> lib );

	std::shared_ptr<llvm::Module> loadBitCode( const Core::FilePath& filepath );
	std::shared_ptr<llvm::Module> loadBitCode( Core::InOutInterface& inny );

	std::string assemble( const Core::FilePath& filepath );
	std::string assemble( Core::InOutInterface& inny );

	std::string make( std::shared_ptr<llvm::Module> bc );

protected:
	BitCoder();
	
private:
	llvm::Triple 								triple;
	std::shared_ptr<llvm::TargetMachine>		tm;
	std::shared_ptr<llvm::MCAsmBackend>			mcab;
	std::shared_ptr<llvm::MCAsmInfo>		 	mcai;
	std::shared_ptr<llvm::MCInstrInfo> 			mcii;
	std::shared_ptr<llvm::MCSubtargetInfo> 		mcsti;
	std::shared_ptr<llvm::MCRegisterInfo>		mcri;

	std::list< std::shared_ptr<llvm::Module> > 	libraries;
};


#endif
