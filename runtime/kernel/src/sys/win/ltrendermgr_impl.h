#ifndef _LTRENDERMGR_H_
#define _LTRENDERMGR_H_

#include "iltrendermgr.h"

class CLTRenderMgr : public ILTRenderMgr {
public:
	declare_interface(CLTRenderMgr);

	virtual void Init();
	virtual void Term();

	virtual LTRESULT AddEffectShader (const char *pFileName, 
		int EffectShaderID, 
		const uint32 *pVertexElements, 
		uint32 VertexElementsSize, 
		HEFFECTPOOL EffectPoolID);
	virtual LTEffectShader* GetEffectShader(int EffectShaderID);
	virtual LTRESULT CreateEffectPool (HEFFECTPOOL EffectPoolID);

	virtual LTRESULT CreateRenderTarget(uint32 nWidth, uint32 nHeight, ERenderTargetFormat eRenderTargetFormat, EStencilBufferFormat eStencilBufferFormat, HRENDERTARGET hRenderTarget);
	virtual LTRESULT InstallRenderTarget(HRENDERTARGET hRenderTarget);
	virtual LTRESULT RemoveRenderTarget(HRENDERTARGET hRenderTarget);

	virtual LTRESULT StretchRectRenderTargetToBackBuffer(HRENDERTARGET hRenderTarget);

	virtual LTRESULT GetRenderTargetDims(HRENDERTARGET hRenderTarget, uint32& nWidth, uint32 nHeight);

	virtual LTRESULT StoreDefaultRenderTarget();
	virtual LTRESULT RestoreDefaultRenderTarget();

	virtual LTRESULT UploadCurrentFrameToEffect(LTEffectShader* pEffect, const char* szParam);
	virtual LTRESULT UploadPreviousFrameToEffect(LTEffectShader* pEffect, const char* szParam);

	virtual LTRESULT SnapshotCurrentFrame();
	virtual LTRESULT SaveCurrentFrameToPrevious();
};

#endif