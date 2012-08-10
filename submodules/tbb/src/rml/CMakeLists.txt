cmake_minimum_required( VERSION 2.8 )

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

LIST( APPEND rml_src_files
  server/rml_server.cpp
)
add_library( tbb_rml ${rml_src_files} )
if( UNIX )
	add_definitions( "-DUSE_PTHREAD")
else()
	add_definitions( "-DTBB_USE_CAPTURED_EXCEPTION -DUSE_WINTHREAD" )
endif()

# make a nice hierachy
set_target_properties( tbb_rml PROPERTIES FOLDER "Libs" )
