///-------------------------------------------------------------------------------------------------
/// \file	core\thread.h
///
/// \brief	Sets up the Core thread namespace. 
///
/// \details	
/// We use the latest c++0x thread standard as the basis
/// of our basic thread system.
/// we can so far choose between boost::threads and tbb::thread and maybe the systems tr2
/// 
/// \remark	Copyright (c) 2012 Cloud Pixies Ltd. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef YOLK_CORE_THREAD_H_
#define YOLK_CORE_THREAD_H_

// todo automate selection
//#define USE_BOOST_THREADS
//#define USE_TBB_THREADS
#define USE_STD_THREADS

#if defined( USE_BOOST_THREADS )
#	include <boost/thread.hpp>
#	include <boost/chrono.hpp>
#endif
#if defined( USE_TBB_THREADS )
#	include <tbb/tbb_thread.h>
#endif
#if defined( USE_STD_THREADS )
#	include <thread>
#	include <future>
#	include	<chrono>
#	include	<atomic>
#	include <condition_variable>
#endif

namespace Core
{
#if defined( USE_BOOST_THREADS )
	using boost::thread;
	namespace this_thread {
		using namespace boost::this_thread;
	}
	using boost::thread_group;

	using boost::thread_specific_ptr;

	// http://www.boost.org/doc/libs/1_44_0/doc/html/thread/synchronization.html#thread.synchronization.locks
	using boost::lock_guard;
	using boost::unique_lock;
	using boost::shared_lock;
	using boost::upgrade_lock;
	using boost::upgrade_to_unique_lock;

	//http://www.boost.org/doc/libs/1_44_0/doc/html/thread/synchronization.html#thread.synchronization.lock_functions
	using boost::lock;
	using boost::try_lock;

	//http://www.boost.org/doc/libs/1_44_0/doc/html/thread/synchronization.html#thread.synchronization.mutex_types
	using boost::mutex;
	using boost::try_mutex;
	using boost::timed_mutex;
	using boost::recursive_mutex;
	using boost::recursive_try_mutex;
	using boost::recursive_timed_mutex;
	using boost::shared_mutex;

	//http://www.boost.org/doc/libs/1_44_0/doc/html/thread/synchronization.html#thread.synchronization.condvar_ref
	using boost::condition_variable;
	using boost::condition_variable_any;
	
	//http://www.boost.org/doc/libs/1_44_0/doc/html/thread/synchronization.html#thread.synchronization.once
	using boost::call_once;

	//http://www.boost.org/doc/libs/1_44_0/doc/html/thread/synchronization.html#thread.synchronization.barriers
	using boost::barrier;

	//http://www.boost.org/doc/libs/1_44_0/doc/html/thread/synchronization.html#thread.synchronization.futures.reference
	using namespace boost::future_state;
	using boost::unique_future;
	using boost::shared_future;
	using boost::promise;
	using boost::packaged_task;
	using boost::wait_for_any;
	using boost::wait_for_all;
#elif defined( USE_TBB_THREADS )
	typedef tbb::tbb_thread thread;

	namespace this_thread {
		using tbb::this_tbb_thread::get_id;
		using tbb::this_tbb_thread::yield;

		inline void sleep_for(const tbb::tick_count::interval_t& rel_time) {
			tbb::internal::thread_sleep_v3( rel_time );
		}

	}
#else
	using std::thread;
	using namespace std::this_thread;
	using std::lock_guard;
	using std::unique_lock;
	using std::lock;
	using std::try_lock;
	using std::mutex;
	using std::timed_mutex;
	using std::recursive_mutex;
	using std::recursive_timed_mutex;
	using std::condition_variable;
	using std::condition_variable_any;
	using std::call_once;
	using std::future;
	using std::shared_future;
	using std::promise;
	using std::packaged_task;

#endif
} // end namespace Core

#endif