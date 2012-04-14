# Locate V8 library
# This module defines
#  V8_FOUND, if false, do not try to link to V8 
#  V8_LIBRARY
#  V8_INCLUDE_DIR, where to find v8.h 
#

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

FIND_PATH(V8_INCLUDE_DIR v8.h
  HINTS
  $ENV{V8_DIR}
  PATH_SUFFIXES include
  PATHS
  ${V8_ROOT}
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

IF( IS_HOST_64_BIT )
	SET( V8_LIBRARY_PATH ${V8_ROOT}/lib/x64 )
ELSE()
	SET( V8_LIBRARY_PATH ${V8_ROOT}/lib/x86 )
ENDIF()

FIND_LIBRARY( V8_LIBRARY_RELEASE v8${_host_COMPILER}
	HINTS
	$ENV{V8_DIR}
	PATH_SUFFIXES ${V8_LIBRARY_PATH}
	PATHS
	${V8_LIBRARY_PATH}
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)
FIND_LIBRARY( V8_LIBRARY_DEBUG v8${_host_COMPILER}_d
	HINTS
	$ENV{V8_DIR}
	PATH_SUFFIXES ${V8_LIBRARY_PATH}
	PATHS
	${V8_LIBRARY_PATH}
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)
SET( V8_LIBRARIES debug ${V8_LIBRARY_DEBUG} optimized ${V8_LIBRARY_RELEASE} )

INCLUDE(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LUA_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(V8  DEFAULT_MSG  V8_LIBRARIES V8_INCLUDE_DIR)

# V8 has a few windows lib dependecies have them linked in
IF( WIN32 )
	LIST( APPEND V8_LIBRARIES winmm.lib ws2_32.lib )
ENDIF( WIN32 )

MARK_AS_ADVANCED( V8_INCLUDE_DIR V8_LIBRARIES V8_LIBRARY_DEBUG V8_LIBRARY_RELEASE V8_LIBRARY_PATH )