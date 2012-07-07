///-------------------------------------------------------------------------------------------------
/// \file	meshops\raycaster.cpp
///
/// \brief	Implements the raycaster class.
///
/// \details	
///		raycaster description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
/// \todo	Fill in detailed file description.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "meshops.h"
#include "raycaster.h"
#include "core/rasteriser.h"
#include "core/ray.h"
#include "core/kdtree.h"

namespace MeshOps {

Raycaster::Raycaster() :
	subSampleCount( 1 ),
	maxDisplacement( Math::infinity<float>() ) {

}

void Raycaster::addTargetVertexSource(	const std::string& elementName, 
										const std::string& subElementName, 
										const TRANSFORM_TYPE transformType ) {
	targetVertexSources[ elementName + subElementName ] = transformType;

}
void Raycaster::setTargetMesh( MeshMod::MeshPtr _targetMesh, bool doAllVertexSources ) {
	using namespace MeshMod;
	if( doAllVertexSources ) {
		// iterate through mesh adding all vertex sources
		//todo
	}
	targetMesh = _targetMesh;

	// create a KD-tree from the target mesh
	// need to flatten the position and indices into a simple array for the kdtree
	const FaceElementsContainer& faceCon = targetMesh->getFaceContainer();
	const VertexElementsContainer& vertCon = targetMesh->getVertexContainer();

	// get base position and normal vertex pointers
	const PositionVertexElements* posEle = vertCon.getElements<PositionVertexElements>();
	const FaceFaceElements* pfEle = faceCon.getElements<FaceFaceElements>();

	targetPositionData.reset( CORE_NEW_ARRAY float[ posEle->size() * 3 ]);
	for( unsigned int i = 0; i < posEle->size(); ++ i ) {
		targetPositionData[ (i*3)+0 ] = (*posEle)[i].x;
		targetPositionData[ (i*3)+1 ] = (*posEle)[i].y;
		targetPositionData[ (i*3)+2 ] = (*posEle)[i].z;
	}
	unsigned int indexCount = 0;
	targetIndexData.reset( CORE_NEW_ARRAY unsigned int[ pfEle->size() * 3] );
	for( unsigned int i = 0; i < pfEle->size(); ++ i ) {
		VertexIndexContainer faceVert;
		baseMesh->getFaceVertices( i, faceVert );
		// TODO triangles only at the moment
		if( faceVert.size() == 3 ) {
			targetIndexData[ (i*3)+0 ] = faceVert[0];
			targetIndexData[ (i*3)+1 ] = faceVert[1];
			targetIndexData[ (i*3)+2 ] = faceVert[2];
			indexCount += 3;
		}
	}
 
	targetTree.reset( new Core::KDTree( targetPositionData.get(), targetIndexData.get(), indexCount ) );
}

void Raycaster::setBaseMesh( MeshMod::MeshPtr _baseMesh, const std::string& uvSetName ) {
	baseMesh = _baseMesh;
	traceUVSetName = uvSetName;
}

void Raycaster::transferTo( FloatTexture& image ) {
	using namespace Core;
	using namespace MeshMod;
	using namespace Math;

	// check the meshes have been set
	// todo change froma assert to user error
	CORE_ASSERT( baseMesh );
	CORE_ASSERT( targetMesh );

#if 0
	// cache the source data pointers
	if( m_tangentSpaceRequired ) {
		m_tangents.LoadMesh( m_sourceMesh );
		m_binormals.LoadMesh( m_sourceMesh );
	}
#endif

	// locally cache the image dimensions
	const unsigned int  imageWidth = image.getWidth();
	const unsigned int imageHeight = image.getHeight();
	const unsigned int imageSize = imageWidth*imageHeight;

	// create an image channel for displacement
	float* displacement = 0;
	{
		FloatLayer& dl = image.addLayer( "displacement", 1 );
		displacement = dl.getData();
		memset( displacement, 0, sizeof( float ) * imageSize );
	}

	// create temporary storage for weights
	boost::scoped_array< unsigned int > sampleWeights( new unsigned int[imageSize] );
	memset( sampleWeights.get(), 0, sizeof( unsigned int )*imageSize );

	// TODO multi-thread work load, oriignal used OpenMP should use TBB now
	Rasteriser rasteriser( imageWidth, imageHeight, subSampleCount );

	const FaceElementsContainer& faceCon = baseMesh->getFaceContainer();
	const VertexElementsContainer& vertCon = baseMesh->getVertexContainer();
	// get base position and normal vertex pointers
	const PositionVertexElements* posEle = vertCon.getElements<PositionVertexElements>();
	const NormalVertexElements* normEle = vertCon.getElements<NormalVertexElements>();

	// get tracing uv vertex pointer
	const UVVertexElements* uvEle = vertCon.getElements<UVVertexElements>( traceUVSetName );

	// get face data 
	const FaceFaceElements* pfEle = faceCon.getElements<FaceFaceElements>();

	FaceFaceElements::const_iterator faceIt = pfEle->elements.begin();
	while( faceIt != pfEle->elements.end() ) {
		const FaceIndex faceNum = pfEle->distance<FaceIndex>( faceIt );

		VertexIndexContainer faceVert;
		baseMesh->getFaceVertices( faceNum, faceVert );

		// only works for triangles (TODO handle the others)
		if( faceVert.size() == 3) {	
			// create a triangle with texture coordinate (rasterising in [0,1]x[0,1])
			RasteriserTriangle tri;
			for( unsigned int i = 0; i < 3; ++i ) {
				tri.x[i] = (*uvEle)[ faceVert[i] ].u;
				tri.y[i] = (*uvEle)[ faceVert[i] ].v;
			}
			rasteriser.Reset( &tri );

			// rasterise each pixel within the triangle
			RasteriserPixel pixel;
			while( rasteriser.GetNext( &pixel ) ) {
				// interpolate the position and normal to this point on the triangle
				float coords[] = { float( 1 ) - pixel.v - pixel.w, pixel.v, pixel.w };
				Vector3 origin, direction;
				for( unsigned int i = 0; i < 3; ++i ) {
					origin += coords[i]*Vector3( ((*posEle)[ faceVert[i] ]).getVector3() );
					direction += coords[i]*Vector3( ((*normEle)[ faceVert[i] ]).getVector3() );
				}
				direction = Normalise( direction );

				// construct a ray from these values
				const Ray ray( origin, direction );

				float fMaxDisplacement = maxDisplacement;

#if 0
				// find max distance with hull
				if(!m_hullMesh.IsEmpty()) {
					KDTREE_COLLISION collisionHull;
					if(m_hullTree->IntersectsRay( ray, FLT_MAX, &collisionHull )) {
						fMaxDisplacement = collisionHull.t;
					}
					else {
						std::cerr<<"Warning: No collision with hull"<<std::endl;
					}
				}
#endif
				// raycast into the target mesh
				KDTREE_COLLISION collision;
				if( targetTree->intersectsRay( ray, fMaxDisplacement, &collision ) ) {
#if 0
					int iMaterial=0;
					if(!materialChannel.IsEmpty())
					{
						int* pData = static_cast<int*>(materialChannel.GetData());
						iMaterial = pData[collision.face];
					}
#endif
					// get the tangent, binormal and normal for this source ray
					Vector3 basis[3];
#if 0
					if( m_tangentSpaceRequired ) {
						for( int i = 0; i < 3; ++i )
						{
							basis[0] += coords[i]*Vector3( m_tangents.GetVertexData( index + i ) );
							basis[1] += coords[i]*Vector3( m_binormals.GetVertexData( index + i ) );
						}
						basis[2] = direction;
					}
#endif

					// get the pixel location
					unsigned int const offset = ( ( pixel.y*imageWidth ) + pixel.x );

					// update the weight
					++sampleWeights[offset];

					// store the displacement
					displacement[offset] += collision.t;

#if 0
					// sample the attributes
					for( std::vector< RaycasterTargetAttribute >::const_iterator it = m_attributes.begin();
						it != m_attributes.end(); ++it ) {
						RaycasterTargetAttribute const& attr( *it );

						// the uv sample will be used if this attribute is texture sampled
						attr.SampleToImage( pixel, collision, basis, iMaterial);
					}
#endif
				}
			}
		}
	++faceIt;
	}

	float* resultSampleWeights = 0;

	{
		FloatLayer& rwl = image.addLayer( "weight", 1 );
		resultSampleWeights = rwl.getData();
		memset( resultSampleWeights, 0, sizeof( float ) * imageSize );
	}

	// normalise the results based on the sample weight at each pixel
	for( unsigned int pixel = 0; pixel < imageSize; ++pixel )
	{
		// store the sample weight attribute
		float const weight = float( sampleWeights[pixel] );
		resultSampleWeights[pixel] = weight;

		// normalise the other attributes
		if( sampleWeights[pixel] > 0 )
		{
			float const weight_rcp = float( 1 ) / weight;

			// normalise the displacement
			displacement[pixel] *= weight_rcp;
#if 0
			// normalise the attributes
			for( std::vector< RaycasterTargetAttribute >::const_iterator it = m_attributes.begin();
				it != m_attributes.end(); ++it ) {
				RaycasterTargetAttribute const& attr( *it );

				// get the destination image texel
				mt_float* image = attr.GetImageData( pixel );

				// normalise each component
				for( mt_uint i = 0; i < attr.GetDataComponentCount(); ++i )
					image[i] *= weight_rcp;
			}
#endif
		}
	}
}

}
