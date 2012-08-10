cmake_minimum_required( VERSION 2.8 )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

file( GLOB cpp_src_files *.cpp *.cxx )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
file( GLOB cl_src_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
								kernels/*.cl
								)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

list( APPEND header_files ${header_files} ${cl_src_files} )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

foreach( sc ${cl_src_files} )
	set_source_files_properties( ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp PROPERTIES GENERATED TRUE )
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp
						DEPENDS bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc}
						COMMAND bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp ${sc}
						COMMENT "${sc}: if this fails create the directory part of ${sc} in the build folders manually" )
	list( APPEND src_files ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp )
endforeach( sc ${cl_src_files} )

#ADD_MSVC_PRECOMPILED_HEADER( "ocl.h" "${cpp_src_files}" )
add_library( cl ${src_files} ${header_files}  )

# make a nice hierachy
set_target_properties( cl PROPERTIES FOLDER "Libs")
