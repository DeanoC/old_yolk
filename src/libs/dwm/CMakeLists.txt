cmake_minimum_required( VERSION 2.8 )

# include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# link_directories(${CMAKE_CURRENT_SOURCE_DIR})

# configure file replace cmake macros (i.e @Wierd_VERSION_MAJOR@) with there value
# from the first file to the second, this makes our build config header
# configure_file ( 
# 	"buildconfig.h.in"
# 	"buildconfig.h"
# )

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx)

# list( APPEND header_files ${header_files} "${CMAKE_CURRENT_BINARY_DIR}/buildconfig.h" )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "core.h" "${cpp_src_files}" )
add_library( dwm ${src_files} ${header_files} )

# make a nice hierachy
set_target_properties( dwm PROPERTIES FOLDER "Libs")
