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
	
	class AsFuncBase : public Core::GcBase {
	public:
		virtual Swf::AsObjectHandle call( Swf::AsAgRuntime* _runtime, int _numParams, Swf::AsObjectHandle* _params ) const = 0;
	};

	class AsFuncThisBase : public Core::GcBase {
	public:
		virtual Swf::AsObjectHandle call( Swf::AsAgRuntime* _runtime, Swf::AsObjectHandle _this, int _numParams, Swf::AsObjectHandle* _params ) const = 0;
	};

	class AsAgFunction : public Swf::AsFuncBase {
	public:
		typedef Swf::AsObjectHandle (Swf::AsAgRuntime::*AsFuncAsAgFunc)( int, Swf::AsObjectHandle* );
			
		AsAgFunction(AsFuncAsAgFunc _func ) :
			function(_func) {};
				
		virtual Swf::AsObjectHandle call( Swf::AsAgRuntime* _runtime, int _numParams, Swf::AsObjectHandle* _params ) const {
				return (_runtime->*function)( _numParams, _params );
			}
				
	private:
		AsFuncAsAgFunc function;
	};
		
	template<class T>
	class AsObjFunction : public Swf::AsFuncThisBase {
	public:
		typedef Swf::AsObjectHandle (T::*AsFuncAsObjFunc)( Swf::AsAgRuntime*, int, Swf::AsObjectHandle* );
			
		AsObjFunction(AsFuncAsObjFunc _func ) :
			function( (BaseFunc)_func) {};
				
		virtual Swf::AsObjectHandle call( Swf::AsAgRuntime* _runtime, Swf::AsObjectHandle _this, int _numParams, Swf::AsObjectHandle* _params ) const {
			assert( _this != NULL );
			return (_this->*function)( _runtime, _numParams, _params );
		}
				
	private:
		typedef Swf::AsObjectHandle (Swf::AsObject::*BaseFunc)( Swf::AsAgRuntime*, int, Swf::AsObjectHandle* );
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
			
		virtual Swf::AsObjectHandle call( Swf::AsAgRuntime* _runtime, int _numParams, Swf::AsObjectHandle* _params ) const;

	private:
		void computeLabelAddress( const AsFunctionBuilder* _builder, std::map<int, int>& labels );
			
		typedef Core::gcvector<AsFuncInstruction*> InstVec;

		InstVec							function;
		const std::string 				name;
		const bool						isCaseSens;
	};
} /* Swf */ 

#endif /* end of include guard: ASFUNCTION_H_3AA74OHD */
