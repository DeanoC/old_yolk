# Locate BOOST library
# This module defines
#  BOOST_FOUND, if false, do not try to link to boost
#  BOOST_LIBRARY
#  BOOST_INCLUDE_DIR, where to find include
#
# this is an "enhanced boost find" if the user doesn't have it correctly installed
#
# it falls back to our copy installed in third party
# setup boost version and handle dual 32/64 buld directories

# required version
#SET(Boost_ADDITIONAL_VERSIONS "1.44" "1.44.0")
	
# extra windows standard version
if( WIN32 )
	if( IS_HOST_64_BIT )
		set( BOOST_ROOT "C:/boost/x64/" )
	else()
		set( BOOST_ROOT "C:/boost/x86/" )
	endif()
endif()

# debug
add_definitions( ${Boost_LIB_DIAGNOSTIC_DEFINITIONS} )

# pick the right boost
set( Boost_USE_STATIC_LIBS   ON )
set( Boost_USE_STATIC_RUNTIME ON )
set( Boost_USE_MULTITHREADED ON )
set( BOOST_VERSION 1.48.0 )
set( BOOST_COMPONENTS program_options thread date_time regex system )
set( Boost_DEBUG OFF )

find_package( Boost ${BOOST_VERSION} QUIET COMPONENTS ${BOOST_COMPONENTS} )

IF( NOT Boost_FOUND )

	set( BOOST_INCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/third/boost/include" )
	set( BOOST_INCLUDE_DIR ${BOOST_INCLUDEDIR} )
	
	if( IS_HOST_64_BIT )
		set( BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third/boost/x64/" )
	else()
		set( BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third/boost/x86/" )
	endif()
	
	find_package( Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} )
	
ENDIF( NOT Boost_FOUND )
