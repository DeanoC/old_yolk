cmake_minimum_required( VERSION 2.8 )

set( APP_NAME resilience )

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

list( APPEND targ_link_libs ${Core_LIBRARIES} scene localworld BulletCollision BulletDynamics LinearMath )
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
	list( APPEND targ_link_libs dx11 d3d11 dxgi d3dcompiler dxguid shcore)
endif()
if( UNIX )
	list( APPEND targ_link_libs pthread dl GL X11 )
endif()

if( UNIX )
	add_executable(  ${APP_NAME} ${src_files} ${header_files} )
else()
	add_executable(  ${APP_NAME} WIN32 ${src_files} ${header_files} )
	SET_TARGET_PROPERTIES(  ${APP_NAME} PROPERTIES LINK_FLAGS "/SAFESEH:NO")	
endif()
target_link_libraries(  ${APP_NAME} ${targ_link_libs} )

# tidy project files
#ADD_MSVC_PRECOMPILED_HEADER( "pch.h" "${cpp_src_files}" )
#SOURCE_GROUP( cameras FILES ${camera_src_files} ${camera_hdr_files}  )

set_target_properties(  ${APP_NAME} PROPERTIES FOLDER "Apps")

