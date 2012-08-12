//!-----------------------------------------------------
//!
//! \file gfx_debug_render.cpp
//! debug primitive renderer
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "rendermatrixcache.h"
#include "gfx_debug_render.h"
#include "wobfile.h"
#include "vdeclmanager.h"
#include "rendercontext.h"
#include "core/resourceman.h"
#include "scrconsole.h"


namespace Graphics
{
GfxDebugRender::GfxDebugRender() {
	pPrevDRI = Core::g_pDebugRender;
	Core::g_pDebugRender = this;
	debugContext = Gfx::Get()->getMainRenderContext();

	vertexBuffer = VBManager::Get()->AllocVertices( MAX_DEBUG_VERTICES * sizeof(Vertex) * NUM_TYPES_OF_DEBUG_PRIM, true );
	pVertices = (Vertex*) vertexBuffer.Lock( debugContext, VBManager::VBInstance::DISCARD );
	numLineVertices = 0;
	numLineWorldVertices = 0;

	WobVertexElement lineElements[] = { { WobVertexElement::WVTU_POSITION, WobVertexElement::WVTT_FLOAT3 },
										{ WobVertexElement::WVTU_COLOUR0, WobVertexElement::WVTT_BYTEARGB } };

	lineShader = ShaderFXHandle::Load( "DebugLine" );

	lineVDeclHandle = VDeclManager::Get()->GetVertexDeclarationHandle( 2, lineElements, lineShader, "" );
}

GfxDebugRender::~GfxDebugRender() {
	Core::ResourceMan::Get()->CloseResource<ShaderFXType>( lineShader );
	vertexBuffer.Unlock( debugContext );
	VBManager::Get()->FreeVertices( vertexBuffer );
	Core::g_pDebugRender = pPrevDRI;
}

void GfxDebugRender::NDCLine( const Core::Colour& colour, const Math::Vector2& a, const Math::Vector2& b ) {
	if( numLineVertices+2 > MAX_DEBUG_VERTICES)
		Flush();

	const uint32_t packCol = Core::RGBAColour::packARGB( colour.getRGBAColour() );

	pVertices[ NDC_LINE_START_INDEX + numLineVertices + 0 ].pos = Math::Vector3( a[0], a[1], 0.f );
	pVertices[ NDC_LINE_START_INDEX + numLineVertices + 0 ].colour =  packCol;
	pVertices[ NDC_LINE_START_INDEX + numLineVertices + 1 ].pos = Math::Vector3( b[0], b[1], 0.f );
	pVertices[ NDC_LINE_START_INDEX + numLineVertices + 1 ].colour = packCol;
	numLineVertices += 2;
}

void GfxDebugRender::WorldLine( const Core::Colour& colour, const Math::Vector3& a, const Math::Vector3& b )
{
	if( numLineWorldVertices+2 > MAX_DEBUG_VERTICES)
		Flush();

	const uint32_t packCol = Core::RGBAColour::packARGB( colour.getRGBAColour() );

	D3DXMATRIX mat = debugContext->matrixCache->getMatrix( RenderMatrixCache::VIEW );
	pVertices[ WORLD_LINE_START_INDEX + numLineWorldVertices + 0 ].pos = Math::TransformAndProject(a, mat);
	pVertices[ WORLD_LINE_START_INDEX + numLineWorldVertices + 0 ].colour = packCol;
	pVertices[ WORLD_LINE_START_INDEX + numLineWorldVertices + 1 ].pos = Math::TransformAndProject(b, mat);
	pVertices[ WORLD_LINE_START_INDEX + numLineWorldVertices + 1 ].colour = packCol;
	numLineWorldVertices+= 2;
}

//! print some text onto some form of screen console
void GfxDebugRender::Print( const char* pText )
{
	ScrConsole::Get()->Print( pText );	
}

void GfxDebugRender::WorldBox( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float lx, const float ly, const float lz, const Math::Matrix4x4& transform )
{
	using namespace Math;
	Vector3 p0( Vector3( - lx/2, - ly/2, - lz/2 ) );
	Vector3 p1( Vector3( + lx/2, - ly/2, - lz/2 ) );
	Vector3 p2( Vector3( - lx/2, + ly/2, - lz/2 ) );
	Vector3 p3( Vector3( + lx/2, + ly/2, - lz/2 ) );
	Vector3 p4( Vector3( - lx/2, - ly/2, + lz/2 ) );
	Vector3 p5( Vector3( + lx/2, - ly/2, + lz/2 ) );
	Vector3 p6( Vector3( - lx/2, + ly/2, + lz/2 ) );
	Vector3 p7( Vector3( + lx/2, + ly/2, + lz/2 ) );

	// orientate the box
	Matrix4x4 rotate = CreateRotationMatrix( orient );
	p0 = TransformNormal( p0, rotate ) + pos;
	p1 = TransformNormal( p1, rotate ) + pos;
	p2 = TransformNormal( p2, rotate ) + pos;
	p3 = TransformNormal( p3, rotate ) + pos;
	p4 = TransformNormal( p4, rotate ) + pos;
	p5 = TransformNormal( p5, rotate ) + pos;
	p6 = TransformNormal( p6, rotate ) + pos;
	p7 = TransformNormal( p7, rotate ) + pos;

	p0 = TransformAndProject( p0, transform );
	p1 = TransformAndProject( p1, transform );
	p2 = TransformAndProject( p2, transform );
	p3 = TransformAndProject( p3, transform );
	p4 = TransformAndProject( p4, transform );
	p5 = TransformAndProject( p5, transform );
	p6 = TransformAndProject( p6, transform );
	p7 = TransformAndProject( p7, transform );

	WorldLine( colour, p0, p4 );
	WorldLine( colour, p1, p5 );
	WorldLine( colour, p2, p6 );
	WorldLine( colour, p3, p7 );

	WorldLine( colour, p0, p1 );
	WorldLine( colour, p2, p3 );
	WorldLine( colour, p4, p5 );
	WorldLine( colour, p6, p7 );

	WorldLine( colour, p0, p2 );
	WorldLine( colour, p1, p3 );
	WorldLine( colour, p4, p6 );
	WorldLine( colour, p5, p7 );

}

void GfxDebugRender::WorldCylinder( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float radius, const float height, const Math::Matrix4x4& transform )
{
	using namespace Math;
	static const unsigned int NUM_DIVISIONS = 20;

	Vector3 radialZ(0, radius, 0);
	Vector3 offset(0, 0, height/2);

	Matrix4x4 rotateZ = CreateZRotationMatrix( degree_to_radian<float>() * (360.f / (float)NUM_DIVISIONS) );

	Matrix4x4 rotate = CreateRotationMatrix( orient );
	offset = TransformNormal( offset, rotate );
	offset = TransformNormal( offset, transform );

	Vector3 z0(radialZ);
	for( unsigned int i=0;i < NUM_DIVISIONS;++i)
	{
		Vector3 z1 = TransformNormal( z0, rotateZ );
		
		Vector3 z0a = TransformNormal( z0, rotate );
		Vector3 z1a = TransformNormal( z1, rotate );
		Vector3 pz0 = pos + z0a - offset;
		Vector3 pz1 = pos + z1a - offset;
		Vector3 pz0o = pos + z0a + offset;
		Vector3 pz1o = pos + z1a + offset;

		pz0 = TransformAndProject( pz0, transform );
		pz1 = TransformAndProject( pz1, transform );
		pz0o = TransformAndProject( pz0o, transform );
		pz1o = TransformAndProject( pz1o, transform );

		WorldLine( colour, pz0, pz1 );
		WorldLine( colour, pz0o, pz1o );
		WorldLine( colour, pz0, pz1o );
		z0 = z1;
	}
}

void GfxDebugRender::WorldSphere( const Core::Colour& colour, const Math::Vector3& pos, const float radius, const Math::Matrix4x4& transform )
{
	using namespace Math;
	static const unsigned int NUM_DIVISIONS = 20;
	Vector3 radialX(0, radius, 0);
	Vector3 radialZ(0, radius, 0);

	Matrix4x4 rotateX = CreateXRotationMatrix( degree_to_radian<float>() * (360.f / (float)NUM_DIVISIONS) );
	Matrix4x4 rotateZ = CreateZRotationMatrix( degree_to_radian<float>() * (360.f / (float)NUM_DIVISIONS) );

	Vector3 x0(radialX);
	Vector3 z0(radialZ);

	for( unsigned int i=0;i < NUM_DIVISIONS;++i)
	{
		Vector3 x1 = TransformNormal( x0, rotateX );
		Vector3 z1 = TransformNormal( z0, rotateZ );
		Vector3 px0 = pos + x0;
		Vector3 px1 = pos + x1;
		Vector3 pz0 = pos + z0;
		Vector3 pz1 = pos + z1;

		px0 = TransformAndProject( px0, transform );
		px1 = TransformAndProject( px1, transform );
		pz0 = TransformAndProject( pz0, transform );
		pz1 = TransformAndProject( pz1, transform );

		WorldLine( colour, px0, px1 );
		WorldLine( colour, pz0, pz1 );
		x0 = x1;
		z0 = z1;
	}

}

void GfxDebugRender::Flush() {
	Gfx::Get()->pushDebugMarker( "Debug Render Flush" );

	HRESULT hr;
	vertexBuffer.Unlock( debugContext );

	ShaderFXPtr effect = lineShader->Acquire();
	D3DXMATRIX mat = debugContext->matrixCache->getMatrix( RenderMatrixCache::PROJECTION );
	DXWARN( effect->m_pEffect->GetVariableByName( "g_mProjection" )->AsMatrix()->SetMatrix((float*)&mat ) );

	VDeclManager::Get()->SetVertexDecleration( debugContext, lineVDeclHandle );
	vertexBuffer.Bind( debugContext, 0, VDeclManager::Get()->GetVertexSize( lineVDeclHandle ) );

	D3DX11_TECHNIQUE_DESC techDesc;
	effect->m_pEffectTechnique->GetDesc( &techDesc );

	debugContext->deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
	for( uint32_t i = 0;i < techDesc.Passes;++i ) {

		int num;
		int offset;
		// first pass is world space line, 2nd is screen space
		if( i == 0 ) {
			num = numLineWorldVertices;
			offset = WORLD_LINE_START_INDEX;
		} else {
			num = numLineVertices;
			offset = NDC_LINE_START_INDEX;
		}
		if( num > 0 ) {
			effect->m_pEffectTechnique->GetPassByIndex( i )->Apply( 0,  Gfx::Get()->GetMainContext() );
			debugContext->deviceContext->Draw( num, offset );
		}
	}

	FlushGpuLineBufs();

	debugContext->deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	vertexBuffer.Unbind( debugContext, 0);

	pVertices = (Vertex*) vertexBuffer.Lock( debugContext, VBManager::VBInstance::DISCARD );
	numLineVertices = 0;
	numLineWorldVertices = 0;

	Gfx::Get()->popDebugMarker();

}

void GfxDebugRender::GpuLineDraw( const D3D11BufferSmartPtr& indirectParams, const D3D11BufferSmartPtr& lineBuffer ) {
	gpuLineBufs.push_back( GpuLineBufs(indirectParams, lineBuffer) );
}

void GfxDebugRender::FlushGpuLineBufs() {

	if( gpuLineBufs.empty() )
		return;

	HRESULT hr;
	ShaderFXPtr effect = lineShader->Acquire();

	// gpu lines are actually world space so need view as well
	D3DXMATRIX mat = debugContext->matrixCache->getMatrix( RenderMatrixCache::VIEW_PROJECTION );
	DXWARN( effect->m_pEffect->GetVariableByName( "g_mProjection" )->AsMatrix()->SetMatrix((float*)&mat ) );
	VDeclManager::Get()->SetVertexDecleration( debugContext, lineVDeclHandle );
	UINT strides[] = { 16 };
	UINT offsets[] = { 0 };

	Core::vector<GpuLineBufs>::iterator it = gpuLineBufs.begin();
	while( it != gpuLineBufs.end() ) {
		const D3D11BufferSmartPtr& indirectParams = (*it).indirectParams;
		const D3D11BufferSmartPtr& lineBuffer = (*it).lineBuffer;

		ID3D11Buffer* vbs[] = { lineBuffer };
		debugContext->deviceContext->IASetVertexBuffers( 0, 1, vbs, strides, offsets );

		effect->m_pEffectTechnique->GetPassByIndex( 0 )->Apply( 0,  Gfx::Get()->GetMainContext() );
		debugContext->deviceContext->DrawInstancedIndirect( indirectParams, 0 );

		++it;
	}

	gpuLineBufs.clear();
}

}