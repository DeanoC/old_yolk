include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/src/rml/include" )

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/src" )
add_subdirectory( src/tbb )
add_subdirectory( src/rml )
