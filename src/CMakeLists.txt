cmake_minimum_required( VERSION 2.8 )

# bake into the library some version
set (${PROJECT}_VERSION_MAJOR 0)
set (${PROJECT}_VERSION_MINOR 1)
set (${PROJECT}_VERSION_REVISION 0) # TODO retrieve from git

# add the binary tree to the search path for include files
# so that we will find buildconfig.h
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libs")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/libs")

add_subdirectory(libs)
add_subdirectory(apps)

