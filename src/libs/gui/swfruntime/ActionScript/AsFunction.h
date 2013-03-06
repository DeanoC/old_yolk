/*
 *  AsFunction.h
 *  SwfPreview
 *
 *  Created by Deano on 20/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef ASFUNCTION_H_3AA74OHD
#define ASFUNCTION_H_3AA74OHD

#include "AsObject.h"
#include "AsFuncInstruction.h"

namespace Swf {
	class AsAgRuntime;
	class AsVM;
	
	class AsFuncBase : public Core::GcBase {
	public:
		virtual AsObjectHandle call( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) const = 0;
	};

	class AsFuncThisBase : public Core::GcBase {
	public:
		virtual AsObjectHandle call( AsAgRuntime* _runtime, AsObjectHandle _this, int _numParams, AsObjectHandle* _params ) const = 0;
	};

	class AsAgFunction : public AsFuncBase {
	public:
		typedef AsObjectHandle (AsAgRuntime::*AsFuncAsAgFunc)( int, AsObjectHandle* );
			
		AsAgFunction(AsFuncAsAgFunc _func ) :
			function(_func) {};
				
		virtual AsObjectHandle call( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) const {
				return (_runtime->*function)( _numParams, _params );
			}
				
	private:
		AsFuncAsAgFunc function;
	};
		
	template<class T>
	class AsObjFunction : public AsFuncThisBase {
	public:
		typedef AsObjectHandle (T::*AsFuncAsObjFunc)( AsAgRuntime*, int, AsObjectHandle* );
			
		AsObjFunction(AsFuncAsObjFunc _func ) :
			function( (BaseFunc)_func) {};
				
		virtual AsObjectHandle call( AsAgRuntime* _runtime, AsObjectHandle _this, int _numParams, AsObjectHandle* _params ) const {
			CORE_ASSERT( _this != NULL );
			return (_this->*function)( _runtime, _numParams, _params );
		}
				
	private:
		typedef AsObjectHandle (AsObject::*BaseFunc)( AsAgRuntime*, int, AsObjectHandle* );
		BaseFunc 					function;
	};
				
	class AsFunctionBuilder : public Core::GcBase {
	public:
		AsFunctionBuilder( const std::string& funcName, const int _maxAddressSpace, const bool _isCaseSensitive ) :
			name( funcName ),
			maxAddrSpace( _maxAddressSpace ),
			isCaseSens( _isCaseSensitive ) {}

		std::string getName() const { return name; }
		bool isCaseSensitive() const { return isCaseSens; }

		void addLabel( int pc, const std::string& label ) {
			labelMap[pc] = label.c_str();
		}

		void addInstruction( int _pc, AsFuncInstruction* _inst );

		void translateByteCode( const AsVM* _vm, const uint8_t* byteCode );

		const std::string getLabel( int pc ) const;
			
		void debugLogFunction() const;
	private:
		friend class AsFunction;
			
		typedef Core::gcvector<AsFuncInstruction*> FuncInstVec;
		typedef Core::gcmap<int, std::string> LabelMap;
		typedef Core::gcmap<int, FuncInstVec* > InstMap;
		LabelMap				labelMap;
		InstMap					instMap;
			
		const std::string 			name;
		const int					maxAddrSpace;
		const bool					isCaseSens;
	};
		
	class AsFunction : public AsFuncBase {
	public:
		AsFunction( const AsFunctionBuilder* _builder );

		std::string getName() const { return name; }
		bool isCaseSensitive() const { return isCaseSens; }
			
		virtual AsObjectHandle call( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) const;

	private:
		void computeLabelAddress( const AsFunctionBuilder* _builder, std::map<int, int>& labels );
			
		typedef Core::gcvector<AsFuncInstruction*> InstVec;

		InstVec							function;
		const std::string 				name;
		const bool						isCaseSens;
	};
} /* Swf */ 

#endif /* end of include guard: ASFUNCTION_H_3AA74OHD */
