cmake_minimum_required( VERSION 2.8 )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions( "-DRENDER_BACKEND=Gl" )
file( GLOB cpp_src_files *.cpp *.cxx )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
	
list( APPEND header_files ${header_files} )
list( APPEND src_files ${cpp_src_files} ${c_src_files}  )

ADD_MSVC_PRECOMPILED_HEADER( "scene.h" "${cpp_src_files}" )
add_library( scene ${src_files} ${header_files}  )

# make a nice hierachy
set_target_properties( scene PROPERTIES SOLUTION_FOLDER "libs")