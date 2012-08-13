cmake_minimum_required( VERSION 2.8 )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
file( GLOB cpp_src_files *.cpp *.cxx )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )

file( GLOB shader_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
								programs/*.hlsl 
								programs/*.glsl 
								)


include_directories("${CMAKE_CURRENT_BINARY_DIR}")
	
list( APPEND header_files ${header_files} )
list( APPEND src_files ${cpp_src_files} ${c_src_files}  )

foreach( sc ${shader_files} )
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp 
						COMMAND bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp ${sc}
						DEPENDS bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} 
						COMMENT "${sc}: if this fails create the directory part of ${sc} in the build folders manually" 
					)
	list( APPEND src_files ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp )
endforeach( sc ${shader_files} )

ADD_MSVC_PRECOMPILED_HEADER( "scene.h" "${cpp_src_files}" )
add_library( scene ${src_files} ${header_files}  ${shader_files} )

# make a nice hierachy
set_target_properties( scene PROPERTIES FOLDER "Libs")
