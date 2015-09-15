cmake_minimum_required( VERSION 2.8 )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
link_directories(${CMAKE_CURRENT_SOURCE_DIR})

# configure file replace cmake macros (i.e @Wierd_VERSION_MAJOR@) with there value
# from the first file to the second, this makes our build config header
configure_file ( 
	"buildconfig.h.in"
	"buildconfig.h"
)

file( GLOB cpp_src_files *.cpp *.cxx)
file( GLOB c_src_files *.c )
file( GLOB header_files *.h *.inl *.hpp *.hxx)

if( WIN32 )
	file( GLOB platform_cpp_src_files platform_windows/*.cpp platform_windows/*.cpp)
	file( GLOB platform_c_src_files platform_windows/*.c )
	file( GLOB platform_header_files platform_windows/*.h platform_windows/*.inl platform_windows/*.hpp platform_windows/*.hxx )
endif()
if( UNIX )
	IF( HEADLESS_PLATFORM )
		list( APPEND platform_header_files platform_posix/debug_std.h )
	ELSE( HEADLESS_PLATFORM )
		add_definitions( "-DLOAD_X11" )
		file( GLOB platform_cpp_src_files platform_posix/*.cpp platform_posix/*.cxx)
		file( GLOB platform_c_src_files platform_posix/*.c )
		file( GLOB platform_header_files platform_posix/*.h platform_posix/*.inl platform_posix/*.hpp platform_posix/*.hxx )
		if( UNIX )
			list( APPEND targ_link_libs X11 )
		endif()
	ENDIF( HEADLESS_PLATFORM )
endif()

if( USE_TBB )
	list( APPEND targ_link_libs tbb )
endif()
if( USE_GC )
	list( APPEND targ_link_libs gc-lib )
endif()



list( APPEND cpp_src_files ${platform_cpp_src_files} )
list( APPEND c_src_files ${platform_c_src_files} )
list( APPEND header_files ${platform_header_files} )

list( APPEND header_files ${header_files} "${CMAKE_CURRENT_BINARY_DIR}/buildconfig.h" )
list( APPEND src_files ${cpp_src_files} ${c_src_files} )

ADD_MSVC_PRECOMPILED_HEADER( "core.h" "${cpp_src_files}" )
add_library( core ${src_files} ${header_files} )

target_link_libraries( core ${targ_link_libs} )

# make a nice hierachy
set_target_properties( core PROPERTIES FOLDER "Libs")
