# make procompiled headers easier
function(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader SourcesVar)
  IF(MSVC AND NOT ${CMAKE_GENERATOR} STREQUAL "NMake Makefiles" )
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "$(IntDir)/${PrecompiledBasename}.pch")
	
    SET_SOURCE_FILES_PROPERTIES(${SourcesVar}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledBinary}\" /FI\"${PrecompiledBinary}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    SET_SOURCE_FILES_PROPERTIES( "${PrecompiledBasename}.cpp"
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
  ENDIF()
endfunction(ADD_MSVC_PRECOMPILED_HEADER)

# check 64 bit
if( CMAKE_SIZEOF_VOID_P MATCHES 4 )
	set( IS_HOST_64_BIT 0 )
else()
	set( IS_HOST_64_BIT 1 )
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"
	OR "${CMAKE_CXX_COMPILER}" MATCHES "icl" 
	OR "${CMAKE_CXX_COMPILER}" MATCHES "icpc")
	if(WIN32)
		set (_host_COMPILER "_iw")
	else()
		set (_host_COMPILER "_il")
	endif()
elseif (MSVC90)
	SET (_host_COMPILER "_vc90")
elseif (MSVC10)
	SET (_host_COMPILER "_vc100")
elseif (UNIX)
	if (CMAKE_COMPILER_IS_GNUCXX)
		if(APPLE)
			SET (_host_COMPILER "_xgcc")
		else()
			SET (_host_COMPILER "_gcc")
		endif()
	else()
		message( "Unknown UNIX compiler" )
	endif()
else()
		message( "Unknown compiler" )
endif()