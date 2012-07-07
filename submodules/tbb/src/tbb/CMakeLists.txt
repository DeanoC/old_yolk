cmake_minimum_required( VERSION 2.8 )

LIST( APPEND tbb_src_files
  arena.cpp
  cache_aligned_allocator.cpp
  concurrent_hash_map.cpp
  concurrent_monitor.cpp
  concurrent_queue.cpp
  concurrent_vector.cpp
  condition_variable.cpp
  dynamic_link.cpp
  governor.cpp
  market.cpp
  mutex.cpp
  observer_proxy.cpp
  pipeline.cpp
  private_server.cpp
  queuing_mutex.cpp
  queuing_rw_mutex.cpp
  reader_writer_lock.cpp
  scheduler.cpp
  semaphore.cpp
  spin_mutex.cpp
  spin_rw_mutex.cpp
  task.cpp
  task_group_context.cpp
  tbb_main.cpp
  tbb_misc.cpp
  tbb_misc_ex.cpp
  tbb_statistics.cpp
  tbb_thread.cpp
)
add_library( tbb ${tbb_src_files} )
add_definitions( "-D__TBB_BUILD=1")
list( APPEND targ_link_libs tbb_rml )

if( UNIX )
	add_definitions( "-DUSE_PTHREAD")
	list( APPEND targ_link_libs pthread dl )
endif()
target_link_libraries( tbb ${targ_link_libs} )

# make a nice hierachy
set_target_properties( tbb PROPERTIES FOLDER "Libs" )
