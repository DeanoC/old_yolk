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
	class AsFunction;
	class AsObjectFunction;
		
	class AsAgRuntime  {
	public:		
		static const int MAX_ARGS = 64;

		AsAgRuntime();
			
		~AsAgRuntime();
			
		// are we executing case sensitive AS
		bool isCaseSensitive() const {
			return isCaseSens;
		}

		void callGlobalCode( AsFunction* _code, MovieClip* _movieClip );
		
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
		void initObject();
		void newMethod();
		void asReturn();
		
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
		void modulo() AS_TODO
		void typeOf() AS_TODO
		void enumerate() AS_TODO
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

		// set the target via an AsObject
		void gotoFrame( const AsObjectHandle& _frame );
		void defineLocalFunction( const AsObjectHandle& _func );
		void defineFunction( const AsObjectHandle& _name, const AsObjectHandle& _func );

	protected:
		// callable functions
		// hasOwnProperty checks the target has the property specified
		static AsObjectHandle hasOwnProperty(  AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
		static AsObjectHandle trace(  AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );

		bool equalEcma262_11_9_3(AsObjectHandle _x, AsObjectHandle _y);			
		
		FrameItem* findTarget(const std::string& _path);
			
		typedef Core::gctraceablestack<AsObjectHandle> AsStack;
		AsStack					stack;

		class AsEnvironmentRecord {
		public:
			AsEnvironmentRecord( AsAgRuntime* _owner, AsEnvironmentRecord* _outer );
			virtual ~AsEnvironmentRecord() {};

			virtual bool hasBinding( const std::string& _name ) const = 0;
			virtual void createMutableBinding( const std::string& _name, const bool _deletable = false ) = 0;
			virtual void setMutableBinding( const std::string& _name, AsObjectHandle _value, const bool _strict = false ) = 0;
			virtual AsObjectHandle getBindingValue( const std::string& _name, const bool _strict = false ) const = 0;
			virtual bool deleteBinding( const std::string& _name ) = 0;
			virtual AsObjectHandle implicitThisValue() const = 0;

		public:
			AsAgRuntime* owner;
			AsEnvironmentRecord* outer;
		};

		class AsDeclEnvironmentRecord : public AsEnvironmentRecord {
		public:
			AsDeclEnvironmentRecord( AsAgRuntime* _owner, AsEnvironmentRecord* _outer );

			bool hasBinding( const std::string& _name ) const override;
			void createMutableBinding( const std::string& _name, const bool _deletable ) override;
			void setMutableBinding( const std::string& _name, AsObjectHandle _value, const bool _strict  ) override;
			AsObjectHandle getBindingValue( const std::string& _name, const bool _strict ) const override;
			bool deleteBinding( const std::string& _name ) override;
			AsObjectHandle implicitThisValue() const override;

			void createImmutableBinding( const std::string& _name );
			void initializeImmutableBinding( const std::string& _name, AsObjectHandle _value );

		private:
			typedef std::map< const std::string, bool > BooleanProperty;

			Core::gctraceablemap<std::string, AsObjectHandle> records;
			BooleanProperty deletable;
			BooleanProperty immutable;
		};

		class AsObjectEnvironmentRecord : public AsEnvironmentRecord {
		public:
			AsObjectEnvironmentRecord( AsAgRuntime* _owner, AsEnvironmentRecord* _outer, AsObjectHandle _bindingObject, bool _provideThis = false );

			bool hasBinding( const std::string& _name ) const override;
			void createMutableBinding( const std::string& _name, const bool _deletable ) override;
			void setMutableBinding( const std::string& _name, AsObjectHandle _value, const bool _strict  ) override;
			AsObjectHandle getBindingValue( const std::string& _name, const bool _strict ) const override;
			bool deleteBinding( const std::string& _name ) override;
			AsObjectHandle implicitThisValue() const override;

			void createImmutableBinding( const std::string& _name );
			void initializeImmutableBinding( const std::string& _name, AsObjectHandle _value );
		private:
			AsObjectHandle bindingObject;
			bool provideThis;
		};
		struct ExecutionContext {
			AsEnvironmentRecord*		variableEnvironment;
			AsEnvironmentRecord*		lexicalEnvironment;
			AsObjectHandle				thisObject;
		};
		typedef std::vector<ExecutionContext> ActiveExecutionContexts;
		ActiveExecutionContexts			activeExecContexts;
		ExecutionContext*				currentExecContext;

		AsObjectEnvironmentRecord*	globals;
		AsObjectHandle				globalObject;
		const AsFuncBase*			currentFunction;

		FrameItem*				target;
		FrameItem*				origTarget;
		MovieClip*				root;
		AsObjectFactory*		objectFactory;
		bool 					isCaseSens;		
			
		AsObjectHandle getIdentifier( const AsEnvironmentRecord* _lex, const std::string& _name ) const;
		bool setIdentifier( AsEnvironmentRecord* _lex, const std::string& _name, const AsObjectHandle _value ) const;

		void enterGlobalCode();
		void enterFunctionCode( AsObjectFunction* _func, AsObjectHandle	_this, int _numArgs, AsObjectHandle _args[MAX_ARGS] );

	};
} /* Swf */ 



#endif /* end of include guard: ASAGRUNTIME_H_S2UQ2TCL */

