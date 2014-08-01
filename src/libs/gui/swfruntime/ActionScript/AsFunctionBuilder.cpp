/*
 *  AsFunctionBuilder.cpp
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
#include "AsFunctionBuilder.h"

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
		strBuilder << "\t\tvoid " << "BYTECODE" << "() {\n";

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
				
	void AsFunctionBuilder::computeLabelAddress( std::map<int, int>& labels )  const{
			
		int newAddr = 0;
		for( int pc = 0; pc < maxAddrSpace;++pc) {
			if( labelMap.find(pc) != labelMap.end() ) {
				labels[pc] = newAddr;
			}
				
			AsFunctionBuilder::InstMap::const_iterator imit = instMap.find(pc);
			if( imit != instMap.end() ) {
				const AsFunctionBuilder::FuncInstVec* it = imit->second;
				newAddr += (int)it->size();
			}
		}			
	}

	void AsFunctionBuilder::setFormalParameters( int _num, const std::string* _names ) {
		for( int i = 0; i < _num; ++i ) {
			formalParameters.push_back( _names[i] );
		}
	}

} /* Swf */ 
