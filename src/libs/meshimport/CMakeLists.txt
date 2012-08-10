cmake_minimum_required( VERSION 2.8 )

#include_directories(${CMAKE_SOURCE_DIR}/submodules/assimp/include)

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx)

list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "meshimport.h" "${cpp_src_files}" )
add_library( meshimport ${src_files} ${header_files} )
target_link_libraries( meshimport lightwaveloader )

# make a nice hierachy
set_target_properties( meshimport PROPERTIES FOLDER "meshimport")
