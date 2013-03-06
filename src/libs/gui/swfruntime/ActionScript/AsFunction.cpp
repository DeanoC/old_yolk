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
#include "AsFunction.h"

namespace Swf {

	const std::string AsFunctionBuilder::getLabel( int _pc ) const {
		LabelMap::const_iterator it = labelMap.find(_pc);
		if( it != labelMap.end() ){
			return it->second;
		} else {
			static std::string empty =  std::string();
			return empty;
		}
	}

	void AsFunctionBuilder::addInstruction( int _pc, AsFuncInstruction* _inst ) {
		if( instMap.find(_pc) == instMap.end() ){
			instMap[_pc] = CORE_NEW FuncInstVec;
		}
		FuncInstVec* it = instMap[_pc];
		it->push_back( _inst );
	}

	void AsFunctionBuilder::debugLogFunction() const {
		std::ostringstream strBuilder;
		strBuilder << "\t\tvoid " << getName() << "() {\n";

		int lineNo = 0;
		for( int pc = 0; pc < maxAddrSpace;++pc) {
			LabelMap::const_iterator lmit = labelMap.find(pc);
			if( lmit != labelMap.end() ) {
				strBuilder << "\t\t" << lmit->second << ":;\n";
			}
			InstMap::const_iterator imit = instMap.find(pc);
			if( imit != instMap.end() ) {
				const FuncInstVec* it = imit->second;
				for( 	FuncInstVec::const_iterator i = it->begin(); 
						i != it->end();
						++i ) {
					strBuilder << "/* " << lineNo << " */ ";
					(*i)->print( strBuilder );
					lineNo++;
				}
			}
		}
			
		strBuilder << "\t\t}\n";
		LOG(INFO) << strBuilder.str();
	}

	void AsFunctionBuilder::translateByteCode( const AsVM* _vm, const uint8_t* byteCode ) {
		// explore program structure (branch destinations etc.)
		int pc = 0;
		while( pc < maxAddrSpace ) {
			int instruction = byteCode[pc];
			(_vm->*(_vm->exploreTable[instruction]))( byteCode, pc, this );
		}

		// explore finished so now translate bytecode
		pc = 0;
		while( pc < maxAddrSpace ) {
			int instruction = byteCode[pc];
			(_vm->*(_vm->functionTable[instruction]))( byteCode, pc, this );
		}
	}
				
	void AsFunction::computeLabelAddress( const AsFunctionBuilder* _builder, std::map<int, int>& labels ) {
			
		int newAddr = 0;
		for( int pc = 0; pc < _builder->maxAddrSpace;++pc) {
			if( _builder->labelMap.find(pc) != _builder->labelMap.end() ) {
				labels[pc] = newAddr;
			}
				
			AsFunctionBuilder::InstMap::const_iterator imit = _builder->instMap.find(pc);
			if( imit != _builder->instMap.end() ) {
				const AsFunctionBuilder::FuncInstVec* it = imit->second;
				newAddr += it->size();
			}
		}			
	}
	AsFunction::AsFunction( const AsFunctionBuilder* _builder ) :
		name( _builder->getName() ),
		isCaseSens( _builder->isCaseSensitive() ) {

		std::map<int, int> labels;
		computeLabelAddress(_builder, labels );
				
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
						ji->absJump = labels[ji->absJump] - function.size();
						function.push_back(*i);
					}
					break;
					case AsFuncInstruction::AFI_IF: {
						AsFuncInstructionIf* ji = (AsFuncInstructionIf*)(*i);
						ji->absJump = labels[ji->absJump] - function.size();
						function.push_back(*i);
					}
					break;
					};
				}
			}
		}
	}

	Swf::AsObjectHandle AsFunction::call( Swf::AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) const {
		// TODO handle parameter passed to a defined function...?
		assert( _numParams == 0);
		InstVec::const_iterator it = function.begin();
		while( it != function.end() ) {
			int inc = (*it)->call( _runtime );
			it += inc;
		}
		// strange rules, if no RET instruction, compiler has pushed undefined
		// which we have to discard?? TODO get this working correctly
		// doubt this correct...
		//AsObjectHandle ret = _this->AsPop();
		return AsObjectUndefined::get();
	}
} /* Swf */ 
