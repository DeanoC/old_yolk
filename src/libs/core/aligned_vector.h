///-------------------------------------------------------------------------------------------------
/// \file	core\aligned_vector.h
///
/// \brief	Declares the aligned vector class. 
///
/// \details	
///		aligned_vector description goes here
///
/// \remark	Copyright (c) 2010 Zombie House Studios. All rights reserved.
/// \remark	mailto://deano@zombiehousestudios.com
///
/// \todo	Fill in detailed file description. 
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Wierd - borrowed and slightly altered from Bullet. Thanks Erwin :)

Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#ifndef WIERD_CORE_ALIGNED_VECTOR_H
#define WIERD_CORE_ALIGNED_VECTOR_H


#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/iterator/reverse_iterator.hpp>


namespace Core {


///The btAlignedAllocator is a portable class for aligned memory allocations.
///Default implementations for unaligned and aligned allocations can be overridden by a custom allocator using btAlignedAllocSetCustom and btAlignedAllocSetCustomAligned.
template < typename T , unsigned Alignment >
class btAlignedAllocator {
	
	typedef btAlignedAllocator< T , Alignment > self_type;
	
public:

	//just going down a list:
	btAlignedAllocator() {}
	/*
	btAlignedAllocator( const self_type & ) {}
	*/

	template < typename Other >
	btAlignedAllocator( const btAlignedAllocator< Other , Alignment > & ) {}

	typedef const T*         const_pointer;
	typedef const T&         const_reference;
	typedef T*               pointer;
	typedef T&               reference;
	typedef T                value_type;

	pointer       address   ( reference ref ) const                           { return &ref; }
	const_pointer address   ( const_reference ref ) const                     { return &ref; }
	pointer       allocate  ( const size_t n, const_pointer * hint = 0 )
	{
		(void)hint;
		return reinterpret_cast< pointer >( CORE_ALIGNED_ALLOC( sizeof(value_type) * n , Alignment ) );
	}
	void          construct ( pointer ptr , const value_type & value ) 
	{ 
		CORE_PLACEMENT_NEW (ptr) value_type( value ); 
	}
	void          deallocate( pointer ptr, const size_t n ) 
	{
		CORE_ALIGNED_FREE( reinterpret_cast< void * >( ptr ) );
	}
	void          destroy   ( pointer ptr ){ ptr->~value_type(); }
	

	template < typename O > struct rebind {
		typedef btAlignedAllocator< O , Alignment > other;
	};
	template < typename O >
	self_type & operator=( const btAlignedAllocator< O , Alignment > & ) { return *this; }

	friend bool operator==( const self_type & , const self_type & ) { return true; }
};

///If the platform doesn't support placement new, you can disable BT_USE_PLACEMENT_NEW
///then the btAlignedObjectArray doesn't support objects with virtual methods, and non-trivial constructors/destructors
///You can enable BT_USE_MEMCPY, then swapping elements in the array will use memcpy instead of operator=
///see discussion here: http://continuousphysics.com/Bullet/phpBB2/viewtopic.php?t=1231 and
///http://www.continuousphysics.com/Bullet/phpBB2/viewtopic.php?t=1240

#define BT_USE_MEMCPY 1

template< typename T >
class alignedvector_iterator :
	public boost::iterator_adaptor < 
		alignedvector_iterator<T>, T*, boost::use_default, boost::random_access_traversal_tag 
	>
{
private:
	typedef boost::iterator_adaptor<
		alignedvector_iterator<T>, T*, boost::use_default, boost::random_access_traversal_tag
	> super_t;

public:
	alignedvector_iterator() : super_t( 0 ) {}

	explicit alignedvector_iterator( T* p ) : super_t( p ) {}

	template <class OtherT>
		alignedvector_iterator( 
			alignedvector_iterator<OtherT> const& other, 
			typename boost::enable_if_convertible< OtherT, T >::type* = 0
		) : super_t( other.base() ) {}
private:
	friend class boost::iterator_core_access;
	void increment() { this->base_reference() = this->base()+1; }
};


///The btAlignedObjectArray template class uses a subset of the stl::vector interface for its methods
///It is developed to replace stl::vector to avoid portability issues, including STL alignment issues to add SIMD/SSE data
template <typename T, class _Ax = btAlignedAllocator<T , 16>, bool BT_USE_PLACEMENT_NEW = true> 
class alignedvector {
private:
	_Ax	m_allocator;

	size_t				m_size;
	size_t				m_capacity;
	T*					m_data;
	//PCK: added this line
	bool				m_ownsMemory;

public:

	typedef		alignedvector_iterator< T >					iterator;
	typedef		alignedvector_iterator< const T >			const_iterator;
	typedef		boost::reverse_iterator< iterator >			reverse_iterator;
	typedef		boost::reverse_iterator< const_iterator >	const_reverse_iterator;
	typedef T*				pointer;
	typedef const T*		const_pointer;
	typedef	T&				reference;
	typedef	const T&		const_reference;
	typedef T				value_type;

protected:
	CORE_FORCE_INLINE	size_t allocSize( size_t size ) {
		return (size ? size * 2 : 1);
	}
	CORE_FORCE_INLINE	void copy(const_iterator start, const_iterator end, iterator dest) const {
		for ( const_iterator i = start; i < end; ++i) {
			if( BT_USE_PLACEMENT_NEW ) {
				CORE_PLACEMENT_NEW(&(*dest)) T(*i);
			} else {
				*dest = *i;
			}
			++dest;
		}
	}

	CORE_FORCE_INLINE	void init() 
	{
		//PCK: added this line
		m_ownsMemory = true;
		m_data = 0;
		m_size = 0;
		m_capacity = 0;
	}

	CORE_FORCE_INLINE	void	destroy(iterator first, iterator last) 
	{
		for ( iterator i=first; i<last;i++) {
			(*i).~T();
		}
	}

	CORE_FORCE_INLINE	iterator allocate(const size_t size) 
	{
		if (size) {
			return iterator( m_allocator.allocate(size) );
		} else {
			return iterator();
		}
	}

	CORE_FORCE_INLINE	void	deallocate() 
	{
		if(m_data)	{
			//PCK: enclosed the deallocation in this block
			if (m_ownsMemory) {
				m_allocator.deallocate(m_data, sizeof(T) );
			}
			m_data = 0;
		}
	}

public:

	alignedvector() {
		init();
	}

	~alignedvector() {
		clear();
	}

	explicit alignedvector( const_reference otherArray ) 
	{
		init();

		size_t otherSize = otherArray.size();
		resize ( otherSize );
		otherArray.copy( begin(), end(), begin());
	}
	template< class fwdIter >
	alignedvector( fwdIter start, fwdIter last )
	{
		init();
		resize( std::distance( start, last ) );
		std::copy( start, last, begin() );
	}

	iterator begin() { return iterator( m_data ); }
	const_iterator begin()  const { return const_iterator( m_data ); }
	iterator end() { return iterator( m_data+size() ); }
	const_iterator end() const { return const_iterator( m_data+size() ); }

	reverse_iterator rbegin() { return reverse_iterator( end() ); }
	reverse_iterator rend() { return reverse_iterator( begin() ); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator( end() ); }
//	const_reverse_iterator rend() const { return const_reverse_iterator( begin() ); }

	void erase( iterator remove ) 
	{
		destroy( remove, remove+1 );
		std::copy( remove+1, end(), remove );
	}

	//! \todo optimize this function more
	iterator insert( iterator iter, const_reference val )
	{
		size_t index = std::distance( iter, begin() );
		if( iter != end() ) 
		{
			resize( size()+1 );
			iter = begin();
			std::advance( iter, index );
			std::copy_backward( iter+1, end()-1, iter+1 );
			*iter = val;
		} else
		{
			resize( size()+1, val );
			iter = end() - 1;
		}

		return iter;
	}


	CORE_FORCE_INLINE bool empty() const {
		return size() == 0;
	}
	
	/// return the number of elements in the array
	CORE_FORCE_INLINE	size_t size() const {	
		return m_size;
	}
	
	CORE_FORCE_INLINE const T& at(int n) const {
		assert( n >= 0 );
		assert( n < size() );
		return m_data[n];
	}

	CORE_FORCE_INLINE T& at(int n) {
		assert( n >= 0 );
		assert( n < size() );
		return m_data[n];
	}

	CORE_FORCE_INLINE const T& operator[](int n) const {
		return m_data[n];
	}

	CORE_FORCE_INLINE T& operator[](int n) {
		return m_data[n];
	}
	
	///clear the array, deallocated memory. Generally it is better to use array.resize(0), to reduce performance overhead of run-time memory (de)allocations.
	CORE_FORCE_INLINE	void clear() 
	{
		destroy( begin(), end() );
		
		deallocate();
		
		init();
	}

	CORE_FORCE_INLINE	void pop_back() {
		m_size--;
		m_data[m_size].~T();
	}

	///resize changes the number of elements in the array. If the new size is larger, the new elements will be constructed using the optional second argument.
	///when the new number of elements is smaller, the destructor will be called, but memory will not be freed, to reduce performance overhead of run-time memory (de)allocations.
	CORE_FORCE_INLINE	void	resize(size_t newsize, const T& fillData=T()) {
		size_t curSize = size();

		if (newsize < curSize) {
			for(size_t i = newsize; i < curSize; i++) {
				m_data[i].~T();
			}
		} else {
			if (newsize > size()) {
				reserve(newsize);
			}
			if( BT_USE_PLACEMENT_NEW ) {
				for (size_t i=curSize;i<newsize;i++) {
					CORE_PLACEMENT_NEW ( &m_data[i]) T(fillData);
				}
			}
		}

		m_size = newsize;
	}

	CORE_FORCE_INLINE	T&  expandNonInitializing( ) {	
		size_t sz = size();
		if( sz == capacity() ) {
			reserve( allocSize(size()) );
		}
		m_size++;

		return m_data[sz];		
	}


	CORE_FORCE_INLINE	T&  expand( const T& fillValue=T()) {	
		int sz = size();
		if( sz == capacity() ) {
			reserve( allocSize(size()) );
		}
		m_size++;
		if( BT_USE_PLACEMENT_NEW ) {
			CORE_PLACEMENT_NEW (&m_data[sz]) T(fillValue); //use the in-place new (not really allocating heap memory)
		}

		return m_data[sz];		
	}


	CORE_FORCE_INLINE	void push_back(const T& _Val) {	
		int sz = size();
		if( sz == capacity() ) {
			reserve( allocSize(size()) );
		}
		
		if( BT_USE_PLACEMENT_NEW ) {
			CORE_PLACEMENT_NEW ( &m_data[m_size] ) T(_Val);
		} else {
			m_data[size()] = _Val;			
		}
		m_size++;
	}


	/// return the pre-allocated (reserved) elements, this is at least as large as the total number of elements,see size() and reserve()
	CORE_FORCE_INLINE	size_t capacity() const {	
		return m_capacity;
	}
	
	CORE_FORCE_INLINE	void reserve(size_t _Count) 
	{
		// determine new minimum length of allocated storage
		if (capacity() < _Count) 
		{	// not enough room, reallocate
			iterator s = allocate(_Count);

			copy( begin(), end(), s );

			destroy( begin(), end() );

			deallocate();
			
			//PCK: added this line
			m_ownsMemory = true;

			m_data = &(*s);
			
			m_capacity = _Count;

		}
	}


	struct less {
		bool operator() ( const T& a, const T& b ) {
			return ( a < b );
		}
	};

	template <typename L>
	void quickSortInternal(L CompareFunc,int lo, int hi) {
	//  lo is the lower index, hi is the upper index
	//  of the region of array a that is to be sorted
		int i=lo, j=hi;
		T x=m_data[(lo+hi)/2];

		//  partition
		do {    
			while (CompareFunc(m_data[i],x)) 
				i++; 
			while (CompareFunc(x,m_data[j])) 
				j--;
			if (i<=j)
			{
				swap(i,j);
				i++; j--;
			}
		} while (i<=j);

		//  recursion
		if (lo<j) 
			quickSortInternal( CompareFunc, lo, j);
		if (i<hi) 
			quickSortInternal( CompareFunc, i, hi);
	}


	template <typename L>
	void quickSort(L CompareFunc) {
		//don't sort 0 or 1 elements
		if (size()>1) {
			quickSortInternal(CompareFunc,0,size()-1);
		}
	}


	///heap sort from http://www.csse.monash.edu.au/~lloyd/tildeAlgDS/Sort/Heap/
	template <typename L>
	void downHeap(T *pArr, int k, int n,L CompareFunc) {
		/*  PRE: a[k+1..N] is a heap */
		/* POST:  a[k..N]  is a heap */
		
		T temp = pArr[k - 1];
		/* k has child(s) */
		while (k <= n/2) 
		{
			int child = 2*k;
			
			if ((child < n) && CompareFunc(pArr[child - 1] , pArr[child]))
			{
				child++;
			}
			/* pick larger child */
			if (CompareFunc(temp , pArr[child - 1]))
			{
				/* move child up */
				pArr[k - 1] = pArr[child - 1];
				k = child;
			}
			else
			{
				break;
			}
		}
		pArr[k - 1] = temp;
	} /*downHeap*/

	void swap( iterator left, iterator right)
	{
		char	temp[sizeof(T)];
		memcpy( temp, left, sizeof(T) );
		memcpy( left, right, sizeof(T) );
		memcpy( right, temp, sizeof(T) );
	}

	template <typename L>
	void heapSort(L CompareFunc)
	{
		/* sort a[0..N-1],  N.B. 0 to N-1 */
		int k;
		int n = m_size;
		for (k = n/2; k > 0; k--) 
		{
			downHeap(m_data, k, n, CompareFunc);
		}

		/* a[1..N] is now a heap */
		while ( n>=1 ) 
		{
			swap(0,n-1); /* largest of a[0..n-1] */


			n = n - 1;
			/* restore a[1..i-1] heap */
			downHeap(m_data, 1, n, CompareFunc);
		} 
	}

	///non-recursive binary search, assumes sorted array
	int	findBinarySearch(const T& key) const
	{
		int first = 0;
		int last = size();

		//assume sorted array
		while (first <= last) {
			int mid = (first + last) / 2;  // compute mid point.
			if (key > m_data[mid]) 
				first = mid + 1;  // repeat search in top half.
			else if (key < m_data[mid]) 
				last = mid - 1; // repeat search in bottom half.
			else
				return mid;     // found it. return position /////
		}
		return size();    // failed to find key
	}


	int	findLinearSearch(const T& key) const
	{
		int index=size();
		int i;

		for (i=0;i<size();i++)
		{
			if (m_data[i] == key)
			{
				index = i;
				break;
			}
		}
		return index;
	}

	void	remove(const T& key) {

		int findIndex = findLinearSearch(key);
		if (findIndex<size())
		{
			swap( findIndex,size()-1);
			pop_back();
		}
	}

	//PCK: whole function
	void initializeFromBuffer(void *buffer, int size, int capacity) {
		clear();
		m_ownsMemory = false;
		m_data = (T*)buffer;
		m_size = size;
		m_capacity = capacity;
	}

};


}	//namespace Core


#endif