/*
 *  AsObject.h
 *  SwfPreview
 *
 *  Created by Deano on 19/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef ASOBJECT_H_4OQQI5Z0
#define ASOBJECT_H_4OQQI5Z0

namespace Swf {
	namespace CodeGen { 	class AsFuncBase; }
namespace AutoGen {
	class AsAgRuntime;
	
	enum AsPrimitiveType {
		APT_UNDEFINED,
		APT_NULL,
		APT_BOOLEAN,
		APT_NUMBER,
		APT_STRING,
		APT_OBJECT, // Technially by ECMA 252 object isn't a primitive type...
		APT_FUNCTION // 
	};
	
	class AsClass;
	class AsObject;
	typedef AsObject* AsObjectHandle;
	
	class AsObject : public Core::GcBase {
	public:		
		AsObject()
#if DEBUG
			: type( APT_OBJECT )
#endif
		{
			prototype = s_objectPrototype;
		}
		

		virtual AsPrimitiveType Type() {
#if DEBUG
			assert( type ==  APT_OBJECT );
#endif
			return APT_OBJECT;
		}
		
		virtual bool Is( AsObjectHandle _b );
		virtual void Construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
		virtual bool ToBoolean() { return false; }		
		virtual std::string ToString() { return ""; }
		virtual double ToNumber() { return 0; }
		virtual int ToInteger() { return (int) ToNumber(); }

		AsObjectHandle CallMethod( AsAgRuntime* _runtime, const std::string& _name, int _numParams, AsObjectHandle* _params ) {
			return CallMethodOn(_runtime, this, _name, _numParams, _params );
		}
		virtual AutoGen::AsObjectHandle GetProperty( const std::string& _name );
		virtual void SetProperty( const std::string& _name, AutoGen::AsObjectHandle _handle );
		virtual bool HasOwnProperty( const std::string& _name );
		
	protected:
		friend class AsObjectFactory;
		AsObject( AsPrimitiveType _type )
		#if DEBUG
					: type( _type )
		#endif
		{}

		// use by prototypes to call there method on another object
		virtual AsObjectHandle CallMethodOn( AsAgRuntime* _runtime, AsObjectHandle _this, const std::string& _name, int _numParams, AsObjectHandle* _params );

		
		AsObjectHandle hasOwnProperty( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
		AsObjectHandle toString( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
		AsObjectHandle toNumber( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );

		AsObjectHandle prototype;
		static AsObjectHandle s_objectPrototype;
		
		typedef Core::gcmap<std::string, AutoGen::AsObjectHandle> PropertyMap;
		PropertyMap			properties;		
#if DEBUG
		AsPrimitiveType type;
#endif
	};	
	
	class AsObjectUndefined : public AsObject {
	public:
		friend class AsObject;
		
		virtual AsPrimitiveType Type() {
#if DEBUG
			assert( type ==  APT_UNDEFINED );
#endif
			return APT_UNDEFINED;
		}
		
		virtual std::string ToString() { return "undefined"; }
		
		static AsObjectUndefined* Get() {
			static AsObjectUndefined* theOne = CORE_GC_NEW_ROOT_ONLY AsObjectUndefined();
			return theOne;
		}
		
	private:
		// private ctor, there is only one Undefined object
		// referenced by the static memver
		AsObjectUndefined() : AsObject(APT_UNDEFINED) {}
	};
	
	class AsObjectNull : public AsObject {
	public:
		virtual AsPrimitiveType Type() {
#if DEBUG
			assert( type ==  APT_NULL );
#endif
			return APT_NULL;
		}

		virtual std::string ToString() { return "null"; }

		static AsObjectNull* Get() {
			static AsObjectNull* theOne = CORE_GC_NEW_ROOT_ONLY AsObjectNull();
			return theOne;
		}
	private:
		// private ctor, there is only one Null object
		// referenced by the static memver
		AsObjectNull() : AsObject(APT_NULL) {}
	};
	
	class AsObjectBool : public AsObject {
	public:
		AsObjectBool( bool b ) :
			AsObject( APT_BOOLEAN ),
			value(b) {
			prototype = s_objectPrototype;			
		}
			
		virtual AsPrimitiveType Type() {
#if DEBUG
			assert( type ==  APT_BOOLEAN );
#endif
			return APT_BOOLEAN;
		}

		virtual std::string ToString() { return value ? "true" : "false"; }
		virtual bool ToBoolean() { return value; }
		virtual double ToNumber() { return value ? 1.0 : 0.0; }
		
		bool value;
	};
	
	class AsObjectString : public AsObject {
	public:		
		AsObjectString( const char* _text ) :
 			AsObject( APT_STRING ),
			value(_text) {
			prototype = s_stringPrototype;			
		}
			
		AsObjectString( const std::string& _text ) :
			AsObject( APT_STRING ),
			value(_text) {
				prototype = s_stringPrototype;				
			}
		
		virtual AsPrimitiveType Type() {
#if DEBUG
			assert( type ==  APT_STRING );
#endif
			return APT_STRING;
		}

		virtual void Construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
		
		virtual std::string ToString() { return value; }
		
		virtual double ToNumber();
		
		virtual bool ToBoolean();
		
		virtual AsObjectHandle GetProperty( const std::string& _name );
		
		
		AsObjectHandle length( AsAgRuntime* _runtime, int _numParams,  AsObjectHandle* _params );
		
		std::string		value;
		
		static AsObjectHandle s_stringPrototype;
	};
	
	class AsObjectNumber : public AsObject {
	public:	
		AsObjectNumber( float _f ) :
 			AsObject( APT_NUMBER ),
 			value( _f ) {
				prototype = s_objectPrototype;
			}

		AsObjectNumber( int32_t _i ) :
 			AsObject( APT_NUMBER ),
			value( _i ) {
				prototype = s_objectPrototype;
			};
				
		AsObjectNumber( double _d ) :
 			AsObject( APT_NUMBER ),
 			value( _d ) {
				prototype = s_objectPrototype;
			}

		virtual AsPrimitiveType Type() {
#if DEBUG
			assert( type ==  APT_NUMBER );
#endif
			return APT_NUMBER;
		}

		virtual std::string ToString();
		
		virtual bool ToBoolean() {
			// TODO epsilon
			if( value == 0.0) 
				return false;
			else 
				return true;
		}

		virtual double ToNumber() { return value; }
		virtual int ToInteger() { return (int) value; }

 		double value;
	};
	
} /* AutoGen */ 
	
} /* Swf */ 


#endif /* end of include guard: ASOBJECT_H_4OQQI5Z0 */

