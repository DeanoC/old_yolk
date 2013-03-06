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
		friend class AsFunctionBuilder;
		AsVM ( const std::string& _name );
		virtual ~AsVM ();

		void processByteCode( MovieClip* _movieClip, SwfActionByteCode* byteCode );
			
	private:						
		std::string readString( const uint8_t* _byteCode, int& pc ) const;

		static const uint32_t LONG_BYTECODE_MASK = 0x80;
		static const EndianBitConverter& endianConverter;
			
		typedef void (AsVM::* ByteCodeDispatcher)(const uint8_t*, int&, AsFunctionBuilder*) const;
		ByteCodeDispatcher functionTable[256];		
		ByteCodeDispatcher exploreTable[256];

		void exploreShortOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void exploreOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void exploreIf( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void exploreJump( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void exploreDefineFunction( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;

		void fallbackShortOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void fallbackOpCode( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void actionJump( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void actionIf( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void actionConstantPool( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void actionPushData( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void actionSetTarget( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void actionGotoFrame( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;
		void actionDefineFunction( const uint8_t* _byteCode, int& pc, AsFunctionBuilder* func ) const;

		mutable std::vector<std::string> constantPool;
		typedef Core::gctraceablemap< const uint8_t*, AsFunction*> FunctionCache;
		mutable FunctionCache functionCache; 

		Swf::AsAgRuntime* g_AsAgRuntime;
	};
	
} /* Swf */ 


#endif /* end of include guard: ASVM_H_Z8J22E87 */

