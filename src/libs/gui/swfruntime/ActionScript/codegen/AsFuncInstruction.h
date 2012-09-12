/*
 *  AsFuncInstruction.h
 *  SwfPreview
 *
 *  Created by Deano on 11/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef ASFUNCINSTRUCTION_H_80ULRPAR
#define ASFUNCINSTRUCTION_H_80ULRPAR

#include "../autogen/AsObject.h"

namespace Swf {
	namespace AutoGen { class AsAgRuntime; };
	
	namespace CodeGen {
		class AsFuncInstruction : public Core::GcBase {
		public:
			AsFuncInstruction( const std::string& _inst ) :
				instruction( _inst ) {}
			
			enum AsFIType {
				AFI_NORMAL,
				AFI_JUMP,
				AFI_IF
			};
			
			virtual AsFIType Type() { return AFI_NORMAL; }

			virtual int Call( Swf::AutoGen::AsAgRuntime* _this ) const = 0;
			virtual void Print() const = 0;
		protected:
			std::string instruction;
		};
				
		class AsFuncInstruction0Param : public AsFuncInstruction {
		public:
			typedef void (Swf::AutoGen::AsAgRuntime::*Function)();
			AsFuncInstruction0Param( std::string _inst, Function _func ) :
				AsFuncInstruction( _inst ),
				function( _func ) {}
				
			virtual int Call( Swf::AutoGen::AsAgRuntime* _this ) const;
			virtual void Print() const;
		protected:
			Function function;
		};

		class AsFuncInstruction1Param : public AsFuncInstruction {
		public:
			typedef void (Swf::AutoGen::AsAgRuntime::*Function)(Swf::AutoGen::AsObjectHandle const&);
			AsFuncInstruction1Param( std::string _inst, Function _func, Swf::AutoGen::AsObjectHandle _param0 ) :
				AsFuncInstruction( _inst ),
				function( _func ),
				param0( _param0 ) {}
				
			virtual int Call( Swf::AutoGen::AsAgRuntime* _this ) const;
			virtual void Print() const;
		protected:
			Function function;
			Swf::AutoGen::AsObjectHandle param0;
		};
		class AsFuncInstruction2Param : public AsFuncInstruction {
		public:
			typedef void (Swf::AutoGen::AsAgRuntime::*Function)(Swf::AutoGen::AsObjectHandle const&, Swf::AutoGen::AsObjectHandle const&);
			AsFuncInstruction2Param( std::string _inst, Function _func, Swf::AutoGen::AsObjectHandle _param0, Swf::AutoGen::AsObjectHandle _param1 ) :
				AsFuncInstruction( _inst ),
				function( _func ),
				param0( _param0 ), 
				param1( _param1 ) {}
				
			virtual int Call( Swf::AutoGen::AsAgRuntime* _this ) const;
			virtual void Print() const;
		protected:
			Function function;
			Swf::AutoGen::AsObjectHandle param0;
			Swf::AutoGen::AsObjectHandle param1;
		};
		
		class AsFuncInstructionJump : public AsFuncInstruction {
		public:
			typedef void (Swf::AutoGen::AsAgRuntime::*Function)(Swf::AutoGen::AsObjectHandle const&);
			AsFuncInstructionJump( std::string _inst, int _abs ) :
				AsFuncInstruction( _inst ),
				absJump( _abs ) {}
				
			virtual AsFIType Type() { return AFI_JUMP; }
				
			virtual int Call( Swf::AutoGen::AsAgRuntime* _this ) const;
			virtual void Print() const;

			int absJump;
		};
		class AsFuncInstructionIf : public AsFuncInstruction {
		public:
			typedef void (Swf::AutoGen::AsAgRuntime::*Function)(Swf::AutoGen::AsObjectHandle const&);
			AsFuncInstructionIf( std::string _inst, int _abs ) :
				AsFuncInstruction( _inst ),
				absJump( _abs ) {}
				
			virtual AsFIType Type() { return AFI_JUMP; }
				
			virtual int Call( Swf::AutoGen::AsAgRuntime* _this ) const;
			virtual void Print() const;

			int absJump;
		};

	} // CodeGen
} // Swf


#endif /* end of include guard: ASFUNCINSTRUCTION_H_80ULRPAR */

