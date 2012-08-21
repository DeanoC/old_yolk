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

#include <atomic>

#include <fstream>
// these are auto forwarding to the tr1 implementations of the platform if it supports them
#include <functional>
#include <memory>
#include <type_traits>
#include <random>
#include <tuple>
#include <utility>
#include <array>
#include <regex>
#include <complex>
#include <unordered_map>
#include <unordered_set>

#if defined( USE_GC )
#	include "gc_allocator.h"
#endif
#include "core/thread.h"

#include <boost/algorithm/string.hpp>
/*#include <boost/asio.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/msm/front/euml/operator.hpp>
#include <boost/msm/front/euml/state_grammar.hpp>*/
#include <boost/scoped_array.hpp>

// tbb concurrent containers
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_vector.h"

#endif // end CORE_STANDARD_H
