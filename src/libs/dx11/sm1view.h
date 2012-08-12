///-------------------------------------------------------------------------------------------------
/// \file	graphics\sm1view.h
///
/// \brief	Declares the sm1view class.
///
/// \details	
///		sm_1view description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
/// \todo	Fill in detailed file description.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef sm_1view_h__
#define sm_1view_h__

namespace Dx11 {

	class RenderContext;

	///---------------------------------------------------------------------------------------------
	/// \class	SM1view
	///
	/// \brief	Sm 1view. 
	///
	/// \todo	Fill in detailed class description.
	////////////////////////////////////////////////////////////////////////////////////////////////
	class SM1view {
	public:
		SM1view();
		~SM1view();

		void startRender( RenderContext* context );
		void endRender( RenderContext* context );

		void debugDraw( RenderContext* context, D3D11UnorderedAccessViewSmartPtr rt );

	protected:
		TextureHandlePtr	fragmentBufferHandle;
		TextureHandlePtr	fragmentHeadBufferHandle;
		TextureHandlePtr	primitiveBufferHandle;

		TextureHandlePtr	specialFragmentChainsHandle;
		TextureHandlePtr	spanBufferDataHandle;
		TextureHandlePtr	fragmentRTHandle;
		TextureHandlePtr	fragmentRT2Handle;
		ShaderFXHandlePtr	spanBufferShaderHandle;
		ShaderFXHandlePtr	sortFragmentsShaderHandle;
		ShaderFXHandlePtr	sortSpecialFragmentsShaderHandle;
		ShaderFXHandlePtr	debugHandle;
		D3D11ShaderResourceViewSmartPtr fragmentRTUintView;
	private:
		static int nameCounter;
		uint32_t	width;
		uint32_t	height;
		uint32_t	maxSpecialFragmentChains;
	};
}

#endif // sm_1view_h__