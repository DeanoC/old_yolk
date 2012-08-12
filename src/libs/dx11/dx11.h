#pragma once

#if !defined(YOLK_DX11_DX11_H_)
#define YOLK_DX11_DX11_H_

#include "core/core.h"
#include "core/vector_math.h"

//#define USE_DIRECTWRITE

#if !defined(DIRECT3D11_H)
#include <d3d11.h>
#include <d3dx11.h>
#include <D3Dcompiler.h>
#include <comdef.h>

typedef _com_ptr_t< _com_IIID< IDXGIAdapter1, &IID_IDXGIAdapter1> >					DXGIAdapterPtr;
typedef _com_ptr_t< _com_IIID< ID3D11Resource, &IID_ID3D11Resource>  >				D3D11ResourceSmartPtr;
typedef _com_ptr_t< _com_IIID< ID3D11Buffer, &IID_ID3D11Buffer>  >					D3D11BufferSmartPtr;
typedef _com_ptr_t< _com_IIID< ID3D11ShaderResourceView, &IID_ID3D11ShaderResourceView>  >	D3D11ShaderResourceViewSmartPtr;
typedef _com_ptr_t< _com_IIID< ID3D11UnorderedAccessView, &IID_ID3D11UnorderedAccessView>  >	D3D11UnorderedAccessViewSmartPtr;
typedef _com_ptr_t< _com_IIID< ID3D11DepthStencilView, &IID_ID3D11DepthStencilView>  >	D3D11DepthStencilViewSmartPtr;
typedef _com_ptr_t< _com_IIID< ID3DBlob, &IID_ID3DBlob>  >						D3D11BlobSmartPtr;

#define DIRECT3D11_H
#endif

//forward decl
struct ID3D11Device;

#if DEBUG_LEVEL >= DEBUG_NONE

#include <dxerr.h>

/// DXFAIL requires HRESULT hr to be in scope
#	define DXFAIL(x)	if( FAILED(hr = (x)) ) { DXTRACE_ERR_MSGBOX("DX Error: "#x,hr ); CoreThrowException( FatalDirectX, "" ); }
/// DXWARN requires HRESULT hr to be in scope
#	define DXWARN(x)	if( FAILED(hr = (x)) ){ DXTRACE_ERR(#x,hr); }

#else

/// DXFAIL requires HRESULT hr to be in scope
#	define DXFAIL(x) hr = (x)
/// DXWARN requires HRESULT hr to be in scope
#	define DXWARN(x) hr = (x)

#endif

#include "gfx.h"
#include "dxgiformat_cracker.h"

// debug helpers 
DECLARE_EXCEPTION(FatalDirectX, Fatal DirectX error occured);


#endif // WIERD_GEOMETRY_STDAFX_H
