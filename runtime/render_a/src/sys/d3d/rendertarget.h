#ifndef _RENDERTARGET_H_
#define _RENDERTARGET_H_

#include "ltbasedefs.h"
#include "iltrendermgr.h"

struct IDirect3DTexture9;
struct IDirect3DSurface9;

struct RenderTargetParams
{
	int						Width;
	int						Height;
	ERenderTargetFormat		RT_Format;
	EStencilBufferFormat	DS_Format;
};

class CRenderTarget
{
public:
	CRenderTarget();
	~CRenderTarget();

	LTRESULT	Init(int nWidth, int nHeight, ERenderTargetFormat eRTFormat, EStencilBufferFormat eDSFormat);
	LTRESULT	Term();
	LTRESULT	Recreate();
	LTRESULT	InstallOnDevice();
	LTRESULT	StretchRectToSurface(IDirect3DSurface9* pDestSurface);
	IDirect3DTexture9* GetRenderTargetTexture(){ return m_pRenderTarget;}
	const RenderTargetParams& GetRenderTargetParams() { return m_RenderTargetParams; }
protected:

	RenderTargetParams			m_RenderTargetParams;
	IDirect3DTexture9*			m_pRenderTarget;
	IDirect3DSurface9*			m_pDepthStencilBuffer;
};

#endif