#ifndef _RENDERTARGETMGR_H_
#define _RENDERTARGETMGR_H_

#include "ltbasedefs.h"
#include "iltrendermgr.h"
#include <map>

class CRenderTarget;
typedef std::map<HRENDERTARGET, CRenderTarget*> LTRenderTargetMap;

class CRenderTargetMgr
{
public:
	~CRenderTargetMgr();

	// singleton access
	static CRenderTargetMgr&	GetSingleton();

	LTRESULT Init();
	LTRESULT Term();

	LTRESULT		AddRenderTarget(int Width, int Height, ERenderTargetFormat eRTFormat, EStencilBufferFormat eDSFormat, HRENDERTARGET hRenderTarget);
	CRenderTarget*	GetRenderTarget(HRENDERTARGET hRenderTarget);
	LTRESULT		RemoveRenderTarget(HRENDERTARGET hRenderTarget);
	void			SetCurrentRenderTargetHandle(HRENDERTARGET hRenderTarget){m_hRenderTarget = hRenderTarget;}
	HRENDERTARGET	GetCurrentRenderTargetHandle(){return m_hRenderTarget;}

	// frees all the device shader handles
	void						FreeDeviceObjects();

	// recreates all the shaders. This is necessary when the device changes.
	void						RecreateRenderTargets();

protected:

	LTRenderTargetMap m_RenderTargets;
	HRENDERTARGET		m_hRenderTarget;
};

#endif