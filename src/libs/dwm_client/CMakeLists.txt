cmake_minimum_required( VERSION 2.8 )

file( GLOB cpp_src_files *.cpp *.cxx apis/*.cpp )
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

#ADD_MSVC_PRECOMPILED_HEADER( "dwm.h" "${cpp_src_files}" )
add_library( dwm_client STATIC ${src_files} ${header_files} )

list( APPEND targ_link_libs ${Core_LIBRARIES} scene )
if( USE_OPENGL )
	list( APPEND targ_link_libs gl )
endif()
if( USE_OPENCL )
	list( APPEND targ_link_libs cl ${OPENCL_LIBRARY} )
endif()
if( USE_DX11 )
	list( APPEND targ_link_libs dx11 D3D11 DXGI )
endif()

if( UNIX )
  list( APPEND targ_link_libs pthread dl )
endif()

target_link_libraries( dwm_client ${targ_link_libs} )

# make a nice hierachy
set_target_properties( dwm_client PROPERTIES FOLDER "Libs")
