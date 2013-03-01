if( NOT WIN32 )
# TODO
	include( FindAssImp.cmake )

else()
	FIND_PATH( ASSIMP_INCLUDE_DIRS "aiConfig.h"
		PATHS
			"${CMAKE_CURRENT_SOURCE_DIR}/third/assimp/include"
		DOC
			"Ass mesh import path"
	)

	SET( Ass_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/third/assimp/lib" )
	IF( IS_HOST_64_BIT )
		SET( Ass_PATH "${Ass_PATH}/x64" )
	ELSE()
		SET( Ass_PATH "${Ass_PATH}/x86" )
	ENDIF()

	FIND_LIBRARY(Ass_LIBRARY_RELEASE 
		assimp${_host_COMPILER}
		PATHS
			${Ass_PATH}
	)
	
	FIND_LIBRARY(Ass_LIBRARY_DEBUG 
		assimp${_host_COMPILER}_d
		PATHS
			${Ass_PATH}
	)

	SET( ASSIMP_FOUND 1 )
	SET( ASSIMP_LIBRARIES debug ${Ass_LIBRARY_DEBUG} optimized ${Ass_LIBRARY_RELEASE} )
	MARK_AS_ADVANCED( Ass_LIBRARY_DEBUG Ass_LIBRARY_RELEASE Ass_PATH )
endif()