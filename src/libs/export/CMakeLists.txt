cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx)
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "export.h" "${cpp_src_files}" )
add_library( export ${src_files} ${header_files} )

list( APPEND targ_link_libs ${Core_LIBRARIES} meshmod meshops binify )
target_link_libraries( export ${targ_link_libs} )

# make a nice hierachy
set_target_properties( export PROPERTIES FOLDER "Libs")
