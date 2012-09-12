/*
 *  AsFuncInstruction.cpp
 *  SwfPreview
 *
 *  Created by Deano on 11/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "gui/swfruntime/swfruntime.h"
#include "AsFunction.h"
#include "../autogen/AsAgRuntime.h"
#include "core/debug.h"
#include "AsFuncInstruction.h"

namespace Swf
{
	using namespace AutoGen;
	namespace CodeGen
	{

		int AsFuncInstruction0Param::Call( Swf::AutoGen::AsAgRuntime* _this ) const {
			(_this->*function)();
			return 1;
		}
		void AsFuncInstruction0Param::Print() const {
			LOG(INFO) << "\t\t\t" << instruction << "();\n";
		}

		int AsFuncInstruction1Param::Call( Swf::AutoGen::AsAgRuntime* _this ) const {
			(_this->*function)(param0);
			return 1;
		}
		void AsFuncInstruction1Param::Print() const {
			LOG(INFO) << "\t\t\t" << instruction << "( ";
			if( param0->Type() == APT_STRING ) {
				LOG(INFO) << "\"" << param0->ToString() << "\"";
			} else {
				LOG(INFO) << param0->ToString();
			}
			LOG(INFO) <<" );\n";
		}
		int AsFuncInstruction2Param::Call( Swf::AutoGen::AsAgRuntime* _this ) const {
			(_this->*function)(param0,param1);
			return 1;
		}
		void AsFuncInstruction2Param::Print() const {
			LOG(INFO) << "\t\t\t" << instruction << "( ";
			if( param0->Type() == APT_STRING ) {
				LOG(INFO) << "\"" << param0->ToString() << "\"";
			} else {
				LOG(INFO) << param0->ToString();
			}
			LOG(INFO) << ", ";
			if( param1->Type() == APT_STRING ) {
				LOG(INFO) << "\"" << param1->ToString() << "\"";
			} else {
				LOG(INFO) << param1->ToString();
			}
			LOG(INFO) <<" );\n";
		}
		int AsFuncInstructionJump::Call( Swf::AutoGen::AsAgRuntime* _this ) const {
			return absJump;
		}
		void AsFuncInstructionJump::Print() const {
			LOG(INFO) << "\t\t\t" << instruction << "\n";
		}
		int AsFuncInstructionIf::Call( Swf::AutoGen::AsAgRuntime* _this ) const {
			if(_this->AsPop()->ToBoolean() == true ) {
				return absJump;
			} else {
				return 1;
			}
		}
		void AsFuncInstructionIf::Print() const {
			LOG(INFO) << "\t\t\t" << instruction << "\n";
		}
	}
}

