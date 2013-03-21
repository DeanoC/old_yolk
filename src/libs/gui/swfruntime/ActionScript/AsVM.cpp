/*
 *  AsVM.cpp
 *  SwfPreview
 *
 *  Created by Deano on 19/07/2009.
 *  Copyright 2013 Cloud Pixies Ltd. All rights reserved.
 *
 */

#include "gui/swfruntime/swfruntime.h"
#include "gui/SwfParser/EndianBitConverter.h"
#include "gui/SwfParser/SwfActionByteCode.h"
#include "gui/swfruntime/movieclip.h"
#include "AsFunctionBuilder.h"
#include "AsFunction.h"
#include "AsByteCode.h"
#include "AsAgRuntime.h"
#include "AsObjectFunction.h"
#include "AsVM.h"

typedef void (Swf::AsAgRuntime::*AsAgRuntimeFunc)();
AsAgRuntimeFunc g_AsAgRuntimeFuncs[256];

namespace Swf {
	const char* s_AsByteCodeNames[256];
	const EndianBitConverter& AsVM::endianConverter = EndianBitConverter::CreateForLittleEndian();
		
	AsVM::AsVM( const std::string& _name ) {
		// create human readibles names
		for (int i = 0; i < 256; ++i) {
			s_AsByteCodeNames[i] = "Unknown";
		}
		#define DECLARE_BYTECODE( name, val ) s_AsByteCodeNames[val] = #name;
				#include "AsByteCode_inc.h"
		#undef DECLARE_BYTECODE

		// fill up dispatch table with fallback processing
		// long instruction get a debug output, 
		// short instructions are processed as they need no decoding
		// fill up explore dispatch table with the short and long explores
		for (int i = 0; i < 256; ++i) {
			if( i < 0x80 ) {
				exploreTable[i] = &AsVM::exploreShortOpCode;
				functionTable[i] = &AsVM::fallbackShortOpCode;
			} else {
				exploreTable[i] = &AsVM::exploreOpCode;
				functionTable[i] = &AsVM::fallbackOpCode;
			}
		}

		// specially process explored jump, if, define function
		exploreTable[Swf::jump] = &AsVM::exploreJump;
		exploreTable[Swf::If] = &AsVM::exploreIf;
		exploreTable[Swf::defineFunction] = &AsVM::exploreDefineFunction;
			
		// only 'long' instructions need special handlers
		functionTable[Swf::gotoFrame] = &AsVM::actionGotoFrame;
		functionTable[Swf::setTarget] = &AsVM::actionSetTarget;
		functionTable[Swf::pushData] = &AsVM::actionPushData;
		functionTable[Swf::jump] = &AsVM::actionJump;
		functionTable[Swf::If] = &AsVM::actionIf;
		functionTable[Swf::constantPool] = &AsVM::actionConstantPool;
		functionTable[Swf::defineFunction] = &AsVM::actionDefineFunction;
						
		#define DECLARE_BYTECODE( name, val ) g_AsAgRuntimeFuncs[val] = &AsAgRuntime:: name;
				#include "AsByteCode_inc.h"
		#undef DECLARE_BYTECODE

		g_AsAgRuntime = CORE_GC_NEW_ROOT_ONLY AsAgRuntime();
	}
		
	AsVM::~AsVM() {
		CORE_GC_DELETE( g_AsAgRuntime );
		g_AsAgRuntime = NULL;
	}
		
	static std::string GetUniqueName( const FrameItem* _fi ) {
		if( _fi->name.empty() ) {
			std::ostringstream stream;
			stream << "ID" << _fi->id;
			return stream.str().c_str();
		} else {
			return _fi->name.c_str();
		}
	}
		
	std::string AsVM::readString( const uint8_t* _byteCode, int& pc ) const {
		int len = strlen( (char*) &_byteCode[pc] );
		int origPc = pc;
		pc += 1 + len;
		return std::string( (char*) &_byteCode[origPc] );
	}
		
	void AsVM::processByteCode( MovieClip* _movieClip, SwfActionByteCode* byteCode ) {

		AsFunction* func;
		if( functionCache.find( byteCode->byteCode ) == functionCache.end() ) {
		
			AsFunctionBuilder* funcBuild = CORE_GC_NEW AsFunctionBuilder( byteCode->lengthInBytes, byteCode->isCaseSensitive );
			funcBuild->translateByteCode( this, byteCode->byteCode );
			funcBuild->debugLogFunction();

			func = CORE_GC_NEW AsFunction( funcBuild );
			functionCache[ byteCode->byteCode ] = func;
			CORE_DELETE( funcBuild );
		} else {
			func = functionCache.find( byteCode->byteCode )->second;
		}

		g_AsAgRuntime->callGlobalCode( func, _movieClip );
	}		
		
		
	void AsVM::exploreShortOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		pc++;
	}

	void AsVM::exploreOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		int instruction = _byteCode[pc];
		if( (instruction & LONG_BYTECODE_MASK) == LONG_BYTECODE_MASK) {
			// length next u16
			uint16_t len = endianConverter.ToUInt16(_byteCode, pc+1);
			pc += 1 + 2 + len;
		} else {
			pc += 1;
		}
	}
		
	void AsVM::exploreIf( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		uint16_t len = endianConverter.ToInt16(_byteCode, pc+1);
		uint16_t offset = endianConverter.ToInt16(_byteCode, pc+3);
			
		pc += 1 + 2 + len;			
		int jumpTo = pc + offset;
		if( func->getLabel( jumpTo ).empty() ) {
			std::ostringstream stream;
			stream << "IfL" << jumpTo;
			func->addLabel( jumpTo, stream.str()  );
		}
	}
		
	void AsVM::exploreJump( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		uint16_t len = endianConverter.ToInt16(_byteCode, pc+1);
		uint16_t offset = endianConverter.ToInt16(_byteCode, pc+3);
			
		pc += 1 + 2 + len;			
		int jumpTo = pc + offset;
		if( func->getLabel( jumpTo ).empty() ) {
			std::ostringstream stream;
			stream << "JmpL" << jumpTo;
			func->addLabel( jumpTo, stream.str()  );
		}
	}

	void AsVM::exploreDefineFunction( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		// we need to skip over DefineFunction for explore, so as too 
		// not generate false jump targets

		// Slip Length
		pc += 1 + 2;

		readString(_byteCode, pc);
		uint16_t numArgs = endianConverter.ToUInt16( _byteCode, pc );
		pc += 2;
			
		for( int i = 0;i < numArgs;i++) {
			readString(_byteCode, pc);
		}
			
		uint16_t codeSize = endianConverter.ToUInt16( _byteCode, pc );
		pc += 2;
		pc += codeSize;
	}
		
	void AsVM::fallbackShortOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		int instruction = _byteCode[pc];
		// short instruction can be decoded directly
		func->addInstruction( pc, new AsFuncInstruction0Param( s_AsByteCodeNames[instruction], g_AsAgRuntimeFuncs[instruction] ) );
		pc += 1;
	}
		
	void AsVM::fallbackOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		int instruction = _byteCode[pc];
		// length next u16
		uint16_t len = endianConverter.ToUInt16(_byteCode, pc+1);
		pc += 1 + 2 + len;
	    LOG(INFO) 	<< s_AsByteCodeNames[instruction] << " ("
					<< instruction << ") - Length " << (int) len << "\n";
	}
		
	void AsVM::actionJump( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		uint16_t len = endianConverter.ToInt16( _byteCode, pc+1 );
		uint16_t offset = endianConverter.ToInt16( _byteCode, pc+3 );
		int origPc = pc;
		pc += 1 + 2 + len;

		std::ostringstream stream;
		stream << "goto " << func->getLabel( pc + offset ) << ";";
		func->addInstruction( origPc, 
			CORE_NEW AsFuncInstructionJump( stream.str(), pc + offset ) );

	}
		
	void AsVM::actionIf( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		uint16_t len = endianConverter.ToInt16( _byteCode, pc+1 );
		uint16_t offset = endianConverter.ToInt16( _byteCode, pc+3 );
		int origPc = pc;
		pc += 1 + 2 + len;

		std::ostringstream stream;
		stream << "if( AsPop()->ToBoolean() == true ) { "
					<< "goto " << func->getLabel( pc + offset ) << "; }";			
		func->addInstruction( origPc, 
				CORE_NEW AsFuncInstructionIf( stream.str(), pc + offset ) );
	}
		
    void AsVM::actionConstantPool( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		// Slip Length
		uint16_t count = endianConverter.ToUInt16( _byteCode, pc+3 );
		pc += 1 + 2 + 2;

		constantPool.resize( 0 );
			
		for (uint16_t i = 0; i < count; ++i) {
			std::string str = readString(_byteCode, pc);
			if( func->isCaseSensitive() == false ) {
				std::transform(str.begin(), str.end(), str.begin(), tolower);
			}
				
			constantPool.push_back( str );
		}
    }		

	void AsVM::actionPushData( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		uint16_t len = endianConverter.ToUInt16(_byteCode, pc+1);

		int origPc = pc;
		pc += 1 + 2;
		int startPc = pc;
		while (pc - startPc < len) {
			uint8_t type = _byteCode[pc++];
			switch (type) {
				case 0: // string literal
				{
					std::string str = readString(_byteCode, pc);
					if( func->isCaseSensitive() == false ) {
						std::transform(str.begin(), str.end(), str.begin(), tolower);
					}
						
					func->addInstruction( origPc, 
						CORE_NEW AsFuncInstruction1Param( "Push", &AsAgRuntime::push, CORE_NEW AsObjectString(str) ) );
					break;
				}
				case 1: // float literal
				{
					// C++ allows aliasing of byte types
					float f = *((float*)&_byteCode[pc]);
					func->addInstruction( origPc, 
						CORE_NEW AsFuncInstruction1Param( "Push", &AsAgRuntime::push, CORE_NEW AsObjectNumber(f) ) );
					pc += 4;
					break;
				}
				case 2: // null
				{
					func->addInstruction( origPc, 
						CORE_NEW AsFuncInstruction0Param( "PushNull", &AsAgRuntime::pushNull ) );
					break;
				}
				case 3: // undefined
				{
					func->addInstruction( origPc, 
						CORE_NEW AsFuncInstruction0Param( "PushUndefined", &AsAgRuntime::pushUndefined ) );
					break;
				}
				case 4: // register
				{
					assert( false );
					uint8_t reg = _byteCode[pc];
					std::ostringstream stream;
					stream << "Push( reg[" << reg << "] );";
//						func->AddInstruction( origPc,  stream.str().c_str() );
//						DO_PREVIEW( Push(reg[reg]) );
					pc += 1;
					break;
				}
				case 5: // Boolean
				{
					bool b = (_byteCode[pc] > 0);
					func->addInstruction( origPc, 
						CORE_NEW AsFuncInstruction1Param( "Push", &AsAgRuntime::push, CORE_NEW AsObjectBool(b) ) );
					pc += 1;
					break;
				}
				case 6: // double
				{
					// C++ allows aliasing of byte types
					double d = *((double*)&_byteCode[pc]);
					func->addInstruction( origPc, 
						CORE_NEW AsFuncInstruction1Param( "Push", &AsAgRuntime::push, CORE_NEW AsObjectNumber(d) ) );
					pc += 8;
					break;
				}
				case 7: // integer
				{
					int32_t i = endianConverter.ToInt32(_byteCode, pc);
					func->addInstruction( origPc, 
						CORE_NEW AsFuncInstruction1Param( "Push", &AsAgRuntime::push, CORE_NEW AsObjectNumber(i) ) );
					pc += 4;
					break;
				}
				case 8: // constant 8
				{
					func->addInstruction( origPc, 
						CORE_NEW AsFuncInstruction1Param( "Push", &AsAgRuntime::push, CORE_NEW AsObjectString(constantPool[(int) _byteCode[pc]]) ) );
					pc += 1;
					break;
				}
				case 9: // constant 16
				{
					func->addInstruction( origPc, 
						CORE_NEW AsFuncInstruction1Param( "Push", &AsAgRuntime::push, CORE_NEW AsObjectString(constantPool[endianConverter.ToUInt16(_byteCode, pc)]) ) );
					pc += 2;
					break;
				}
			}
		}
	}
		
    void AsVM::actionSetTarget( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		// Slip Length
		int origPc = pc;
		pc += 1 + 2;
		std::string targ = readString(_byteCode, pc);
		if( func->isCaseSensitive() == false ) {
			std::transform(targ.begin(), targ.end(), targ.begin(), tolower);
		}
			
		func->addInstruction( origPc, 
			CORE_NEW AsFuncInstruction1Param( "Push", &AsAgRuntime::push, CORE_NEW AsObjectString(targ) ) );
		func->addInstruction( origPc, 
			CORE_NEW AsFuncInstruction0Param( "SetTarget2", &AsAgRuntime::setTarget2 ) );
	}
		
	void AsVM::actionGotoFrame( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		// Slip Length
		uint16_t frame = endianConverter.ToInt16( _byteCode, pc+1+2 );
		int origPc = pc;
		pc += 1 + 2 + 2;

		func->addInstruction( origPc, 
			CORE_NEW AsFuncInstruction1Param( "GotoFrame", &AsAgRuntime::gotoFrame, CORE_NEW AsObjectNumber((float)frame) ) );

	}
	void AsVM::actionDefineFunction( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const {
		// Slip Length
		pc += 1 + 2;
		int origPc = pc;
		std::string name = readString(_byteCode, pc);
		uint16_t numArgs = endianConverter.ToUInt16( _byteCode, pc );
		pc += 2;
		std::string argNames[AsAgRuntime::MAX_ARGS];
		assert( numArgs < AsAgRuntime::MAX_ARGS );

		for( int i = 0;i < numArgs;i++) {
			argNames[i] = readString(_byteCode, pc);
		}

		uint16_t codeSize = endianConverter.ToUInt16( _byteCode, pc );
		pc += 2;

		AsFunction* newFunc;

		if( functionCache.find( &_byteCode[pc] ) == functionCache.end() ) {
			AsFunctionBuilder* funcBuild = CORE_GC_NEW AsFunctionBuilder( codeSize, func->isCaseSensitive() );
			funcBuild->translateByteCode( this, &_byteCode[pc] );
			funcBuild->debugLogFunction();

			newFunc = CORE_GC_NEW AsFunction( funcBuild );
			functionCache[ &_byteCode[pc] ] = newFunc;
			CORE_DELETE( funcBuild );
		} else {
			newFunc = functionCache.find( &_byteCode[pc] )->second;
		}
		if( name.empty() ) {
			func->addInstruction( origPc, 
				CORE_NEW AsFuncInstruction1Param( "DefineLocalFunction", &AsAgRuntime::defineLocalFunction, CORE_NEW AsObjectFunction(newFunc) ) );

		} else {
			func->addInstruction( origPc, 
				CORE_NEW AsFuncInstruction2Param( "DefineFunction", &AsAgRuntime::defineFunction, CORE_NEW AsObjectString(name), CORE_NEW AsObjectFunction(newFunc) ) );
		}

		pc += codeSize;
	}
} /* Swf */ 
