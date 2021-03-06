//!-----------------------------------------------------
//!
//! \file debugprims.cpp
//! debug primitive renderer
//!
//!-----------------------------------------------------

#include "scene.h"
#include "core/resourceman.h"
#include "databuffer.h"
#include "vertexinput.h"
#include "constantcache.h"
#include "rendercontext.h"

//#include "scrconsole.h"

#include "debugprims.h"


namespace Scene {
DebugPrims::DebugPrims() {

	pPrevDRI = Core::g_pDebugRender;
	Core::g_pDebugRender = this;
	numLineVertices = 0;
	numLineWorldVertices = 0;

	DataBuffer::CreationInfo vbcs ( Resource::BufferCtor(
		RCF_BUF_VERTEX | RCF_ACE_CPU_WRITE, 
		MAX_DEBUG_VERTICES * sizeof(Vertex) * NUM_TYPES_OF_DEBUG_PRIM
	) );
	vertexBufferHandle = DataBufferHandle::create( "_debugPrimVertexBuffer", &vbcs );

	VertexInput::CreationInfo vocs = {
		2, {
			{ VE_POSITION, VT_FLOAT3 },
			{ VE_COLOUR0, VT_BYTEARGB },
		}, {
			vertexBufferHandle, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM,
			vertexBufferHandle, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM,
		}
	};

	const std::string vaoName = "_debugPrimVao_" + VertexInput::genEleString(vocs.elementCount, vocs.elements );
	vaoHandle = VertexInputHandle::create( vaoName.c_str(), &vocs );
	debugProgramHandle = Scene::ProgramHandle::load( "debugline" );

}

DebugPrims::~DebugPrims() {
	debugProgramHandle->close();
	vaoHandle->close();
	vertexBufferHandle->close();
	Core::g_pDebugRender = pPrevDRI;
}

void DebugPrims::ndcLine( const Core::Colour& colour, const Math::Vector2& a, const Math::Vector2& b ) {
	int nlv = numLineVertices.fetch_add( 2 );
	if( nlv+2 > MAX_DEBUG_VERTICES) {
		// TODO as drawing can occur on any thread, we can't flush like we used to (as uses render context)
		// so for now drop it, TODO queue buffers
//		flush();
		numLineVertices.fetch_add( -2 );
		return;
	}

	const uint32_t packCol = Core::RGBAColour::packARGB( colour.getRGBAColour() );

	vertices[ NDC_LINE_START_INDEX + nlv + 0 ].pos = Math::Vector3( a[0], a[1], 0.f );
	vertices[ NDC_LINE_START_INDEX + nlv + 0 ].colour =  packCol;
	vertices[ NDC_LINE_START_INDEX + nlv + 1 ].pos = Math::Vector3( b[0], b[1], 0.f );
	vertices[ NDC_LINE_START_INDEX + nlv + 1 ].colour = packCol;
}

void DebugPrims::worldLine( const Core::Colour& colour, 
							const Math::Vector3& a, 
							const Math::Vector3& b ) {
	using namespace Math;

	int nwv = numLineWorldVertices.fetch_add( 2 );
	if( nwv+2 > MAX_DEBUG_VERTICES) {
		// TODO as drawing can occur on any thread, we can't flush like we used to (as uses GL context)
		// so for now drop it, TODO queue up flushes
//		flush();
		numLineWorldVertices.fetch_add( -2 );
		return;
	}

	const uint32_t packCol = Core::RGBAColour::packARGB( colour.getRGBAColour() );

	vertices[ WORLD_LINE_START_INDEX + nwv + 0 ].pos = a;
	vertices[ WORLD_LINE_START_INDEX + nwv + 0 ].colour = packCol;
	vertices[ WORLD_LINE_START_INDEX + nwv + 1 ].pos = b;
	vertices[ WORLD_LINE_START_INDEX + nwv + 1 ].colour = packCol;
}

//! print some text onto some form of screen console
void DebugPrims::print( const char* pText ) {
//	ScrConsole::Get()->Print( pText );	
}

void DebugPrims::worldBox( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float lx, const float ly, const float lz, const Math::Matrix4x4& transform ) {
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

	worldLine( colour, p0, p4 );
	worldLine( colour, p1, p5 );
	worldLine( colour, p2, p6 );
	worldLine( colour, p3, p7 );

	worldLine( colour, p0, p1 );
	worldLine( colour, p2, p3 );
	worldLine( colour, p4, p5 );
	worldLine( colour, p6, p7 );

	worldLine( colour, p0, p2 );
	worldLine( colour, p1, p3 );
	worldLine( colour, p4, p6 );
	worldLine( colour, p5, p7 );
}

void DebugPrims::worldCylinder( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float radius, const float height, const Math::Matrix4x4& transform ) {
	using namespace Math;
	static const unsigned int NUM_DIVISIONS = 20;

	Vector3 radialZ(0, radius, 0);
	Vector3 offset(0, 0, height/2);

	Matrix4x4 rotateZ = CreateZRotationMatrix( degree_to_radian<float>() * (360.f / (float)NUM_DIVISIONS) );

	Matrix4x4 rotate = CreateRotationMatrix( orient );
	offset = TransformNormal( offset, rotate );
	offset = TransformNormal( offset, transform );

	Vector3 z0(radialZ);
	for( unsigned int i=0;i < NUM_DIVISIONS;++i) {
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

		worldLine( colour, pz0, pz1 );
		worldLine( colour, pz0o, pz1o );
		worldLine( colour, pz0, pz1o );
		z0 = z1;
	}
}

void DebugPrims::worldSphere( const Core::Colour& colour, const Math::Vector3& pos, const float radius, const Math::Matrix4x4& transform ) {
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

		worldLine( colour, px0, px1 );
		worldLine( colour, pz0, pz1 );
		x0 = x1;
		z0 = z1;
	}

}

void DebugPrims::render( RenderContext* context ) {
	if( numLineWorldVertices + numLineVertices == 0 )
		return;
	context->pushDebugMarker( "Debug Render Flush" );

	auto vao = vaoHandle->tryAcquire();
	if( !vao ) {
		numLineVertices = 0;
		numLineWorldVertices = 0;
		return;
	}
	// TODO only copy required
	DataBufferPtr vertexBuffer = vertexBufferHandle->acquire();
	Vertex* gpuVerts = (Vertex*) vertexBuffer->map( context, (RESOURCE_MAP_ACCESS)( RMA_WRITE | RMA_DISCARD ) );
	memcpy( gpuVerts, vertices, (MAX_DEBUG_VERTICES * 2) * sizeof(Vertex) );
	vertexBuffer->unmap( context );

/*	glEnable( GL_BLEND );
	glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD );
	glBlendFuncSeparate( GL_ONE, GL_ONE, GL_ONE, GL_ONE );*/


	auto debugProgram = debugProgramHandle->acquire();
	vao->validate( debugProgram );
	context->bind( vao );

	context->getConstantCache().updateGPU( context, debugProgram );
	context->bind( debugProgram );

	if( numLineWorldVertices != 0 ) {
		context->draw( PT_LINE_LIST,numLineWorldVertices, WORLD_LINE_START_INDEX );
	}
	if( numLineVertices != 0 ) {
		context->getConstantCache().setMatrixBypassCache( CVN_VIEW_PROJ, Math::IdentityMatrix() );
		context->getConstantCache().updateGPU( context, debugProgram );
		context->draw( PT_LINE_LIST,numLineVertices, NDC_LINE_START_INDEX );
	}

	numLineVertices = 0;
	numLineWorldVertices = 0;
	context->getConstantCache().invalidCacheOfType( CVN_VIEW_PROJ );
	context->popDebugMarker();

}


}