cmake_minimum_required( VERSION 2.8 )

# include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# link_directories(${CMAKE_CURRENT_SOURCE_DIR})



file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx)
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

# take vm_code files and whack them in (source and all into the code)
# TODO convert to bitcode first
file( GLOB vm_src_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
								vm_code/*.cpp
								vm_code/*.c
								vm_code/*.asm
								)
if( WIN32 )
	set( CLANG_EXE ${LLVM_ROOT}/bin/${CMAKE_CFG_INTDIR}/clang++.exe )
else()
	set( CLANG_EXE ${LLVM_ROOT}/bin/${CMAKE_CFG_INTDIR}/clang++ )
endif()
foreach( sc ${vm_src_files} )
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc 
						COMMAND ${CLANG_EXE} -c -nostdinc -emit-llvm -o ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc  ${CMAKE_CURRENT_SOURCE_DIR}/${sc}
						DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${sc} VERBATIM )
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc.cpp 
						COMMAND bin2c ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc.cpp ${sc}
						DEPENDS bin2c ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc )
	list( APPEND src_files ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc.cpp )
endforeach( sc ${vm_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "dwm.h" "${cpp_src_files}" )
add_library( dwm ${src_files} ${header_files} )

# make a nice hierachy
set_target_properties( dwm PROPERTIES FOLDER "Libs")
