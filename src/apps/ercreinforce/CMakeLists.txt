cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

list( APPEND targ_link_libs json_spirit )
add_executable( ercreinforce ${src_files} ${header_files} )

if( UNIX )
	list( APPEND targ_link_libs pthread dl )
else()
	SET_TARGET_PROPERTIES( ercreinforce PROPERTIES LINK_FLAGS "/SAFESEH:NO" )
endif()
target_link_libraries( ercreinforce ${targ_link_libs} )

# make a nice hierachy
set_target_properties( ercreinforce PROPERTIES FOLDER "Apps")

