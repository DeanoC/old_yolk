#pragma once
/** \file property.h

(c) 2012 Dean Calver
 */

#if !defined( MESH_MOD_PROPERTY_H_ )
#define MESH_MOD_PROPERTY_H_

#include <boost/any.hpp>

namespace MeshMod {

	class Property {
	public:
		Property( const std::string& _name, const boost::any _val ) :
			name( _name ),
			value( _val ) {}

		const std::string& getName() const { return name; }
		const boost::any getValue() const { return value; }

		template< typename T>
		bool isType() const { return value.type() == typeid(T); }

		template< typename T>
		const T getAs() const { 
			return boost::any_cast<T>(value); 
		}

	protected:
		const std::string 	name;
		const boost::any	value;
	};
	typedef std::shared_ptr<Property> 	PropertyPtr;

}

#endif
