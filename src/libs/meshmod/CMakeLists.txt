cmake_minimum_required( VERSION 2.8 )

include_directories(${WIERD_SOURCE_DIR}/source/core)
include_directories(${WIERD_SOURCE_DIR}/source/meshmod)

link_directories(${WIERD_BINARY_DIR}/source/meshmod)

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx)

list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "meshmod.h" "${cpp_src_files}" )
add_library( meshmod ${src_files} ${header_files} )

# make a nice hierachy
set_target_properties( meshmod PROPERTIES FOLDER "Libs")
