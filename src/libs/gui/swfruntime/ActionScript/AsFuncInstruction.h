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

#include "AsObject.h"

namespace Swf {
	class AsAgRuntime;
	
	class AsFuncInstruction : public Core::GcBase {
	public:
		AsFuncInstruction( const std::string& _inst ) :
			instruction( _inst ) {}
			
		enum AsFIType {
			AFI_NORMAL,
			AFI_JUMP,
			AFI_IF
		};
			
		virtual AsFIType type() const { return AFI_NORMAL; }

		virtual int call( Swf::AsAgRuntime* _this ) const = 0;
		virtual void print( std::ostringstream& strBuilder ) const = 0;
	protected:
		const std::string instruction;
	};
				
	class AsFuncInstruction0Param : public AsFuncInstruction {
	public:
		typedef void (Swf::AsAgRuntime::*Function)();
		AsFuncInstruction0Param( std::string _inst, Function _func ) :
			AsFuncInstruction( _inst ),
			function( _func ) {}
				
		virtual int call( Swf::AsAgRuntime* _this ) const override;
		virtual void print(  std::ostringstream& strBuilder ) const override;
	protected:
		const Function function;
	};

	class AsFuncInstruction1Param : public AsFuncInstruction {
	public:
		typedef void (Swf::AsAgRuntime::*Function)(Swf::AsObjectHandle const&);
		AsFuncInstruction1Param( std::string _inst, Function _func, Swf::AsObjectHandle _param0 ) :
			AsFuncInstruction( _inst ),
			function( _func ),
			param0( _param0 ) {}
				
		virtual int call( Swf::AsAgRuntime* _this ) const override;
		virtual void print(  std::ostringstream& strBuilder  ) const override;
	protected:
		const Function function;
		const Swf::AsObjectHandle param0;
	};
	class AsFuncInstruction2Param : public AsFuncInstruction {
	public:
		typedef void (Swf::AsAgRuntime::*Function)(Swf::AsObjectHandle const&, Swf::AsObjectHandle const&);
		AsFuncInstruction2Param( std::string _inst, Function _func, Swf::AsObjectHandle _param0, Swf::AsObjectHandle _param1 ) :
			AsFuncInstruction( _inst ),
			function( _func ),
			param0( _param0 ), 
			param1( _param1 ) {}
				
		virtual int call( Swf::AsAgRuntime* _this ) const override;
		virtual void print(  std::ostringstream& strBuilder ) const override;
	protected:
		const Function function;
		const Swf::AsObjectHandle param0;
		const Swf::AsObjectHandle param1;
	};
		
	class AsFuncInstructionJump : public AsFuncInstruction {
	public:
		typedef void (Swf::AsAgRuntime::*Function)(Swf::AsObjectHandle const&);
		AsFuncInstructionJump( std::string _inst, int _abs ) :
			AsFuncInstruction( _inst ),
			absJump( _abs ) {}
				
		virtual AsFIType type() const override { return AFI_JUMP; }
				
		virtual int call( Swf::AsAgRuntime* _this ) const override;
		virtual void print(  std::ostringstream& strBuilder ) const override;

		int absJump;
	};
	class AsFuncInstructionIf : public AsFuncInstruction {
	public:
		typedef void (Swf::AsAgRuntime::*Function)(Swf::AsObjectHandle const&);
		AsFuncInstructionIf( std::string _inst, int _abs ) :
			AsFuncInstruction( _inst ),
			absJump( _abs ) {}
				
		virtual AsFIType type() const override { return AFI_JUMP; }
				
		virtual int call( Swf::AsAgRuntime* _this ) const override;
		virtual void print(  std::ostringstream& strBuilder ) const override;

		int absJump;
	};
} // Swf


#endif /* end of include guard: ASFUNCINSTRUCTION_H_80ULRPAR */

