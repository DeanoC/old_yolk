/*
 *  AsAgRuntime.cpp
 *  SwfPreview
 *
 *  Created by Deano on 21/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "gui/swfruntime/swfruntime.h"
#include "gui/swfparser/parser.h"
#include "AsFunction.h"
#include "../movieclip.h"
#include "../utils.h"
#include "../player.h"
#include "aslib/AsDate.h"
#include "aslib/AsArray.h"
#include "AsObjectFactory.h"
#include "AsObjectFunction.h"
#include "AsAgRuntime.h"

namespace Swf {
	static bool parseVarTarget(const std::string& _path, std::string& path, std::string& var) {
		size_t startOfVar = _path.find_last_of(':');
		if ( startOfVar == std::string::npos ) {
			var = _path;
			return false;
		}
		else {
			path = _path.substr(0, startOfVar);
			var = _path.substr(startOfVar, _path.size() - startOfVar);
			return true;
		}
	}
	static void tokenize(const std::string& str,std::vector<std::string>& tokens,
		                    const std::string& delimiters) {
		using namespace std;
		// Skip delimiters at beginning.
		string::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		string::size_type pos     = str.find_first_of(delimiters, lastPos);

		while (string::npos != pos || string::npos != lastPos) {
			// Found a token, add it to the vector.
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	}
				
	AsAgRuntime::AsAgRuntime() : 
		target(0),
		root(0),
		isCaseSens( false ) {

		objectFactory = CORE_GC_NEW_ROOT_ONLY AsObjectFactory( this );
		objectFactory->registerFunc( "Date", &AsDate::constructFunction );
		objectFactory->registerFunc( "Array", &AsArray::constructFunction );

		// create the global object
		globalObject = objectFactory->construct( "object", 0, nullptr );
		globals = CORE_GC_NEW_ROOT_ONLY AsObjectEnvironmentRecord( this, nullptr, globalObject );
		// global function/var table (need to work out what should be in here AND how
		// things get added..)
		globalObject->put( "trace", CORE_GC_NEW AsObjectFunction( &AsAgRuntime::trace ) );

	}
		
	AsAgRuntime::~AsAgRuntime() {
		CORE_GC_DELETE( globals );
		CORE_GC_DELETE( objectFactory );
	}
		
	void AsAgRuntime::callGlobalCode( AsFunction* _code, MovieClip* _movieClip ) {
		setRoot( _movieClip );
		setTarget( _movieClip );

		ExecutionContext tmp;
		tmp.variableEnvironment = globals;
		tmp.lexicalEnvironment = globals;
		tmp.thisObject = globalObject;
		activeExecContexts.push_back( tmp );
		currentExecContext = &activeExecContexts.back();
		currentFunction = _code;

		_code->call( this, nullptr, 0, nullptr );

	}
	void AsAgRuntime::setRoot( MovieClip* _root ) {
		root = _root;
		isCaseSens = root->getPlayer()->parser->fileVersion >= 7;
	}		
    static std::string s_targetseperators = "\\/";
    bool IsTargetAbsolute(const std::string& _path) {
		if (_path[0] == '\\' || _path[0] == '/')
			return true;
		else
			return false;
	}
	FrameItem* AsAgRuntime::findTarget(const std::string& _path) {
		if ( _path.empty() )
			return origTarget;

		std::vector<std::string> path;
		tokenize( _path, path, s_targetseperators );
		if (IsTargetAbsolute(_path)) {
			return root->findTarget(path, 0);
		} else {
			if( target->getAsMovieClip() != NULL ) {
				return target->getAsMovieClip()->findTarget(path, 0);
			} else {
				return target;
			}
		}
	}
						
	void AsAgRuntime::getVariable() {
		AsObjectHandle nameObj = asPop();
		std::string name = nameObj->toString();
						
		// try get a path for the name
		std::string pathS;
		std::string varS;
		if (parseVarTarget(name, pathS, varS) == false) {
			// try locals first
			auto ob = getIdentifier( currentExecContext->variableEnvironment, name );
			if( ob->type() != APT_UNDEFINED ) {
				push( ob );
			} else {
				// try members as not a local?
				if(target) {
					push( target->getProperty(name) );
				}
			}
		} else {
			// TODO external lookup
			pushUndefined();
		}
	}
		
	void AsAgRuntime::setVariable() {
		AsObjectHandle value = asPop();
		AsObjectHandle nameObj = asPop();
		std::string name = nameObj->toString();
			
		// try get a path for the name
		std::string pathS;
		std::string varS;
		if (parseVarTarget(name, pathS, varS) == false) {
			bool success = setIdentifier( currentExecContext->variableEnvironment, name, value );
			if( !success ) {
				// try members as not a local
				if(target) {
					target->setProperty(ToLowerIfReq(name, isCaseSensitive()), value);
				}
			}
		} else {
			// TODO external lookup
		}
	}
	void AsAgRuntime::getMember() {
		AsObjectHandle nameObj = asPop();
		AsObjectHandle obj = asPop();
		std::string name = nameObj->toString();
		AsObjectHandle ret = obj->getProperty(name);
		push( ret );
	}
	void AsAgRuntime::setMember() {
		AsObjectHandle value = asPop();
		AsObjectHandle nameObj = asPop();
		AsObjectHandle obj = asPop();
		std::string name = nameObj->toString();
		obj->put( ToLowerIfReq(name, isCaseSensitive()), value);
	}
		
	void AsAgRuntime::defineLocal() {
		AsObjectHandle val = asPop();
		AsObjectHandle nameObj = asPop();
		std::string name = nameObj->toString();
		currentExecContext->variableEnvironment->createMutableBinding( name );
		currentExecContext->variableEnvironment->setMutableBinding( name, val );
	}
		
	void AsAgRuntime::defineLocal2() {
		AsObjectHandle nameObj = asPop();
		std::string name = nameObj->toString();
		currentExecContext->variableEnvironment->createMutableBinding( name );
	}
		
	void AsAgRuntime::newObject() {
		AsObjectHandle nameObj = asPop();
		std::string str = nameObj->toString();

		AsObjectHandle argCountObj = asPop();
		int numArgs = argCountObj->toInteger();
		AsObjectHandle args[MAX_ARGS]; 
		for(int i=0;i < numArgs;++i) {
			if( i < MAX_ARGS ) {
				args[i] = asPop();
			} else {
				pop();
			}
		}
			
		AsObjectHandle no = objectFactory->construct( str.c_str(), numArgs, args );
		push(no);
	}

	void AsAgRuntime::newMethod() {
		AsObjectHandle nameObj = asPop();
		AsObjectHandle obj = asPop();

		std::string str = nameObj->toString();

		AsObjectHandle argCountObj = asPop();
		int numArgs = argCountObj->toInteger();
		AsObjectHandle args[MAX_ARGS]; 
		for(int i=0;i < numArgs;++i) {
			if( i < MAX_ARGS ) {
				args[i] = asPop();
			} else {
				pop();
			}
		}
		AsObjectHandle ret = objectFactory->construct( "object", 0, nullptr );
		obj->callMethodOn( this, ret, str, numArgs, args );
		push( ret );
	}
	
	void AsAgRuntime::callFunction() {
		AsObjectHandle nameObj = asPop();
		AsObjectHandle argCountObj = asPop();
		std::string str = nameObj->toString();
			
		int numArgs = argCountObj->toInteger();
		AsObjectHandle args[MAX_ARGS]; 
		for(int i=0;i < numArgs;++i) {
			if( i < MAX_ARGS ) {
				args[i] = asPop();
			} else {
				pop();
			}
		}

		auto ob = getIdentifier( currentExecContext->lexicalEnvironment, str );
		if( ob->type() == APT_FUNCTION ) {
			activeExecContexts.resize( activeExecContexts.size() + 1 );
			currentExecContext =  &activeExecContexts.back();
			currentExecContext->variableEnvironment = globals;
			currentExecContext->lexicalEnvironment = globals;
			currentExecContext->thisObject = globalObject;

			auto func = (AsObjectFunction*) ob;
			currentFunction = func->value;
			push( func->value->call( this, currentExecContext->thisObject, numArgs, args ) );

			activeExecContexts.pop_back();
			currentExecContext = &activeExecContexts.back();
		} else 
		{
			pushUndefined();
		}			
	}
				
	void AsAgRuntime::callMethod() {
		AsObjectHandle nameObj = asPop();
		AsObjectHandle obj = asPop();
		AsObjectHandle argCountObj = asPop();
		std::string str;
		if( nameObj->type() == APT_UNDEFINED ) {
			str = "()";
		} else {
			str = nameObj->toString();
			if( str.empty() ) {
				str = "()";
			}
		}
			
		int numArgs = argCountObj->toInteger();
		AsObjectHandle args[MAX_ARGS]; 
		for(int i=0;i < numArgs;++i) {
			if( i < MAX_ARGS ) {
				args[i] = asPop();
			} else {
				pop();
			}
		}
		AsObjectHandle ret = obj->callMethod( this, str, numArgs, args );
		push( ret );
	}
	void AsAgRuntime::play() {
		if(target && target->type == FIT_MOVIECLIP ) {
			MovieClip* mc = (MovieClip*) target;
			mc->play();	
		}
	}
	void AsAgRuntime::stop() {
		if(target && target->type == FIT_MOVIECLIP ) {
			MovieClip* mc = (MovieClip*) target;
			mc->stop();	
		}
	}
	void AsAgRuntime::nextFrame() {
		if(target && target->type == FIT_MOVIECLIP ) {
			MovieClip* mc = (MovieClip*) target;
			mc->nextFrame();
			mc->stop();	
		}
	}
	void AsAgRuntime::prevFrame() {
		if(target && target->type == FIT_MOVIECLIP ) {
			MovieClip* mc = (MovieClip*) target;
			mc->prevFrame();
			mc->stop();	
		}
	}
		
	void AsAgRuntime::gotoFrame( uint16_t _frame ) {
		if(target && target->type == FIT_MOVIECLIP ) {
			MovieClip* mc = (MovieClip*) target;
			mc->gotoFrame( _frame );
		}
	}

	void AsAgRuntime::gotoFrame( const AsObjectHandle& _frame ) {
		if(target && target->type == FIT_MOVIECLIP ) {
			MovieClip* mc = (MovieClip*) target;
			mc->gotoFrame( _frame->toInteger() );
		}
	}
		
	void AsAgRuntime::add() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		double a = oa->toNumber();
		double b = ob->toNumber();
		double aaddb = a + b;
		push(aaddb);
	}
	void AsAgRuntime::subtract() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		double a = oa->toNumber();
		double b = ob->toNumber();
		double asubb = a - b;
		push(asubb);
	}
	void AsAgRuntime::multiply() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		double a = oa->toNumber();
		double b = ob->toNumber();
		double ab = a * b;
		push(ab);
	}
	void AsAgRuntime::divide() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		double a = oa->toNumber();
		double b = ob->toNumber();
		double bovera = b / a;
		push(bovera);
	}
		
	void AsAgRuntime::equals() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		double a = oa->toNumber();
		double b = ob->toNumber();
		bool aeqb = a == b;
		push(aeqb);
	}
		
	void AsAgRuntime::less() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		double a = oa->toNumber();
		double b = ob->toNumber();
		bool blta = b < a;
		push(blta);
	}
		
    void AsAgRuntime::and() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		double a = oa->toNumber();
		double b = ob->toNumber();
		bool aandb = (a != 0.0) && (b != 0.0);
		push(aandb);
	}
		
    void AsAgRuntime::or() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		double a = oa->toNumber();
		double b = ob->toNumber();
		bool aorb = (a != 0.0) || (b != 0.0);
		push(aorb);
	}

    void AsAgRuntime::not() {
		/* Swf 4 behavior I believe
		object oa = currentFrame.stack.Pop();
		double a = IsNumeric(oa) ? ToNumber(oa) : 0;
		bool nota = (a == 0.0);
		currentFrame.stack.Push(nota); */
		AsObjectHandle oa = asPop();
		bool a = oa->toBoolean();
		bool nota = !a;
		push(nota);
	}

    void AsAgRuntime::stringEquals() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		bool eq = oa->toString() == ob->toString();
		push(eq);
	}

	void AsAgRuntime::stringLength() {
		AsObjectHandle oa = asPop();
		int len = oa->toString().size();
		push((int32_t)len);
	}
		
	void AsAgRuntime::stringAdd() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		std::string badda = ob->toString() + oa->toString();
		push(badda);
	}
		
	void AsAgRuntime::stringExtract() {
		int count = asPop()->toInteger();
		int index = asPop()->toInteger();
		std::string str = asPop()->toString();
		std::string substr = str.substr(index, count);
		push(substr);
	}

	void AsAgRuntime::stringLess() {
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		bool lt = ob->toString() < oa->toString();
		push(lt);
	}

	void AsAgRuntime::MBStringLength() {
		AsObjectHandle oa = asPop();
		int len = oa->toString().size();
		push((int32_t)len);
	}

	void AsAgRuntime::MBStringExtract() {
		int count = asPop()->toInteger();
		int index = asPop()->toInteger();
		std::string str = asPop()->toString();
		std::string substr = str.substr(index, count);
		push(substr);
	}
	void AsAgRuntime::toInteger() {
		AsObjectHandle oa = asPop();
		push( (int32_t)oa->toInteger() );
	}
		
	void AsAgRuntime::add2() {
		// Add2 is addition following ECMA-262 Section 11.6.1
		// this is not complaint yet (tho handles string and numeric)
		AsObjectHandle oa = asPop();
		AsObjectHandle ob = asPop();
		if( oa->type() == APT_STRING || ob->type() == APT_STRING ) {
			std::string ret = ob->toString() + oa->toString();
			push( ret );
		} else {
			double ret = ob->toNumber() + oa->toNumber();
			push( ret );
		}			
	}
	void AsAgRuntime::getProperty(){
		int index = asPop()->toInteger();
		std::string targstr = asPop()->toString();
		FrameItem* targ = findTarget(targstr);
		push( targ->getProperty(index) );
	}
	void AsAgRuntime::setProperty(){
		AsObjectHandle oval = asPop();
		int index = asPop()->toInteger();
		std::string targstr = asPop()->toString();
		FrameItem* targ = findTarget(targstr);
		targ->setProperty(index, oval);
	}
	void AsAgRuntime::trace() {
		std::string str = asPop()->toString();
		LOG(INFO) << str << "\n";
	}
	void AsAgRuntime::initArray() {
		AsObjectHandle argCountObj = asPop();
			
		int numArgs = argCountObj->toInteger();
		AsObjectHandle args[1024]; 
		for(int i=0;i < numArgs;++i) {
			if( i < 1024 ) {
				args[i] = asPop();
			} else {
				pop();
			}
		}
		AsObjectHandle ret = objectFactory->construct( "Array", numArgs, args );
		push( ret );
	}
	void AsAgRuntime::setTarget2() {
		std::string targ = asPop()->toString();
		FrameItem* obj = findTarget(targ);
		if(obj)
			target = obj; 
	}
	AsObjectHandle AsAgRuntime::hasOwnProperty( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert(_numParams == 1 );
		AsObjectHandle nameObj = _params[0];
		std::string name = nameObj->toString();
		bool ret = _runtime->target->hasOwnProperty( name );
		return CORE_NEW AsObjectBool(ret);
	}

	AsObjectHandle AsAgRuntime::trace( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		for( int i = 0; i < _numParams; ++i ) {
			LOG(INFO) << _params[i]->toString() << "\n";
		}

		return AsObjectNull::get();
	}

	bool AsAgRuntime::equalEcma262_11_9_3(AsObjectHandle _x, AsObjectHandle _y) {
		// Ecma 262 11.9.3 equality
		if( _x->type() == _y->type() ) {
			// same types
			switch( _x->type() ) {
			default:
			case APT_UNDEFINED: return true;
			case APT_NULL: return true;
			case APT_NUMBER: {
				double dx = ((AsObjectNumber*)_x)->value;
				double dy = ((AsObjectNumber*)_y)->value;
//					if( std::isnan(dx) || std::isnan(dx)) { return false; }
				if( dx == dy ){ return true; }
				if( fabs(dx) == 0.0 && fabs(dy) == 0.0 ) { return true; }
				return false;
			}
			break;
			case APT_STRING: return ((AsObjectString*)_x)->value ==((AsObjectString*)_y)->value;
			case APT_BOOLEAN: return ((AsObjectBool*)_x)->value ==((AsObjectBool*)_y)->value;
			case APT_OBJECT: return (_x == _y);
			}
		} else {
			// different types, so coerce first
			if ( 	(_x->type() == APT_UNDEFINED && _y->type() == APT_NULL) ||
				 	(_x->type() == APT_NULL && _y->type() == APT_UNDEFINED) ) {
						return true;					
			} else if( _x->type() != APT_OBJECT && _y->type() != APT_OBJECT ) {
				return (_x->toNumber() == _y->toNumber() );
			} else {
				// TODO proper object coerce (calls involved??)
				return (_x->toNumber() == _y->toNumber() );
			}
		}		
	}
	void AsAgRuntime::equal2() {
		AsObjectHandle x = asPop();
		AsObjectHandle y = asPop();
		push( CORE_NEW AsObjectBool( equalEcma262_11_9_3(x,y) ) );
	}
		
	void AsAgRuntime::defineLocalFunction( const AsObjectHandle& _func ) {
		if( _func->type() == APT_FUNCTION ) {
			push( _func );
		} else {
			pushUndefined();
		}
	}
		
	void AsAgRuntime::defineFunction( const AsObjectHandle& _name, const AsObjectHandle& _func ) {
		if( _func->type() == APT_FUNCTION ) {
			target->setProperty( _name->toString(), _func );
		} else {
			pushUndefined();
		}
	}

	void AsAgRuntime::toNumber() {
		push( asPop()->toNumber() );
	}
	void AsAgRuntime::toString() {
		push( asPop()->toString() );
	}
	void AsAgRuntime::increment() {
		push( asPop()->toNumber() + 1.0 );
	}
	void AsAgRuntime::decrement() {
		push( asPop()->toNumber() - 1.0 );
	}
	void AsAgRuntime::pushDuplicate() {
		stack.push( stack.top() );
	}
	void AsAgRuntime::swapStack() {
		AsObjectHandle a = asPop();
		AsObjectHandle b = asPop();
		push( a );
		push( b );
	}
	void AsAgRuntime::initObject() {
		AsObjectHandle argCountObj = asPop();
		int numArgs = argCountObj->toInteger();
		AsObjectHandle args[MAX_ARGS]; 
		// name + value per arg
		for(int i=(numArgs*2)-1;i >= 0;--i) {
			if( i < MAX_ARGS ) {
				args[i] = asPop();
			} else {
				pop();
			}
		}
		AsObjectHandle no = objectFactory->construct( "object", numArgs, args );
		push( no );
	}

	AsAgRuntime::AsEnvironmentRecord::AsEnvironmentRecord( AsAgRuntime* _owner, AsEnvironmentRecord* _outer ) :
		owner( _owner ),
		outer( _outer ) {
	}

	AsAgRuntime::AsDeclEnvironmentRecord::AsDeclEnvironmentRecord( AsAgRuntime* _owner, AsEnvironmentRecord* _outer ) :
		AsEnvironmentRecord( _owner, _outer ) {
	}

	bool AsAgRuntime::AsDeclEnvironmentRecord::hasBinding( const std::string& _name ) const {
		return( records.find( _name ) != records.end() );
	}

	void AsAgRuntime::AsDeclEnvironmentRecord::createMutableBinding( const std::string& _name, bool _deletable ) {
		CORE_ASSERT( !hasBinding(_name) );
		records[ _name ] = AsObjectUndefined::get();
		deletable[ _name ] = _deletable;
		immutable[ _name ] = false;
		// TODO pack all properties and value together for efficiency
	}

	void AsAgRuntime::AsDeclEnvironmentRecord::setMutableBinding( const std::string& _name, AsObjectHandle _value, const bool _strict ) {
		CORE_ASSERT( hasBinding(_name) );
		if( immutable[ _name ] == true ) {
			// TODO ECMA-262 Throw here
		} else {
			records[ _name ] = _value;
		}
	}
	AsObjectHandle AsAgRuntime::AsDeclEnvironmentRecord::getBindingValue( const std::string& _name, const bool _strict ) const {
		CORE_ASSERT( hasBinding(_name) );
		auto imm = immutable.find( _name);
		if( imm != immutable.end() ) {
			if( imm->second == false ) {
				if( _strict == false ) {
					return AsObjectUndefined::get();
				} else {
					// TODO ECMA-262 Throw
					return AsObjectUndefined::get();
				}
			}
		}
		auto ret = records.find( _name );
		if( ret == records.end() ) {
			return AsObjectUndefined::get();
		} else {
			return ret->second;
		}
	}

	bool AsAgRuntime::AsDeclEnvironmentRecord::deleteBinding( const std::string& _name ) {
		if( hasBinding( _name ) == false ) {
			return true;
		}
		if( deletable[ _name ] == false ) {
			return false;
		}
		records.erase( records.find( _name ) );
		return true;
	}

	AsObjectHandle AsAgRuntime::AsDeclEnvironmentRecord::implicitThisValue() const {
		return AsObjectUndefined::get();
	}


	void AsAgRuntime::AsDeclEnvironmentRecord::createImmutableBinding( const std::string& _name ) {
		CORE_ASSERT( !hasBinding(_name) );
		records[ _name ] = nullptr;
		deletable[ _name ] = false;
		immutable[ _name ] = true;
	}

	void AsAgRuntime::AsDeclEnvironmentRecord::initializeImmutableBinding( const std::string& _name, AsObjectHandle _value ) {
		CORE_ASSERT( hasBinding(_name) );
		CORE_ASSERT( immutable[ _name ] == true );
		CORE_ASSERT( records[ _name ] == nullptr );
		records[ _name ] = _value;
	}

	AsAgRuntime::AsObjectEnvironmentRecord::AsObjectEnvironmentRecord( AsAgRuntime* _owner, AsEnvironmentRecord* _outer, AsObjectHandle _bindingObject, bool _provideThis ) :
		AsEnvironmentRecord( _owner, _outer ),
		bindingObject( _bindingObject ),
		provideThis( _provideThis ) {
	}

	bool AsAgRuntime::AsObjectEnvironmentRecord::hasBinding( const std::string& _name ) const {
		return bindingObject->hasProperty( _name );
	}

	void AsAgRuntime::AsObjectEnvironmentRecord::createMutableBinding( const std::string& _name, bool _deletable ) {
		CORE_ASSERT( !hasBinding( _name ) );
		bindingObject->defineOwnProperty( _name, AsObjectUndefined::get(), true, true, _deletable, true );
	}

	void AsAgRuntime::AsObjectEnvironmentRecord::setMutableBinding( const std::string& _name, AsObjectHandle _value, const bool _strict  ) {
		bindingObject->put( _name, _value, _strict );
	}

	AsObjectHandle AsAgRuntime::AsObjectEnvironmentRecord::getBindingValue( const std::string& _name, const bool _strict ) const {
		if( bindingObject->hasProperty( _name ) ) {
			return bindingObject->getProperty( _name );
		} else {
			if( _strict ) {
				// TODO ECMA-262 Throw
				return AsObjectUndefined::get();
			} else {
				return AsObjectUndefined::get();
			}
		}
	}

	bool AsAgRuntime::AsObjectEnvironmentRecord::deleteBinding( const std::string& _name ) {
		bindingObject->deleteProperty( _name, false );
		return true;
	}

	AsObjectHandle AsAgRuntime::AsObjectEnvironmentRecord::implicitThisValue() const {
		if( provideThis ) {
			return bindingObject;
		} else {
			return AsObjectUndefined::get();
		}
	}

	AsObjectHandle AsAgRuntime::getIdentifier( const AsEnvironmentRecord* _lex, const std::string& _name ) const {
		if( _lex == nullptr ) {
			return AsObjectUndefined::get();
		} else {
			if( _lex->hasBinding( _name ) ) {
				return _lex->getBindingValue( _name );
			} else {
				return getIdentifier( _lex->outer, _name );
			}
		}
	}
	bool AsAgRuntime::setIdentifier( AsEnvironmentRecord* _lex, const std::string& _name, const AsObjectHandle _value ) const {
		if( _lex == nullptr ) {
			return false;
		} else {
			if( _lex->hasBinding( _name ) ) {
				_lex->setMutableBinding( _name, _value );
				return true;
			} else {
				return setIdentifier( _lex->outer, _name, _value );
			}
		}
	}

	void AsAgRuntime::enterGlobalCode() {
		CORE_ASSERT( activeExecContexts.empty() );
		activeExecContexts.resize( 1 );
		currentExecContext = &activeExecContexts.back();
		currentExecContext->lexicalEnvironment = globals;
		currentExecContext->variableEnvironment = globals;
		currentExecContext->thisObject = globalObject;
	}
	void AsAgRuntime::enterFunctionCode( AsObjectFunction* _func, AsObjectHandle _this, int _numArgs, AsObjectHandle _args[MAX_ARGS] ) {
		CORE_ASSERT( !activeExecContexts.empty() );

		auto outerExec = currentExecContext;
		activeExecContexts.resize( activeExecContexts.size() + 1 );
		currentExecContext = &activeExecContexts.back();
		if( _this == nullptr || _this->type() == APT_UNDEFINED || _this->type() == APT_NULL ) {
			currentExecContext->thisObject = globalObject;
		} else {
			currentExecContext->thisObject = _this;
		}
		currentExecContext->lexicalEnvironment = CORE_GC_NEW AsDeclEnvironmentRecord( this, outerExec->lexicalEnvironment );
		currentExecContext->variableEnvironment = currentExecContext->lexicalEnvironment;

	}
	void AsAgRuntime::asReturn() {
		// TODO return logic currentFunction->asReturn();
	}

} /* Swf */ 
