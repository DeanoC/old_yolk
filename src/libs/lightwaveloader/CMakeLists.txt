cmake_minimum_required( VERSION 2.8 )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

file( GLOB cpp_src_files *.cpp *.cxx )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )

list( APPEND header_files ${header_files} )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )


ADD_MSVC_PRECOMPILED_HEADER( "lightwaveloader.h" "${cpp_src_files}" )
add_library( lightwaveloader ${src_files} ${header_files}  )

list( APPEND targ_link_libs core )
target_link_libraries( lightwaveloader ${targ_link_libs} )

# make a nice hierachy
set_target_properties( lightwaveloader PROPERTIES SOLUTION_FOLDER "libs")
