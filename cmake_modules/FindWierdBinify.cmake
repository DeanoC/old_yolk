if( NOT WIN32 )
# TODO
	include( FindAssImp.cmake )

else()
#	FIND_PATH( ASSIMP_INCLUDE_DIRS "aiConfig.h"
#		PATHS
#			"${CMAKE_CURRENT_SOURCE_DIR}/third/assimp/include"
#		DOC
#			"Ass mesh import path"
#	)

	SET( Binify_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/third/binify/lib" )
	IF( IS_HOST_64_BIT )
		SET( Binify_PATH "${Binify_PATH}/x64" )
	ELSE()
		SET( Binify_PATH "${Binify_PATH}/x86" )
	ENDIF()

	FIND_LIBRARY(Binify_LIBRARY_RELEASE 
		binify${_host_COMPILER}
		PATHS
			${Binify_PATH}
	)
	
	FIND_LIBRARY(Binify_LIBRARY_DEBUG 
		binify${_host_COMPILER}_d
		PATHS
			${Binify_PATH}
	)

	SET( BINIFY_FOUND 1 )
	SET( BINIFY_LIBRARIES debug ${Binify_LIBRARY_DEBUG} optimized ${Binify_LIBRARY_RELEASE} )
	MARK_AS_ADVANCED( Binify_LIBRARY_DEBUG Binify_LIBRARY_RELEASE Binify_PATH )
endif()