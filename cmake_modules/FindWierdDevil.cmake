if( NOT WIN32 )
# TODO
	include( FindDevIL.cmake )

else()
	FIND_PATH( DEVIL_INCLUDE_DIRS "IL/config.h"
		PATHS
			"${CMAKE_CURRENT_SOURCE_DIR}/third/devil/include"
		DOC
			"devil import path"
	)

	SET( Devil_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/third/devil/lib" )
	IF( IS_HOST_64_BIT )
		SET( Devil_PATH "${Devil_PATH}/x64" )
	ELSE()
		SET( Devil_PATH "${Devil_PATH}/x86" )
	ENDIF()

	FIND_LIBRARY(Devil_LIBRARY_RELEASE 
		devil
		PATHS
			${Devil_PATH}
	)
	
	FIND_LIBRARY(Devil_LIBRARY_DEBUG 
		devil
		PATHS
			${Devil_PATH}
	)

	SET( DEVIL_FOUND 1 )
	SET( DEVIL_LIBRARIES debug ${Devil_LIBRARY_DEBUG} optimized ${Devil_LIBRARY_RELEASE} )
	MARK_AS_ADVANCED( Devil_LIBRARY_DEBUG Devil_LIBRARY_RELEASE Devil_PATH )
endif()