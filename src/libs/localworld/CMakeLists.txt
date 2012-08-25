cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx apis/*.cpp )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "localworld.h" "${cpp_src_files}" )
add_library( localworld STATIC ${src_files} ${header_files} )

# make a nice hierachy
set_target_properties( localworld PROPERTIES FOLDER "Libs")
