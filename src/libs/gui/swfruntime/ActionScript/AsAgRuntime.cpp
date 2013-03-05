/*
 *  AsAgRuntime.cpp
 *  SwfPreview
 *
 *  Created by Deano on 21/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "gui/swfruntime/swfruntime.h"
#include "gui/SwfParser/parser.h"
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
		isCaseSens( false )  {
		objectFactory = CORE_GC_NEW_ROOT_ONLY AsObjectFactory( this );
		objectFactory->registerFunc( "Date", &AsDate::constructFunction );
		objectFactory->registerFunc( "Array", &AsArray::constructFunction );
		
		// global function/var table (need to work out what should be in here AND how
		// things get added..)
		functions[ "hasOwnProperty" ] = CORE_NEW AsAgFunction( &AsAgRuntime::hasOwnProperty );
		functions[ "trace" ] = CORE_NEW AsAgFunction( &AsAgRuntime::trace );

	}
		
	AsAgRuntime::~AsAgRuntime() {
		reset();
		CORE_GC_DELETE( objectFactory );
	}
		
	void AsAgRuntime::reset() {
		while ( !stack.empty() ) {
			stack.pop();
		}
			
//			locals.clear();
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
			if( locals.find(name.c_str()) != locals.end()) {
				push( locals[name.c_str()] );
			} else {
				// try members as not a local
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
			// try locals first
			if( locals.find(name) != locals.end()) {
				locals[ name ] = value;
			} else {
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
		obj->setProperty( ToLowerIfReq(name, isCaseSensitive()), value);
	}
		
	void AsAgRuntime::defineLocal() {
		AsObjectHandle val = asPop();
		AsObjectHandle nameObj = asPop();
		std::string name = nameObj->toString();
		locals[ name.c_str() ] = val;
	}
		
	void AsAgRuntime::defineLocal2() {
		AsObjectHandle nameObj = asPop();
		std::string name = nameObj->toString();
		locals[ name.c_str() ] = AsObjectHandle( AsObjectUndefined::get() );
	}
		
	void AsAgRuntime::newObject() {
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
			
		AsObjectHandle no = objectFactory->construct( str.c_str(), numArgs, args );
		push(no);
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
			
		AsFunctions::iterator it = functions.find( str );
		if( it != functions.end() ) {
			push( it->second->call( this, numArgs, args ) );
		} else {
			pushUndefined();
		}			
	}
		
	AsObjectHandle AsAgRuntime::hasOwnProperty( int _numParams, AsObjectHandle* _params ) {
		assert(_numParams == 1 );
		AsObjectHandle nameObj = _params[0];
		std::string name = nameObj->toString();
		bool ret = target->hasOwnProperty( name );
		return CORE_NEW AsObjectBool(ret);
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

	AsObjectHandle AsAgRuntime::trace( int _numParams, AsObjectHandle* _params ) {
		for( int i = 0; i < _numParams; ++i ) {
			LOG(INFO) << _params[i]->toString() << "\n";
		}

		return AsObjectNull::get();
	}

	void AsAgRuntime::toNumber() {
		push( asPop()->toNumber() );
	}
	void AsAgRuntime::toString() {
		push( asPop()->toString() );
	}
	void AsAgRuntime::increment() {
		push( asPop()->toNumber()+1.0 );
	}
	void AsAgRuntime::decrement() {
		push( asPop()->toNumber()-1.0 );
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
} /* Swf */ 
