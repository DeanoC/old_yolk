cmake_minimum_required( VERSION 2.8 )

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
file( GLOB vm_hdr_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
								vm_code/*.h )
#list( APPEND vm_files ${vm_src_files} ${vm_hdr_files} )
list( APPEND vm_files ${vm_hdr_files} )
SOURCE_GROUP( vm_code FILES ${vm_files} )

if( WIN32 )
	set( CLANG_EXE ${LLVM_ROOT}/bin/${CMAKE_CFG_INTDIR}/clang++.exe )
else()
	set( CLANG_EXE ${LLVM_ROOT}/bin/${CMAKE_CFG_INTDIR}/clang++ -cc1 )
endif()
foreach( sc ${vm_src_files} )


	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc 
						COMMAND ${CLANG_EXE} -triple i386-pc-linux-gnu -emit-llvm-bc -disable-free -main-file-name ${sc} -mrelocation-model static -masm-verbose -mconstructor-aliases -target-cpu pentium4 -momit-leaf-frame-pointer -v -coverage-file /home/desktop/build/yolk/src/libs/dwm/vm_code/${sc}.bc -nostdsysteminc -nobuiltininc -resource-dir /home/desktop/build/yolk/submodules/llvm/bin/../lib/clang/3.1 -isystem /${CMAKE_CURRENT_SOURCE_DIR}/vm_code/system/include -D __STDC__ -fmodule-cache-path /var/tmp/clang-module-cache -std=c++0x -fdeprecated-macro -fdebug-compilation-dir /home/desktop/build/yolk/src/libs/dwm -ferror-limit 19 -fmessage-length 169 -mstackrealign -fcxx-exceptions -fexceptions -fdiagnostics-show-option -fcolor-diagnostics -o ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc -x c++ ${CMAKE_CURRENT_SOURCE_DIR}/${sc}
						DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${sc} VERBATIM )
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc.cpp 
						COMMAND bin2c ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc.cpp ${sc}
						DEPENDS bin2c ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc )
	list( APPEND src_files ${CMAKE_CURRENT_BINARY_DIR}/${sc}.bc.cpp )
endforeach( sc ${vm_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "dwm.h" "${cpp_src_files}" )
add_library( dwm ${src_files} ${header_files} ${vm_files} )

# make a nice hierachy
set_target_properties( dwm PROPERTIES FOLDER "Libs")
