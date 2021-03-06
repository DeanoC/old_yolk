#define CF_STATIC			0
#define CF_PER_FRAME		1
#define CF_PER_PIPELINE		2
#define CF_PER_VIEWS		3
#define CF_PER_TARGETS		4
#define CF_STD_OBJECT		5
#define CF_PER_MATERIAL		6
#define CF_USER_BLOCK_0		7
#define CF_USER_BLOCK_1		8
#define CF_USER_BLOCK_2		9
#define CF_USER_BLOCK_3		10
#define CF_USER_BLOCK_4		11
#define CF_USER_BLOCK_5		12
#define CF_USER_BLOCK_6		13
#define CF_USER_BLOCK_7		14
#define CF_USER_BLOCK_8		15

#if defined(D3D_SM5)
#define VE_POSITION			POSITION
#define VE_NORMAL			NORMAL
#define VE_BINORMAL			BINORMAL
#define VE_TANGENT			TANGENT
#define VE_TEXCOORD0		TEXCOORD0
#define VE_TEXCOORD1		TEXCOORD1
#define VE_TEXCOORD2		TEXCOORD2
#define VE_TEXCOORD3		TEXCOORD3
#define VE_COLOUR0			COLOR0
#define VE_COLOUR1			COLOR1

#else
#define VE_POSITION			0
#define VE_NORMAL			1
#define VE_BINORMAL			2
#define VE_TANGENT			3
#define VE_TEXCOORD0		4
#define VE_TEXCOORD1		5
#define VE_TEXCOORD2		6
#define VE_TEXCOORD3		7
#define VE_COLOUR0			8
#define VE_COLOUR1			9
#endif

constant_buffer( Static, CF_STATIC )  {
};

constant_buffer( PerFrame, CF_PER_FRAME ) {
	uint4		frameCount;
};

constant_buffer( PerPipeline, CF_PER_PIPELINE )  {
	uint4		lightCounts; // x = num directional, 
};

constant_buffer( PerViews, CF_PER_VIEWS )  {
	float16		matrixProjection;
	float16		matrixProjectionInverse;
	float16		matrixProjectionIT;
	float16		matrixView;
	float16		matrixViewInverse;
	float16		matrixViewIT;
	float16		matrixViewProjection;
	float16		matrixViewProjectionInverse;
	float16		matrixViewProjectionIT;

	float4		zPlanes;		//x = near, y = far, z = far - near
	float4		fov;
};

constant_buffer( PerTargets, CF_PER_TARGETS ) {
	uint4		targetDims;
};

constant_buffer( StdObject, CF_STD_OBJECT ) {
	float16		matrixWorld;
	float16		matrixWorldInverse;
	float16		matrixWorldIT;
	float16		matrixWorldView;
	float16		matrixWorldViewInverse;
	float16		matrixWorldViewIT;
	float16		matrixWorldViewProjection;
	float16		matrixWorldViewProjectionInverse;
	float16		matrixWorldViewProjectionIT;
	float16		matrixPreviousWorldViewProjection;
	float16		matrixUser0;
	float16		matrixUser1;
};

constant_buffer( PerMaterial, CF_PER_MATERIAL ) {
	uint4 		materialIndex;	// x only
};
