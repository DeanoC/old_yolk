cmake_minimum_required( VERSION 2.8 )


# We incorporate the CMake features provided by LLVM:
include(LLVMConfig)

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

# ADD_MSVC_PRECOMPILED_HEADER( "convertmesh_pch.h" "${cpp_src_files}" )
add_executable( dvm_owned_dedi ${src_files} ${header_files} )

list( APPEND targ_link_libs ${Core_LIBRARIES} dwm riak-cpp protobuf json_spirit protocols)
if( UNIX )
	list( APPEND targ_link_libs pthread dl )
endif()

target_link_libraries( dvm_owned_dedi ${targ_link_libs} )

llvm_config( dvm_owned_dedi   mcjit jit nativecodegen 
					  interpreter
					  BitReader
					  AsmParser
					  Core
					  Support
)

# make a nice hierachy
set_target_properties( dvm_owned_dedi PROPERTIES FOLDER "Apps")
