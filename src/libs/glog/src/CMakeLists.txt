cmake_minimum_required( VERSION 2.8 )

include( CheckIncludeFiles )
include( CheckFunctionExists )
include( CheckLibraryExists )
include( CheckTypeSize )

check_include_file( dlfcn.h HAVE_DLFCN_H )
check_include_file( execinfo.h HAVE_EXECINFO_H )
check_include_file( inttypes.h HAVE_INTTYPES_H )
check_include_file( libunwind.h HAVE_LIBUNWIND_H )
check_include_file( memory.h HAVE_MEMORY_H )
check_include_file( stdint.h HAVE_STDINT_H )
check_include_file( stdlib.h HAVE_STDLIB_H )
check_include_file( strings.h HAVE_STRINGS_H )
check_include_file( string.h HAVE_STRING_H )
check_include_file( syscall.h HAVE_SYSCALL_H )
check_include_file( sys/stat.h HAVE_SYS_STAT_H )
check_include_file( sys/syscall.h HAVE_SYS_SYSCALL_H )
check_include_file( sys/types.h HAVE_SYS_TYPES_H )
check_include_file( ucontext.h HAVE_UCONTEXT_H )
check_include_file( unistd.h HAVE_UNISTD_H )
check_include_file( sys/utsname.h HAVE_SYS_UTSNAME_H )
check_include_file( sys/time.h HAVE_SYS_TIME_H )

check_function_exists( dladdr HAVE_DLADDR )
check_function_exists( sigaltstack HAVE_SIGALTSTACK )

check_library_exists( gflags HandleCommandLineCompletions "" HAVE_LIB_GFLAGS )
check_library_exists( libunwind unw_create_addr_space "" HAVE_LIB_UNWIND )
check_library_exists( pthread pthread_create "" HAVE_PTHREAD ) 
check_library_exists( pthread pthread_rwlock_init "" HAVE_RWLOCK ) 

check_type_size(void* SIZEOF_VOID_P)

if( WIN32 )
	set( _ITERATOR_DEBUG_LEVEL 0 )
	set( HAVE_NAMESPACES 1 )
elseif( UNIX )
	set( HAVE_NAMESPACES 1 )
	set( HAVE_USING_OPERATOR 1 )
	set( HAVE___ATTRIBUTE__ 1 )
	set( HAVE___BUILTIN_EXPECT 1 )
	set( HAVE___SYNC_VAL_COMPARE_AND_SWAP 1 )
endif()

include_directories( "${CMAKE_CURRENT_BINARY_DIR}" )

configure_file ( 
	"config.h.in"
	"config.h"
)

SET( cpp_src_files logging.cc raw_logging.cc utilities.cc vlog_is_on.cc )
file( GLOB header_files *.h ../include/glog/*.h)

if( WIN32 )
	list( APPEND cpp_src_files port.cc )
	list( APPEND cpp_src_files port.h )
else( WIN32 )
	list( APPEND cpp_src_files demangle.cc signalhandler.cc symbolize.cc )
endif( WIN32 )

list( APPEND src_files ${cpp_src_files} ${c_src_files} )

add_library( glog ${src_files} ${header_files} )

# make a nice hierachy
set_target_properties( glog PROPERTIES FOLDER "Libs")
