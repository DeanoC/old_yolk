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
IF( UNIX )
ELSE()
	IF( IS_HOST_64_BIT )
		LIST( APPEND tbb_asm_files "intel64-masm/atomic_support.asm" "intel64-masm/intel64_misc.asm" )
		foreach( tas ${tbb_asm_files} )
		  GET_FILENAME_COMPONENT( asmfname ${tas} NAME_WE ) 
		  SET( asmname  "${CMAKE_CURRENT_BINARY_DIR}/${asmfname}.obj" )
			add_custom_command( OUTPUT ${asmname}
								COMMAND ml64 ${CMAKE_CURRENT_SOURCE_DIR}/${tas} /c /Fo ${asmfname}
								DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${tas} 
						)
			list( APPEND tbb_obj_files ${asmname} )
		endforeach( tas ${tbb_asm_files} )
  ELSE()
    LIST( APPEND tbb_asm_files "ia32-masm/atomic_support.asm" "ia32-masm/lock_byte.asm" )
    foreach( tas ${tbb_asm_files} )
      GET_FILENAME_COMPONENT( asmfname ${tas} NAME_WE ) 
      SET( asmname  "${CMAKE_CURRENT_BINARY_DIR}/${asmfname}.obj" )
      add_custom_command( OUTPUT ${asmname}
                COMMAND ml ${CMAKE_CURRENT_SOURCE_DIR}/${tas} /c /Fo ${asmfname}
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${tas} 
            )
      list( APPEND tbb_obj_files ${asmname} )
    endforeach( tas ${tbb_asm_files} )
	ENDIF()
ENDIF()

add_library( tbb ${tbb_src_files} ${tbb_obj_files} )
add_definitions( "-D__TBB_BUILD=1")
list( APPEND targ_link_libs tbb_rml )

if( UNIX )
	add_definitions( "-D__TBB_USE_GENERIC_DWORD_LOAD_STORE=1" )
	add_definitions( "-DUSE_PTHREAD -DTBB_USE_GCC_BUILTINS -D__TBB_GCC_BUILTIN_ATOMICS_PRESENT" )
	list( APPEND targ_link_libs pthread dl )
else()
	add_definitions( "-DTBB_USE_CAPTURED_EXCEPTION -DUSE_WINTHREAD" )
endif()
target_link_libraries( tbb ${targ_link_libs} )

# make a nice hierachy
set_target_properties( tbb PROPERTIES FOLDER "Libs" )
