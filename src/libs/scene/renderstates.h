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
		static const int MAX_RENDER_TARGETS = 8;
		struct CreationInfo {
			enum FLAGS {
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
		struct CreationInfo {
			enum FLAGS {
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

}
#endif