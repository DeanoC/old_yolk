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
	class MovieClip;
	class FrameItem;
	class AsObjectFactory;
		
	class AsAgRuntime  {
	public:		
		AsAgRuntime();
			
		~AsAgRuntime();
			
		// are we executing case sensitive AS
		bool isCaseSensitive() const {
			return isCaseSens;
		}
			
		// reset all VM state
		void reset();
		
		// Set the target to the specified item
		void setTarget( FrameItem* _target) {
			target = _target;
			origTarget = _target;
		}
			
		// Set the root clip
		void setRoot( MovieClip* _root );
			
		// set the target to frame X
		void gotoFrame( uint16_t _frame );
			
		AsObjectHandle asPop() {
			AsObjectHandle t = stack.top();
			stack.pop();
			return t;
		}
			
		void pop() {
			stack.pop();
		}
			
		void push( const char* str ) {
			AsObjectHandle t( CORE_NEW AsObjectString(str) );
			push(t);
		}
		void push( const std::string& str ) {
			AsObjectHandle t( CORE_NEW AsObjectString(str) );
			push(t);
		}			
		void push( float f ) {
			AsObjectHandle t( CORE_NEW AsObjectNumber(f) );
			push(t);
		}
		void push( double d ) {
			AsObjectHandle t( CORE_NEW AsObjectNumber(d) );
			push(t);
		}
		void push( int32_t i ) {
			AsObjectHandle t( CORE_NEW AsObjectNumber(i) );
			push(t);
		}
		void push( bool b ){
			AsObjectHandle t( CORE_NEW AsObjectBool(b) );
			push(t);				
		}
			
		void pushNull() {
			AsObjectHandle t( AsObjectNull::get() );
			push(t);
		}
		void pushUndefined() {
			AsObjectHandle t( AsObjectUndefined::get() );
			push(t);				
		}
			
		void push( const AsObjectHandle& handle ) {
			stack.push(handle);
		}
			
		// implemented As2 functions
		void getVariable();
		void setVariable();
		void defineLocal();
		void defineLocal2();
		void newObject();
		void callMethod();
		void end(){};
		void play();
		void stop();		  			
		void nextFrame();
		void prevFrame();
		void add();
		void subtract();
		void multiply();
		void divide();		  
		void equals();
		void less();
		void and();
		void or();
		void not();
		void stringEquals();
		void stringLength();
		void stringExtract();
		void stringAdd();
		void stringLess();
		void MBStringLength();
		void MBStringExtract();
		void toInteger();
		void getMember();
		void setMember();
		void add2();
		void getProperty();
		void setProperty();
		void trace();
		void initArray();
		void setTarget2();
		void callFunction();
		void equal2();
		void toNumber();
		void toString();
		void increment();
		void decrement();
		void pushDuplicate();
		void swapStack();
			
		// As2 functions not yet implemented
		#define AS_TODO { assert(false); } 
			
		void less2() AS_TODO
		void greater() AS_TODO
		void toggleQuality() AS_TODO
		void stopSounds() AS_TODO
		void dupClip() AS_TODO
		void removeClip() AS_TODO
		void startDragMovie() AS_TODO
		void stopDragMovie() AS_TODO
		void random() AS_TODO
		void ord() AS_TODO	  
		void chr() AS_TODO		  
		void getTimer()AS_TODO
		void MBOrd() AS_TODO
		void MBChr() AS_TODO
		void asDelete() AS_TODO
		void asReturn() AS_TODO
		void modulo() AS_TODO
		void initObject() AS_TODO
		void typeOf() AS_TODO
		void enumerate() AS_TODO
		void newMethod() AS_TODO
		void bitwiseAnd() AS_TODO
		void bitwiseEor() AS_TODO
		void bitwiseXor() AS_TODO
		void shiftLeft() AS_TODO
		void shiftRight() AS_TODO
		void shiftRight2() AS_TODO
		void gotoFrame() AS_TODO
		void getURL() AS_TODO
		void setTarget() AS_TODO
		void gotoLabel() AS_TODO
		void pushData() AS_TODO
		void jump() AS_TODO
		void If() AS_TODO
		void storeRegister() AS_TODO
		void constantPool() AS_TODO
		void waitForFrame() AS_TODO
		void waitForFrameExpression() AS_TODO
		void with() AS_TODO
		void getURL2() AS_TODO
		void defineFunction() AS_TODO
		void callFrame() AS_TODO
		void gotoFrame2() AS_TODO
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
		void gotoFrame( const AsObjectHandle& _frame );
		void defineLocalFunction( const AsObjectHandle& _func );
		void defineFunction( const AsObjectHandle& _name, const AsObjectHandle& _func );

	protected:
		// hasOwnProperty checks the target has the property specified
		AsObjectHandle hasOwnProperty( int _numParams, AsObjectHandle* _params );
			
		bool equalEcma262_11_9_3(AsObjectHandle _x, AsObjectHandle _y);			
			
		FrameItem* findTarget(const std::string& _path);
			
		typedef Core::gctraceablestack<AsObjectHandle> AsStack;
		typedef Core::gctraceablemap<std::string, AsObjectHandle> AsLocals;
			
		AsStack					stack;
		AsLocals				locals;
		FrameItem*				target;
		FrameItem*				origTarget;
		MovieClip*				root;
		AsObjectFactory*		objectFactory;
		bool 					isCaseSens;
			
		typedef Core::gctraceablemap<std::string, Swf::AsFuncBase*> AsFunctions;
		AsFunctions functions;
			
		static const int MAX_ARGS = 16;
	};
} /* Swf */ 



#endif /* end of include guard: ASAGRUNTIME_H_S2UQ2TCL */

