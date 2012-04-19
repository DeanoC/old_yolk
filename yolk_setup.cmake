# CMake policies ( essentially pragma to use old or new behaviour)
# use all new policies as of 2.8.2
cmake_policy( VERSION 2.8.2 )

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set( LLVM_ROOT "${CMAKE_CURRENT_BINARY_DIR}/submodules/llvm" CACHE PATH "Root of LLVM install.")

LIST( APPEND CMAKE_MODULE_PATH 	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules" )

find_package( CMakeCommon REQUIRED )
find_package( WierdBoost REQUIRED )

SET( USE_V8 false CACHE BOOL "Use Googles V8 Javascript system" )
SET( USE_DX11 false CACHE BOOL "Use DirectX 11 " )
SET( USE_OPENGL false CACHE BOOL "Use OpenGL render back end" )
SET( USE_TBB false CACHE BOOL "Use Intel Thread Building Blocks system" )
SET( USE_ASSIMP false CACHE BOOL "Use ASS mesh importer system" )
SET( USE_DEVIL false CACHE BOOL "Use Devil image importer and exporter system" )

IF( USE_DX11 )
	find_package( DirectX  )
ENDIF()
IF( USE_V8 )
	find_package( V8 )
ENDIF()
#find_package( WierdProtobuf  )
IF( USE_TBB )
	find_package( WierdTBB  )
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
IF( USE_OPENGL )
	find_package( OpenCL )
ENDIF()

SET( USE_DOXYGEN_GENERATOR 0 CACHE BOOL "Generate documentation using doxygen" ) 
if( USE_DOXYGEN_GENERATOR )
include( UseDoxygen.cmake )
endif()

if( USE_OPENGL )
	set( USE_OPENCL true )
elseif()
	SET( USE_OPENCL false CACHE BOOL "Use OpenCL compute language" )
endif()
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

if( USE_OPENGL )
	include_directories("${WIERD_PATH}/third/glload/include")
endif( USE_OPENGL )

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

# special handling for the various submodules bits and peices
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/glog/include/" )
LIST( APPEND CMAKE_MODULE_PATH "${LLVM_ROOT}/share/llvm/cmake" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/submodules/llvm/include/" )
include_directories( "${CMAKE_CURRENT_BINARY_DIR}/submodules/llvm/include/" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/submodules/json-spirit/" )
include_directories( "${CMAKE_CURRENT_BINARY_DIR}/submodules/zlib/" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/submodules/zlib/" )
link_directories( ${CMAKE_CURRENT_BINARY_DIR}/submodules/llvm/lib )

set( Core_LIBRARIES core ${Boost_LIBRARIES} glog )
