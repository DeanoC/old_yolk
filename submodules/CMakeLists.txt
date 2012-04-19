#do it manually for now, as lots of time and project make sln ugly
SET( BUILD_SHARED_LIBS, OFF )
add_subdirectory( zlib )
#add_subdirectory( llvm )
add_subdirectory( json-spirit )
add_subdirectory( protobuf )