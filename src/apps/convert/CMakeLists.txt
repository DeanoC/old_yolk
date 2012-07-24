cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

add_executable( convert ${src_files} ${header_files} )

list( APPEND targ_link_libs ${Core_LIBRARIES} meshimport meshmod meshops export binify assimp json_spirit )
if( UNIX )
	list( APPEND targ_link_libs pthread dl )
endif()

target_link_libraries( convert ${targ_link_libs} )

# make a nice hierachy
set_target_properties( convert PROPERTIES FOLDER "Apps")

