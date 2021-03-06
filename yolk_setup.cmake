cmake_minimum_required( VERSION 3.9)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# TODO work out HEADLESS from lack of X on UNIX??
SET( HEADLESS_PLATFORM 0 CACHE STRING "Set to 1 if platform is headless and has no keyboard etc.")

LIST( APPEND CMAKE_MODULE_PATH 	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules" )


find_package( CMakeCommon REQUIRED )
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
find_package( Boost REQUIRED COMPONENTS thread chrono filesystem program_options system )

IF( NOT HEADLESS_PLATFORM )
	IF( WIN32 )
		SET( USE_DX11 true CACHE BOOL "Use DirectX 11 render back end" )
		SET( USE_OPENGL false CACHE BOOL "Use OpenGL render back end" )
	ELSE( WIN32 )
		SET( USE_DX11 false CACHE BOOL "Use DirectX 11 render back end" )
		SET( USE_OPENGL true CACHE BOOL "Use OpenGL render back end" )
	ENDIF( WIN32 )
ELSE( NOT HEADLESS_PLATFORM )
	SET( USE_DX11 false CACHE BOOL "Use DirectX 11 render back end" )
	SET( USE_OPENGL false CACHE BOOL "Use OpenGL render back end" )	
ENDIF( NOT HEADLESS_PLATFORM )

SET( USE_GC false CACHE BOOL "Use BDW C/C++ Garbage Collection system (needed for Swf Runtime)" )

SET( USE_OPENCL false CACHE BOOL "Use OpenCL" )
SET( USE_TBB true CACHE BOOL "Use Intel Thread Building Blocks system" )
SET( USE_ASSIMP false CACHE BOOL "Use ASS mesh importer system" )
SET( USE_DEVIL false CACHE BOOL "Use Devil image importer and exporter system" )
SET( USE_V8 false CACHE BOOL "Use Googles V8 Javascript system" )
SET( USE_DWM false CACHE BOOL "Use C++ Sandbox" )
SET( USE_RAIK false CACHE BOOL "Use RIAK cpp library" )
SET( USE_RAKNET false CACHE BOOL "Use RAKNET library" )
SET( USE_OPENCV false CACHE BOOL "Use OpenCV library" )

SET( USE_AMP false CACHE BOOL "Use C++ AMP GPU Accelerator libs" )

SET( USE_LLVM false CACHE BOOL "Use LLVM compiler libs" )

IF( USE_DOXYGEN_GENERATOR )
	include( UseDoxygen.cmake )
ENDIF()
IF( USE_DX11 )
	find_package( DirectX  )
ENDIF()
IF( USE_V8 )
	find_package( V8 )
ENDIF()
IF( USE_TBB )
#	find_package( WierdTBB  )
ENDIF()
IF( USE_ASSIMP )
	find_package( WierdAssImp  )
ENDIF()
IF( USE_BINIFY )
	find_package( WierdBinify  )
ENDIF()
IF( USE_DEVIL )
	find_package( WierdDevil  )
ENDIF()
IF( USE_OPENCL )
	find_package( OpenCL )
ENDIF()
IF( USE_LLVM ) 
	find_package( LLVM )
ENDIF()

# if we have boost setup the include and libs
if( Boost_FOUND )
	include_directories( ${Boost_INCLUDE_DIRS} )
	link_directories( ${Boost_LIBRARY_DIRS} )
endif()

# if we have dx11 set it up
if( DX11_FOUND AND USE_DX11 )
	include_directories( ${DX11_INCLUDE_DIRS} )
	link_directories( ${DX11_LIBRARY_DIRS} )
endif( DX11_FOUND AND USE_DX11 )

if( V8_FOUND AND USE_V8 )
	include_directories( ${V8_INCLUDE_DIR} )
	link_directories( ${V8_LIBRARY_DIRS} )
endif( V8_FOUND AND USE_V8 )

if( TBB_FOUND AND USE_TBB )
	include_directories( ${TBB_INCLUDE_DIRS} )
	link_directories( ${TBB_LIBRARY_DIRS} )
endif( TBB_FOUND AND USE_TBB )

if( OPENCL_FOUND AND USE_OPENCL )
	include_directories( ${OPENCL_INCLUDE_DIRS} )
	link_directories( ${OPENCL_LIB_DIR} )
endif( OPENCL_FOUND AND USE_OPENCL )

if( USE_LLVM )
	include_directories( ${LLVM_INCLUDE_DIRS} )
	link_directories( ${LLVM_LIBRARY_DIRS} )
endif( USE_LLVM )

# special handling for the various submodules bits and peices
include_directories( ${CMAKE_SOURCE_DIR}/src/libs/glog/include/ )
IF( USE_PROTOBUFFERS )
	find_package( WierdProtobuf  )
	include_directories( ${CMAKE_SOURCE_DIR}/src/libs/protobuf/src/ )
ENDIF( USE_PROTOBUFFERS )

include_directories( ${CMAKE_BINARY_DIR}/src/libs/zlib/ )
include_directories( ${CMAKE_SOURCE_DIR}/src/libs/zlib/ )
include_directories( ${CMAKE_SOURCE_DIR}/src/libs/jpeg/ )
include_directories( ${CMAKE_SOURCE_DIR}/src/libs/tbb/include/ )
include_directories( ${CMAKE_SOURCE_DIR}/src/libs/bullet/src/ )
include_directories( ${CMAKE_SOURCE_DIR}/src/libs/bdwgc/include/ )

IF( USE_RIAK )
	include_directories( ${CMAKE_SOURCE_DIR}/src/libs/riak-cpp/ )
	include_directories( ${CMAKE_BINARY_DIR}/src/libs/riak-cpp/ )
ENDIF( USE_RIAK )

IF( USE_RAKNET )
	include_directories( ${CMAKE_SOURCE_DIR}/src/libs/raknet/Source )
ENDIF( USE_RAKNET )

IF( USE_LLVM )
	add_definitions( ${LLVM_CFLAGS} )
	include_directories( ${LLVM_INCLUDE_DIR} )
	link_directories( ${LLVM_LIBRARY_DIR} )
ENDIF( USE_LLVM )

IF( USE_DWM )
if( WIN32 )
	set( LLVM_PREPROCESSOR ${CMAKE_SOURCE_DIR}/../vt_repo/bin/clang.exe -S )
	set( LLVM_ASM ${CMAKE_SOURCE_DIR}/../vt_repo/bin/llvm-mc.exe )
else()
  set( LLVM_PREPROCESSOR ${CMAKE_SOURCE_DIR}/../vt_repo/bin/clang -S )
  set( LLVM_ASM ${CMAKE_SOURCE_DIR}/../vt_repo/bin/llvm-mc )
endif()
ENDIF( USE_DWM )

set( USE_GLOG TRUE )
set( Core_LIBRARIES core ${Boost_LIBRARIES} glog )
