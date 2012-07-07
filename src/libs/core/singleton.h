//!-----------------------------------------------------
//!
//! \file singleton.h
//! a simple singleton class
//! \todo Thread safety
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_SINGLETON_H
#define WIERD_CORE_SINGLETON_H


namespace Core
{

template< class T>
class Singleton
{
public:
	static void init( void ) 
	{
		CORE_ASSERT( s_pSingleton == NULL && "Singleton already created" );
		s_pSingleton = CORE_NEW T();
	}
	static void shutdown( void ) 
	{
		CORE_DELETE( s_pSingleton );
		s_pSingleton = 0;
	}
	static T* get() 
	{
		CORE_ASSERT( s_pSingleton != NULL && "Singleton not created" );
		return s_pSingleton;
	}
	static bool exists() 
	{
		return (s_pSingleton != NULL); 
	}
protected:
	Singleton<T>() {}
	~Singleton<T>() {}
		
	static T* s_pSingleton;
};

template< class T > T* Singleton<T>::s_pSingleton = NULL;


}	//namespace Core

#endif
