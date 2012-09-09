//!-----------------------------------------------------
//!
//! @file vtpipeline.cpp
//! @brief VT pipeline is the main pipeline for the VT games 
//!
//!-----------------------------------------------------

#include "scene.h"
#include <boost/lexical_cast.hpp>
#include "core/resourceman.h"
#include "wobfile.h"
#include "texture.h"
#include "databuffer.h"
#include "vertexinput.h"
#include "rendercontext.h"
#include "wobfile.h"
#include "imagecomposer.h"
#include "gpu_constants.h"
#include "vtpipeline.h"

#define FIXED_WIDTH 1280
#define FIXED_HEIGHT 960

namespace Scene {

VtPipeline::VtPipeline( ) : gpuMaterialStoreOk( false ), gpuLightStoreOk( false ) {
	static const int MAX_TRANSPARENT_FRAGMENTS = FIXED_WIDTH * FIXED_HEIGHT * Scene::GPUConstants::TOTAL_TRANS_OR_AA_FRAGS;

	// NOTE programs are loaded off disk even when from the internal default programs (which are actually bound in the code)
	opaqueProgramHandle.reset( Scene::ProgramHandle::load( "vtopaque" ) );
	resolveProgramHandle.reset( Scene::ProgramHandle::load( "vtresolve" ) );
	lightingProgramHandle.reset( Scene::ProgramHandle::load( "vtlighting" ) );
	transparentProgramHandle.reset( Scene::ProgramHandle::load( "vttransparent" ) );

	Texture::CreationInfo ccs = Texture::TextureCtor(
			RCF_TEX_2D | RCF_OUT_RENDER_TARGET | RCF_PRG_READ | RCF_OUT_UNORDERED_ACCESS,
			GTF_RGBA8,
			FIXED_WIDTH, FIXED_HEIGHT
		);
	static const std::string colTargetName = "VtPipe_ColourTarget";
	colourTargetHandle.reset( TextureHandle::create( colTargetName.c_str(), &ccs ) );

	// depth
	Texture::CreationInfo dmscs = Texture::TextureCtor(
			RCF_TEX_2D | RCF_OUT_RENDER_TARGET | RCF_PRG_READ,
			GTF_DEPTH24_STENCIL8,
			FIXED_WIDTH, FIXED_HEIGHT, 1, 1, 1, NUM_MSAA_SAMPLES
		);
	static const std::string depthTargetMSName = "VtPipe_DepthTargetMS";
	depthTargetMSHandle.reset( TextureHandle::create( depthTargetMSName.c_str(), &dmscs ) );
	
	// packed: material index + 2D normal
	Texture::CreationInfo gb0mscs = Texture::TextureCtor(
		RCF_TEX_2D | RCF_OUT_RENDER_TARGET | RCF_PRG_READ,
		GTF_RG32UI,
		FIXED_WIDTH, FIXED_HEIGHT, 1, 1, 1, NUM_MSAA_SAMPLES
	);
	static const std::string gbTargetMSName = "VtPipe_GBufferMS0";
	gBufferMSHandle0.reset( TextureHandle::create( gbTargetMSName.c_str(), &gb0mscs ) );

	// transparent fragment count
	Texture::CreationInfo tfscs = Texture::TextureCtor(
		RCF_TEX_2D | RCF_PRG_READ | RCF_OUT_UNORDERED_ACCESS,
		GTF_R32UI,
		FIXED_WIDTH, FIXED_HEIGHT
	);
	static const std::string tfcTargetName = "VtPipe_TransFragmentCount";
	tfcHandle.reset( TextureHandle::create( tfcTargetName.c_str(), &tfscs ) );
	// transparent fragments as opaque + depth and coverage count
	DataBuffer::CreationInfo tfdcs = Resource::BufferCtor(
		RCF_BUF_GENERAL | RCF_PRG_STRUCTURED | RCF_PRG_READ | RCF_OUT_UNORDERED_ACCESS,
		sizeof( GPUConstants::VtTransparentFragment ) * MAX_TRANSPARENT_FRAGMENTS
	);
	tfdcs.structureSize = sizeof( GPUConstants::VtTransparentFragment );
	static const std::string transparentFragmentDataName = "VtPipe_TransparentFragments";
	transparentFragmentsHandle.reset( DataBufferHandle::create( transparentFragmentDataName .c_str(), &tfdcs ) );

	depthStencilStateHandle.reset( DepthStencilStateHandle::create( "_DSS_Normal" ) );
	depthStencilNoWriteStateHandle.reset( DepthStencilStateHandle::create( "_DSS_Less_NoWrite" ) );
	rasterStateHandle.reset( RasteriserStateHandle::create( "_RS_Normal" ) );
	rasterStateNoMSHandle.reset( RasteriserStateHandle::create( "_RS_Normal_NoMS_NoCull" ) );
	renderTargetWriteHandle.reset( RenderTargetStatesHandle::create( "_RTS_NoBlend_WriteAll" ) );

	RenderTargetStates::CreationInfo aartci = {
		RenderTargetStates::CreationInfo::NONE, 
		1, 
		{ TargetState::FLAGS::NONE, TWE_NONE },
	};
	renderTargetNoWriteHandle.reset( RenderTargetStatesHandle::create( "VTPipe_NoWrite", &aartci ) );
	
	// null material
	GPUConstants::VtMaterial nullMaterial = {
		Math::Vector4(0,0,0,0),
		Math::Vector4(0,0,0,0),
		Math::Vector4(0,0,0,0),
	};
	materialStoreSystemMem.push_back( nullMaterial );

	// always at least one directional light
	GPUConstants::VtLight defaultLight = {
		Math::Vector4( 0.707f*10000.0f, 0.707f*10000.f, 0.0f*10000.f, 1.0f ),
		Math::Vector4( 1.0f, 1.0f, 1.0f, 1.0f )
	};

	lightStoreSystemMem.push_back( defaultLight );
}
	
VtPipeline::~VtPipeline() {

	materialStoreHandle.reset();
	lightStoreHandle.reset();

	renderTargetNoWriteHandle.reset();
	renderTargetWriteHandle.reset();
	depthStencilNoWriteStateHandle.reset();
	depthStencilStateHandle.reset();
	rasterStateHandle.reset();

	tfcHandle.reset();

	transparentFragmentsHandle.reset();
	gBufferMSHandle0.reset();
	depthTargetMSHandle.reset();
	colourTargetHandle.reset();

	transparentProgramHandle.reset();
	lightingProgramHandle.reset();
	resolveProgramHandle.reset();
	opaqueProgramHandle.reset();
}

void VtPipeline::bind( Scene::RenderContext* ctx ) {
	ctx->pushDebugMarker( getName() );

	if( !gpuMaterialStoreOk ) {
		// free existing resource
		materialStoreHandle.reset();

		// material data buffer
		DataBuffer::CreationInfo mdcs = Resource::BufferCtor(
				RCF_BUF_GENERAL | RCF_PRG_STRUCTURED | RCF_PRG_READ | RCF_ACE_IMMUTABLE,
				sizeof( GPUConstants::VtMaterial ) * materialStoreSystemMem.size(),
				&materialStoreSystemMem[0]
			);
		mdcs.structureSize = sizeof( GPUConstants::VtMaterial );
		static const std::string materialDataName = "VtPipe_MaterialData";
		materialStoreHandle.reset( DataBufferHandle::create( materialDataName.c_str(), &mdcs ) );
		gpuMaterialStoreOk = true;
	}
	if( !gpuLightStoreOk ) {
		// free existing resource
		lightStoreHandle.reset();

		// light data buffer, TODO Raw or make all lights same structure size?
		DataBuffer::CreationInfo ldcs = Resource::BufferCtor(
				RCF_BUF_GENERAL | RCF_PRG_STRUCTURED | RCF_PRG_READ | RCF_ACE_IMMUTABLE,
				sizeof( GPUConstants::VtLight ) * lightStoreSystemMem.size(),
				&lightStoreSystemMem[0]
			);
		ldcs.structureSize = sizeof( GPUConstants::VtLight );
		static const std::string lightDataName = "VtPipe_LightData";
		lightStoreHandle.reset( DataBufferHandle::create( lightDataName.c_str(), &ldcs ) );
		gpuLightStoreOk = true;
	}
}

void VtPipeline::unbind( Scene::RenderContext* ctx ) {
	ctx->unbindTexture( Scene::ST_FRAGMENT, 0, 12 );

	// light
	{
		auto prg = lightingProgramHandle.acquire();
		auto ogb = gBufferMSHandle0.acquire();
		auto tfc = tfcHandle.acquire();
		auto db = depthTargetMSHandle.acquire();

		auto tfrags = transparentFragmentsHandle.acquire();
		auto materialStore = materialStoreHandle.acquire();
		auto lightStore = lightStoreHandle.acquire();	

		ctx->unbindRenderTargets();
		ctx->bind( prg );
		ctx->getConstantCache().updateGPU( ctx, prg );

		Scene::TexturePtr targets[] = {
			colourTargetHandle.acquire(),
		};

		ctx->bindUnorderedViews( 1, targets );
		ctx->bind( Scene::ST_COMPUTE, 0, ogb );
		ctx->bind( Scene::ST_COMPUTE, 1, tfc );
		ctx->bind( Scene::ST_COMPUTE, 2, tfrags );
		ctx->bind( Scene::ST_COMPUTE, 8, db );

		ctx->bind( Scene::ST_COMPUTE, 10, materialStore );
		ctx->bind( Scene::ST_COMPUTE, 11, lightStore );
		ctx->dispatch( FIXED_WIDTH, FIXED_HEIGHT, 1 );
		ctx->unbindUnorderedViews();
		ctx->unbindTexture( Scene::ST_COMPUTE, 0, 12 );
	}

	ctx->popDebugMarker();
}

static enum VTPIPE_GEOMETRY_PASSES {
	GBUFFER_RENDER_OPAQUE_PASS = 0,

	GBUFFER_RENDER_TRANS_PASS,

	MAX_RENDER_PASSES,
};

int VtPipeline::getGeomPassCount() { 
	return MAX_RENDER_PASSES; 
}
bool VtPipeline::isGeomPassOpaque( int pass ) {
	if( pass < GBUFFER_RENDER_TRANS_PASS ) {
		return true;
	} else {
		return false;
	}
}

void VtPipeline::startGeomPass ( RenderContext* ctx, int i ) {
	switch ( i ) {
	case GBUFFER_RENDER_OPAQUE_PASS:
		startGeomRenderOpaquePass( ctx ); break;
	case GBUFFER_RENDER_TRANS_PASS: 
		startGeomRenderTransparentPass( ctx ); break;
	default: CORE_ASSERT(false) break;
	}
}

void VtPipeline::endGeomPass ( RenderContext* ctx, int i ) {
	switch ( i ) {
	case GBUFFER_RENDER_OPAQUE_PASS:
		endGeomRenderOpaquePass( ctx ); break;
	case GBUFFER_RENDER_TRANS_PASS: 
		endGeomRenderTransparentPass( ctx ); break;
	default: CORE_ASSERT(false) break;
	}
}
void VtPipeline::startGeomRenderOpaquePass ( RenderContext* ctx ) {
	Scene::TexturePtr tgt = gBufferMSHandle0.acquire();

	auto depthTarget = depthTargetMSHandle.acquire();
	auto program = opaqueProgramHandle.acquire();
	auto rasterState = rasterStateHandle.acquire();
	auto dss = depthStencilStateHandle.acquire();
	auto rtw = renderTargetWriteHandle.acquire();

	ctx->clear( tgt, Core::RGBAColour(0,0,0,0) );
	ctx->clear( depthTarget, true, 1.0f, true, 0 );

	ctx->bindRenderTargets( tgt, depthTarget );

	ctx->getConstantCache().updateGPU( ctx, program );
	ctx->bind( program );
	ctx->bind( rasterState );
	ctx->bind( dss );
	ctx->bind( rtw );
}

void VtPipeline::startGeomRenderTransparentPass ( RenderContext* ctx ) {
	Scene::ViewPtr uavs[] = {
		tfcHandle.acquire()->getView( Resource::UNORDERED_ACCESS_VIEW ), 
		transparentFragmentsHandle.acquire()->getView( Resource::UNORDERED_ACCESS_VIEW )
	};

	ctx->clear( uavs[0] );

	Scene::TexturePtr dum[] = { gBufferMSHandle0.acquire() };
	auto depthTarget = depthTargetMSHandle.acquire();
	auto program = transparentProgramHandle.acquire();
	auto rasterState = rasterStateNoMSHandle.acquire();
	auto dss = depthStencilNoWriteStateHandle.acquire();
	auto rtw = renderTargetNoWriteHandle.acquire();

	ctx->bindRenderTargetsAndUnorderedViews( 1, dum, depthTarget,
											 2, uavs );

	ctx->getConstantCache().updateGPU( ctx, program );
	ctx->bind( program );
	ctx->bind( rasterState );
	ctx->bind( dss );
	ctx->bind( rtw );

}
void VtPipeline::endGeomRenderOpaquePass( RenderContext* ctx ) {
}
void VtPipeline::endGeomRenderTransparentPass( RenderContext* ctx ) {
}


void VtPipeline::conditionWob( Scene::Wob* wob ) {
	using namespace Scene;
	WobFileHeader* header = wob->header.get();

	auto program = opaqueProgramHandle.acquire();

	VtPipelineDataStore* pds =  CORE_NEW VtPipelineDataStore();

	// count opaque and transparency/translucent surfaces
	for( uint16_t i = 0; i < header->uiNumMaterials; ++i ) {
		WobMaterial* mat = &header->pMaterials.p[i];
	}

	pds->numMaterials = header->uiNumMaterials;
	pds->materials.reset( CORE_NEW_ARRAY VtPipelineDataStore::PerMaterial[ pds->numMaterials ] );
	wob->pipelineDataStores[pipelineIndex] = std::unique_ptr<VtPipelineDataStore>( pds );

	const std::string name( header->pName.p );

	for( uint16_t i = 0; i < header->uiNumMaterials; ++i ) {
		WobMaterial* mat = &header->pMaterials.p[i];

		VtPipelineDataStore::PerMaterial* mds = &pds->materials[i];

		mds->name = name + std::string( mat->pName.p );
		const int indexSize = (mat->uiFlags & WobMaterial::WM_32BIT_INDICES) ? sizeof(uint32_t) : sizeof(uint16_t);
		mds->numIndices = mat->uiNumIndices;
		mds->indexSize = indexSize;

		// index buffer
		DataBuffer::CreationInfo indbcs = Resource::BufferCtor(
			RCF_ACE_IMMUTABLE | RCF_BUF_INDEX, mat->uiNumIndices * indexSize, mat->pIndexData.p
		);
		mds->indexBuffer = DataBufferHandle::create( (mds->name + "_ib").c_str(), &indbcs, Core::RESOURCE_FLAGS::RMRF_NONE );

		mds->vacs.elementCount = mat->numVertexElements;
		for( int i = 0; i < mds->vacs.elementCount; ++i ) { 
			mds->vacs.elements[i].type = (VIN_TYPE) mat->pElements.p[i].uiType; // identical at the moment
			mds->vacs.elements[i].usage = (VIN_ELEMENT) mat->pElements.p[i].uiUsage; // identical at the moment
		}
		const size_t vertexSize = VertexInput::getVertexSize( mds->vacs.elementCount, mds->vacs.elements );
		DataBuffer::CreationInfo vdbcs = Resource::BufferCtor(
			RCF_ACE_IMMUTABLE | RCF_BUF_VERTEX, (uint32_t)(mat->uiNumVertices *  vertexSize), mat->pVertexData.p
		);
		DataBufferHandlePtr vertexBuffer = DataBufferHandle::create( (mds->name + "_vb").c_str(), &vdbcs, Core::RESOURCE_FLAGS::RMRF_NONE );

		for( int i = 0; i < mds->vacs.elementCount; ++i ) { 
			mds->vacs.data[i].buffer = vertexBuffer;
			mds->vacs.data[i].offset = VI_AUTO_OFFSET;
			mds->vacs.data[i].stream = 0;
			mds->vacs.data[i].stride = VI_AUTO_STRIDE;
		}
		gpuMaterialStoreOk = false;
		Math::Vector3 diffuse;
		Math::Vector3 emissive;
		Math::Vector3 specular;
		float specExp = 40.f;
		float translucency = 1.f;
		float transparency = 1.f;
		float reflection = 0.f;

		GPUConstants::VtMaterial gpuMat;
		for( uint8_t i = 0; i < mat->uiNumParameters; ++i ) {
			if( std::string( mat->pParameters.p[i].pName.p ) == "Diffuse" ) {
				CORE_ASSERT( mat->pParameters.p[i].uiType == WobMaterialParameter::WMPT_VEC3_FLOAT ); 
				diffuse = Math::Vector3( (const float*)mat->pParameters.p[i].pData.p );
			} else if( std::string( mat->pParameters.p[i].pName.p ) == "Specular" ) {
				CORE_ASSERT( mat->pParameters.p[i].uiType == WobMaterialParameter::WMPT_VEC3_FLOAT ); 
				specular = Math::Vector3( (const float*)mat->pParameters.p[i].pData.p );
			} else if( std::string( mat->pParameters.p[i].pName.p ) == "Emissive" ) {
				CORE_ASSERT( mat->pParameters.p[i].uiType == WobMaterialParameter::WMPT_VEC3_FLOAT ); 
				emissive = Math::Vector3( (const float*)mat->pParameters.p[i].pData.p );
			} else if( std::string( mat->pParameters.p[i].pName.p ) == "Shininess" ) {
				specExp = *(const float*)mat->pParameters.p[i].pData.p;
				specExp = Math::Max( specExp, 1e-2f ); // get NANs in the render with pow( x, 0.0f)...
			} else if( std::string( mat->pParameters.p[i].pName.p ) == "Transparency" ) {
				transparency = *(const float*)mat->pParameters.p[i].pData.p;
			} else if( std::string( mat->pParameters.p[i].pName.p ) == "Translucency" ) {
				translucency = *(const float*)mat->pParameters.p[i].pData.p;
			} else if( std::string( mat->pParameters.p[i].pName.p ) == "Reflection" ) {
				reflection = *(const float*)mat->pParameters.p[i].pData.p;
			}
		}
		// pack specular exponent and specular colour
		gpuMat.diffuse_transp = Math::Vector4( diffuse[0], diffuse[1], diffuse[2], transparency );
		gpuMat.emissive_transl = Math::Vector4( emissive[0], emissive[1], emissive[2], translucency );
		gpuMat.specular = Math::Vector4( specular[0], specular[1], specular[2], specExp );
		gpuMat.reflection = Math::Vector4( reflection, 0.0f, 0.0f, 0.0f );
		materialStoreSystemMem.push_back( gpuMat );

		mds->isTransparent = (transparency < (1.f - 1e-2f) );
		mds->materialIndex = (uint32_t) (materialStoreSystemMem.size() - 1);

		mds->vaoHandle = VertexInputHandle::create( 
				(mds->name + VertexInput::genEleString(mds->vacs.elementCount, mds->vacs.elements)).c_str(), &mds->vacs );
		mds->vaoHandle->acquire()->validate( program );
	}
}

void VtPipelineDataStore::render( Scene::RenderContext* rc ) {

	RenderContext* ctx = (RenderContext*) rc;
	ctx->getConstantCache().updateGPUBlock( rc, Scene::CF_STD_OBJECT );

	for( int i = 0;i < numMaterials; ++i ) {
		const PerMaterial* mds = &materials[i];

		if( !mds->isTransparent ) {
			const uint32_t materialIndex[4] = { mds->materialIndex, mds->materialIndex, mds->materialIndex, mds->materialIndex };
			ctx->getConstantCache().setVector( CVN_MATERIAL_INDEX, materialIndex );
			ctx->getConstantCache().updateGPUBlock( rc, Scene::CF_PER_MATERIAL );

			auto vao = mds->vaoHandle->tryAcquire();
			if( !vao ) { /* LOG(INFO) << "vao not ready\n"; */ return; }
			auto ib = mds->indexBuffer->tryAcquire();
			if( !ib ) { /* LOG(INFO) << "ib not ready\n"; */ return; }
			ctx->bind( vao );
			ctx->bindIndexBuffer( ib, mds->indexSize );
			ctx->drawIndexed( PT_TRIANGLE_LIST, mds->numIndices );
		}
	}
}
void VtPipelineDataStore::renderTransparent( Scene::RenderContext* rc ) {
	RenderContext* ctx = (RenderContext*) rc;
	ctx->getConstantCache().updateGPUBlock( rc, Scene::CF_STD_OBJECT );

	for( int i = 0;i < numMaterials; ++i ) {
		const PerMaterial* mds = &materials[i];

		if( mds->isTransparent ) {
			const uint32_t materialIndex[4] = { mds->materialIndex, mds->materialIndex, mds->materialIndex, mds->materialIndex };
			ctx->getConstantCache().setVector( CVN_MATERIAL_INDEX, materialIndex );
			ctx->getConstantCache().updateGPUBlock( rc, Scene::CF_PER_MATERIAL );

			auto vao = mds->vaoHandle->tryAcquire();
			if( !vao ) { /* LOG(INFO) << "vao not ready\n"; */ return; }
			auto ib = mds->indexBuffer->tryAcquire();
			if( !ib ) { /* LOG(INFO) << "ib not ready\n"; */ return; }
			ctx->bind( vao );
			ctx->bindIndexBuffer( ib, mds->indexSize );
			ctx->drawIndexed( PT_TRIANGLE_LIST, mds->numIndices );
		}
	}

}

VtPipelineDataStore::~VtPipelineDataStore() {
	for( int i = 0;i < numMaterials; ++i ) {
		VtPipelineDataStore::PerMaterial* mds = &materials[i];

		if( mds->indexBuffer )
			mds->indexBuffer->close();

		if( mds->vacs.data[0].buffer )
			mds->vacs.data[0].buffer->close();

		if(mds->vaoHandle)
			mds->vaoHandle->close();
	}
}

}

