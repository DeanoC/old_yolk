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
#include "vmthread.h"

Dwm::Dwm() :
	context( llvm::getGlobalContext() ) {
}

llvm::Module* Dwm::loadBitCode( const Core::FilePath& filepath ) {
	using namespace Core;
	using namespace llvm;

	File bcFile;
	
	if( bcFile.open( filepath.value().c_str() ) == false ) {
		// file not found
		return nullptr;
	}

	uint64_t bcLen = bcFile.tell();

	// note on 32 bit system only load max 32 bit file size (no harm)
	MemoryBuffer *bcBuffer = MemoryBuffer::getNewMemBuffer( (size_t) bcLen );
	bcFile.read( (uint8_t*) bcBuffer->getBuffer().data(), (size_t) bcLen );
	llvm::Module* mod = llvm::ParseBitcodeFile( bcBuffer, context );

	return mod;
}

void Dwm::bootstrapLocal() {
	using namespace Core;
	
	auto hwThreads = Core::thread::hardware_concurrency();

	// load initial bitcode modules
	auto init0bc = loadBitCode( FilePath( FILE_PATH_LITERAL("./init0.bc") ) );
//	auto initNbc = loadBitCode( FilePath( FILE_PATH_LITERAL("./initN.bc") ) );

	// init thread0 into llvm execution environment
	auto thread0 = Core::shared_ptr<VMThread>( new VMThread( *this, init0bc ) );
	vmThreads.push_back( thread0 );

}



