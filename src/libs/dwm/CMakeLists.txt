cmake_minimum_required( VERSION 2.8 )

include_directories( ${CMAKE_SOURCE_DIR}/submodules/nacl-llvm/llvm/include/ )
include_directories( ${CMAKE_BINARY_DIR}/submodules/nacl-llvm/llvm/include/ )
link_directories( ${CMAKE_BINARY_DIR}/submodules/nacl-llvm/llvm/lib )

file( GLOB cpp_src_files *.cpp *.cxx apis/*.cpp )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx apis/*.h )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "dwm.h" "${cpp_src_files}" )
add_library( dwm STATIC ${src_files} ${header_files} )

list( APPEND targ_link_libs ${Core_LIBRARIES} )
if( UNIX )
  list( APPEND targ_link_libs pthread dl )
endif()

target_link_libraries( dwm ${targ_link_libs} )

# make a nice hierachy
set_target_properties( dwm PROPERTIES FOLDER "Libs")
