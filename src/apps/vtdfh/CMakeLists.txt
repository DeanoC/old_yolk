cmake_minimum_required( VERSION 2.8 )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

# source files
file( GLOB camera_src_files cameras/*.cpp )
file( GLOB enemies_src_files enemies/*.cpp )
file( GLOB items_src_files items/*.cpp )
file( GLOB levels_src_files levels/*.cpp )
file( GLOB player_src_files player/*.cpp )

file( GLOB camera_hdr_files cameras/*.h )
file( GLOB enemies_hdr_files enemies/*.h )
file( GLOB items_hdr_files items/*.h )
file( GLOB levels_hdr_files levels/*.h )
file( GLOB player_hdr_files player/*.h )

file( GLOB cpp_src_files 	*.cpp *.cxx 
							${camera_src_files} 
							${enemies_src_files} 
							${items_src_files} 
							${levels_src_files} 
							${player_src_files} 
	)
file( GLOB c_src_files *.c )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

file( GLOB hdr_files 	*.h *.inl *.hpp *.hxx 							
							${camera_hdr_files} 
							${enemies_hdr_files} 
							${items_hdr_files} 
							${levels_hdr_files} 
							${player_hdr_files} 
	)

# exe settings
if( UNIX )
	add_executable( vtdfh ${src_files} ${hdr_files} )
else()
	add_executable( vtdfh WIN32 ${src_files} ${hdr_files} )
	SET_TARGET_PROPERTIES( vtdfh PROPERTIES LINK_FLAGS "/SAFESEH:NO")	
endif()

# link settings
list( APPEND targ_link_libs ${Core_LIBRARIES} json_spirit scene localworld BulletCollision BulletDynamics LinearMath swfruntime )
IF( USE_DWM )
	list( APPEND targ_link_libs dwm_client )
ENDIF( USE_DWM )
if( USE_OPENGL )
	list( APPEND targ_link_libs gl )
endif()
if( USE_OPENCL )
	list( APPEND targ_link_libs cl ${OPENCL_LIBRARY} )
endif()
if( USE_DX11 )
	list( APPEND targ_link_libs dx11 d3d11 dxgi d3dcompiler dxguid)
endif()
if( UNIX )
	list( APPEND targ_link_libs pthread dl GL X11 )
endif()
target_link_libraries( vtdfh ${targ_link_libs} )


# tidy project files
ADD_MSVC_PRECOMPILED_HEADER( "vtdfh.h" "${cpp_src_files}" )
SOURCE_GROUP( cameras FILES ${camera_src_files} ${camera_hdr_files}  )
SOURCE_GROUP( enemies FILES ${enemies_src_files} ${enemies_hdr_files}  )
SOURCE_GROUP( items FILES ${items_src_files} ${items_hdr_files}  )
SOURCE_GROUP( levels FILES ${levels_src_files} ${levels_hdr_files}  )
SOURCE_GROUP( player FILES ${player_src_files} ${player_hdr_files}  )
set_target_properties( vtdfh PROPERTIES FOLDER "Apps")
