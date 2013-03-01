cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx *.proto )

# convert proto files
file( GLOB proto_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.proto )
PROTOBUF_GENERATE_CPP( proto_src_files proto_header_files ${proto_files} )
list( APPEND cpp_src_files ${proto_src_files} )
list( APPEND header_files ${proto_header_files} )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

add_library( protocols ${src_files} ${header_files} ${vm_files} )

# make a nice hierachy
set_target_properties( protocols PROPERTIES FOLDER "Libs")
