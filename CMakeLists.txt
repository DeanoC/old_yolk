cmake_minimum_required( VERSION 2.8.2 )
if( UNIX )
	set(CMAKE_CXX_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_CXX_FLAGS} -std=c++0x" )
elseif( WIN32 )
	set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm1000 /EHsc /GR /MP /D_ITERATOR_DEBUG_LEVEL=0" CACHE STRING "C++ Flags" )
endif()

# top level wierd cmake file
project( Yolk )

include( yolk_setup.cmake )

# tell it to recurse into the third party dir
# do manually for now
add_subdirectory( submodules )
# tell it to recurse into the source dir
add_subdirectory( src )

# Workaround for MSVS10 to avoid the Dialog Hell
# FIXME: This could be removed with future version of CMake.
if(MSVC_VERSION EQUAL 1600)
  set(SLN_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/Yolk.sln")
  if( EXISTS "${SLN_FILENAME}" )
    file(APPEND "${SLN_FILENAME}" "\n# This should be regenerated!\n")
  endif()
endif()
