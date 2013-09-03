/*
 *  AsFunctionBuilder.h
 *  SwfPreview
 *
 *  Copyright 2013 Cloud Pixies Ltd. All rights reserved.
 *
 */
#ifndef YOLK_ASFUNCTIONBUILDER_H_
#define YOLK_ASFUNCTIONBUILDER_H_

#include "AsObject.h"
#include "AsFuncInstruction.h"

namespace Swf {
	class AsAgRuntime;
	class AsVM;
	
	class AsFunctionBuilder : public Core::GcBase {
	public:
		typedef Core::gcvector<std::string>	FormalParameters;

		AsFunctionBuilder( const int _maxAddressSpace, const bool _isCaseSensitive ) :
			maxAddrSpace( _maxAddressSpace ),
			isCaseSens( _isCaseSensitive ) {}

		bool isCaseSensitive() const { return isCaseSens; }

		void setFormalParameters( int _num, const std::string* _names );
		const FormalParameters& getFormalParameters() const { return formalParameters; }


		void addLabel( int pc, const std::string& label ) {
			labelMap[pc] = label.c_str();
		}

		void addInstruction( int _pc, AsFuncInstruction* _inst );

		void translateByteCode( const AsVM* _vm, const uint8_t* byteCode );

		const std::string getLabel( int pc ) const;
			
		void debugLogFunction() const;
	private:
		friend class AsFunction;
		void computeLabelAddress( std::map<int, int>& labels ) const;
			
		typedef Core::gcvector<AsFuncInstruction*> FuncInstVec;
		typedef Core::gcmap<int, std::string> LabelMap;
		typedef Core::gcmap<int, FuncInstVec* > InstMap;

		LabelMap				labelMap;
		InstMap					instMap;
		FormalParameters		formalParameters;
			
		const int					maxAddrSpace;
		const bool					isCaseSens;
	};
} /* Swf */ 

#endif /* end of include guard: ASFUNCTION_H_3AA74OHD */
