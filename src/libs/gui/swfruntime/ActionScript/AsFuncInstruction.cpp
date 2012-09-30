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
	void AsFuncInstruction0Param::print() const {
		LOG(INFO) << "\t\t\t" << instruction << "();\n";
	}

	int AsFuncInstruction1Param::call( Swf::AsAgRuntime* _this ) const {
		(_this->*function)(param0);
		return 1;
	}
	void AsFuncInstruction1Param::print() const {
		LOG(INFO) << "\t\t\t" << instruction << "( ";
		if( param0->type() == APT_STRING ) {
			LOG(INFO) << "\"" << param0->toString() << "\"";
		} else {
			LOG(INFO) << param0->toString();
		}
		LOG(INFO) <<" );\n";
	}
	int AsFuncInstruction2Param::call( Swf::AsAgRuntime* _this ) const {
		(_this->*function)(param0,param1);
		return 1;
	}
	void AsFuncInstruction2Param::print() const {
		LOG(INFO) << "\t\t\t" << instruction << "( ";
		if( param0->type() == APT_STRING ) {
			LOG(INFO) << "\"" << param0->toString() << "\"";
		} else {
			LOG(INFO) << param0->toString();
		}
		LOG(INFO) << ", ";
		if( param1->type() == APT_STRING ) {
			LOG(INFO) << "\"" << param1->toString() << "\"";
		} else {
			LOG(INFO) << param1->toString();
		}
		LOG(INFO) <<" );\n";
	}
	int AsFuncInstructionJump::call( Swf::AsAgRuntime* _this ) const {
		return absJump;
	}
	void AsFuncInstructionJump::print() const {
		LOG(INFO) << "\t\t\t" << instruction << "\n";
	}
	int AsFuncInstructionIf::call( Swf::AsAgRuntime* _this ) const {
		if(_this->asPop()->toBoolean() == true ) {
			return absJump;
		} else {
			return 1;
		}
	}
	void AsFuncInstructionIf::print() const {
		LOG(INFO) << "\t\t\t" << instruction << "\n";
	}
}

