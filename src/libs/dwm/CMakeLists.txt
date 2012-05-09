cmake_minimum_required( VERSION 2.8 )

include_directories( ${CMAKE_SOURCE_DIR}/submodules/nacl-llvm/llvm/include/ )
include_directories( ${CMAKE_BINARY_DIR}/submodules/nacl-llvm/llvm/include/ )
link_directories( ${CMAKE_BINARY_DIR}/submodules/nacl-llvm/llvm/lib )
include_directories( ${CMAKE_SOURCE_DIR}/submodules/native_client/src/ ) # slightly odd but same as the actual project uses...

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx)
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "dwm.h" "${cpp_src_files}" )
add_library( dwm ${src_files} ${header_files} )

# make a nice hierachy
set_target_properties( dwm PROPERTIES FOLDER "Libs")
# TODO proper arch settings
if ( WIN32 )
  add_definitions( "/DNACL_WINDOWS=1" )
  add_definitions( "/DNACL_OSX=0" )
  add_definitions( "/DNACL_LINUX=0" )
  add_definitions( "/D_WIN32_WINNT=0x501" )
  add_definitions( "/D__STDC_LIMIT_MACROS=1" )
  add_definitions( "/DNOMINMAX=1" )
  add_definitions( "/DNACL_BUILD_ARCH=x86" )
  add_definitions( "/DNACL_BUILD_SUBARCH=32" )
  add_definitions( "/DNACL_TARGET_ARCH=x86" )
  add_definitions( "/DNACL_TARGET_SUBARCH=32" )
else()
  add_definitions( "-DNACL_WINDOWS=0" )
  add_definitions( "-DNACL_OSX=0" )
  add_definitions( "-DNACL_LINUX=1" )
  add_definitions( "-D__STDC_LIMIT_MACROS=1" )
  add_definitions( "-DNACL_BUILD_ARCH=x86" )
  add_definitions( "-DNACL_BUILD_SUBARCH=32" )
  add_definitions( "-DNACL_TARGET_ARCH=x86" )
  add_definitions( "-DNACL_TARGET_SUBARCH=32" )
endif()