/*
 *  AsVM.h
 *  SwfPreview
 *
 *  Created by Deano on 19/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef ASVM_H_Z8J22E87
#define ASVM_H_Z8J22E87

#define AS_PREVIEW

namespace Swf
{
	// forward decl
	class SwfActionByteCode;
	class SwfMovieClip;
	class SwfFrame;
	class EndianBitConverter;
	namespace AutoGen { class AsAgRuntime; }

	namespace CodeGen {
		class AsFunctionBuilder;
		class AsFunction;
		
		class AsVM : public Core::GcBase {
		public:
			AsVM ( const std::string& _name );
			virtual ~AsVM ();

			void ProcessByteCode( SwfMovieClip* _movieClip, SwfActionByteCode* byteCode );
			
		private:						
			AsFunctionBuilder* ProcessFunctionByteCode( const std::string& _name, 
														const uint8_t* _byteCode, 
														const int _programLength, 
														const bool _isCaseSensitive );

			static const uint32_t LONG_BYTECODE_MASK = 0x80;
			static const EndianBitConverter& endianConverter;
			
			typedef void (AsVM::* ByteCodeDispatcher)(const uint8_t*, int&, AsFunctionBuilder*);		
			ByteCodeDispatcher functionTable[256];		
			ByteCodeDispatcher exploreTable[256];

			void ExploreShortOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ExploreOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ExploreIf( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ExploreJump( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ExploreDefineFunction( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );

			std::string ReadString( const uint8_t* _byteCode, int& pc );

			void FallbackShortOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void FallbackOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ActionJump( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ActionIf( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ActionConstantPool( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ActionPushData( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ActionSetTarget( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ActionGotoFrame( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
			void ActionDefineFunction( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );

#if defined(AS_PREVIEW)
			std::string constantPool[ 100 ];
			Swf::AutoGen::AsAgRuntime* g_AsAgRuntime;
#endif

		};
	} /* CodeGen */ 
	
} /* Swf */ 


#endif /* end of include guard: ASVM_H_Z8J22E87 */

