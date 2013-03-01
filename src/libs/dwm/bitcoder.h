/*
 * /Z/Projects/Cyberspace/yolk_repo/src/libs/dwm/bitcoder.h
 *
 * (c) Copyright Cloud Pixies Ltd.
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
//	void addLibrary( llvm::Module* lib );
//	void removeLibrary( llvm::Module* lib );

	static const int UNTRUSTED = 0;
	static const int TRUSTED = 1;

	llvm::Module* loadBitCode( const Core::FilePath& filepath );
	llvm::Module* loadBitCode( Core::InOutInterface& inny );

	std::string assemble( const int type, const Core::FilePath& filepath );
	std::string assemble( const int type, Core::InOutInterface& inny );

	// make owns (and destroys) prg whilst making it!
	std::string make( const int type, llvm::Module* prg );

protected:
	BitCoder();
	
private:
	llvm::Triple 								untrustedTriple;
	llvm::Triple 								trustedTriple;

	// code generator for trusted and untrusted code generation
	std::shared_ptr<llvm::TargetMachine>		tm[2];
	llvm::MCAsmBackend*							mcab[2];
	std::shared_ptr<llvm::MCAsmInfo>		 	mcai[2];
	std::shared_ptr<llvm::MCInstrInfo> 			mcii[2];
	std::shared_ptr<llvm::MCSubtargetInfo> 		mcsti[2];
	std::shared_ptr<llvm::MCRegisterInfo>		mcri[2];


//	std::list< llvm::Module* > 	libraries;
};


#endif
