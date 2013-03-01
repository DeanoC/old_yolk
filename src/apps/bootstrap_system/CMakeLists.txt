cmake_minimum_required( VERSION 2.8 )


file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

# ADD_MSVC_PRECOMPILED_HEADER( "convertmesh_pch.h" "${cpp_src_files}" )
add_executable( bootstrap_system WIN32 ${src_files} ${header_files} )

list( APPEND targ_link_libs ${Core_LIBRARIES} riak-cpp protobuf json_spirit )
if( UNIX )
	list( APPEND targ_link_libs pthread dl boost_thread)
endif()

target_link_libraries( bootstrap_system ${targ_link_libs} )

# make a nice hierachy
set_target_properties( bootstrap_system PROPERTIES FOLDER "Apps")

