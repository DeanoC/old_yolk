cmake_minimum_required( VERSION 2.8 )

include_directories( ${CMAKE_SOURCE_DIR}/submodules/nacl-llvm/llvm/include/ )
include_directories( ${CMAKE_BINARY_DIR}/submodules/nacl-llvm/llvm/include/ )
link_directories( ${CMAKE_BINARY_DIR}/submodules/nacl-llvm/llvm/lib )

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

# ADD_MSVC_PRECOMPILED_HEADER( "convertmesh_pch.h" "${cpp_src_files}" )
add_executable( dvm_owned_dedi ${src_files} ${header_files} )

list( APPEND targ_link_libs ${Core_LIBRARIES} dwm riak-cpp protocols protobuf json_spirit gl cl render )
if( UNIX )
	list( APPEND targ_link_libs pthread dl GL OpenCL X11 )
endif()

target_link_libraries( dvm_owned_dedi ${targ_link_libs} )

llvm_config( dvm_owned_dedi   
						BitReader
					  	AsmParser
					  	Core
					  	Support
						X86CodeGen
						MC
						Linker
						IPO
						RuntimeDyld
						ExecutionEngine
						JIT
						MCJIT
						X86AsmParser
						Instrumentation
)

# make a nice hierachy
set_target_properties( dvm_owned_dedi PROPERTIES FOLDER "Apps")

