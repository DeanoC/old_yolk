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
/// \remark	Copyright (c) 20110 Deano Ca;ver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CORE_STANDARD_H_
#define CORE_STANDARD_H_

// stl altogether (this file will hurt non PCH based compilers!)
#include <algorithm>

namespace Core
{	
	// Algorithms
	using std::adjacent_find;
	using std::binary_search;
	using std::copy;
	using std::copy_backward;
	using std::count;
	using std::count_if;
	using std::equal;
	using std::equal_range;
	using std::fill;
	using std::fill_n;
	using std::find;
	using std::find_end;
	using std::find_first_of;
	using std::find_if;
	using std::for_each;
	using std::generate;
	using std::generate_n;
	using std::includes;
	using std::inplace_merge;
	using std::iter_swap;
	using std::lexicographical_compare;
	using std::lower_bound;
	using std::make_heap;
	using std::max;
	using std::max_element;
	using std::merge;
	using std::min;
	using std::min_element;
	using std::mismatch;
	using std::next_permutation;
	using std::nth_element;
	using std::partial_sort;
	using std::partial_sort_copy;
	using std::partition;
	using std::pop_heap;
	using std::prev_permutation;
	using std::push_heap;
	using std::random_shuffle;
	using std::remove;
	using std::remove_copy;
	using std::remove_copy_if;
	using std::remove_if;
	using std::replace;
	using std::replace_copy;
	using std::replace_copy_if;
	using std::replace_if;
	using std::reverse;
	using std::reverse_copy;
	using std::rotate;
	using std::rotate_copy;
	using std::search;
	using std::search_n;
	using std::set_difference;
	using std::set_intersection;
	using std::set_symmetric_difference;
	using std::set_union;
	using std::sort;
	using std::sort_heap;
	using std::stable_partition;
	using std::stable_sort;
	using std::swap;
	using std::swap_ranges;
	using std::transform;
	using std::unique;
	using std::unique_copy;
	using std::upper_bound;
}

#include <bitset>
namespace Core
{	
	using std::bitset;
}

#include <complex>
namespace Core
{	
	// Complex Number Library
	using std::complex;
}

#include <deque>
namespace Core
{	
	using std::deque;
}

#include <functional>
namespace Core
{	
	// Function Objects
	using std::binary_function;
	using std::binary_negate;
	using std::binder1st;
	using std::binder2nd;
	using std::divides;
	using std::equal_to;
	using std::greater;
	using std::greater_equal;
	using std::less;
	using std::less_equal;
	using std::logical_and;
	using std::logical_not;
	using std::logical_or;
	using std::minus;
	using std::modulus;
	using std::negate;
	using std::not_equal_to;
	using std::plus;
	using std::pointer_to_binary_function;
	using std::pointer_to_unary_function;
	using std::multiplies;
	using std::unary_function;
	using std::unary_negate;

	// Function adaptors
	using std::bind1st;
	using std::bind2nd;
	using std::not1;
	using std::not2;
	using std::ptr_fun;
}

#include <iterator>
namespace Core
{	
	// Insert Iterators
	using std::back_insert_iterator;
	using std::back_inserter;
	using std::front_insert_iterator;
	using std::front_inserter;
	using std::insert_iterator;
	using std::inserter;
	// Iterator operations
	using std::advance;
	using std::distance;
	using std::iterator_traits;
	using std::iter_swap;
	using std::iterator;
	using std::reverse_iterator;
	using std::reverse_bidirectional_iterator;
	using std::input_iterator_tag;
	using std::output_iterator_tag;
	using std::forward_iterator_tag;
	using std::bidirectional_iterator_tag;
	using std::random_access_iterator_tag;
}

#include <istream>
namespace Core
{	
}

#include <list>
namespace Core
{	
	using std::list;
}

#include <map>
namespace Core
{	
	using std::map;
	using std::multimap;
}

#include <memory>
namespace Core
{	
	// Memory Handling Primitives
	using std::get_temporary_buffer;
	using std::return_temporary_buffer;
	// Memory Management
	using std::allocator;
	using std::auto_ptr;
	using std::raw_storage_iterator;
	using std::uninitialized_copy;
	using std::uninitialized_fill;
	using std::uninitialized_fill_n;
#if defined(_HAS_CPP0X)
	using std::unique_ptr;
#endif
}

#include <limits>
namespace Core
{	
	//Numeric Limits Library
	using std::numeric_limits;
}

#include <queue>
namespace Core
{	
	using std::queue;
	using std::priority_queue;
}

#include <set>
namespace Core
{	
	using std::set;
	using std::multiset;
}

#include <stack>
namespace Core
{	
	using std::stack;
}

#include <string>
namespace Core
{	
	//String Library
	using std::basic_string;
	using std::string;
	using std::char_traits;
}

#include <sstream>
namespace Core
{	
	// StrStream
	using std::ios_base;
	using std::ios;
	using std::basic_streambuf;
	using std::basic_stringbuf;
	using std::basic_istringstream;
	using std::basic_istream;
	using std::basic_ostringstream;
	using std::basic_ostream;
	using std::basic_stringstream;
	using std::basic_iostream;
	using std::streambuf;
	using std::istringstream;
	using std::istream;
	using std::stringstream;
	using std::iostream;
}

#include <utility>
namespace Core
{	
	//Utility Classes and operators
	using std::pair;
	using std::operator!=;
	using std::operator>;
	using std::operator<=;
	using std::operator>=;
	using std::swap;
	using std::swap_ranges;

}

#if COMPILER == MS_COMPILER && !defined( _HAS_CPP0X )

// !EVIL WORKS HERE!
#define alignedvector vector
#	include "core/aligned_vector.h"
#undef alignedvector

#else

#	include <vector>
namespace Core
{	
	using std::vector;	
}
#endif

#include <fstream>
namespace Core
{	
	using std::filebuf;	
	using std::ifstream;	
	using std::ofstream;	
	using std::fstream;	
}

// these are auto forwarding to the tr1 implementations of the platform if it supports them
#	include	<boost/tr1/functional.hpp>
#	include "boost/tr1/memory.hpp"
#	include "boost/tr1/type_traits.hpp"
#	include "boost/tr1/random.hpp"
#	include "boost/tr1/tuple.hpp"
#	include "boost/tr1/utility.hpp"
#	include "boost/tr1/array.hpp"
#	include "boost/tr1/regex.hpp"
#	include "boost/tr1/complex.hpp"
#	include <boost/tr1/unordered_map.hpp>
#	include <boost/tr1/unordered_set.hpp>

#if defined( USE_GC )
#	include "gc_allocator.h"
#endif

// TODO replace default allocator with one that goes through CORE_NEW/DELETE
#include <boost/pool/pool.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/pool_alloc.hpp>
namespace Core
{
	using boost::pool;
	using boost::object_pool;
	using boost::singleton_pool;
	using boost::pool_allocator;
}
#include <boost/flyweight.hpp>
#include <boost/flyweight/key_value.hpp>
namespace Core
{
	using boost::flyweight;
	using namespace boost::flyweights;
}

#include <boost/algorithm/minmax.hpp>
#include <boost/algorithm/minmax_element.hpp>
namespace Core
{
	using boost::minmax;
	using boost::minmax_element;
}
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
namespace Core
{
	using boost::filter_iterator;
	using boost::indirect_iterator;
	using boost::iterator_adaptor;
	using boost::iterator_facade;
	using boost::use_default;
	// TODO add the rest of the iterator tags
	using boost::forward_traversal_tag;
	using boost::random_access_traversal_tag;
}

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/linux_error.hpp>
#include <boost/system/windows_error.hpp>
namespace Core
{
	namespace system 
	{
		using namespace boost::system;
	}
}

#include <boost/swap.hpp>
namespace Core
{
	using boost::swap;
}

#include <boost/static_assert.hpp>

#include <boost/algorithm/string.hpp>
namespace Core
{
	namespace algorithm
	{
		using namespace boost::algorithm;
	}
}

#include <boost/asio.hpp>
namespace Core
{
	namespace asio
	{
		using namespace boost::asio;
		namespace placeholders 
		{
			using namespace boost::asio::placeholders;
		}
	}
}

#include <boost/scoped_array.hpp>
namespace Core
{
	using boost::scoped_array;
}

#include <boost/scoped_ptr.hpp>
namespace Core
{
	using boost::scoped_ptr;
}

namespace Core
{
	namespace mpl
	{
		using namespace boost::mpl;
	}
}

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/msm/front/euml/operator.hpp>
#include <boost/msm/front/euml/state_grammar.hpp>
namespace Core
{
	namespace msm
	{
		using namespace boost::msm;
	}
}

#include <boost/utility/enable_if.hpp>

#include "core/aligned_vector.h"
#include "core/thread.h"

namespace Core
{	
	// std namespace importation
	// http://wwwasd.web.cern.ch/wwwasd/lhc++/RW/stdlibcr/classref.htm

	using std::tr1::swap;
	using std::tr1::operator ==;
	using std::tr1::operator !=;
	using std::tr1::operator <;
	using std::tr1::operator <=;
	using std::tr1::operator >;
	using std::tr1::operator >=;
	using std::tr1::hash;
	using std::tr1::bad_function_call;
	using std::tr1::bind;
	using std::tr1::cref;
	using std::tr1::function;
	using std::tr1::is_bind_expression;
	using std::tr1::is_placeholder;
	using std::tr1::mem_fn;
	using std::tr1::ref;
	using std::tr1::reference_wrapper;
	using std::tr1::result_of;
	using std::tr1::swap;


	// tr1 array
	using std::tr1::array;
	// tr1 tuple
	using std::tr1::tuple;
	using std::tr1::tuple_element;
	using std::tr1::tuple_size;
	using std::tr1::get;
	using std::tr1::make_tuple;
	using std::tr1::tie;

	// tr1 type traits
	using std::tr1::integral_constant;
	using std::tr1::false_type;
	using std::tr1::true_type;
	using std::tr1::is_void;
	using std::tr1::is_integral;
	using std::tr1::is_floating_point;
	using std::tr1::is_array;
	using std::tr1::is_pointer;
	using std::tr1::is_reference;
	using std::tr1::is_member_function_pointer;
	using std::tr1::is_member_object_pointer;
	using std::tr1::is_enum;
	using std::tr1::is_union;
	using std::tr1::is_class;
	using std::tr1::is_function;
	using std::tr1::is_object;
	using std::tr1::is_scalar;
	using std::tr1::is_compound;
	using std::tr1::is_member_pointer;
	using std::tr1::is_const;
	using std::tr1::is_volatile;
	using std::tr1::is_pod;
	using std::tr1::is_empty;
	using std::tr1::is_polymorphic;
	using std::tr1::is_abstract;
	using std::tr1::has_trivial_constructor;
	using std::tr1::has_trivial_copy;
	using std::tr1::has_trivial_assign;
	using std::tr1::has_trivial_destructor;
	using std::tr1::has_trivial_constructor;
	using std::tr1::has_nothrow_constructor;
	using std::tr1::has_nothrow_copy;
	using std::tr1::has_nothrow_assign;
	using std::tr1::has_virtual_destructor;
	using std::tr1::is_signed;
	using std::tr1::is_unsigned;
	using std::tr1::alignment_of;
	using std::tr1::rank;
	using std::tr1::extent;
	using std::tr1::is_same;
	using std::tr1::is_convertible;
	using std::tr1::is_base_of;
	using std::tr1::remove_const;
	using std::tr1::remove_volatile;
	using std::tr1::remove_cv;
	using std::tr1::add_const;
	using std::tr1::add_volatile;
	using std::tr1::add_cv;
	using std::tr1::remove_extent;
	using std::tr1::remove_all_extents;
	using std::tr1::remove_pointer;
	using std::tr1::add_pointer;
	using std::tr1::aligned_storage;

	// tr1 regex
	using std::tr1::regex_traits;
	using std::tr1::basic_regex;
	using std::tr1::regex;
	using std::tr1::sub_match;
	using std::tr1::csub_match;
	using std::tr1::ssub_match;
	using std::tr1::match_results;
	using std::tr1::cmatch;
	using std::tr1::smatch;
	namespace regex_consants {
		using namespace std::tr1::regex_constants;
	}
	using std::tr1::regex_error;
	using std::tr1::regex_match;
	using std::tr1::regex_search;
	using std::tr1::regex_replace;
	using std::tr1::regex_iterator;
	using std::tr1::cregex_iterator;
	using std::tr1::sregex_iterator;
	using std::tr1::regex_token_iterator;
	using std::tr1::cregex_token_iterator;
	using std::tr1::sregex_token_iterator;
	using std::tr1::operator <<;

	// tr1 random
	using std::tr1::variate_generator;
	using std::tr1::linear_congruential;
	using std::tr1::mersenne_twister;
	using std::tr1::subtract_with_carry;
	using std::tr1::subtract_with_carry_01;
	using std::tr1::random_device;
	using std::tr1::discard_block;
	using std::tr1::xor_combine;
	using std::tr1::minstd_rand0;
	using std::tr1::minstd_rand;
	using std::tr1::mt19937;
	using std::tr1::ranlux_base_01;
	using std::tr1::ranlux64_base_01;
	using std::tr1::ranlux3;
	using std::tr1::ranlux4;
	using std::tr1::ranlux3_01;
	using std::tr1::ranlux4_01;
	using std::tr1::uniform_int;
	using std::tr1::bernoulli_distribution;
	using std::tr1::geometric_distribution;
	using std::tr1::poisson_distribution;
	using std::tr1::binomial_distribution;
	using std::tr1::uniform_real;
	using std::tr1::exponential_distribution;
	using std::tr1::normal_distribution;
	using std::tr1::gamma_distribution;
	using std::tr1::_Rng_abort;

	// tr1 unordered_map and unordered_set
	using std::tr1::unordered_set;
	using std::tr1::unordered_multiset;
	using std::tr1::unordered_map;
	using std::tr1::unordered_multimap;

	// tr1 memory shared ptr
   using std::tr1::bad_weak_ptr;
   using std::tr1::shared_ptr;
   using std::tr1::static_pointer_cast;
   using std::tr1::dynamic_pointer_cast;
   using std::tr1::const_pointer_cast;
   using std::tr1::get_deleter;
   using std::tr1::weak_ptr;
   using std::tr1::enable_shared_from_this;

#define STRING_CTORS_DECLARE(x,y,z)															\
	class x : public Core::basic_string< y , Core::char_traits< y >, z < y > > {			\
		public: 																			\
		typedef Core::basic_string< y , Core::char_traits< y >, z < y > > base_type;		\
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
	template<typename X> class gcvector : public Core::vector<X, gc_allocator<X> > {};
	template<typename KEY, typename VAL> class gcmap : public Core::map<KEY, VAL, Core::less<KEY>, 
												gc_allocator<Core::pair<const KEY,VAL> > > {};	
	template<typename X> class gcstack : public Core::stack<X, gcvector<X> > {};
	template<typename X> class gclist : public Core::list<X, gc_allocator<X> > {};
	template<typename KEY, typename VAL> class gcunordered_map : public 
				Core::unordered_map<KEY, VAL, Core::hash<KEY>, Core::equal_to<KEY>, 
				gc_allocator<Core::pair<const KEY,VAL> > > {};	

	STRING_CTORS_DECLARE( gcstring, char, gc_allocator );
	template<typename X> class gctraceablevector : public Core::vector<X, traceable_allocator<X> >{};
	template<typename KEY, typename VAL> class gctraceablemap : public Core::map<KEY, VAL, 
				Core::less<KEY>, traceable_allocator<Core::pair<const KEY,VAL> > > {};	
	template<typename X> class gctraceablestack : public Core::stack<X, gctraceablevector<X> > {};
	template<typename X> class gctraceablelist : public Core::list<X, traceable_allocator<X> > {};
	template<typename KEY, typename VAL> class gctraceableunordered_map : public 
				Core::unordered_map<KEY, VAL, Core::hash<KEY>, Core::equal_to<KEY>, 
				gc_allocator<Core::pair<const KEY,VAL> >  > {};	
#else
	template<typename X> class gcvector : public Core::vector<X, Core::allocator<X> > {};
	template<typename KEY, typename VAL> class gcmap : public Core::map<KEY, VAL, Core::less<KEY>, 
				Core::allocator<Core::pair<const KEY,VAL> > > {};	
	template<typename X> class gcstack : public Core::stack<X, gcvector<X> > {};
	template<typename X> class gclist : public Core::list<X, Core::allocator<X> > {};
	template<typename KEY, typename VAL> class gcunordered_map : public 
				Core::unordered_map<KEY, VAL, Core::hash<KEY>, Core::equal_to<KEY>, 
				Core::allocator<Core::pair<const KEY,VAL> >  > {};	
	STRING_CTORS_DECLARE( gcstring, char, Core::allocator );
	template<typename X> class gctraceablevector : public Core::vector<X> {};
	template<typename KEY, typename VAL> class gctraceablemap : public 
				Core::map<KEY, VAL, Core::less<KEY> > {};	
	template<typename X> class gctraceablestack : public Core::stack<X, gctraceablevector<X> > {};
	template<typename X> class gctraceablelist : public Core::list<X> {};
	template<typename KEY, typename VAL> class gctraceableunordered_map : public 
				Core::unordered_map<KEY, VAL, Core::hash<KEY>, Core::equal_to<KEY> > {};	
#endif

	template<typename X> class poolvector : public Core::vector<X, Core::pool_allocator<X> > {};
	template<typename KEY, typename VAL> class poolmap : public Core::map<KEY, VAL, 
				Core::less<KEY>, Core::pool_allocator<Core::pair<const KEY,VAL> > > {};	
	template<typename X> class poolstack : public Core::stack<X, poolvector<X> > {};
	template<typename X> class poollist : public Core::list<X, Core::pool_allocator<X> > {};
	template<typename KEY, typename VAL> class poolunordered_map : public 
				Core::unordered_map<KEY, VAL, Core::hash<KEY>, Core::equal_to<KEY>, 
				Core::pool_allocator<Core::pair<const KEY,VAL> >  > {};	
	STRING_CTORS_DECLARE( poolstring, char, Core::pool_allocator );

	STRING_CTORS_IMPL( gcstring )
	STRING_CTORS_IMPL( poolstring )
	#undef STRING_CTORS_DECLARE
	#undef STRING_CTORS_IMPL

}	//namespace Core

// its quite legit for external libs (google, etc.) to use std
// but we shouldn't so every now and again use this to hunt down any in
// our code. You will get lots of false positives in external code but
// ignore them and change std:: usage in code owned by us
#if defined( FIND_STD_USAGE_IN_WIERD )
#	define std Use_Core_namespace_instead
#endif

#endif // end CORE_STANDARD_H
