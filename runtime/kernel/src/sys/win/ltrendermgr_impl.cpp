#include "bdefs.h"
#include "ltrendermgr_impl.h"

#include "render.h"
#include "d3d_device.h"
#include "d3d_shell.h"

#include <d3d9.h>

#include "lteffectshadermgr.h"
#include "rendertargetmgr.h"
#include "rendertarget.h"

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//instantiate our implementation class
define_interface(CLTRenderMgr, ILTRenderMgr);

void CLTRenderMgr::Init()
{

}

void CLTRenderMgr::Term()
{

	if(g_Device.m_pRenderTarget)
	{
		g_Device.m_pRenderTarget->Release();
		g_Device.m_pRenderTarget = NULL;

	}

}

LTRESULT CLTRenderMgr::AddEffectShader (const char *pFileName, 
								  int EffectShaderID, 
								  const uint32 *pVertexElements, 
								  uint32 VertexElementsSize, 
								  HEFFECTPOOL EffectPoolID)
{
	if (!pFileName)
	{
		RETURN_ERROR(1, CLTRenderMgr::AddEffectShader, LT_INVALIDPARAMS);
	}

	FileRef ref;
	ref.m_FileType = FILE_ANYFILE;
	ref.m_pFilename = pFileName;
	ILTStream *pStream = client_file_mgr->OpenFile(&ref);
	if (!pStream)
	{
		RETURN_ERROR(3, CLTRenderMgr::AddEffectShader, LT_NOTFOUND);
	}

	if(!LTEffectShaderMgr::GetSingleton().AddEffectShader(pStream, pFileName, EffectShaderID, pVertexElements, VertexElementsSize, EffectPoolID))
	{
		// Close the file.
		if (pStream != NULL)
		{
			pStream->Release();
			pStream = NULL;
		}

		return LT_ERROR;
	}
	

	// Close the file.
	if (pStream != NULL)
	{
		pStream->Release();
		pStream = NULL;
	}

	return LT_OK;
}

LTEffectShader* CLTRenderMgr::GetEffectShader(int EffectShaderID)
{
	return LTEffectShaderMgr::GetSingleton().GetEffectShader(EffectShaderID);
}

LTRESULT CLTRenderMgr::CreateEffectPool (HEFFECTPOOL EffectPoolID)
{
	bool bSuccess = LTEffectShaderMgr::GetSingleton().AddEffectPool(EffectPoolID);
	if(bSuccess)
	{
		return LT_OK;
	}

	return LT_ERROR;
}

LTRESULT CLTRenderMgr::CreateRenderTarget(uint32 nWidth, uint32 nHeight, ERenderTargetFormat eRenderTargetFormat, EStencilBufferFormat eStencilBufferFormat, HRENDERTARGET hRenderTarget)
{
	LTRESULT hr = CRenderTargetMgr::GetSingleton().AddRenderTarget(nWidth, nHeight, eRenderTargetFormat, eStencilBufferFormat, hRenderTarget);
	return hr;
}

LTRESULT CLTRenderMgr::InstallRenderTarget(HRENDERTARGET hRenderTarget)
{
	CRenderTarget* pRenderTarget = CRenderTargetMgr::GetSingleton().GetRenderTarget(hRenderTarget);
	if(pRenderTarget)
	{
		if(pRenderTarget->InstallOnDevice() == LT_OK)
		{
			CRenderTargetMgr::GetSingleton().SetCurrentRenderTargetHandle(hRenderTarget);
			return LT_OK;
		}
	}

	return LT_ERROR;
}

LTRESULT CLTRenderMgr::RemoveRenderTarget(HRENDERTARGET hRenderTarget)
{
	return CRenderTargetMgr::GetSingleton().RemoveRenderTarget(hRenderTarget);
}

LTRESULT CLTRenderMgr::StretchRectRenderTargetToBackBuffer(HRENDERTARGET hRenderTarget)
{

	CRenderTarget* pRenderTarget = CRenderTargetMgr::GetSingleton().GetRenderTarget(hRenderTarget);
	if(pRenderTarget)
	{

		LPDIRECT3DSURFACE9		pBackBuffer = NULL;
		if (FAILED(PD3DDEVICE->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer)))	
		{ 
			return LT_ERROR; 
		}

		LTRESULT hr = pRenderTarget->StretchRectToSurface(pBackBuffer); //g_Device.m_pDefaultRenderTarget

		pBackBuffer->Release();

		return hr;
	}

	return LT_ERROR;
}

LTRESULT CLTRenderMgr::GetRenderTargetDims(HRENDERTARGET hRenderTarget, uint32& nWidth, uint32 nHeight)
{
	CRenderTarget* pRenderTarget = CRenderTargetMgr::GetSingleton().GetRenderTarget(hRenderTarget);
	if(pRenderTarget)
	{
		const RenderTargetParams params = pRenderTarget->GetRenderTargetParams();
		nWidth = params.Width;
		nHeight = params.Height;

		return LT_OK;
	}

	return LT_ERROR;
}

LTRESULT CLTRenderMgr::StoreDefaultRenderTarget()
{
	//LPDIRECT3DSURFACE9 pBackSurface = NULL;
	if(FAILED(PD3DDEVICE->GetRenderTarget(&g_Device.m_pDefaultRenderTarget)))
	{
		dsi_ConsolePrint("CLTRenderMgr::StoreDefaultRenderTarget() failed to get render target surface!");
		return LT_ERROR;
	}

	if(FAILED(PD3DDEVICE->GetDepthStencilSurface(&g_Device.m_pDefaultDepthStencilBuffer)))
	{
		dsi_ConsolePrint("CLTRenderMgr::StoreDefaultRenderTarget() failed to get stencil surface!");
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT CLTRenderMgr::RestoreDefaultRenderTarget()
{
	if((g_Device.m_pDefaultRenderTarget == NULL) || (g_Device.m_pDefaultDepthStencilBuffer == NULL))
	{
		return LT_ERROR;
	}

	if(FAILED(PD3DDEVICE->SetRenderTarget(g_Device.m_pDefaultRenderTarget, g_Device.m_pDefaultDepthStencilBuffer)))
	{
		dsi_ConsolePrint("CLTRenderMgr::RestoreDefaultRenderTarget() failed to set render target surface!");
		return LT_ERROR;
	}

	// Release out hold on these surfaces
	g_Device.m_pDefaultRenderTarget->Release();
	g_Device.m_pDefaultDepthStencilBuffer->Release();
	g_Device.m_pDefaultRenderTarget = NULL;
	g_Device.m_pDefaultDepthStencilBuffer = NULL;

	CRenderTargetMgr::GetSingleton().SetCurrentRenderTargetHandle(INVALID_RENDER_TARGET);

	return LT_OK;
}


LTRESULT CLTRenderMgr::SnapshotCurrentFrame()
{
	if(g_Device.m_pCurrentFrame == NULL)
	{		
		IDirect3DSurface9* pSurface = NULL;
		PD3DDEVICE->GetRenderTarget(&pSurface);

		if(pSurface)
		{	
			D3DSURFACE_DESC desc;
			if (FAILED(pSurface->GetDesc(&desc)))
			{
				dsi_ConsolePrint("Failed to get surface desc: CLTRenderMgr::SnapshotCurrentFrame()");
				return LT_ERROR;
			}

			D3DFORMAT iTargetFormat = desc.Format;

			if(FAILED(PD3DDEVICE->CreateTexture(g_Device.GetModeInfo()->Width, 
				g_Device.GetModeInfo()->Height, 
				1, 
				D3DUSAGE_RENDERTARGET, 
				iTargetFormat, 
				D3DPOOL_DEFAULT, 
				(IDirect3DTexture9**)&g_Device.m_pCurrentFrame)))
			{
				dsi_ConsolePrint("Error creating %dx%d rendertarget texture: CLTRenderMgr::SnapshotCurrentFrame()", g_Device.GetModeInfo()->Width, g_Device.GetModeInfo()->Height);
				pSurface->Release();
				return LT_ERROR;
			}
		}

		pSurface->Release();
	}

	IDirect3DSurface9 *pSrcSurface = NULL;
	if (FAILED(PD3DDEVICE->GetRenderTarget(&pSrcSurface)))
	{
		dsi_ConsolePrint("Failed to get the current RenderTarget: CLTRenderMgr::SnapshotCurrentFrame()");
		return LT_ERROR;
	}

	IDirect3DSurface9 *pDestSurface = NULL;
	if(FAILED(g_Device.m_pCurrentFrame->GetSurfaceLevel(0, &pDestSurface)))
	{
		dsi_ConsolePrint("Failed to get the current RenderTexture surface: CLTRenderMgr::SnapshotCurrentFrame()");
		return LT_ERROR;
	}

	if(pSrcSurface && pDestSurface)
	{
		if(FAILED(PD3DDEVICE->GetDevice()->StretchRect(pSrcSurface, NULL, pDestSurface, NULL, D3DTEXF_NONE)))
		{
			dsi_ConsolePrint("Failed to StretchRect: CLTRenderMgr::SnapshotCurrentFrame()");
			return LT_ERROR;
		}

		pSrcSurface->Release();
		pDestSurface->Release();
	}

	return LT_OK;
}

LTRESULT CLTRenderMgr::SaveCurrentFrameToPrevious()
{
	if(g_Device.m_pCurrentFrame == NULL)
	{
		dsi_ConsolePrint("Current frame is not valid, can not copy to previous frame buffer!: SaveCurrentFrameToPrevious()");
		return LT_ERROR;
	}

	if(g_Device.m_pPreviousFrame == NULL)
	{		
		IDirect3DSurface9* pSurface = NULL;
		if(FAILED(g_Device.m_pCurrentFrame->GetSurfaceLevel(0, &pSurface)))
		{
			return LT_ERROR;
		}

		if(pSurface)
		{	
			D3DSURFACE_DESC desc;
			if (FAILED(pSurface->GetDesc(&desc)))
			{
				dsi_ConsolePrint("Failed to get surface desc: SaveCurrentFrameToPrevious()");
				//FreeTextures();
				return LT_ERROR;
			}

			D3DFORMAT iTargetFormat = desc.Format;

			if(FAILED(PD3DDEVICE->CreateTexture(g_Device.GetModeInfo()->Width, 
				g_Device.GetModeInfo()->Height, 
				1, 
				D3DUSAGE_RENDERTARGET, 
				iTargetFormat, 
				D3DPOOL_DEFAULT, 
				(IDirect3DTexture9**)&g_Device.m_pPreviousFrame)))
			{
				dsi_ConsolePrint("Error creating %dx%d rendertarget texture: SaveCurrentFrameToPrevious()", g_Device.GetModeInfo()->Width, g_Device.GetModeInfo()->Height);
				//FreeTextures();
				pSurface->Release();
				return LT_ERROR;
			}
		}

		pSurface->Release();
	}

	IDirect3DSurface9 *pSrcSurface = NULL;
	if(FAILED(g_Device.m_pCurrentFrame->GetSurfaceLevel(0, &pSrcSurface)))
	{
		return LT_ERROR;
	}

	IDirect3DSurface9 *pDestSurface = NULL;
	if(FAILED(g_Device.m_pPreviousFrame->GetSurfaceLevel(0, &pDestSurface)))
	{
		dsi_ConsolePrint("Failed to get the current RenderTexture surface: SaveCurrentFrameToPrevious()");
		return LT_ERROR;
	}

	if(pSrcSurface && pDestSurface)
	{
		if(FAILED(PD3DDEVICE->GetDevice()->StretchRect(pSrcSurface, NULL, pDestSurface, NULL, D3DTEXF_NONE)))
		{
			dsi_ConsolePrint("Failed to StretchRect: SaveCurrentFrameToPrevious()");
			return LT_ERROR;
		}

		pSrcSurface->Release();
		pDestSurface->Release();
	}

	return LT_OK;
}

LTRESULT CLTRenderMgr::UploadCurrentFrameToEffect(LTEffectShader* pEffect, const char* szParam)
{
	LTEffectImpl* pEffectImpl = (LTEffectImpl*)pEffect;
	if(pEffectImpl && (szParam[0] != '\0'))
	{
		ID3DXEffect* pD3DEffect = pEffectImpl->GetEffect();

		if(pD3DEffect && g_Device.m_pCurrentFrame)
		{
			if(FAILED(pD3DEffect->SetTexture(szParam, g_Device.m_pCurrentFrame)))
			{
				return LT_ERROR;
			}
			else
			{
				// It worked!
				return LT_OK;
			}
		}
	}

	return LT_ERROR;
}

LTRESULT CLTRenderMgr::UploadPreviousFrameToEffect(LTEffectShader* pEffect, const char* szParam)
{
	LTEffectImpl* pEffectImpl = (LTEffectImpl*)pEffect;
	if(pEffectImpl && (szParam[0] != '\0'))
	{
		ID3DXEffect* pD3DEffect = pEffectImpl->GetEffect();

		if(pD3DEffect && g_Device.m_pPreviousFrame)
		{
			if(FAILED(pD3DEffect->SetTexture(szParam, g_Device.m_pPreviousFrame)))
			{
				return LT_ERROR;
			}
			else
			{
				// It worked!
				return LT_OK;
			}
		}
	}

	return LT_ERROR;
}