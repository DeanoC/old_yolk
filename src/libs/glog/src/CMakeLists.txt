cmake_minimum_required( VERSION 2.8 )

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

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
