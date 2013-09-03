#pragma once
#ifndef YOLK_CORE_FREELIST_H_
#define YOLK_CORE_FREELIST_H_ 1

namespace Core {

// enabled a view of free list chain in debug
#define USE_DEBUG_CRACKER

// NOT MT safe in general
// keeps freelist always big enough for all data
// TODO make this std-a-like interface 
template< typename TYPE, typename INDEX_TYPE = uintptr_t >
class ExplicitFreeList {
public:
	typedef typename std::vector< TYPE >::value_type value_type;
	typedef typename std::vector< TYPE >::size_type size_type;
	typedef typename std::vector< TYPE >::difference_type difference_type;
	typedef typename std::vector< TYPE >::pointer pointer;
	typedef typename std::vector< TYPE >::const_pointer const_pointer;
	typedef typename std::vector< TYPE >::reference reference;
	typedef typename std::vector< TYPE >::const_reference const_reference;	

	ExplicitFreeList() : capacity(0), current(0), currentFree(0) {}

	explicit ExplicitFreeList( size_type _count ) : data( _count ), freelist( _count ), currentFree( _count ) {
				for( size_type i = 0; i < _count; ++i ) { 
					freelist[i] = i; 
				}

			}

	INDEX_TYPE push( const value_type& _val ) {
		INDEX_TYPE index = alloc();
		data[ index ] = _val;
		return index;
	}

	INDEX_TYPE alloc() {
		INDEX_TYPE index;
		if( currentFree == 0 ) {
			resize( (data.size() * 2) + 1 );
		}
		CORE_ASSERT( currentFree != 0 );
		index = freelist[--currentFree];
		CORE_ASSERT( index != INVALID_INDEX );
		IF_DEBUG( freelist[ currentFree ] = INVALID_INDEX );

		return index;
	}

	void resize( const size_type _count ) {
		size_type oldcount = data.size();
		CORE_ASSERT( _count > oldcount );
		data.resize( _count );
		freelist.resize( _count );
		for( size_type i = oldcount; i < _count; ++i ) { 
			IF_DEBUG( freelist[i] = INVALID_INDEX );
			freelist[currentFree++] = i; 
		}
	}

	bool empty() const { return currentFree == freelist.size(); }

	void erase( const INDEX_TYPE _index ) {
		CORE_ASSERT( _index < freelist.size() );
		freelist[ currentFree++ ] = _index;
	}

	TYPE& get( const INDEX_TYPE _index ) { return data[_index]; }
	const TYPE& get( const INDEX_TYPE _index ) const { return data[_index]; }

private:
	std::vector< TYPE >				data;
	std::vector< INDEX_TYPE > 		freelist;
	INDEX_TYPE						currentFree;
};

// NOT MT safe in general
// keeps freelist always big enough for all data
// TODO make this std-a-like interface 
template< typename TYPE,typename INDEX_TYPE = uintptr_t >
class FreeList {
public:
	static_assert( sizeof(TYPE) >= sizeof(INDEX_TYPE), "INDEX_TYPE must be less than size in bytes of the TYPE being stored" );
	static const INDEX_TYPE INVALID_INDEX = ~0;

	typedef typename std::vector< TYPE >::value_type value_type;
	typedef typename std::vector< TYPE >::size_type size_type;
	typedef typename std::vector< TYPE >::difference_type difference_type;
	typedef typename std::vector< TYPE >::pointer pointer;
	typedef typename std::vector< TYPE >::const_pointer const_pointer;
	typedef typename std::vector< TYPE >::reference reference;
	typedef typename std::vector< TYPE >::const_reference const_reference;

	FreeList() : capacity(0), current(0), currentFree(0) {}

	explicit FreeList( size_type _count ) : data( _count ) {

#if defined(USE_DEBUG_CRACKER)
		IF_DEBUG( debugCracker = reinterpret_cast<DebugCracker*>( &data[0] ) );
#endif
		
		// make each entry point to the next one
		for( size_type i = 0; i < _count-1; ++i ) {
			INDEX_TYPE* freeptr = reinterpret_cast<INDEX_TYPE*>( &data[i] );
			*freeptr = i + 1;
		}
		// tail of the list
		INDEX_TYPE* freeptr = reinterpret_cast<INDEX_TYPE*>( &data[_count-1] );
		*freeptr = INVALID_INDEX;
		// head of the list
		freeHead = 0;
	}

	INDEX_TYPE push( const value_type& _val ) {
		INDEX_TYPE index = alloc();
		data[ index ] = _val;
		return index;
	}

	INDEX_TYPE alloc() {
		if( freeHead == INVALID_INDEX ) {
			resize( (data.size() * 2) + 1 );
		}
		CORE_ASSERT( freeHead != INVALID_INDEX );

		INDEX_TYPE oldHead = freeHead;
		freeHead = *reinterpret_cast<INDEX_TYPE*>( &data[ freeHead ] );
		return oldHead;
	}

	void resize( const size_type _count ) {
		size_type oldcount = data.size();
		CORE_ASSERT( _count > oldcount );
		data.resize( _count );
#if defined(USE_DEBUG_CRACKER)
		IF_DEBUG( debugCracker = reinterpret_cast<DebugCracker*>( &data[0] ) );
#endif
		for( size_type i = oldcount; i < _count-1; ++i ) { 
			*reinterpret_cast<INDEX_TYPE*>( &data[i] ) = i + 1;
		}
		*reinterpret_cast<INDEX_TYPE*>( &data[ _count -1 ] ) = freeHead;
		freeHead = oldcount;
	}

	bool empty() const { return currentFree == freelist.size(); }

	void erase( const INDEX_TYPE _index ) {
		*reinterpret_cast<INDEX_TYPE*>( &data[ _index ] ) = freeHead;
		freeHead = _index;
	}

	TYPE& get( const INDEX_TYPE _index ) { return data[ _index ]; }
	const TYPE& get( const INDEX_TYPE _index ) const { return data[ _index ]; }

private:
	std::vector< TYPE >				data;
	INDEX_TYPE						freeHead;
#if DEBUG_LEVEL > DEBUG_NONE && defined(USE_DEBUG_CRACKER)
	union DebugCracker {
		TYPE						data;
		INDEX_TYPE					index;
	} *debugCracker;
#endif

};

} // end Core namespace

#endif