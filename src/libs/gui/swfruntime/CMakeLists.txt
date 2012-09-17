cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx ActionScript/*.cpp ActionScript/aslib/*.cpp ActionScript/autogen/*.cpp ActionScript/codegen/*.cpp )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx ActionScript/*.h ActionScript/aslib/*.h ActionScript/autogen/*.h ActionScript/codegen/*.h )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

file( GLOB shader_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
								programs/*.hlsl 
								programs/programs/*.hlsl 
								programs/shaders/*.hlsl 
								)

foreach( sc ${shader_files} )
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp 
						COMMAND bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp ${sc}
						DEPENDS bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} 
						COMMENT "${sc}: if this fails create the directory part of ${sc} in the build folders manually" 
					)
	list( APPEND src_files ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp )
endforeach( sc ${shader_files} )

ADD_MSVC_PRECOMPILED_HEADER( "swfruntime.h" "${cpp_src_files}" )
add_library( swfruntime STATIC ${src_files} ${header_files} ${shader_files} )

list( APPEND targ_link_libs core swfparser scene zlib jpeg )
target_link_libraries( swfruntime ${targ_link_libs} )

# make a nice hierachy
set_target_properties( swfruntime PROPERTIES FOLDER "Gui/Libs")
