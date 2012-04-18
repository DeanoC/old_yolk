cmake_minimum_required( VERSION 2.8.2 )

# make overrides installed
if(MSVC)
	set( CMAKE_USER_MAKE_RULES_OVERRIDE
   		${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/c_flag_overrides.cmake )
	set( CMAKE_USER_MAKE_RULES_OVERRIDE_CXX
   		${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/cxx_flag_overrides.cmake )
else()
#	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" CACHE STRING "CXX Flags" )
endif()

# top level wierd cmake file
project( Yolk )

include( yolk_setup.cmake )

# tell it to recurse into the third party dir
# do manually for now
add_subdirectory( submodules )
link_directories( ${CMAKE_CURRENT_BINARY_DIR}/submodules/llvm/lib )
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
