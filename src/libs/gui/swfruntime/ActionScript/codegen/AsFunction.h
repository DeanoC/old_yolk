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

#include "../autogen/AsObject.h"
#include "AsFuncInstruction.h"

namespace Swf {
	namespace AutoGen { class AsAgRuntime; };
	
	namespace CodeGen {
		class AsFuncBase : public Core::GcBase {
		public:
			virtual Swf::AutoGen::AsObjectHandle Call( Swf::AutoGen::AsAgRuntime* _runtime, int _numParams, Swf::AutoGen::AsObjectHandle* _params ) const = 0;
		};

		class AsFuncThisBase : public Core::GcBase {
		public:
			virtual Swf::AutoGen::AsObjectHandle Call( Swf::AutoGen::AsAgRuntime* _runtime, Swf::AutoGen::AsObjectHandle _this, int _numParams, Swf::AutoGen::AsObjectHandle* _params ) const = 0;
		};

		class AsAgFunction : public Swf::CodeGen::AsFuncBase {
		public:
			typedef Swf::AutoGen::AsObjectHandle (Swf::AutoGen::AsAgRuntime::*AsFuncAsAgFunc)( int, Swf::AutoGen::AsObjectHandle* );
			
			AsAgFunction(AsFuncAsAgFunc _func ) :
				function(_func) {};
				
			virtual Swf::AutoGen::AsObjectHandle Call( Swf::AutoGen::AsAgRuntime* _runtime, int _numParams, Swf::AutoGen::AsObjectHandle* _params ) const {
					return (_runtime->*function)( _numParams, _params );
				}
				
		private:
			AsFuncAsAgFunc function;
		};
		
		template<class T>
		class AsObjFunction : public Swf::CodeGen::AsFuncThisBase {
		public:
			typedef Swf::AutoGen::AsObjectHandle (T::*AsFuncAsObjFunc)( Swf::AutoGen::AsAgRuntime*, int, Swf::AutoGen::AsObjectHandle* );
			
			AsObjFunction(AsFuncAsObjFunc _func ) :
				function( (BaseFunc)_func) {};
				
			virtual Swf::AutoGen::AsObjectHandle Call( Swf::AutoGen::AsAgRuntime* _runtime, Swf::AutoGen::AsObjectHandle _this, int _numParams, Swf::AutoGen::AsObjectHandle* _params ) const {
				assert( _this != NULL );
				return (_this->*function)( _runtime, _numParams, _params );
			}
				
		private:
			typedef Swf::AutoGen::AsObjectHandle (Swf::AutoGen::AsObject::*BaseFunc)( Swf::AutoGen::AsAgRuntime*, int, Swf::AutoGen::AsObjectHandle* );
			BaseFunc 					function;
		};
				
		class AsFunctionBuilder : public Core::GcBase {
		public:
			AsFunctionBuilder( const std::string& funcName, const int _maxAddressSpace, const bool _isCaseSensitive ) :
				name( funcName ),
				maxAddrSpace( _maxAddressSpace ),
				isCaseSensitive( _isCaseSensitive ) {}

			std::string GetName() const { return name; }
			bool IsCaseSensitive() const { return isCaseSensitive; }

			void AddLabel( int pc, const std::string& label ) {
				labelMap[pc] = label.c_str();
			}

			void AddInstruction( int _pc, AsFuncInstruction* _inst );

			const std::string GetLabel( int pc ) const;
			
			void DebugLogFunction() const;
			
		private:
			friend class AsFunction;
			
			typedef Core::gcvector<AsFuncInstruction*> FuncInstVec;
			typedef Core::gcmap<int, std::string> LabelMap;
			typedef Core::gcmap<int, FuncInstVec* > InstMap;
			LabelMap				labelMap;
			InstMap					instMap;
			
			std::string 			name;
			int						maxAddrSpace;
			bool					isCaseSensitive;
		};
		
		class AsFunction : public AsFuncBase {
		public:
			AsFunction( const AsFunctionBuilder* _builder );

			std::string GetName() const { return name; }
			bool IsCaseSensitive() const { return isCaseSensitive; }
			
			virtual Swf::AutoGen::AsObjectHandle Call( Swf::AutoGen::AsAgRuntime* _runtime, int _numParams, Swf::AutoGen::AsObjectHandle* _params ) const;

		private:
			void ComputeLabelAddress( const AsFunctionBuilder* _builder, std::map<int, int>& labels );
			
			typedef Core::gcvector<AsFuncInstruction*> InstVec;

			InstVec						function;
			std::string 				name;
			bool						isCaseSensitive;
		};
	} /* CodeGen */ 
} /* Swf */ 

#endif /* end of include guard: ASFUNCTION_H_3AA74OHD */
