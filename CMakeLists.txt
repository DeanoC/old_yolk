cmake_minimum_required( VERSION 3.9)
set( CMAKE_CXX_STANDARD 17 )

# top level wierd cmake file
project( Yolk )

include( yolk_setup.cmake )

# tell it to recurse into the third party dir
# do manually for now
#add_subdirectory( submodules )
# tell it to recurse into the source dir
add_subdirectory( src )
