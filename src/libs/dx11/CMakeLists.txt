cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx)

list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "dx11.h" "${cpp_src_files}" )
add_library( dx11 ${src_files} ${header_files} )

# make a nice hierachy
set_target_properties( dx11 PROPERTIES FOLDER "Libs")
