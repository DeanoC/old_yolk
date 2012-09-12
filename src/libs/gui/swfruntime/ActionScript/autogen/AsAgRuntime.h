/*
 *  AsAgRuntime.h
 *  SwfPreview
 *
 *  Created by Deano on 21/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef ASAGRUNTIME_H_S2UQ2TCL
#define ASAGRUNTIME_H_S2UQ2TCL

#include "AsObject.h"

namespace Swf {
	class SwfMovieClip;
	class SwfFrameItem;
	
	namespace CodeGen { class AsFuncBase; }
	
	namespace AutoGen {
		class AsObjectFactory;
		
		class AsAgRuntime  {
		public:		
			AsAgRuntime();
			
			~AsAgRuntime();
			
			// are we executing case sensitive AS
			bool IsCaseSensitive() const {
				return isCaseSensitive;
			}
			
			// reset all VM state
			void Reset();
		
			// Set the target to the specified item
			void SetTarget( SwfFrameItem* _target) {
				target = _target;
				origTarget = _target;
			}
			
			// Set the root clip
			void SetRoot( SwfMovieClip* _root );
			
			// set the target to frame X
			void GotoFrame( uint16_t _frame );
			
			AsObjectHandle AsPop() {
				AsObjectHandle t = stack.top();
				stack.pop();
				return t;
			}
			
			void Pop() {
				stack.pop();
			}
			
			void Push( const char* str ) {
				AsObjectHandle t( CORE_NEW AsObjectString(str) );
				Push(t);
			}
			void Push( const std::string& str ) {
				AsObjectHandle t( CORE_NEW AsObjectString(str) );
				Push(t);
			}			
			void Push( float f ) {
				AsObjectHandle t( CORE_NEW AsObjectNumber(f) );
				Push(t);
			}
			void Push( double d ) {
				AsObjectHandle t( CORE_NEW AsObjectNumber(d) );
				Push(t);
			}
			void Push( int32_t i ) {
				AsObjectHandle t( CORE_NEW AsObjectNumber(i) );
				Push(t);
			}
			void Push( bool b ){
				AsObjectHandle t( CORE_NEW AsObjectBool(b) );
				Push(t);				
			}
			
			void PushNull() {
				AsObjectHandle t( AsObjectNull::Get() );
				Push(t);
			}
			void PushUndefined() {
				AsObjectHandle t( AsObjectUndefined::Get() );
				Push(t);				
			}
			
			void Push( const AsObjectHandle& handle ) {
				stack.push(handle);
			}
			
			// implemented As2 functions
			void GetVariable();
			void SetVariable();
			void DefineLocal();
			void DefineLocal2();
			void NewObject();
			void CallMethod();
			void End(){};
			void Play();
			void Stop();		  			
			void NextFrame();
			void PrevFrame();
			void Add();
			void Subtract();
			void Multiply();
			void Divide();		  
			void Equals();
			void Less();
			void And();
			void Or();
			void Not();
			void StringEquals();
			void StringLength();
			void StringExtract();
			void StringAdd();
			void StringLess();
			void MBStringLength();
			void MBStringExtract();
			void ToInteger();
			void GetMember();
			void SetMember();
			void Add2();
			void GetProperty();
			void SetProperty();
			void Trace();
			void InitArray();
			void SetTarget2();
			void CallFunction();
			void Equal2();
			void ToNumber();
			void ToString();
			void Increment();
			void Decrement();
			void PushDuplicate();
			void SwapStack();
			void Less2();
			void Greater();
			
			// As2 functions not yet implemented
			#define AS_TODO { assert(false); } 
			
			void ToggleQuality() AS_TODO
			void StopSounds() AS_TODO
			void DupClip() AS_TODO
			void RemoveClip() AS_TODO
			void StartDragMovie() AS_TODO
			void StopDragMovie() AS_TODO
			void Random() AS_TODO
			void Ord() AS_TODO	  
			void Chr() AS_TODO		  
			void GetTimer()AS_TODO
			void MBOrd() AS_TODO
			void MBChr() AS_TODO
			void Delete() AS_TODO
			void Return() AS_TODO
			void Modulo() AS_TODO
			void InitObject() AS_TODO
			void TypeOf() AS_TODO
			void Enumerate() AS_TODO
			void NewMethod() AS_TODO
			void BitwiseAnd() AS_TODO
			void BitwiseEor() AS_TODO
			void BitwiseXor() AS_TODO
			void ShiftLeft() AS_TODO
			void ShiftRight() AS_TODO
			void ShiftRight2() AS_TODO
			void GotoFrame() AS_TODO
			void GetURL() AS_TODO
			void SetTarget() AS_TODO
			void GotoLabel() AS_TODO
			void PushData() AS_TODO
			void Jump() AS_TODO
			void If() AS_TODO
			void StoreRegister() AS_TODO
			void ConstantPool() AS_TODO
			void WaitForFrame() AS_TODO
			void WaitForFrameExpression() AS_TODO
			void With() AS_TODO
			void GetURL2() AS_TODO
			void DefineFunction() AS_TODO
			void CallFrame() AS_TODO
			void GotoFrame2() AS_TODO
			void ACTION_INSTANCEOF() AS_TODO
			void ACTION_ENUMERATE2() AS_TODO
			void ACTION_STRICTEQ() AS_TODO
			void ACTION_STRINGGREATER() AS_TODO
			void ACTION_DEFINEFUNCTION2() AS_TODO
			void ACTION_EXTENDS() AS_TODO
			void ACTION_CAST() AS_TODO
			void ACTION_IMPLEMENTS() AS_TODO
			void ACTION_TRY() AS_TODO
			void ACTION_THROW() AS_TODO
						
			#undef AS_TODO

			// some fake helpers that implement some thing via
			// AsObjectHandle, just for function previews

			// set the target via an AsObject
			void GotoFrame( const AsObjectHandle& _frame );
			void DefineLocalFunction( const AsObjectHandle& _func );
			void DefineFunction( const AsObjectHandle& _name, const AsObjectHandle& _func );

		protected:
			// hasOwnProperty checks the target has the property specified
			AsObjectHandle hasOwnProperty( int _numParams, AsObjectHandle* _params );
			
			bool EqualEcma262_11_9_3(AsObjectHandle _x, AsObjectHandle _y);			
			
			SwfFrameItem* FindTarget(const std::string& _path);
			
			typedef Core::gctraceablestack<AsObjectHandle> AsStack;
			typedef Core::gctraceablemap<std::string, AsObjectHandle> AsLocals;
			
			AsStack					stack;
			AsLocals				locals;
			SwfFrameItem*			target;
			SwfFrameItem*			origTarget;
			SwfMovieClip*			root;
			AsObjectFactory*		objectFactory;
			bool 					isCaseSensitive;
			
			typedef Core::gctraceablemap<std::string, Swf::CodeGen::AsFuncBase*> AsFunctions;
			AsFunctions functions;
			
			static const int MAX_ARGS = 16;
		};
	} /* AutoGen */ 
} /* Swf */ 



#endif /* end of include guard: ASAGRUNTIME_H_S2UQ2TCL */

