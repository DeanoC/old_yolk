cmake_minimum_required( VERSION 2.8 )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
file( GLOB cpp_src_files *.cpp *.cxx )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )

file( GLOB shader_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
								programs/*.hlsl 
								programs/programs/*.hlsl 
								programs/shaders/*.hlsl 
								programs/structs/*.hlsl 
								)

include_directories(${CMAKE_SOURCE_DIR}/src/libs/bullet/src/ )

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
	
list( APPEND header_files ${header_files} )
list( APPEND src_files ${cpp_src_files} ${c_src_files}  )

foreach( sc ${shader_files} )
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp 
						COMMAND bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp ${sc}
						DEPENDS bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} 
					)
	list( APPEND gen_src_files ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp )
endforeach( sc ${shader_files} )

list( APPEND src_files ${cpp_src_files} ${c_src_files} ${gen_src_files} )
set_source_files_properties(${shader_files} PROPERTIES HEADER_FILE_ONLY TRUE)

ADD_MSVC_PRECOMPILED_HEADER( "scene.h" "${cpp_src_files}" )
add_library( scene ${src_files} ${header_files}  ${shader_files} )

target_link_libraries( 	scene
						BulletCollision
						BulletDynamics
						swfruntime )

# make a nice hierachy

SOURCE_GROUP( Generated FILES ${gen_src_files} )
SOURCE_GROUP( Shaders FILES ${shader_files} )
set_target_properties( scene PROPERTIES FOLDER "Libs")
