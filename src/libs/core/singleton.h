//!-----------------------------------------------------
//!
//! \file singleton.h
//! a simple singleton class
//! \todo Thread safety
//!
//!-----------------------------------------------------

#pragma once

#ifndef YOLK_CORE_SINGLETON_H_
#define YOLK_CORE_SINGLETON_H_


namespace Core
{

template< class T, bool derivedOnly = false >
class Singleton
{
public:
	static void init( void )  {
		CORE_ASSERT( s_pSingleton == nullptr && "Singleton already created" );
		CORE_ASSERT( derivedOnly == false && "This singleton is meant for derived classes only" );
		s_pSingleton = CORE_NEW T();
	}
	static void derivedInit( T* derived ) {
		CORE_ASSERT( s_pSingleton == nullptr && "Singleton already created" );
		s_pSingleton = derived;
	}

	static void shutdown( void ) {
		CORE_DELETE( s_pSingleton );
		s_pSingleton = 0;
	}
	static T* get() {
		CORE_ASSERT( s_pSingleton != nullptr && "Singleton not created" );
		return s_pSingleton;
	}
	static T& getr() {
		CORE_ASSERT( s_pSingleton != nullptr && "Singleton not created" );
		return *s_pSingleton;
	}
	static bool exists() {
		return (s_pSingleton != nullptr); 
	}
protected:
	Singleton<T,derivedOnly>() {}
	~Singleton<T,derivedOnly>() {}
		
	static T* s_pSingleton;
};

template< class T, bool derivedOnly > T* Singleton< T, derivedOnly >::s_pSingleton = nullptr;


}	//namespace Core

#endif
