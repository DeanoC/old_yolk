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
#include "../codegen/AsFunction.h"
#include "../../SwfMovieClip.h"
#include "../../SwfRuntimeUtils.h"
#include "../../SwfPlayer.h"
#include "../aslib/AsDate.h"
#include "../aslib/AsArray.h"
#include "AsObjectFactory.h"
#include "AsObjectFunction.h"
#include "AsAgRuntime.h"

namespace Swf {
	using namespace CodeGen;
	
	namespace AutoGen {
		static bool ParseVarTarget(const std::string& _path, std::string& path, std::string& var) {
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
		static void Tokenize(const std::string& str,std::vector<std::string>& tokens,
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
			isCaseSensitive( false )  {
			objectFactory = CORE_GC_NEW_ROOT_ONLY AsObjectFactory( this );
			objectFactory->Register( "Date", &AsDate::ConstructFunction );
			objectFactory->Register( "Array", &AsArray::ConstructFunction );
			
			functions[ "hasOwnProperty" ] = CORE_NEW AsAgFunction( &AsAgRuntime::hasOwnProperty );
		}
		
		AsAgRuntime::~AsAgRuntime() {
			Reset();
			CORE_GC_DELETE( objectFactory );
		}
		
		void AsAgRuntime::Reset() {
			while ( !stack.empty() ) {
			    stack.pop();
			}
			
//			locals.clear();
		}
		void AsAgRuntime::SetRoot( SwfMovieClip* _root ) {
			root = _root;
			isCaseSensitive = root->GetPlayer()->parser->fileVersion >= 7;
		}		
      static std::string s_targetseperators = "\\/";
      bool IsTargetAbsolute(const std::string& _path) {
			if (_path[0] == '\\' || _path[0] == '/')
				return true;
			else
				return false;
		}
		SwfFrameItem* AsAgRuntime::FindTarget(const std::string& _path) {
			if ( _path.empty() )
				return origTarget;

			std::vector<std::string> path;
			Tokenize( _path, path, s_targetseperators );
			if (IsTargetAbsolute(_path)) {
				return root->FindTarget(path, 0);
			} else {
				if( target->GetAsMovieClip() != NULL ) {
					return target->GetAsMovieClip()->FindTarget(path, 0);
				} else {
					return target;
				}
			}
		}
						
		void AsAgRuntime::GetVariable() {
			AsObjectHandle nameObj = AsPop();
			std::string name = nameObj->ToString();
						
			// try get a path for the name
			std::string pathS;
			std::string varS;
			if (ParseVarTarget(name, pathS, varS) == false) {
				// try locals first
				if( locals.find(name.c_str()) != locals.end()) {
					Push( locals[name.c_str()] );
				} else {
					// try members as not a local
					if(target)
						Push( target->GetProperty(name) );
				}
			} else {
				// TODO external lookup
				PushUndefined();
			}
		}
		
		void AsAgRuntime::SetVariable() {
			AsObjectHandle value = AsPop();
			AsObjectHandle nameObj = AsPop();
			std::string name = nameObj->ToString();
			
			// try get a path for the name
			std::string pathS;
			std::string varS;
			if (ParseVarTarget(name, pathS, varS) == false) {
				// try locals first
				if( locals.find(name) != locals.end()) {
					locals[ name ] = value;
				} else {
					// try members as not a local
					if(target)
						target->SetProperty(ToLowerIfReq(name, IsCaseSensitive()), value);
				}
			} else {
				// TODO external lookup
			}
		}
		void AsAgRuntime::GetMember() {
			AsObjectHandle nameObj = AsPop();
			AsObjectHandle obj = AsPop();
			std::string name = nameObj->ToString();
			AsObjectHandle ret = obj->GetProperty(name);
			Push( ret );
		}
		void AsAgRuntime::SetMember() {
			AsObjectHandle value = AsPop();
			AsObjectHandle nameObj = AsPop();
			AsObjectHandle obj = AsPop();
			std::string name = nameObj->ToString();
			obj->SetProperty( ToLowerIfReq(name, IsCaseSensitive()), value);
		}
		
		void AsAgRuntime::DefineLocal() {
			AsObjectHandle val = AsPop();
			AsObjectHandle nameObj = AsPop();
			std::string name = nameObj->ToString();
			locals[ name.c_str() ] = val;
		}
		
		void AsAgRuntime::DefineLocal2() {
			AsObjectHandle nameObj = AsPop();
			std::string name = nameObj->ToString();
			locals[ name.c_str() ] = AsObjectHandle( AsObjectUndefined::Get() );
		}
		
		void AsAgRuntime::NewObject() {
			AsObjectHandle nameObj = AsPop();
			AsObjectHandle argCountObj = AsPop();
			std::string str = nameObj->ToString();
			int numArgs = argCountObj->ToInteger();

			AsObjectHandle args[MAX_ARGS]; 
			for(int i=0;i < numArgs;++i) {
				if( i < MAX_ARGS ) {
					args[i] = AsPop();
				} else {
					Pop();
				}
			}
			
			AsObjectHandle no = objectFactory->Construct( str.c_str(), numArgs, args );
			Push(no);
		}
		
		void AsAgRuntime::CallMethod() {
			AsObjectHandle nameObj = AsPop();
			AsObjectHandle obj = AsPop();
			AsObjectHandle argCountObj = AsPop();
			std::string str;
			if( nameObj->Type() == APT_UNDEFINED ) {
				str = "()";
			} else {
			 	str = nameObj->ToString();
				if( str.empty() ) {
					str = "()";
				}
			}
			
			int numArgs = argCountObj->ToInteger();
			AsObjectHandle args[MAX_ARGS]; 
			for(int i=0;i < numArgs;++i) {
				if( i < MAX_ARGS ) {
					args[i] = AsPop();
				} else {
					Pop();
				}
			}
			AsObjectHandle ret = obj->CallMethod( this, str, numArgs, args );
			Push( ret );
		}
		void AsAgRuntime::Play() {
			if(target && target->type == FIT_MOVIECLIP ) {
				SwfMovieClip* mc = (SwfMovieClip*) target;
				mc->Play();	
			}
		}
		void AsAgRuntime::Stop() {
			if(target && target->type == FIT_MOVIECLIP ) {
				SwfMovieClip* mc = (SwfMovieClip*) target;
				mc->Stop();	
			}
		}
		void AsAgRuntime::NextFrame() {
			if(target && target->type == FIT_MOVIECLIP ) {
				SwfMovieClip* mc = (SwfMovieClip*) target;
				mc->NextFrame();
				mc->Stop();	
			}
		}
		void AsAgRuntime::PrevFrame() {
			if(target && target->type == FIT_MOVIECLIP ) {
				SwfMovieClip* mc = (SwfMovieClip*) target;
				mc->PrevFrame();
				mc->Stop();	
			}
		}
		
		void AsAgRuntime::GotoFrame( uint16_t _frame ) {
			if(target && target->type == FIT_MOVIECLIP ) {
				SwfMovieClip* mc = (SwfMovieClip*) target;
				mc->GotoFrame( _frame );
			}
		}

		void AsAgRuntime::GotoFrame( const AsObjectHandle& _frame ) {
			if(target && target->type == FIT_MOVIECLIP ) {
				SwfMovieClip* mc = (SwfMovieClip*) target;
				mc->GotoFrame( _frame->ToInteger() );
			}
		}
		
		void AsAgRuntime::Add() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			double a = oa->ToNumber();
			double b = ob->ToNumber();
			double aaddb = a + b;
			Push(aaddb);
		}
		void AsAgRuntime::Subtract() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			double a = oa->ToNumber();
			double b = ob->ToNumber();
			double asubb = a - b;
			Push(asubb);
		}
		void AsAgRuntime::Multiply() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			double a = oa->ToNumber();
			double b = ob->ToNumber();
			double ab = a * b;
			Push(ab);
		}
		void AsAgRuntime::Divide() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			double a = oa->ToNumber();
			double b = ob->ToNumber();
			double bovera = b / a;
			Push(bovera);
		}
		
		void AsAgRuntime::Equals() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			double a = oa->ToNumber();
			double b = ob->ToNumber();
			bool aeqb = a == b;
			Push(aeqb);
		}
		
		void AsAgRuntime::Less() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			double a = oa->ToNumber();
			double b = ob->ToNumber();
			bool blta = b < a;
			Push(blta);
		}
		
      void AsAgRuntime::And() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			double a = oa->ToNumber();
			double b = ob->ToNumber();
			bool aandb = (a != 0.0) && (b != 0.0);
			Push(aandb);
		}
		
      void AsAgRuntime::Or() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			double a = oa->ToNumber();
			double b = ob->ToNumber();
			bool aorb = (a != 0.0) || (b != 0.0);
			Push(aorb);
		}

      void AsAgRuntime::Not() {
			/* Swf 4 behavior I believe
			object oa = currentFrame.stack.Pop();
			double a = IsNumeric(oa) ? ToNumber(oa) : 0;
			bool nota = (a == 0.0);
			currentFrame.stack.Push(nota); */
			AsObjectHandle oa = AsPop();
			bool a = oa->ToBoolean();
			bool nota = !a;
			Push(nota);
		}

      void AsAgRuntime::StringEquals() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			bool eq = oa-> ToString() == ob->ToString();
			Push(eq);
		}

		void AsAgRuntime::StringLength() {
			AsObjectHandle oa = AsPop();
			int len = oa->ToString().size();
			Push((int32_t)len);
		}
		
		void AsAgRuntime::StringAdd() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			std::string badda = ob->ToString() + oa->ToString();
			Push(badda);
		}
		
		void AsAgRuntime::StringExtract() {
			int count = AsPop()->ToInteger();
			int index = AsPop()->ToInteger();
			std::string str = AsPop()->ToString();
			std::string substr = str.substr(index, count);
			Push(substr);
		}

		void AsAgRuntime::StringLess() {
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			bool lt = ob->ToString() < oa->ToString();
			Push(lt);
		}

		void AsAgRuntime::MBStringLength() {
			AsObjectHandle oa = AsPop();
			int len = oa->ToString().size();
			Push((int32_t)len);
		}

		void AsAgRuntime::MBStringExtract() {
			int count = AsPop()->ToInteger();
			int index = AsPop()->ToInteger();
			std::string str = AsPop()->ToString();
			std::string substr = str.substr(index, count);
			Push(substr);
		}
		void AsAgRuntime::ToInteger() {
			AsObjectHandle oa = AsPop();
			Push( (int32_t)oa->ToInteger() );
		}
		
		void AsAgRuntime::Add2() {
			// Add2 is addition following ECMA-262 Section 11.6.1
			// this is not complaint yet (tho handles string and numeric)
			AsObjectHandle oa = AsPop();
			AsObjectHandle ob = AsPop();
			if( oa->Type() == APT_STRING || ob->Type() == APT_STRING ) {
				std::string ret = ob->ToString() + oa->ToString();
				Push( ret );
			} else {
				double ret = ob->ToNumber() + oa->ToNumber();
				Push( ret );
			}			
		}
		void AsAgRuntime::GetProperty(){
			int index = AsPop()->ToInteger();
			std::string targstr = AsPop()->ToString();
			SwfFrameItem* targ = FindTarget(targstr);
			Push( targ->GetProperty(index) );
		}
		void AsAgRuntime::SetProperty(){
			AsObjectHandle oval = AsPop();
			int index = AsPop()->ToInteger();
			std::string targstr = AsPop()->ToString();
			SwfFrameItem* targ = FindTarget(targstr);
			targ->SetProperty(index, oval);
		}
		void AsAgRuntime::Trace() {
			std::string str = AsPop()->ToString();
			LOG(INFO) << str << "\n";
		}
		void AsAgRuntime::InitArray() {
			AsObjectHandle argCountObj = AsPop();
			
			int numArgs = argCountObj->ToInteger();
			AsObjectHandle args[1024]; 
			for(int i=0;i < numArgs;++i) {
				if( i < 1024 ) {
					args[i] = AsPop();
				} else {
					Pop();
				}
			}
			AsObjectHandle ret = objectFactory->Construct( "Array", numArgs, args );
			Push( ret );
		}
		void AsAgRuntime::SetTarget2() {
			std::string targ = AsPop()->ToString();
			SwfFrameItem* obj = FindTarget(targ);
			if(obj)
				target = obj; 
		}
		void AsAgRuntime::CallFunction() {
			AsObjectHandle nameObj = AsPop();
			AsObjectHandle argCountObj = AsPop();
			std::string str = nameObj->ToString();
			
			int numArgs = argCountObj->ToInteger();
			AsObjectHandle args[MAX_ARGS]; 
			for(int i=0;i < numArgs;++i) {
				if( i < MAX_ARGS ) {
					args[i] = AsPop();
				} else {
					Pop();
				}
			}
			
			AsFunctions::iterator it = functions.find( str );
			if( it != functions.end() ) {
				Push( it->second->Call( this, numArgs, args ) );
			} else {
				PushUndefined();
			}			
		}
		
		AsObjectHandle AsAgRuntime::hasOwnProperty( int _numParams, AsObjectHandle* _params ) {
			assert(_numParams == 1 );
			AsObjectHandle nameObj = _params[0];
			std::string name = nameObj->ToString();
			bool ret = target->HasOwnProperty( name );
			return CORE_NEW AsObjectBool(ret);
		}
		
		bool AsAgRuntime::EqualEcma262_11_9_3(AsObjectHandle _x, AsObjectHandle _y) {
			// Ecma 262 11.9.3 equality
			if( _x->Type() == _y->Type() ) {
				// same types
				switch( _x->Type() ) {
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
				if ( 	(_x->Type() == APT_UNDEFINED && _y->Type() == APT_NULL) ||
				 		(_x->Type() == APT_NULL && _y->Type() == APT_UNDEFINED) ) {
							return true;					
				} else if( _x->Type() != APT_OBJECT && _y->Type() != APT_OBJECT ) {
					return (_x->ToNumber() == _y->ToNumber() );
				} else {
					// TODO proper object coerce (calls involved??)
					return (_x->ToNumber() == _y->ToNumber() );
				}
			}

			
		}
		void AsAgRuntime::Equal2() {
			AsObjectHandle x = AsPop();
			AsObjectHandle y = AsPop();
			Push( new AsObjectBool( EqualEcma262_11_9_3(x,y) ) );
		}
		
		void AsAgRuntime::DefineLocalFunction( const AsObjectHandle& _func ) {
			if( _func->Type() == APT_FUNCTION ) {
				Push( _func );
			} else {
				PushUndefined();
			}
		}
		
		void AsAgRuntime::DefineFunction( const AsObjectHandle& _name, const AsObjectHandle& _func ) {
			if( _func->Type() == APT_FUNCTION ) {
				target->SetProperty( _name->ToString(), _func );
			} else {
				PushUndefined();
			}
		}
		void AsAgRuntime::ToNumber() {
			Push( AsPop()->ToNumber() );
		}
		void AsAgRuntime::ToString() {
			Push( AsPop()->ToString() );
		}
		void AsAgRuntime::Increment() {
			Push( AsPop()->ToNumber()+1.0 );
		}
		void AsAgRuntime::Decrement() {
			Push( AsPop()->ToNumber()-1.0 );
		}
		void AsAgRuntime::PushDuplicate() {
			stack.push( stack.top() );
		}
		void AsAgRuntime::SwapStack() {
			AsObjectHandle a = AsPop();
			AsObjectHandle b = AsPop();
			Push( a );
			Push( b );
		}
		void AsAgRuntime::Less2() {
			
		}
		void AsAgRuntime::Greater() {
			
		}
		
	} /* AutoGen */ 
} /* Swf */ 
