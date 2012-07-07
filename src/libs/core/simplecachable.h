#pragma once
//!
//! @file simplecachable.h
//! @brief	An add-in incrementable counter, useful to know if something has changed
//!			so whether a cache entry needs refreshing
#if !defined( WIERD_CORE_SIMPLECACHABLE_H__ )
#define WIERD_CORE_SIMPLECACHABLE_H__

namespace Core { 
template< typename T, bool threadSafe >
class SimpleCachable {};

template< typename T>
class SimpleCachable< T, false > {
public:
	SimpleCachable() : counter(0) {}
	void invalidate() { counter++; }

	T getCounter() const { return counter; }
	void setCounter( T c ) { counter = c; }
private:
	mutable T counter;
};

};

#endif // WIERD_CORE_SIMPLECACHABLE_H__