/*
 *  AsFunction.cpp
 *  SwfPreview
 *
 *  Created by Deano on 20/07/2009.
 *  Copyright 2013 CloudPixies Ltd. All rights reserved.
 *
 */
#include "gui/swfruntime/swfruntime.h"
#include "gui/swfparser/SwfActionByteCode.h"
#include "AsAgRuntime.h"
#include "AsVM.h"
#include "core/debug.h"
#include "AsFunctionBuilder.h"
#include "AsFunction.h"

namespace Swf {
	AsFunction::AsFunction( const AsFunctionBuilder* _builder ) :
		returnCalled( false )
	{

		std::map<int, int> labels;
		_builder->computeLabelAddress( labels );
				
		for( int pc = 0; pc < _builder->maxAddrSpace;++pc) {
			AsFunctionBuilder::InstMap::const_iterator imit = _builder->instMap.find(pc);
			if( imit != _builder->instMap.end() ) {
				const AsFunctionBuilder::FuncInstVec* it = imit->second;
				for( 	AsFunctionBuilder::FuncInstVec::const_iterator i = it->begin(); 
						i != it->end();
						++i ) {
					switch( (*i)->type() ){
					case AsFuncInstruction::AFI_NORMAL:
						function.push_back(*i);
					break;
					case AsFuncInstruction::AFI_JUMP: {
						AsFuncInstructionJump* ji = (AsFuncInstructionJump*)(*i);
						ji->absJump = labels[ji->absJump] - (int)function.size();
						function.push_back(*i);
					}
					break;
					case AsFuncInstruction::AFI_IF: {
						AsFuncInstructionIf* ji = (AsFuncInstructionIf*)(*i);
						ji->absJump = labels[ji->absJump] - (int)function.size();
						function.push_back(*i);
					}
					break;
					};
				}
			}
		}
	}

	Swf::AsObjectHandle AsFunction::call( Swf::AsAgRuntime* _runtime, AsObjectHandle _this, int _numParams, AsObjectHandle* _params ) const {

		returnCalled = false;

		for( int i = 0; i < _numParams; ++i ) {
			_runtime->push( _params[i] );
		}

		InstVec::const_iterator it = function.begin();
		while( it != function.end() && returnCalled == false) {
			int inc = (*it)->call( _runtime );
			it += inc;
		}
		// strange rules, if no RET instruction, compiler has pushed undefined
		// which we have to discard?? TODO get this working correctly
		// doubt this correct...
		AsObjectHandle ret = _runtime->asPop();
		return ret;
	}

	void AsFunction::asReturn() const {
		returnCalled = true;
	}

} /* Swf */ 
