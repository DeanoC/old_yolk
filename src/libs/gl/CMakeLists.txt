cmake_minimum_required( VERSION 2.8 )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
 
file( GLOB cpp_src_files *.cpp *.cxx )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
if( UNIX )
	add_definitions( "-DLOAD_X11" )
	list( APPEND c_src_files posix/gfx.cpp )
endif()

file( GLOB shader_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
								programs/*.glsl
								programs/fragments/*.glsl 
								programs/programs/*.glsl 
								)

file( GLOB cl_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
								programs/kernels/*.cl
								)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
	
file( GLOB glload_header_src_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
									glload/source/*.h 
									glload/include/*.h 
									)

list( APPEND header_files ${header_files} ${glload_header_src_files}  ${shader_files} ${cl_files} )
list( APPEND src_files ${cpp_src_files} ${c_src_files} ${glload_c_src_files} )


foreach( sc ${shader_files} )
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp 
						COMMAND bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp ${sc}
						DEPENDS bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} )
	list( APPEND src_files ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp )
endforeach( sc ${shader_files} )

foreach( clc ${cl_files} )
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${clc}.cpp 
						COMMAND bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${clc} ${CMAKE_CURRENT_BINARY_DIR}/${clc}.cpp ${clc}
						DEPENDS bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${clc} )
	list( APPEND src_files ${CMAKE_CURRENT_BINARY_DIR}/${clc}.cpp )
endforeach( clc ${cl_files} )

ADD_MSVC_PRECOMPILED_HEADER( "gl.h" "${cpp_src_files}" )
add_library( gl ${src_files} ${header_files}  )

list( APPEND targ_link_libs core )
target_link_libraries( gl ${targ_link_libs} )

# make a nice hierachy
set_target_properties( gl PROPERTIES SOLUTION_FOLDER "libs")
