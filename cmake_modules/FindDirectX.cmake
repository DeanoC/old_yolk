IF (WIN32)

	FIND_PATH(DX9_INCLUDE_DIRS d3d9.h
		PATHS
			"$ENV{DXSDK_DIR}/Include"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Include"
		DOC "The directory where D3D9.h resides")

	FIND_PATH(DX10_INCLUDE_DIRS D3D10.h
		PATHS
			"$ENV{DXSDK_DIR}/Include"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Include"
		DOC "The directory where D3D10.h resides")

	FIND_PATH(DX11_INCLUDE_DIRS D3D11.h
		PATHS
			"$ENV{DXSDK_DIR}/Include"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Include"
		DOC "The directory where D3D10.h resides")

	# DX libraries
	IF(IS_HOST_64_BIT)
		FIND_PATH(DX9_LIBRARY_DIRS d3d9.lib
			PATHS
				"$ENV{DXSDK_DIR}/Lib/x64"
				"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x64"
			DOC "The directory where D3D9.lib resides")

		FIND_PATH(DX10_LIBRARY_DIRS d3d10.lib
			PATHS
				"$ENV{DXSDK_DIR}/Lib/x64"
				"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x64"
			DOC "The directory where D3D10.lib resides")

		FIND_PATH(DX11_LIBRARY_DIRS d3d11.lib
			PATHS
				"$ENV{DXSDK_DIR}/Lib/x64"
				"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x64"
			DOC "The directory where D3D11.lib resides")
			
	ELSE()
		FIND_PATH(DX9_LIBRARY_DIRS d3d9.lib
			PATHS
				"$ENV{DXSDK_DIR}/Lib/x86"
				"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x86"
			DOC "The directory where D3D9.lib resides")

		FIND_PATH(DX10_LIBRARY_DIRS d3d10.lib
			PATHS
				"$ENV{DXSDK_DIR}/Lib/x86"
				"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x86"
			DOC "The directory where D3D10.lib resides")

		FIND_PATH(DX11_LIBRARY_DIRS d3d11.lib
			PATHS
				"$ENV{DXSDK_DIR}/Lib/x86"
				"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x86"
			DOC "The directory where D3D11.lib resides")

	ENDIF()

	SET(  DX9_LIBRARIES debug ${DX9_LIBRARY_DIRS}/d3d9.lib ${DX9_LIBRARY_DIRS}/dxerr.lib optimized ${DX9_LIBRARY_DIRS}/d3d9.lib )
	SET( DX10_LIBRARIES debug ${DX10_LIBRARY_DIRS}/d3d10.lib ${DX10_LIBRARY_DIRS}/dxerr.lib optimized ${DX10_LIBRARY_DIRS}/d3d10.lib )
	SET( DX11_LIBRARIES debug ${DX11_LIBRARY_DIRS}/d3d11.lib ${DX11_LIBRARY_DIRS}/dxerr.lib optimized ${DX11_LIBRARY_DIRS}/d3d11d.lib )	

	SET(  DX9X_LIBRARIES debug ${DX9_LIBRARY_DIRS}/d3dx9d.lib optimized ${DX9_LIBRARY_DIRS}/d3dx9.lib )
	SET( DX10X_LIBRARIES debug ${DX10_LIBRARY_DIRS}/d3dx10d.lib optimized ${DX10_LIBRARY_DIRS}/d3dx10.lib )
	SET( DX11X_LIBRARIES debug ${DX11_LIBRARY_DIRS}/d3dx11d.lib ${DX11_LIBRARY_DIRS}/d3dcsx.lib optimized ${DX11_LIBRARY_DIRS}/d3dx11.lib ${DX11_LIBRARY_DIRS}/d3dcsxd.lib )
	
	LIST( APPEND DX9_LIBRARIES ${DX10_LIBRARY_DIRS}/dxguid.lib )
	LIST( APPEND DX10_LIBRARIES ${DX10_LIBRARY_DIRS}/dxguid.lib )
	LIST( APPEND DX11_LIBRARIES ${DX11_LIBRARY_DIRS}/dxguid.lib )
	
	LIST( APPEND DX10_LIBRARIES ${DX10_LIBRARY_DIRS}/dxgi.lib )
	LIST( APPEND DX11_LIBRARIES ${DX11_LIBRARY_DIRS}/dxgi.lib )
	LIST( APPEND DX11_LIBRARIES ${DX11_LIBRARY_DIRS}/d3dcompiler.lib )
	
ENDIF (WIN32)

IF (DX9_INCLUDE_DIRS AND DX9_LIBRARY_DIRS )
	SET( DX9_FOUND 1 CACHE STRING "Set to 1 if DX9 is found, 0 otherwise")
ENDIF()
IF (DX10_INCLUDE_DIRS AND DX10_LIBRARY_DIRS )
	SET( DX10_FOUND 1 CACHE STRING "Set to 1 if DX10 is found, 0 otherwise")
ENDIF()
IF (DX11_INCLUDE_DIRS AND DX11_LIBRARY_DIRS )
	SET( DX11_FOUND 1 CACHE STRING "Set to 1 if DX11 is found, 0 otherwise")
ENDIF()

MARK_AS_ADVANCED( DX9_FOUND )
MARK_AS_ADVANCED( DX10_FOUND )
MARK_AS_ADVANCED( DX11_FOUND )