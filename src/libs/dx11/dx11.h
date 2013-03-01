#pragma once

#if !defined(YOLK_DX11_DX11_H_)
#define YOLK_DX11_DX11_H_

#include "core/core.h"
#include "core/vector_math.h"

#if !defined(DIRECT3D11_H)
#include <d3d11.h>

#include <boost/intrusive_ptr.hpp>
#include <boost/type_traits.hpp>

// intrusive pointer code borrowed from Tk11 - tk11.sourceforge.net
namespace boost {
template<typename T> inline typename boost::enable_if< boost::is_base_of<IUnknown, T> >::type intrusive_ptr_add_ref(T* p) { p->AddRef(); }
template<typename T> inline typename boost::enable_if< boost::is_base_of<IUnknown, T> >::type intrusive_ptr_release(T* p) { p->Release(); }
}
struct ID3D11ShaderReflection;
namespace Dx11 {
template<class Target, class Source> boost::intrusive_ptr<Target> query_interface(Source ps, const IID& iid = __uuidof(Target)) {
	void* pv = 0;
	ps->QueryInterface(iid, &pv);
	Target* pv2 = static_cast<Target*>(pv);
	boost::intrusive_ptr<Target> target(pv2, false);
	return target;
}
typedef boost::intrusive_ptr< ID3D11Device >				D3DDevicePtr;
typedef boost::intrusive_ptr< ID3D11DeviceContext >			D3DDeviceContextPtr;
typedef boost::intrusive_ptr< IDXGIAdapter1 >				DXGIAdapterPtr;
typedef boost::intrusive_ptr< IDXGISwapChain >				DXGISwapChainPtr;
typedef boost::intrusive_ptr< ID3D11Resource >				D3DResourcePtr;
typedef boost::intrusive_ptr< ID3D11View >					D3DViewPtr;
typedef boost::intrusive_ptr< ID3D11Buffer >				D3DBufferPtr;
typedef boost::intrusive_ptr< ID3D11ShaderResourceView >	D3DShaderResourceViewPtr;
typedef boost::intrusive_ptr< ID3D11UnorderedAccessView >	D3DUnorderedAccessViewPtr;
typedef boost::intrusive_ptr< ID3D11DepthStencilView >		D3DDepthStencilViewPtr;
typedef boost::intrusive_ptr< ID3DBlob >					D3DBlobPtr;
typedef boost::intrusive_ptr< ID3D11DeviceChild >			D3DDeviceChildPtr;
typedef boost::intrusive_ptr< ID3D11ShaderReflection >		D3DShaderReflectionPtr;
typedef boost::intrusive_ptr< ID3D11DeviceChild >			D3DDeviceChildPtr;
typedef boost::intrusive_ptr< ID3D11InputLayout >			D3DInputLayoutPtr;
typedef boost::intrusive_ptr< ID3D11SamplerState >			D3DSamplerStatePtr;
typedef boost::intrusive_ptr< ID3D11BlendState >			D3DBlendStatePtr;
typedef boost::intrusive_ptr< ID3D11DepthStencilState >		D3DDepthStencilStatePtr;
typedef boost::intrusive_ptr< ID3D11RasterizerState >		D3DRasterizerStatePtr;

}
#define DIRECT3D11_H
#endif

//forward decl
struct ID3D11Device;

#if DEBUG_LEVEL >= DEBUG_NONE

// todo new error handler
/// DXFAIL requires HRESULT hr to be in scope
#	define DXFAIL(x)	if( FAILED(hr = (x)) ) { /*DXTRACE_ERR_MSGBOX("DX Error: "#x,hr ); */ CoreThrowException( FatalDirectX, "" ); }
/// DXWARN requires HRESULT hr to be in scope
#	define DXWARN(x)	if( FAILED(hr = (x)) ){ /*DXTRACE_ERR(#x,hr);*/ }

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


#endif // YOLK_DX11_DX11_H_
