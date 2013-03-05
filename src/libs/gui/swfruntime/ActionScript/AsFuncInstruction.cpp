/*
 *  AsFuncInstruction.cpp
 *  SwfPreview
 *
 *  Created by Deano on 11/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "gui/swfruntime/swfruntime.h"
#include "core/debug.h"
#include "AsFunction.h"
#include "AsAgRuntime.h"
#include "AsFuncInstruction.h"

namespace Swf
{
	int AsFuncInstruction0Param::call( Swf::AsAgRuntime* _this ) const {
		(_this->*function)();
		return 1;
	}
	void AsFuncInstruction0Param::print(  std::ostringstream& strBuilder ) const {
		strBuilder << "\t\t\t" << instruction << "();\n";
	}

	int AsFuncInstruction1Param::call( Swf::AsAgRuntime* _this ) const {
		(_this->*function)(param0);
		return 1;
	}
	void AsFuncInstruction1Param::print(  std::ostringstream& strBuilder ) const {
		strBuilder << "\t\t\t" << instruction << "( ";
		if( param0->type() == APT_STRING ) {
			strBuilder << "\"" << param0->toString() << "\"";
		} else {
			strBuilder << param0->toString();
		}
		strBuilder <<" );\n";
	}
	int AsFuncInstruction2Param::call( Swf::AsAgRuntime* _this ) const {
		(_this->*function)(param0,param1);
		return 1;
	}
	void AsFuncInstruction2Param::print(  std::ostringstream& strBuilder ) const {
		strBuilder << "\t\t\t" << instruction << "( ";
		if( param0->type() == APT_STRING ) {
			strBuilder << "\"" << param0->toString() << "\"";
		} else {
			strBuilder << param0->toString();
		}
		strBuilder << ", ";
		if( param1->type() == APT_STRING ) {
			strBuilder << "\"" << param1->toString() << "\"";
		} else {
			strBuilder << param1->toString();
		}
		strBuilder <<" );\n";
	}
	int AsFuncInstructionJump::call( Swf::AsAgRuntime* _this ) const {
		return absJump;
	}
	void AsFuncInstructionJump::print(  std::ostringstream& strBuilder ) const {
		strBuilder << "\t\t\t" << instruction << "\n";
	}
	int AsFuncInstructionIf::call( Swf::AsAgRuntime* _this ) const {
		if(_this->asPop()->toBoolean() == true ) {
			return absJump;
		} else {
			return 1;
		}
	}
	void AsFuncInstructionIf::print(  std::ostringstream& strBuilder ) const {
		strBuilder << "\t\t\t" << instruction << "\n";
	}
}

