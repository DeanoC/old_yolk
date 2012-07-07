///-------------------------------------------------------------------------------------------------
/// \file	core\standard.h
///
/// \brief	Declares the standard members and classes of Core namespace. 
///
/// \details	
/// To ease development and get for 'free' as many high quality implementations as possible wierd 
/// provides access to most/all of the C++ standard library, TR1 extensions and useful boost 
/// libraries by default.
/// Whilst there are numerous reason why these can be bad, wierd is designed to be used by coders
/// who know when to use and when to avoid. As such providing a rich set of base libraries is the 
/// right choice.
/// 
/// To work around various issues related to STL usage, we import the entire STL into Core, patches 
/// as required to give a platform independent version. Currently up to tr1 is supported, tr2 soon. 
/// All tr's are imported into root Core namespace no need to remember which edition what was added 
/// to STL.
///
/// c++11'ism gently use to allow the compilers to keep up, MS10/11 and GCC 4.5 very good support
///
/// With a few exceptions all STL and TR1 features are support.
/// TR2 support
///		- threads (in core/thread.h) included form here
///		- system
/// The not supported exceptions are
///		- <numerics> Not helpful as non SIMD /todo Core replacement
///		- std::wstring Wierd is UTF8 based so wstring not required
///		- tr1 complex cmath functions.
/// Boost libraries considered standard are: (excluding TR1 ones)
///		- pool_allocator
///		- flyweight
///		- minmax
///		- iterator (not all are currently brought in, just cos I haven't seen a need)
///		- swap
///		- string algo
///		- static_assert (used for portable CORE_STATIC_ASSERT)
///		- asio
///		- scoped_array
///      - foreach (due to VC10 not supporting c++11 range for
///		- mpl (note you have to include boost/mpl/XXX.hpp) yourself as so many of them
/// To use simple replace std:: or std::tr1:: boost, with Core:: and viola will work as you expect.
///
/// C++11 support - 
///		= auto keyword (no libray)
///		- std::unique_ptr -> included if _HAS_CPP0X is defined
/// Known work around.
/// Lack of tr1 - 95% supported via boost
/// MS crappy vector - replaced with alignedvector
/// 
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CORE_STANDARD_H_
#define CORE_STANDARD_H_

// stl altogether (this file will hurt non PCH based compilers!)
#include <algorithm>
#include <bitset>
#include <complex>
#include <deque>
#include <functional>
#include <iterator>
#include <istream>
#include <list>
#include <map>
#include <memory>
#include <limits>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <sstream>
#include <utility>
#if COMPILER == MS_COMPILER && !defined( _HAS_CPP0X )

// !EVIL WORKS HERE!
#define alignedvector vector
#	include "core/aligned_vector.h"
#undef alignedvector

#else
#	include <vector>
#endif

#include <fstream>
// these are auto forwarding to the tr1 implementations of the platform if it supports them
#	include <functional>
#	include <memory>
#	include <type_traits>
#	include <random>
#	include <tuple>
#	include <utility>
#	include <array>
#	include <regex>
#	include <complex>
#	include <unordered_map>
#	include <unordered_set>

#if defined( USE_GC )
#	include "gc_allocator.h"
#endif

// TODO replace default allocator with one that goes through CORE_NEW/DELETE
#include <boost/pool/pool.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/flyweight.hpp>
#include <boost/flyweight/key_value.hpp>
#include <boost/algorithm/minmax.hpp>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/linux_error.hpp>
#include <boost/system/windows_error.hpp>
#include <boost/swap.hpp>
#include <boost/static_assert.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/msm/front/euml/operator.hpp>
#include <boost/msm/front/euml/state_grammar.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/foreach.hpp>
#include "core/aligned_vector.h"
#include "core/thread.h"

// tbb concurrent containers
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_vector.h"

namespace Core
{

#define STRING_CTORS_DECLARE(x,y,z)															\
	class x : public std::basic_string< y , std::char_traits< y >, z < y > > {			\
		public: 																			\
		typedef std::basic_string< y , std::char_traits< y >, z < y > > base_type;		\
		x (){};																				\
		x(base_type const & other);															\
		explicit x(allocator_type const & al);												\
		x(x const & other, size_type off, size_type count = npos);							\
		x(x const & other, size_type off, size_type count, allocator_type const & al);		\
		x(value_type const * ptr, size_type count);											\
		x(value_type const * ptr, size_type count, allocator_type const & al);				\
		x(value_type const * ptr);															\
		x(value_type const * ptr, allocator_type const & al);								\
		x(size_type count, value_type ch);													\
		x(size_type count, value_type ch, allocator_type const & al);						\
		template <typename InIt> inline x(InIt first, InIt last);							\
		template <typename InIt> inline x(InIt first, InIt last, allocator_type const & al); \
		x & operator=(value_type const * ptr);												\
		x & operator=(value_type ch);														\
	}
#define STRING_CTORS_IMPL(x)																\
	inline x :: x(base_type const & other) : base_type(other) {}							\
	inline x :: x(allocator_type const & al) : base_type(al) {}								\
	inline x :: x(x const & other, size_type off, size_type count) :						\
								base_type(other, off, count) {}								\
	inline x :: x(x const & other, size_type off, size_type count, allocator_type const & al) : \
								base_type(other, off, count, al) {}							\
	inline x :: x(value_type const * ptr, size_type count) : base_type(ptr, count) {}		\
	inline x :: x(value_type const * ptr, size_type count, allocator_type const & al) :		\
								base_type(ptr, count, al) {}								\
	inline x :: x(value_type const * ptr) : base_type(ptr) {}								\
	inline x :: x(value_type const * ptr, allocator_type const & al) : base_type(ptr, al) {} \
	inline x :: x(size_type count, value_type ch) : base_type(count, ch) {}					\
	inline x :: x(size_type count, value_type ch, allocator_type const & al) :				\
								base_type(count, ch, al) {}									\
	template <typename InIt> inline x :: x(InIt first, InIt last) : base_type(first, last) {}	\
	template <typename InIt> inline x :: x(InIt first, InIt last, allocator_type const & al) :  \
								base_type(first, last, al) {}								\
	inline x & x :: operator=(value_type const * ptr) { base_type::operator=(ptr); return *this; } \
	inline x & x :: operator=(value_type ch) { base_type::operator=(ch); return *this; }
	
	// forward decl	
	class gcstring;
	class poolstring;

#if defined( USE_GC )
	template<typename X> class gcvector : public std::vector<X, gc_allocator<X> > {};
	template<typename KEY, typename VAL> class gcmap : public std::map<KEY, VAL, std::less<KEY>, 
												gc_allocator<std::pair<const KEY,VAL> > > {};	
	template<typename X> class gcstack : public std::stack<X, gcvector<X> > {};
	template<typename X> class gclist : public std::list<X, gc_allocator<X> > {};
	template<typename KEY, typename VAL> class gcunordered_map : public 
				std::unordered_map<KEY, VAL, std::hash<KEY>, std::equal_to<KEY>, 
				gc_allocator<std::pair<const KEY,VAL> > > {};	

	STRING_CTORS_DECLARE( gcstring, char, gc_allocator );
	template<typename X> class gctraceablevector : public std::vector<X, traceable_allocator<X> >{};
	template<typename KEY, typename VAL> class gctraceablemap : public std::map<KEY, VAL, 
				std::less<KEY>, traceable_allocator<std::pair<const KEY,VAL> > > {};	
	template<typename X> class gctraceablestack : public std::stack<X, gctraceablevector<X> > {};
	template<typename X> class gctraceablelist : public std::list<X, traceable_allocator<X> > {};
	template<typename KEY, typename VAL> class gctraceableunordered_map : public 
				std::unordered_map<KEY, VAL, std::hash<KEY>, std::equal_to<KEY>, 
				gc_allocator<std::pair<const KEY,VAL> >  > {};	
#else
	template<typename X> class gcvector : public std::vector<X, std::allocator<X> > {};
	template<typename KEY, typename VAL> class gcmap : public std::map<KEY, VAL, std::less<KEY>, 
				std::allocator<std::pair<const KEY,VAL> > > {};	
	template<typename X> class gcstack : public std::stack<X, gcvector<X> > {};
	template<typename X> class gclist : public std::list<X, std::allocator<X> > {};
	template<typename KEY, typename VAL> class gcunordered_map : public 
				std::unordered_map<KEY, VAL, std::hash<KEY>, std::equal_to<KEY>, 
				std::allocator<std::pair<const KEY,VAL> >  > {};	
	STRING_CTORS_DECLARE( gcstring, char, std::allocator );
	template<typename X> class gctraceablevector : public std::vector<X> {};
	template<typename KEY, typename VAL> class gctraceablemap : public 
				std::map<KEY, VAL, std::less<KEY> > {};	
	template<typename X> class gctraceablestack : public std::stack<X, gctraceablevector<X> > {};
	template<typename X> class gctraceablelist : public std::list<X> {};
	template<typename KEY, typename VAL> class gctraceableunordered_map : public 
				std::unordered_map<KEY, VAL, std::hash<KEY>, std::equal_to<KEY> > {};	
#endif

	template<typename X> class poolvector : public std::vector<X, boost::pool_allocator<X> > {};
	template<typename KEY, typename VAL> class poolmap : public std::map<KEY, VAL, 
				std::less<KEY>, boost::pool_allocator<std::pair<const KEY,VAL> > > {};	
	template<typename X> class poolstack : public std::stack<X, poolvector<X> > {};
	template<typename X> class poollist : public std::list<X, boost::pool_allocator<X> > {};
	template<typename KEY, typename VAL> class poolunordered_map : public 
				std::unordered_map<KEY, VAL, std::hash<KEY>, std::equal_to<KEY>, 
				boost::pool_allocator<std::pair<const KEY,VAL> >  > {};	
	STRING_CTORS_DECLARE( poolstring, char, boost::pool_allocator );

	STRING_CTORS_IMPL( gcstring )
	STRING_CTORS_IMPL( poolstring )
	#undef STRING_CTORS_DECLARE
	#undef STRING_CTORS_IMPL

}	//namespace std

#endif // end CORE_STANDARD_H
