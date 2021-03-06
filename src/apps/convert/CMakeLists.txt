cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

include_directories( ${CMAKE_BINARY_DIR}/src/libs/freetype-2.5.3/include )
include_directories( ${CMAKE_SOURCE_DIR}/src/libs/freetype-2.5.3/include )

list( APPEND targ_link_libs 
			${Core_LIBRARIES} 
			meshimport 
			meshmod 
			meshops 
			export 
			binify 
			freetype )
add_executable( convert ${src_files} ${header_files} )

if( UNIX )
	list( APPEND targ_link_libs pthread dl )
else()
	list( APPEND targ_link_libs shcore)
	SET_TARGET_PROPERTIES( convert PROPERTIES LINK_FLAGS "/SAFESEH:NO" )
endif()
target_link_libraries( convert ${targ_link_libs} )

# make a nice hierachy
set_target_properties( convert PROPERTIES FOLDER "Tools")

