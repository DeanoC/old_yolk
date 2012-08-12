#define CF_STATIC			0
#define CF_PER_FRAME		1
#define CF_PER_PIPELINE		2
#define CF_PER_VIEWS		3
#define CF_PER_TARGETS		4
#define CF_STD_OBJECT		5

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
#define VE_BONEINDICES		10
#define VE_BONEWEIGHTS		11

binding( CF_STATIC ) struct Static {
	float4		dummy;
};

binding( CF_PER_FRAME ) struct PerFrame {
	uint4		frameCount;
};

binding( CF_PER_PIPELINE ) struct PerPipeline {
	float16		matrixProjection;
	float16		matrixProjectionInverse;
	float16		matrixProjectionIT;
	float4		zPlanes;		//x = near, y = far, z = far - near
	float4		fov;
};

binding( CF_PER_VIEWS ) struct PerViews {
	float16		matrixView;
	float16		matrixViewInverse;
	float16		matrixViewIT;
	float16		matrixViewProjection;
	float16		matrixViewProjectionInverse;
	float16		matrixViewProjectionIT;
};

binding( CF_PER_TARGETS ) struct PerTargets {
	uint4		targetDims;
};

binding( CF_STD_OBJECT ) struct StdObject {
	float16		 matrixWorld;
	float16		 matrixWorldInverse;
	float16		 matrixWorldIT;
	float16		 matrixWorldView;
	float16		 matrixWorldViewInverse;
	float16		 matrixWorldViewIT;
	float16		 matrixWorldViewProjection;
	float16		 matrixWorldViewProjectionInverse;
	float16		 matrixWorldViewProjectionIT;
	float16		 matrixPreviousWorldViewProjection;
};

