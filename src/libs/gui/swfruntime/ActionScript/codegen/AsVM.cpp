/*
 *  AsVM.cpp
 *  SwfPreview
 *
 *  Created by Deano on 19/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "gui/swfruntime/swfruntime.h"
#include "gui/SwfParser/EndianBitConverter.h"
#include "gui/SwfParser/SwfActionByteCode.h"
#include "gui/swfruntime/movieclip.h"
#include "AsFunction.h"
#include "../AsByteCode.h"
#include "../autogen/AsAgRuntime.h"
#include "../autogen/AsObjectFunction.h"
#include "AsVM.h"

#if defined(AS_PREVIEW)
bool g_doPreview = true;
typedef void (Swf::AutoGen::AsAgRuntime::*AsAgRuntimeFunc)();
AsAgRuntimeFunc g_AsAgRuntimeFuncs[256];
#define DO_PREVIEW( x ) if(g_doPreview){ g_AsAgRuntime-> x; }
#else
#define DO_PREVIEW( x )
#endif

namespace Swf {
	using namespace AutoGen;
	namespace CodeGen {
		const char* s_AsByteCodeNames[256];
		const EndianBitConverter& AsVM::endianConverter = EndianBitConverter::CreateForLittleEndian();
		
		AsVM::AsVM( const std::string& _name ) {
			// create human readibles names
			for (int i = 0; i < 256; ++i) {
				s_AsByteCodeNames[i] = "Unknown";
			}
			#define DECLARE_BYTECODE( name, val ) s_AsByteCodeNames[val] = #name;
					#include "../AsByteCode_inc.h"
			#undef DECLARE_BYTECODE

			// fill up dispatch table with fallback processing
			// long instruction get a debug output, 
			// short instructions are processed as they need no decoding
			// fill up explore dispatch table with the short and long explores
			for (int i = 0; i < 256; ++i) {
				if( i < 0x80 ) {
					exploreTable[i] = &AsVM::ExploreShortOpCode;
					functionTable[i] = &AsVM::FallbackShortOpCode;
				} else {
					exploreTable[i] = &AsVM::ExploreOpCode;
					functionTable[i] = &AsVM::FallbackOpCode;
				}
			}

			// specially process explored jump, if, define function
			exploreTable[Swf::Jump] = &AsVM::ExploreJump;
			exploreTable[Swf::If] = &AsVM::ExploreIf;
			exploreTable[Swf::DefineFunction] = &AsVM::ExploreDefineFunction;
			
			// only 'long' instructions need special handlers
			functionTable[Swf::GotoFrame] = &AsVM::ActionGotoFrame;
			functionTable[Swf::SetTarget] = &AsVM::ActionSetTarget;
			functionTable[Swf::PushData] = &AsVM::ActionPushData;
			functionTable[Swf::Jump] = &AsVM::ActionJump;
			functionTable[Swf::If] = &AsVM::ActionIf;
			functionTable[Swf::ConstantPool] = &AsVM::ActionConstantPool;
			functionTable[Swf::DefineFunction] = &AsVM::ActionDefineFunction;
						
//			LOG(INFO) << "// Swf ActionScript AutoGen Source File\n";
//			LOG(INFO) << "#include \"AsAgRuntime.h\"\n";
//			LOG(INFO) << "namespace Swf {\n";
//			LOG(INFO) << "namespace AutoGen {\n";
//			LOG(INFO) << "\tclass AsAg_" << _name << " : public AsAgRuntime {\n";
//			LOG(INFO) << "\tpublic:\n";
#if defined(AS_PREVIEW)
			#define DECLARE_BYTECODE( name, val ) g_AsAgRuntimeFuncs[val] = &AutoGen::AsAgRuntime:: name;
					#include "../AsByteCode_inc.h"
			#undef DECLARE_BYTECODE

			g_AsAgRuntime = CORE_GC_NEW_ROOT_ONLY AutoGen::AsAgRuntime();
			
#endif
		}
		
		AsVM::~AsVM() {
#if defined(AS_PREVIEW)
			CORE_GC_DELETE( g_AsAgRuntime );
			g_AsAgRuntime = NULL;
#endif
			
//			LOG(INFO) << "\t};\n";
			
//			LOG(INFO) << "} // end AutoGen\n} // end Swf\n";			
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
		
		std::string AsVM::ReadString( const uint8_t* _byteCode, int& pc )
		{
			int len = strlen( (char*) &_byteCode[pc] );
			int origPc = pc;
			pc += 1 + len;
			return std::string( (char*) &_byteCode[origPc] );
		}
		
		AsFunctionBuilder* AsVM::ProcessFunctionByteCode( const std::string& _name, const uint8_t* _byteCode, const int _programLength, const bool _isCaseSensitive ) {
			
			AsFunctionBuilder* func = CORE_GC_NEW_ROOT_ONLY AsFunctionBuilder( _name, _programLength, _isCaseSensitive );
			
			// explore program structure (branch destinations etc.)
			int pc = 0;
			while( pc < _programLength ) {
				int instruction = _byteCode[pc];
				(this->*exploreTable[instruction])( _byteCode, pc, func );
			}

			// explore finished so now translate bytecode
			pc = 0;
			while( pc < _programLength ) {
				int instruction = _byteCode[pc];
				(this->*functionTable[instruction])( _byteCode, pc, func );
			}
			
			return func;
		}
				
		void AsVM::ProcessByteCode( MovieClip* _movieClip, SwfActionByteCode* byteCode ) {
			
			std::ostringstream stream;
			stream << GetUniqueName(_movieClip) << "_" << _movieClip->getCurrentFrameNumber();
			
#if defined(AS_PREVIEW)
			g_AsAgRuntime->Reset();
			g_AsAgRuntime->SetRoot( _movieClip );
			g_AsAgRuntime->SetTarget( _movieClip );
#endif
			AsFunctionBuilder* funcBuild = ProcessFunctionByteCode( stream.str(), byteCode->byteCode, byteCode->lengthInBytes, byteCode->isCaseSensitive );

			funcBuild->DebugLogFunction();
			
			AsFunction* func = CORE_NEW AsFunction( funcBuild );
			func->Call( g_AsAgRuntime, 0, NULL );
			
#if defined(AS_PREVIEW)
			CORE_GC_DELETE( func );
			CORE_GC_DELETE( funcBuild );
#endif			
		}		
		
		
		void AsVM::ExploreShortOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			pc++;
		}

		void AsVM::ExploreOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			int instruction = _byteCode[pc];
			if( (instruction & LONG_BYTECODE_MASK) == LONG_BYTECODE_MASK) {
				// length next u16
				uint16_t len = endianConverter.ToUInt16(_byteCode, pc+1);
				pc += 1 + 2 + len;
			} else {
				pc += 1;
			}
		}
		
		void AsVM::ExploreIf( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			uint16_t len = endianConverter.ToInt16(_byteCode, pc+1);
			uint16_t offset = endianConverter.ToInt16(_byteCode, pc+3);
			
			pc += 1 + 2 + len;			
			int jumpTo = pc + offset;
			if( func->GetLabel( jumpTo ).empty() ) {
				std::ostringstream stream;
				stream << func->GetName() << "_" << "IfL" << jumpTo;
				func->AddLabel( jumpTo, stream.str()  );
			}
		}
		
		void AsVM::ExploreJump( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			uint16_t len = endianConverter.ToInt16(_byteCode, pc+1);
			uint16_t offset = endianConverter.ToInt16(_byteCode, pc+3);
			
			pc += 1 + 2 + len;			
			int jumpTo = pc + offset;
			if( func->GetLabel( jumpTo ).empty() ) {
				std::ostringstream stream;
				stream << func->GetName() << "_" << "JmpL" << jumpTo;
				func->AddLabel( jumpTo, stream.str()  );
			}
		}

		void AsVM::ExploreDefineFunction( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			// we need to skip over DefineFunction for explore, so as too 
			// not generate false jump targets

			// Slip Length
			pc += 1 + 2;

			ReadString(_byteCode, pc);
			uint16_t numArgs = endianConverter.ToUInt16( _byteCode, pc );
			pc += 2;
			
			for( int i = 0;i < numArgs;i++) {
				ReadString(_byteCode, pc);
			}
			
			uint16_t codeSize = endianConverter.ToUInt16( _byteCode, pc );
			pc += 2;
			pc += codeSize;
		}
		
		void AsVM::FallbackShortOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			int instruction = _byteCode[pc];
			// short instruction can be decoded directly
			func->AddInstruction( pc, new AsFuncInstruction0Param( s_AsByteCodeNames[instruction], g_AsAgRuntimeFuncs[instruction] ) );
			pc += 1;
		}
		
		void AsVM::FallbackOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			int instruction = _byteCode[pc];
			// length next u16
			uint16_t len = endianConverter.ToUInt16(_byteCode, pc+1);
			pc += 1 + 2 + len;
	       	LOG(INFO) 	<< s_AsByteCodeNames[instruction] << " ("
						<< instruction << ") - Length " << (int) len << "\n";
		}
		
		void AsVM::ActionJump( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			uint16_t len = endianConverter.ToInt16( _byteCode, pc+1 );
			uint16_t offset = endianConverter.ToInt16( _byteCode, pc+3 );
			int origPc = pc;
			pc += 1 + 2 + len;

			std::ostringstream stream;
			stream << "goto " << func->GetLabel( pc + offset ) << ";";
			func->AddInstruction( origPc, 
				new AsFuncInstructionJump( stream.str(), pc + offset ) );

		}
		
		void AsVM::ActionIf( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			uint16_t len = endianConverter.ToInt16( _byteCode, pc+1 );
			uint16_t offset = endianConverter.ToInt16( _byteCode, pc+3 );
			int origPc = pc;
			pc += 1 + 2 + len;

			std::ostringstream stream;
			stream << "if( AsPop()->ToBoolean() == true ) { "
					 << "goto " << func->GetLabel( pc + offset ) << "; }";			
			func->AddInstruction( origPc, 
					new AsFuncInstructionIf( stream.str(), pc + offset ) );
		}
		
      void AsVM::ActionConstantPool( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			// Slip Length
			uint16_t count = endianConverter.ToUInt16( _byteCode, pc+3 );
			pc += 1 + 2 + 2;
			
			for (uint16_t i = 0; i < count; ++i) {
				std::string str = ReadString(_byteCode, pc);
				if( func->IsCaseSensitive() == false ) {
					std::transform(str.begin(), str.end(), str.begin(), tolower);
				}
				
				assert( i < 100 );
				constantPool[i] = str;
			}
      }		

		void AsVM::ActionPushData( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			uint16_t len = endianConverter.ToUInt16(_byteCode, pc+1);

			int origPc = pc;
			pc += 1 + 2;
			int startPc = pc;
			while (pc - startPc < len) {
				uint8_t type = _byteCode[pc++];
				switch (type) {
					case 0: // string literal
					{
						std::string str = ReadString(_byteCode, pc);
						if( func->IsCaseSensitive() == false ) {
							std::transform(str.begin(), str.end(), str.begin(), tolower);
						}
						
						func->AddInstruction( origPc, 
							new AsFuncInstruction1Param( "Push", &AsAgRuntime::Push, new AsObjectString(str) ) );
						break;
					}
					case 1: // float literal
					{
						// C++ allows aliasing of byte types
						float f = *((float*)&_byteCode[pc]);
						func->AddInstruction( origPc, 
							new AsFuncInstruction1Param( "Push", &AutoGen::AsAgRuntime::Push, new AsObjectNumber(f) ) );
						pc += 4;
						break;
					}
					case 2: // null
					{
						func->AddInstruction( origPc, 
							new AsFuncInstruction0Param( "PushNull", &AutoGen::AsAgRuntime::PushNull ) );
						break;
					}
					case 3: // undefined
					{
						func->AddInstruction( origPc, 
							new AsFuncInstruction0Param( "PushUndefined", &AutoGen::AsAgRuntime::PushUndefined ) );
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
						func->AddInstruction( origPc, 
							new AsFuncInstruction1Param( "Push", &AutoGen::AsAgRuntime::Push, new AsObjectBool(b) ) );
						pc += 1;
						break;
					}
					case 6: // double
					{
						// C++ allows aliasing of byte types
						double d = *((double*)&_byteCode[pc]);
						func->AddInstruction( origPc, 
							new AsFuncInstruction1Param( "Push", &AutoGen::AsAgRuntime::Push, new AsObjectNumber(d) ) );
						pc += 8;
						break;
					}
					case 7: // integer
					{
						int32_t i = endianConverter.ToInt32(_byteCode, pc);
						func->AddInstruction( origPc, 
							new AsFuncInstruction1Param( "Push", &AutoGen::AsAgRuntime::Push, new AsObjectNumber(i) ) );
						pc += 4;
						break;
					}
					case 8: // constant 8
					{
						func->AddInstruction( origPc, 
							new AsFuncInstruction1Param( "Push", &AutoGen::AsAgRuntime::Push, new AsObjectString(constantPool[(int) _byteCode[pc]]) ) );
						pc += 1;
						break;
					}
					case 9: // constant 16
					{
						func->AddInstruction( origPc, 
							new AsFuncInstruction1Param( "Push", &AutoGen::AsAgRuntime::Push, new AsObjectString(constantPool[endianConverter.ToUInt16(_byteCode, pc)]) ) );
						pc += 2;
						break;
					}
				}
			}
		}
		
      void AsVM::ActionSetTarget( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			// Slip Length
			int origPc = pc;
			pc += 1 + 2;
			std::string targ = ReadString(_byteCode, pc);
			if( func->IsCaseSensitive() == false ) {
				std::transform(targ.begin(), targ.end(), targ.begin(), tolower);
			}
			
			func->AddInstruction( origPc, 
				new AsFuncInstruction1Param( "Push", &AsAgRuntime::Push, new AsObjectString(targ) ) );
			func->AddInstruction( origPc, 
				new AsFuncInstruction0Param( "SetTarget2", &AsAgRuntime::SetTarget2 ) );
		}
		
		void AsVM::ActionGotoFrame( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			// Slip Length
			uint16_t frame = endianConverter.ToInt16( _byteCode, pc+1+2 );
			int origPc = pc;
			pc += 1 + 2 + 2;

			func->AddInstruction( origPc, 
				new AsFuncInstruction1Param( "GotoFrame", &AutoGen::AsAgRuntime::GotoFrame, new AsObjectNumber((float)frame) ) );

		}
		void AsVM::ActionDefineFunction( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) {
			// Slip Length
			pc += 1 + 2;
			int origPc = pc;
			std::string name = ReadString(_byteCode, pc);
			uint16_t numArgs = endianConverter.ToUInt16( _byteCode, pc );
			pc += 2;
			std::string argNames[16];
			assert( numArgs < 16 );

			for( int i = 0;i < numArgs;i++) {
				argNames[i] = ReadString(_byteCode, pc);
			}

			uint16_t codeSize = endianConverter.ToUInt16( _byteCode, pc );
			pc += 2;

			AsFunctionBuilder* newFuncBuild = ProcessFunctionByteCode( name, &_byteCode[pc], codeSize, func->IsCaseSensitive() );
			newFuncBuild->DebugLogFunction();
			AsFunction* newFunc = CORE_NEW AsFunction( newFuncBuild );

			if( name.empty() ) {
				func->AddInstruction( origPc, 
					CORE_NEW AsFuncInstruction1Param( "DefineLocalFunction", &AutoGen::AsAgRuntime::DefineLocalFunction, CORE_NEW AsObjectFunction(newFunc) ) );
				
			} else {
				func->AddInstruction( origPc, 
					CORE_NEW AsFuncInstruction2Param( "DefineFunction", &AutoGen::AsAgRuntime::DefineFunction, CORE_NEW AsObjectString(name), CORE_NEW AsObjectFunction(newFunc) ) );
			}
			
			pc += codeSize;
		}

	} /* CodeGen */ 
} /* Swf */ 