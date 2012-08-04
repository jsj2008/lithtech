#include "precompile.h"

#include "rendertargetmgr.h"
#include "rendertarget.h"


CRenderTargetMgr::~CRenderTargetMgr()
{
	Term();
}

CRenderTargetMgr& CRenderTargetMgr::GetSingleton()
{
	static CRenderTargetMgr s_Singleton;
	return s_Singleton;
}

LTRESULT CRenderTargetMgr::Init()
{
	m_hRenderTarget = INVALID_RENDER_TARGET;

	return LT_OK;
}

LTRESULT CRenderTargetMgr::Term()
{
	LTRenderTargetMap::iterator iter = m_RenderTargets.begin();
	while(iter != m_RenderTargets.end())
	{
		delete iter->second;

		++iter;
	}
	m_RenderTargets.clear();

	m_hRenderTarget = INVALID_RENDER_TARGET;

	return LT_OK;
}

LTRESULT CRenderTargetMgr::AddRenderTarget(int Width, 
											int Height, 
											ERenderTargetFormat eRTFormat, 
											EStencilBufferFormat eDSFormat, 
											HRENDERTARGET hRenderTarget)
{
	if((Width < 4) || (Height < 4))
	{
		return LT_ERROR;
	}

	//see if this render target exists first
	LTRenderTargetMap::iterator iter = m_RenderTargets.find(hRenderTarget);
	if(iter != m_RenderTargets.end())
	{
		dsi_ConsolePrint("Error: RenderTarget %d already exists. Use a different ID. ", hRenderTarget);
		return LT_ERROR;
	}

	//ok, create it then.
	CRenderTarget* pRenderTarget = new CRenderTarget;
	if(pRenderTarget)
	{
		LTRESULT hr = pRenderTarget->Init(Width, Height, eRTFormat, eDSFormat);
		if(hr == LT_OK)
		{
			m_RenderTargets[hRenderTarget] = pRenderTarget;
			return LT_OK;
		}
	}

	return LT_ERROR;
}

CRenderTarget* CRenderTargetMgr::GetRenderTarget(HRENDERTARGET hRenderTarget)
{
	LTRenderTargetMap::iterator iter = m_RenderTargets.find(hRenderTarget);
	if(iter != m_RenderTargets.end())
	{
		return iter->second;
	}

	return NULL;
}

LTRESULT CRenderTargetMgr::RemoveRenderTarget(HRENDERTARGET hRenderTarget)
{
	LTRenderTargetMap::iterator iter = m_RenderTargets.find(hRenderTarget);
	if(iter != m_RenderTargets.end())
	{
		if(iter->second)
		{
			delete iter->second;
			return LT_OK;
		}
		else
		{
			return LT_ERROR;
		}
	}

	return LT_ERROR;
}

// frees all the device shader handles
void CRenderTargetMgr::FreeDeviceObjects()
{
	LTRenderTargetMap::iterator iter = m_RenderTargets.begin();
	while(iter != m_RenderTargets.end())
	{
		iter->second->Term();

		++iter;
	}
}

// recreates all the render targets. This is necessary when the device changes.
void CRenderTargetMgr::RecreateRenderTargets()
{
	LTRenderTargetMap::iterator iter = m_RenderTargets.begin();
	while(iter != m_RenderTargets.end())
	{
		iter->second->Recreate();

		++iter;
	}
}
