#include "precompile.h"
#include "rendertarget.h"
#include "rendererconsolevars.h"

#include "d3d_device.h"
#include <d3d9.h>
#include "d3d_utils.h"


D3DFORMAT GetD3DFormatFromRTFormat(ERenderTargetFormat eFormat)
{
	switch(eFormat)
	{
	case RTFMT_A8R8G8B8:
		return D3DFMT_A8R8G8B8;
	case RTFMT_X8R8G8B8:
		return D3DFMT_X8R8G8B8;
	case RTFMT_R16F:
		return D3DFMT_R16F;
	case RTFMT_R32F:
		return D3DFMT_R32F;
	default:
		return D3DFMT_X8R8G8B8;
	}
}

D3DFORMAT GetD3DFormatFromDSFormat(EStencilBufferFormat eFormat)
{
	switch(eFormat)
	{
	case STFMT_D24S8:
		return D3DFMT_D24S8;
	case STFMT_D24X8:
		return D3DFMT_D24X8;
	case STFMT_D24X4S4:
		return D3DFMT_D24X4S4;
	case STFMT_D32:
		return D3DFMT_D32;
	case STFMT_D15S1:
		return D3DFMT_D15S1;
	case STFMT_D16:
		return D3DFMT_D16;
	default:
		return D3DFMT_UNKNOWN;
	}
}

CRenderTarget::CRenderTarget():
m_pRenderTarget(NULL),
m_pDepthStencilBuffer(NULL)
{
	memset(&m_RenderTargetParams, 0, sizeof(RenderTargetParams));
}

CRenderTarget::~CRenderTarget()
{
	Term();
}

LTRESULT CRenderTarget::Init(int nWidth, int nHeight, ERenderTargetFormat eRTFormat, EStencilBufferFormat eDSFormat)
{
	m_RenderTargetParams.Width = nWidth;
	m_RenderTargetParams.Height = nHeight;
	m_RenderTargetParams.RT_Format = eRTFormat;
	m_RenderTargetParams.DS_Format = eDSFormat;

	return Recreate();
}

LTRESULT CRenderTarget::Recreate()
{
	// If we have active surfaces then delete them
	Term();

	// Now recreate them.
	
	// Create the render target texture
	if(FAILED(PD3DDEVICE->CreateTexture(m_RenderTargetParams.Width, 
		m_RenderTargetParams.Height, 
		1, 
		D3DUSAGE_RENDERTARGET, 
		GetD3DFormatFromRTFormat(m_RenderTargetParams.RT_Format), 
		D3DPOOL_DEFAULT, 
		&m_pRenderTarget)))
	{
		dsi_ConsolePrint("Error creating %dx%d rendertarget texture: CLTRenderMgr::CreateRenderTarget() with RenderTargetFormat (%d)", m_RenderTargetParams.Width, m_RenderTargetParams.Height, m_RenderTargetParams.RT_Format);
		return LT_ERROR;
	}

	//Create the DepthStencil buffer if needed
	D3DFORMAT eDSFormat = GetD3DFormatFromDSFormat(m_RenderTargetParams.DS_Format);
	if(eDSFormat == D3DFMT_UNKNOWN)
	{
		// Just create the default
		D3DFORMAT iDepthFormat = g_Device.GetDefaultDepthStencilFormat(g_CV_ZBitDepth, g_CV_StencilBitDepth);

		//create a new one
		if(FAILED(PD3DDEVICE->CreateDepthStencilSurface(m_RenderTargetParams.Width, 
			m_RenderTargetParams.Height, 
			iDepthFormat, 
			D3DMULTISAMPLE_NONE, 
			0, 
			TRUE, 
			&m_pDepthStencilBuffer, 
			NULL)))
		{
			dsi_ConsolePrint("Failed to CreateDepthStencilSurface: CRenderTarget::Recreate()");
			return LT_ERROR;
		}
	}
	else
	{
		//create a new one
		if(FAILED(PD3DDEVICE->CreateDepthStencilSurface(m_RenderTargetParams.Width, 
			m_RenderTargetParams.Height, 
			eDSFormat, 
			D3DMULTISAMPLE_NONE, 
			0, 
			TRUE, 
			&m_pDepthStencilBuffer, 
			NULL)))
		{
			dsi_ConsolePrint("Failed to CreateDepthStencilSurface: CRenderTarget::Recreate()");
			return LT_ERROR;
		}
	}

	return LT_OK;
}

LTRESULT CRenderTarget::Term()
{
	if(m_pRenderTarget)
	{
		m_pRenderTarget->Release();
		m_pRenderTarget = NULL;
	}

	if(m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = NULL;
	}

	return LT_OK;
}

LTRESULT CRenderTarget::InstallOnDevice()
{
	if(m_pRenderTarget == NULL || m_pDepthStencilBuffer == NULL)
	{
		return LT_ERROR;
	}

	IDirect3DSurface9* pRTSurface;
	if(FAILED(m_pRenderTarget->GetSurfaceLevel(0, &pRTSurface)))
	{
		return LT_ERROR;
	}

	if(FAILED(PD3DDEVICE->SetRenderTarget(pRTSurface, m_pDepthStencilBuffer)))
	{
		dsi_ConsolePrint("Failed to set the new render target!");
		pRTSurface->Release();
		return LT_ERROR;
	}

	// We need to clear to make it a valid render target.
	if(FAILED(PD3DDEVICE->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DRGBA_255(0, 0, 0, 0), 1.0f, 0)))
	{
		PD3DDEVICE->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DRGBA_255(0, 0, 0, 0), 1.0f, 0);
	}

	pRTSurface->Release();

	return LT_OK;
}

LTRESULT CRenderTarget::StretchRectToSurface(IDirect3DSurface9* pDestSurface)
{
	IDirect3DSurface9* pSrcSurface = NULL;
	if(FAILED(m_pRenderTarget->GetSurfaceLevel(0, &pSrcSurface)))
	{
		return LT_ERROR;
	}

	if(pSrcSurface && pDestSurface)
	{
		if(FAILED(PD3DDEVICE->GetDevice()->StretchRect(pSrcSurface, NULL, pDestSurface, NULL, D3DTEXF_NONE)))
		{
			dsi_ConsolePrint("Failed to StretchRect: CRenderTarget::StretchRectToSurface");
			return LT_ERROR;
		}

		pSrcSurface->Release();
		return LT_OK;
	}

	pSrcSurface->Release();
	return LT_ERROR;
}
