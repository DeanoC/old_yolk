cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx apis/*.cpp )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "swfparser.h" "${cpp_src_files}" )
add_library( swfparser STATIC ${src_files} ${header_files} )

list( APPEND targ_link_libs core  )
target_link_libraries( swfparser ${targ_link_libs} )

# make a nice hierachy
set_target_properties( swfparser PROPERTIES FOLDER "Libs/Gui")
