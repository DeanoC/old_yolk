
function(PROTOBUF_GENERATE_CPP SRCS HDRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without any proto files")
    return()
  endif(NOT ARGN)

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    
    list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc")
    list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h")

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h"
      COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS --cpp_out  ${CMAKE_CURRENT_BINARY_DIR} --proto_path ${CMAKE_CURRENT_SOURCE_DIR} ${ABS_FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C++ protocol buffer compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

FIND_PATH( Protobuf_INCLUDE_DIRS "google/protobuf/service.h"
	PATHS
		"${CMAKE_CURRENT_SOURCE_DIR}/third/"
	DOC
		"The Google Protocol Buffers Include Directory"
)
find_program( PROTOBUF_PROTOC_EXECUTABLE NAMES protoc
		DOC 
			"The Google Protocol Buffers Compiler"
		PATHS
			"${CMAKE_CURRENT_SOURCE_DIR}/third/google/protobuf/bin"
				
)

# Google's provided vcproj files generate libraries with a "lib"
# prefix on Windows
if(WIN32)
    set(PROTOBUF_ORIG_FIND_LIBRARY_PREFIXES "${CMAKE_FIND_LIBRARY_PREFIXES}")
    set(CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
endif()

SET( PROTOBUF_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/third/google/protobuf/lib" )
IF( IS_HOST_64_BIT )
	SET( PROTOBUF_PATH "${PROTOBUF_PATH}/x64" )
ELSE()
	SET( PROTOBUF_PATH "${PROTOBUF_PATH}/x86" )
ENDIF()

FIND_LIBRARY(Protobuf_LIBRARY_RELEASE 
	protobuf${_host_COMPILER}
	PATHS
		${PROTOBUF_PATH}
)
FIND_LIBRARY(Protobuf_LIBRARY_DEBUG 
	protobuf${_host_COMPILER}_d
	PATHS
		${PROTOBUF_PATH}
)
FIND_LIBRARY(Protobuf-lite_LIBRARY_RELEASE 
	protobuf-lite${_host_COMPILER}
	PATHS
		${PROTOBUF_PATH}
)
FIND_LIBRARY(Protobuf-lite_LIBRARY_DEBUG 
	protobuf-lite${_host_COMPILER}_d
	PATHS
		${PROTOBUF_PATH}
)

# Restore original find library prefixes
if(WIN32)
    set(CMAKE_FIND_LIBRARY_PREFIXES "${PROTOBUF_ORIG_FIND_LIBRARY_PREFIXES}")
endif()

SET( Protobuf_FOUND 1 )
SET( Protobuf_LIBRARIES debug ${Protobuf_LIBRARY_DEBUG} optimized ${Protobuf_LIBRARY_RELEASE} )
SET( Protobuf-lite_LIBRARIES debug ${Protobuf-lite_LIBRARY_DEBUG} optimized ${Protobuf-lite_LIBRARY_RELEASE} )
MARK_AS_ADVANCED(Protobuf_INCLUDE_DIRS Protobuf-lite_LIBRARY_DEBUG Protobuf-lite_LIBRARY_RELEASE Protobuf_LIBRARY_DEBUG Protobuf_LIBRARY_RELEASE PROTOBUF_PATH )
