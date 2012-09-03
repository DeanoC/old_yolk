//!-----------------------------------------------------
//!
//! @file vtpipeline.cpp
//! @brief VT pipeline is the main pipeline for the VT games 
//!
//!-----------------------------------------------------

#include "scene.h"
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

VtPipeline::VtPipeline( ) : gpuMaterialStoreOk( false ) {
	// NOTE programs are loaded off disk even when from the internal default programs (which are actually bound in the code)
	solidWireFrameProgramHandle.reset( Scene::ProgramHandle::load( "vtsolidwireframe" ) );
	resolveProgramHandle.reset( Scene::ProgramHandle::load( "vtresolve" ) );

	Texture::CreationInfo ccs = Texture::TextureCtor(
			RCF_TEX_2D | RCF_OUT_RENDER_TARGET |RCF_PRG_READ,
			GTF_RGBA8,
			FIXED_WIDTH, FIXED_HEIGHT
		);
	static const std::string colTargetName = "VtPipe_ColourTarget";
	colourTargetHandle.reset( TextureHandle::create( colTargetName.c_str(), &ccs ) );

	Texture::CreationInfo cmscs = Texture::TextureCtor(
		RCF_TEX_2D | RCF_OUT_RENDER_TARGET |RCF_PRG_READ,
		GTF_R11F_G11F_B10F,
		FIXED_WIDTH, FIXED_HEIGHT, 1, 1, 1, 8
	);
	static const std::string colTargetMSName = "VtPipe_ColourTargetMS";
	colourTargetMSHandle.reset( TextureHandle::create( colTargetMSName.c_str(), &cmscs ) );
	// depth
	Texture::CreationInfo dmscs = Texture::TextureCtor(
			RCF_TEX_2D | RCF_OUT_RENDER_TARGET | RCF_PRG_READ,
			GTF_DEPTH24_STENCIL8,
			FIXED_WIDTH, FIXED_HEIGHT, 1, 1, 1, 8
		);
	static const std::string depthTargetMSName = "VtPipe_DepthTargetMS";
	depthTargetMSHandle.reset( TextureHandle::create( depthTargetMSName.c_str(), &dmscs ) );
	// material index, edge alpha, coverage, 
	Texture::CreationInfo gb0mscs = Texture::TextureCtor(
		RCF_TEX_2D | RCF_OUT_RENDER_TARGET |RCF_PRG_READ,
		GTF_RGBA16UI,
		FIXED_WIDTH, FIXED_HEIGHT, 1, 1, 1, 8
	);
	static const std::string gb0TargetMSName = "VtPipe_GBuffer0MS";
	gBuffer0MSHandle.reset( TextureHandle::create( colTargetMSName.c_str(), &gb0mscs ) );

	depthStencilStateHandle.reset( DepthStencilStateHandle::create( "_DSS_Normal" ) );
	rasterStateHandle.reset( RasteriserStateHandle::create( "_RS_Normal" ) );
	renderTargetWriteHandle.reset( RenderTargetStatesHandle::create( "_RTS_NoBlend_WriteAll" ) );

}
	
VtPipeline::~VtPipeline() {
	renderTargetWriteHandle.reset();
	rasterStateHandle.reset();
	depthStencilStateHandle.reset();

	materialStoreCPUHandle.reset();
	materialStoreHandle.reset();

	depthTargetMSHandle.reset();
	colourTargetMSHandle.reset();
	colourTargetHandle.reset();

	solidWireFrameProgramHandle.reset();
	resolveProgramHandle.reset();
}

void VtPipeline::bind( Scene::RenderContext* ctx ) {
	ctx->pushDebugMarker( getName() );

	if( !gpuMaterialStoreOk ) {
		// free existing resource
		materialStoreHandle.reset();
		materialStoreCPUHandle.reset();

		// material data buffer
		DataBuffer::CreationInfo mdcs = Resource::BufferCtor(
				RCF_BUF_GENERAL | RCF_PRG_STRUCTURED | RCF_PRG_READ,
				sizeof( GPUConstants::VtMaterial ) * materialStoreSystemMem.size()
			);
		mdcs.structureSize = sizeof( GPUConstants::VtMaterial );
		static const std::string materialDataName = "VtPipe_MaterialData";
		materialStoreHandle.reset( DataBufferHandle::create( materialDataName.c_str(), &mdcs ) );
		mdcs.flags = (Scene::RESOURCE_CREATION_FLAGS)( RCF_BUF_GENERAL | RCF_PRG_STRUCTURED | RCF_ACE_CPU_STAGING | RCF_ACE_CPU_WRITE );
		static const std::string materialDataCPUName = "VtPipe_MaterialDataCPU";
		materialStoreCPUHandle.reset( DataBufferHandle::create( materialDataCPUName.c_str(), &mdcs ) );

		// copy to GPU async
		auto materialStore =  materialStoreHandle.acquire();
		auto materialStoreCPU =  materialStoreCPUHandle.acquire();
		void* dst = materialStoreCPU->map( ctx, DBMA_WRITE_ONLY, DBMF_NONE, 0, materialStoreSystemMem.size() * sizeof(GPUConstants::VtMaterial) );
		memcpy( dst, &materialStoreSystemMem[0], materialStoreSystemMem.size() * sizeof(GPUConstants::VtMaterial) );
		materialStoreCPU->unmap( ctx );
		ctx->copy( materialStore, materialStoreCPU );
		gpuMaterialStoreOk = true;
	}

	auto colourTarget = colourTargetMSHandle.acquire();
	auto depthTarget = depthTargetMSHandle.acquire();
	auto program = solidWireFrameProgramHandle.acquire();
	auto rasterState = rasterStateHandle.acquire();
	auto depthStencilState = depthStencilStateHandle.acquire();
	auto rtw = renderTargetWriteHandle.acquire();
	auto materialStore =  materialStoreHandle.acquire();

	ctx->clear( colourTarget, Core::RGBAColour(0,0,0,0) );
	ctx->clearDepthStencil( depthTarget, true, 1.0f, true, 0 );

	ctx->bindRenderTargets( colourTarget, depthTarget );

	ctx->getConstantCache().updateGPU( ctx, program );
	ctx->bind( program );
	ctx->bind( rasterState );
	ctx->bind( depthStencilState );
	ctx->bind( rtw );
	ctx->bind( Scene::ST_FRAGMENT, 5, materialStore );
}

void VtPipeline::unbind( Scene::RenderContext* ctx ) {
	// resolve
	auto target = colourTargetHandle.acquire();
	auto tex = colourTargetMSHandle.acquire();
	auto prg = resolveProgramHandle.acquire();
	auto rtw = renderTargetWriteHandle.acquire();

	ctx->bindRenderTarget( target );
	ctx->bind( Scene::ST_FRAGMENT, 0, tex );
	ctx->bind( prg );
	ctx->bind( rtw );
	ctx->draw( Scene::PT_POINT_LIST, 1 );

	ctx->popDebugMarker();
}

void VtPipeline::conditionWob( Scene::Wob* wob ) {
	using namespace Scene;
	WobFileHeader* header = wob->header.get();

	auto program = solidWireFrameProgramHandle.acquire();

	VtPipelineDataStore* pds =  CORE_NEW VtPipelineDataStore();
	pds->numMaterials = header->uiNumMaterials;
	pds->materials.reset( CORE_NEW_ARRAY VtPipelineDataStore::PerMaterial[ pds->numMaterials ] );
	wob->pipelineDataStores[pipelineIndex] = std::unique_ptr<VtPipelineDataStore>( pds );

	const std::string name( header->pName.p );

	for( uint16_t i = 0; i < header->uiNumMaterials; ++i ) {
		WobMaterial* mat = &header->pMaterials.p[i];

		VtPipelineDataStore::PerMaterial* mds = &pds->materials[i];
		mat->backEndData.p = (void*)mds;

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
		Math::Vector3 specular;
		float specExp = 40.f;
		GPUConstants::VtMaterial gpuMat;
		for( uint8_t i = 0; i < mat->uiNumParameters; ++i ) {
			if( std::string( mat->pParameters.p[i].pName.p ) == "DiffuseColour" ) {
				CORE_ASSERT( mat->pParameters.p[i].uiType == WobMaterialParameter::WMPT_VEC3_FLOAT ); 
				gpuMat.diffuse = Math::Vector4( (const float*)mat->pParameters.p[i].pData.p );
			} else if( std::string( mat->pParameters.p[i].pName.p ) == "SpecularColour" ) {
				CORE_ASSERT( mat->pParameters.p[i].uiType == WobMaterialParameter::WMPT_VEC3_FLOAT ); 
				auto spec = (const float*)mat->pParameters.p[i].pData.p;
				specular = Math::Vector3( spec[0], spec[1], spec[2] );
			} else if( std::string( mat->pParameters.p[i].pName.p ) == "EmissiveColour" ) {
				CORE_ASSERT( mat->pParameters.p[i].uiType == WobMaterialParameter::WMPT_VEC3_FLOAT ); 
				gpuMat.emissive = Math::Vector4( (const float*)mat->pParameters.p[i].pData.p );
			} else if( std::string( mat->pParameters.p[i].pName.p ) == "Shininess" ) {
				specExp = *(const float*)mat->pParameters.p[i].pData.p;
			}
		}
		// pack specular exponent and specular colour
		gpuMat.specular = Math::Vector4( specular[0], specular[1], specular[2], specExp );
		materialStoreSystemMem.push_back( gpuMat );
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

		const uint32_t materialIndex[4] = { mds->materialIndex, mds->materialIndex, mds->materialIndex, mds->materialIndex };
		ctx->getConstantCache().setUIVector( CVN_MATERIAL_INDEX, materialIndex );
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
