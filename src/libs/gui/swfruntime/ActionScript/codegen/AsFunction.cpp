/*
 *  AsFunction.cpp
 *  SwfPreview
 *
 *  Created by Deano on 20/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "gui/swfruntime/swfruntime.h"
#include "AsFunction.h"
#include "../autogen/AsAgRuntime.h"
#include "core/debug.h"

namespace Swf {
	using namespace AutoGen;
	namespace CodeGen {
		
		const std::string AsFunctionBuilder::GetLabel( int _pc ) const {
			LabelMap::const_iterator it = labelMap.find(_pc);
			if( it != labelMap.end() ){
				return it->second;
			} else {
				static std::string empty =  std::string();
				return empty;
			}
		}

		void AsFunctionBuilder::AddInstruction( int _pc, AsFuncInstruction* _inst ) {
			if( instMap.find(_pc) == instMap.end() ){
				instMap[_pc] = new FuncInstVec;
			}
			FuncInstVec* it = instMap[_pc];
			it->push_back( _inst );
		}

		void AsFunctionBuilder::DebugLogFunction() const {
			
			LOG(INFO) << "\t\tvoid " << GetName() << "() {\n";
			
			int lineNo = 0;
			for( int pc = 0; pc < maxAddrSpace;++pc) {
				LabelMap::const_iterator lmit = labelMap.find(pc);
				if( lmit != labelMap.end() ) {
					LOG(INFO) << "\t\t" << lmit->second << ":;\n";
				}
				InstMap::const_iterator imit = instMap.find(pc);
				if( imit != instMap.end() ) {
					const FuncInstVec* it = imit->second;
					for( 	FuncInstVec::const_iterator i = it->begin(); 
							i != it->end();
							++i ) {
						LOG(INFO) << "/* " << lineNo << " */ ";
						(*i)->Print();
						lineNo++;
					}
				}
			}
			
			LOG(INFO) << "\t\t}\n";
		}
				
		void AsFunction::ComputeLabelAddress( const AsFunctionBuilder* _builder, std::map<int, int>& labels ) {
			
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
			name( _builder->GetName() ),
			isCaseSensitive( _builder->IsCaseSensitive() ) {

			std::map<int, int> labels;
			ComputeLabelAddress(_builder, labels );
				
			for( int pc = 0; pc < _builder->maxAddrSpace;++pc) {
				AsFunctionBuilder::InstMap::const_iterator imit = _builder->instMap.find(pc);
				if( imit != _builder->instMap.end() ) {
					const AsFunctionBuilder::FuncInstVec* it = imit->second;
					for( 	AsFunctionBuilder::FuncInstVec::const_iterator i = it->begin(); 
							i != it->end();
							++i ) {
						switch( (*i)->Type() ){
						case AsFuncInstruction::AFI_NORMAL:
							function.push_back(*i);
						break;
						case AsFuncInstruction::AFI_JUMP:
						{
							AsFuncInstructionJump* ji = (AsFuncInstructionJump*)(*i);
							ji->absJump = labels[ji->absJump] - function.size();
							function.push_back(*i);
						}
						break;
						case AsFuncInstruction::AFI_IF:
						{
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
		
		Swf::AutoGen::AsObjectHandle AsFunction::Call( Swf::AutoGen::AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) const {
			// TODO handle parameter passed to a defined function...?
			assert( _numParams == 0);
			InstVec::const_iterator it = function.begin();
			while( it != function.end() ) {
				int inc = (*it)->Call( _runtime );
				it += inc;
			}
			// strange rules, if no RET instruction, compiler has pushed undefined
			// which we have to discard?? TODO get this working correctly
			// doubt this correct...
			//AsObjectHandle ret = _this->AsPop();
			return AsObjectUndefined::Get();
		}
		
	} /* CodeGen */ 

	
} /* Swf */ 
