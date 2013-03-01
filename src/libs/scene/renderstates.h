#ifndef YOLK_SCENE_RENDER_STATES_H_
#define YOLK_SCENE_RENDER_STATES_H_ 1

#ifndef YOLK_SCENE_RENDER_STATES_ENUMS_H_
#include "renderstates_enums.h"
#endif

#ifndef YOLK_CORE_COLOUR_H_
#include "core/colour.h"
#endif

#ifndef YOLK_CORE_RESOURCES_H_
#include "core/resources.h"
#endif

namespace Scene {

	static const uint32_t SamplerStateType = RESOURCE_NAME('G','S','A','S');
	static const uint32_t RenderTargetStatesType = RESOURCE_NAME('G','R','T','S');
	static const uint32_t DepthStencilStateType = RESOURCE_NAME('G','D','S','S');
	static const uint32_t RasteriserStateType = RESOURCE_NAME('G','R','S','S');

	class SamplerState : public Core::Resource<SamplerStateType> {
	public:
		friend class ResourceLoader;
		virtual ~SamplerState(){}
		struct CreationInfo {
			FILTER_MODE 		filter;
			ADDRESS_MODE 		addressU;
			ADDRESS_MODE 		addressV;
			ADDRESS_MODE 		addressW;
			COMPARE_FUNC 		compareFunc;

			float				mipLODBias;
			float				minLOD;
			float				maxLOD;

			int					maxAnisotropy;

			Core::RGBAColour	borderColour;
		};
	protected:
		static const void* internalPreCreate( const char* name, const CreationInfo *creation );
	};
	typedef const Core::ResourceHandle<SamplerStateType, SamplerState> SamplerStateHandle;
	typedef SamplerStateHandle* SamplerStateHandlePtr;
	typedef std::shared_ptr<SamplerState> SamplerStatePtr;

	struct TargetState {
		enum FLAGS {
			NONE		= 0,
			BLEND_ENABLE = BIT(0),
			LOGIC_OP_ENABLE	= BIT(1),		// not supported on dx11 api/hw, so be careful
		};
		FLAGS 				flags;

		TARGET_WRITE_ENABLE	writeEnable;

		BLEND_MODE			blendSrcColour;
		BLEND_MODE			blendDstColour;
		BLEND_OP			blendColourOp;

		BLEND_MODE			blendSrcAlpha;
		BLEND_MODE			blendDstAlpha;
		BLEND_OP			blendAlphaOp;

		LOGIC_OP			logicOp;		// not supported on dx11 api/hw, so be careful
	};

	class RenderTargetStates : public Core::Resource<RenderTargetStatesType> {
	public:
		friend class ResourceLoader;
		virtual ~RenderTargetStates(){}
		static const int MAX_RENDER_TARGETS = 8;
		struct CreationInfo {
			enum FLAGS {
				NONE					= 0,
				ALPHA_TO_COV 			= BIT(0),
			}					flags;

			int 				numActive;
			TargetState			renderTargetState[ MAX_RENDER_TARGETS ];
		};
	protected:
		static const void* internalPreCreate( const char* name, const CreationInfo *creation );
	};
	typedef const Core::ResourceHandle<RenderTargetStatesType, RenderTargetStates> RenderTargetStatesHandle;
	typedef RenderTargetStatesHandle* RenderTargetStatesHandlePtr;
	typedef std::shared_ptr<RenderTargetStates> RenderTargetStatesPtr;

	class DepthStencilState : public Core::Resource<DepthStencilStateType> {
	public:
		friend class ResourceLoader;
		virtual ~DepthStencilState(){}

		struct CreationInfo {
			enum FLAGS {
				DEPTH_ENABLE	= BIT(0),
				STENCIL_ENABLE	= BIT(1),
				DEPTH_WRITE		= BIT(2),
			};

			FLAGS 				flags;
			COMPARE_FUNC 		depthCompare;
			
			uint8_t				stencilReadMask;
			uint8_t 			stencilWriteMask;
			
			STENCIL_OP			stencilFrontFailOp;
			STENCIL_OP			stencilFrontDepthFailOp;
			STENCIL_OP			stencilFrontPassOp;
			COMPARE_FUNC 		stencilFrontCompare;

			STENCIL_OP			stencilBackFailOp;
			STENCIL_OP			stencilBackDepthFailOp;
			STENCIL_OP			stencilBackPassOp;
			COMPARE_FUNC 		stencilBackCompare;
		};
	protected:
		static const void* internalPreCreate( const char* name, const CreationInfo *creation );
	};
	typedef const Core::ResourceHandle<DepthStencilStateType, DepthStencilState> DepthStencilStateHandle;
	typedef DepthStencilStateHandle* DepthStencilStateHandlePtr;
	typedef std::shared_ptr<DepthStencilState> DepthStencilStatePtr;

	class RasteriserState : public Core::Resource<RasteriserStateType> {
	public:
		friend class ResourceLoader;
		virtual ~RasteriserState(){}
		struct CreationInfo {
			enum FLAGS {
				NONE			= 0,
				FRONT_CCW		= BIT(0),
				DEPTH_CLAMP		= BIT(1),
				SCISSOR			= BIT(2),
				MULTISAMPLE		= BIT(3),
				AA_LINES		= BIT(4),
			};

			FLAGS				flags;
			FILL_MODE			fillMode;
			CULL_MODE			cullMode;

			int					depthBias;
			float				depthBiasClamp;
			float				depthScaledSlopeBias;
		};
	protected:
		static const void* internalPreCreate( const char* name, const CreationInfo *creation );
	};
	typedef const Core::ResourceHandle<RasteriserStateType, RasteriserState> RasteriserStateHandle;
	typedef RasteriserStateHandle* RasteriserStateHandlePtr;
	typedef std::shared_ptr<RasteriserState> RasteriserStatePtr;

	// standard states (can be got from the resource manager with just a name
}

#define SAMPLER_STATE_POINT_CLAMP					"_SS_Point_Clamp"
#define SAMPLER_STATE_LINEAR_CLAMP					"_SS_Linear_Clamp"
#define SAMPLER_STATE_ANISO16_CLAMP					"_SS_Aniso16_Clamp"
#define SAMPLER_STATE_POINT_WRAP					"_SS_Point_Wrap"
#define SAMPLER_STATE_LINEAR_WRAP					"_SS_Linear_Wrap"
#define SAMPLER_STATE_ANISO16_WRAP					"_SS_Aniso16_Wrap"

#define RENDER_STATE_NORMAL 						"_RS_Normal"
#define RENDER_STATE_NORMAL_NOCULL 					"_RS_Normal_NoCull"
#define DEPTH_STENCIL_STATE_NORMAL 					"_DSS_Normal"
#define DEPTH_STENCIL_STATE_LESS_NOWRITE 			"_DSS_Less_NoWrite"
#define RENDER_TARGET_STATES_NOWRITE		 		"_RTS_NoWrite"
#define RENDER_TARGET_STATES_NOBLEND_WRITEALL 		"_RTS_NoBlend_WriteAll"
#define RENDER_TARGET_STATES_NOBLEND_WRITECOLOUR	"_RTS_NoBlend_WriteColour"
#define RENDER_TARGET_STATES_NOBLEND_WRITEALPHA		"_RTS_NoBlend_WriteAlpha"
#define RENDER_TARGET_STATES_OVER_WRITEALL			"_RTS_Over_WriteAll"
#define RENDER_TARGET_STATES_PMOVER_WRITEALL		"_RTS_PMOver_WriteAll"
#define RENDER_TARGET_STATES_ADD_WRITEALL			"_RTS_Add_WriteAll"
#endif