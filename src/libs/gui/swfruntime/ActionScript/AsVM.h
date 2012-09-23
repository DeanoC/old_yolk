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
	class SwfFrame;
	class MovieClip;
	class EndianBitConverter;
	class AsAgRuntime;

	class AsFunctionBuilder;
	class AsFunction;
		
	class AsVM : public Core::GcBase {
	public:
		AsVM ( const std::string& _name );
		virtual ~AsVM ();

		void processByteCode( MovieClip* _movieClip, SwfActionByteCode* byteCode );
			
	private:						
		AsFunctionBuilder* processFunctionByteCode( const std::string& _name, 
													const uint8_t* _byteCode, 
													const int _programLength, 
													const bool _isCaseSensitive );

		static const uint32_t LONG_BYTECODE_MASK = 0x80;
		static const EndianBitConverter& endianConverter;
			
		typedef void (AsVM::* ByteCodeDispatcher)(const uint8_t*, int&, AsFunctionBuilder*);		
		ByteCodeDispatcher functionTable[256];		
		ByteCodeDispatcher exploreTable[256];

		void exploreShortOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void exploreOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void exploreIf( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void exploreJump( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void exploreDefineFunction( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );

		std::string readString( const uint8_t* _byteCode, int& pc );

		void fallbackShortOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void fallbackOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void actionJump( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void actionIf( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void actionConstantPool( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void actionPushData( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void actionSetTarget( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void actionGotoFrame( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );
		void actionDefineFunction( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func );

		std::string constantPool[ 100 ];
		Swf::AsAgRuntime* g_AsAgRuntime;
	};
	
} /* Swf */ 


#endif /* end of include guard: ASVM_H_Z8J22E87 */

