cmake_minimum_required( VERSION 2.8 )

# source files
file( GLOB cpp_src_files *.cpp *.cxx )
file( GLOB c_src_files *.c )
file( GLOB c_hdr_files *.h *.inl *.hpp *.hxx  )
file( GLOB as_lib_src_files ActionScript/aslib/*.cpp )
file( GLOB as_src_files  ActionScript/*.cpp  )
file( GLOB prg_shader_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} programs/*.hlsl )
file( GLOB prg_prg_shader_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} programs/programs/*.hlsl )
file( GLOB prg_shader_shader_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} programs/shaders/*.hlsl )
file( GLOB as_lib_hdr_files ActionScript/aslib/*.h )
file( GLOB as_hdr_files ActionScript/*.h  )

# shaders
list( APPEND shader_files ${prg_shader_files} ${prg_prg_shader_files} ${prg_shader_shader_files} )
foreach( sc ${shader_files} )
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp 
						COMMAND bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp ${sc}
						DEPENDS bin2c ${CMAKE_CURRENT_SOURCE_DIR}/${sc} 
					)
	list( APPEND gen_src_files ${CMAKE_CURRENT_BINARY_DIR}/${sc}.cpp )
endforeach( sc ${shader_files} )

list( APPEND src_files ${cpp_src_files} ${c_src_files} ${as_src_files} ${as_lib_src_files} ${gen_src_files} )
list( APPEND hdr_files ${c_hdr_files} ${as_hdr_files} ${as_lib_hdr_files} )

add_library( swfruntime STATIC ${src_files} ${hdr_files} ${shader_files} )

list( APPEND targ_link_libs core swfparser scene zlib jpeg )
target_link_libraries( swfruntime ${targ_link_libs} )

# tidy project files
ADD_MSVC_PRECOMPILED_HEADER( "swfruntime.h" "${cpp_src_files}" {"as_src_file"} )
SOURCE_GROUP( ActionScript FILES ${as_src_files} ${as_hdr_files}  )
SOURCE_GROUP( ActionScript/lib FILES ${as_lib_src_files} ${as_lib_hdr_files}  )
SOURCE_GROUP( Shader FILES ${shader_files} )
SOURCE_GROUP( Generated FILES ${gen_src_files} )
set_target_properties( swfruntime PROPERTIES FOLDER "Gui/Libs")
