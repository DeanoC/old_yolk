cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx ActionScript/*.cpp ActionScript/aslib/*.cpp ActionScript/autogen/*.cpp ActionScript/codegen/*.cpp )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx ActionScript/*.h ActionScript/aslib/*.h ActionScript/autogen/*.h ActionScript/codegen/*.h )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "swfruntime.h" "${cpp_src_files}" )
add_library( swfruntime STATIC ${src_files} ${header_files} )

list( APPEND targ_link_libs swfparser scene )
target_link_libraries( swfruntime ${targ_link_libs} )

# make a nice hierachy
set_target_properties( swfruntime PROPERTIES FOLDER "Libs")
