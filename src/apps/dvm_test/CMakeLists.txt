cmake_minimum_required( VERSION 2.8 )


# We incorporate the CMake features provided by LLVM:
include(LLVMConfig)

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

# ADD_MSVC_PRECOMPILED_HEADER( "convertmesh_pch.h" "${cpp_src_files}" )
add_executable( dvm_test WIN32 ${src_files} ${header_files} )
target_link_libraries( dvm_test	${Core_LIBRARIES} )
llvm_config( dvm_test   mcjit jit nativecodegen 
					  interpreter
					  BitReader
					  AsmParser
					  Core
					  Support
)

# make a nice hierachy
set_target_properties( dvm_test PROPERTIES FOLDER "Apps")

