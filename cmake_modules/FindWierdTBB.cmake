if( NOT WIN32 )
	include( FindTBB.cmake )

else()
	FIND_PATH( TBB_INCLUDE_DIRS "tbb/atomic.h"
		PATHS
			"${CMAKE_CURRENT_SOURCE_DIR}/libs/tbb/include"
		DOC
			"The Intel Thread Building Blocks Include Directory"
	)

	SET( Tbb_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/libs/tbb/lib" )
	IF( IS_HOST_64_BIT )
		SET( Tbb_PATH "${Tbb_PATH}/x64" )
	ELSE()
		SET( Tbb_PATH "${Tbb_PATH}/x86" )
	ENDIF()

	FIND_LIBRARY(Tbb_LIBRARY_RELEASE 
		tbb${_host_COMPILER}
		PATHS
			${Tbb_PATH}
	)

	FIND_LIBRARY(Tbb_LIBRARY_DEBUG 
		tbb${_host_COMPILER}_d
		PATHS
			${Tbb_PATH}
	)
	FIND_LIBRARY(Tbbmalloc_LIBRARY_RELEASE 
		tbbmalloc${_host_COMPILER}
		PATHS
			${Tbb_PATH}
	)
	FIND_LIBRARY(Tbbmalloc_LIBRARY_DEBUG 
		tbbmalloc${_host_COMPILER}_d
		PATHS
			${Tbb_PATH}
	)

	FIND_LIBRARY(Tbbmallocproxy_LIBRARY_RELEASE 
		tbbmalloc_proxy
		PATHS
			${Tbb_PATH}
	)
	FIND_LIBRARY(Tbbmallocproxy_LIBRARY_DEBUG 
		tbbmalloc_proxy_debug
		PATHS
			${Tbb_PATH}
	)

	SET( TBB_FOUND 1 )
	SET( TBB_LIBRARY_DIRS 	debug ${Tbb_LIBRARY_DEBUG} optimized ${Tbb_LIBRARY_RELEASE} 
							debug ${Tbbmalloc_LIBRARY_DEBUG} optimized ${Tbbmalloc_LIBRARY_RELEASE} 
							debug ${Tbbmallocproxy_LIBRARY_DEBUG} optimized ${Tbbmallocproxy_LIBRARY_RELEASE} 
							)
	MARK_AS_ADVANCED( 	Tbbmalloc_LIBRARY_DEBUG Tbbmalloc_LIBRARY_RELEASE 
						Tbb_LIBRARY_DEBUG Tbb_LIBRARY_RELEASE 
						Tbbmallocproxy_LIBRARY_DEBUG Tbbmallocproxy_LIBRARY_RELEASE
						Tbb_PATH )
endif()