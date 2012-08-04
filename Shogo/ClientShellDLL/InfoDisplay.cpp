#include "clientheaders.h"
#include "clientheaders.h"
#include "InfoDisplay.h"
#include "TextHelper.h"

LTBOOL CInfoDisplay::Init (ILTClient* pClientDE)
{
	if (!pClientDE) return LTFALSE;

	m_pClientDE = pClientDE;

	return LTTRUE;
}

void CInfoDisplay::Term()
{
	if (!m_pClientDE) return;

	DI_INFO* pInfo = m_pInfoList;
	while (pInfo)
	{
		if (pInfo->hSurface && pInfo->bDeleteSurface)
		{
			m_pClientDE->DeleteSurface (pInfo->hSurface);
		}
		pInfo = pInfo->pNext;
	}

	DI_INFO* pPrev = LTNULL;
	pInfo = m_pInfoList;
	while (pInfo)
	{
		pPrev = pInfo;
		pInfo = pInfo->pNext;
		delete pPrev;
	}

	m_pInfoList = LTNULL;
	m_pClientDE = LTNULL;
}

LTBOOL CInfoDisplay::AddInfo (HSURFACE hSurface, LTFLOAT nSeconds, uint32 nLocationFlags, LTBOOL bDeleteSurface)
{
	if (!m_pClientDE) return LTFALSE;
	return AddToList (hSurface, nSeconds, nLocationFlags, bDeleteSurface);
}

LTBOOL CInfoDisplay::AddInfo (char* str, CBitmapFont* pFont, LTFLOAT nSeconds, uint32 nLocationFlags)
{
	if (!m_pClientDE) return LTFALSE;
	HSURFACE hSurf = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFont, str);
	if (!hSurf) return LTFALSE;
	
	HLTCOLOR hTrans = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, LTTRUE);
	m_pClientDE->OptimizeSurface (hSurf, hTrans);

	return AddToList (hSurf, nSeconds, nLocationFlags, LTTRUE);
}

LTBOOL CInfoDisplay::AddInfo (int nStringID, CBitmapFont* pFont, LTFLOAT nSeconds, uint32 nLocationFlags)
{
	if (!m_pClientDE) return LTFALSE;
	HSURFACE hSurf = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFont, nStringID);
	if (!hSurf) return LTFALSE;

	HLTCOLOR hTrans = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, LTTRUE);
	m_pClientDE->OptimizeSurface (hSurf, hTrans);

	return AddToList (hSurf, nSeconds, nLocationFlags, LTTRUE);
}

LTBOOL CInfoDisplay::AddInfo (char* str, FONT* pFontDef, HLTCOLOR hForeColor, LTFLOAT nSeconds, uint32 nLocationFlags)
{
	if (!m_pClientDE) return LTFALSE;
	HSURFACE hSurf = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontDef, str, hForeColor);
	if (!hSurf) return LTFALSE;

	HLTCOLOR hTrans = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, LTTRUE);
	m_pClientDE->OptimizeSurface (hSurf, hTrans);

	return AddToList (hSurf, nSeconds, nLocationFlags, LTTRUE);
}

LTBOOL CInfoDisplay::AddInfo (int nStringID, FONT* pFontDef, HLTCOLOR hForeColor, LTFLOAT nSeconds, uint32 nLocationFlags)
{
	if (!m_pClientDE) return LTFALSE;
	HSURFACE hSurf = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontDef, nStringID, hForeColor);
	if (!hSurf) return LTFALSE;

	HLTCOLOR hTrans = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, LTTRUE);
	m_pClientDE->OptimizeSurface (hSurf, hTrans);

	return AddToList (hSurf, nSeconds, nLocationFlags, LTTRUE);
}

void CInfoDisplay::Draw()
{
	if (!m_pClientDE) return;

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	CSize szScreen;
	m_pClientDE->GetSurfaceDims (hScreen, &szScreen.cx, &szScreen.cy);

	DI_INFO* pInfo = m_pInfoList;
	while (pInfo)
	{
		int nY = 0;
		if (pInfo->nLocationFlags & DI_TOP)
		{
			nY = 20;
		}
		else if (pInfo->nLocationFlags & DI_BOTTOM)
		{
			nY = (int)szScreen.cy - 20 - (int)pInfo->szSurface.cy;
		}
		else
		{
			nY = ((int)szScreen.cy - (int)pInfo->szSurface.cy) / 2;
		}

		int nX = 0;
		if (pInfo->nLocationFlags & DI_LEFT)
		{
			nX = 20;
		}
		else if (pInfo->nLocationFlags & DI_RIGHT)
		{
			nX = (int)szScreen.cx - 20 - (int)pInfo->szSurface.cx;
		}
		else
		{
			nX = ((int)szScreen.cx - (int)pInfo->szSurface.cx) / 2;
		}

		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, pInfo->hSurface, LTNULL, nX, nY, LTNULL);
		pInfo = pInfo->pNext;
	}

	LTFLOAT nTime = m_pClientDE->GetTime();
	DI_INFO* pPrev = LTNULL;
	DI_INFO* pNext = LTNULL;
	pInfo = m_pInfoList;
	while (pInfo)
	{
		pNext = pInfo->pNext;
		
		if (pInfo->nExpireTime < nTime)
		{
			if (pInfo->hSurface && pInfo->bDeleteSurface)
			{
				m_pClientDE->DeleteSurface (pInfo->hSurface);
			}
	
			if (pPrev)
			{
				pPrev->pNext = pNext;
			}
			else
			{
				m_pInfoList = pNext;
			}

			delete pInfo;
		}
		else
		{
			pPrev = pInfo;
		}

		pInfo = pNext;
	}
}

LTBOOL CInfoDisplay::AddToList (HSURFACE hSurface, LTFLOAT nSeconds, uint32 nLocationFlags, LTBOOL bDeleteSurface)
{
	if (!m_pClientDE || !hSurface) return LTFALSE;

	LTFLOAT nTime = m_pClientDE->GetTime();

	// first see if there's already one in the list with the same position - if so, just replace it with this one

	DI_INFO* pInfo = m_pInfoList;
	while (pInfo)
	{
		if (pInfo->nLocationFlags == nLocationFlags)
		{
			if (pInfo->hSurface && pInfo->bDeleteSurface)
			{
				m_pClientDE->DeleteSurface (pInfo->hSurface);
			}

			pInfo->hSurface = hSurface;
			m_pClientDE->GetSurfaceDims (pInfo->hSurface, &pInfo->szSurface.cx, &pInfo->szSurface.cy);
			pInfo->bDeleteSurface = bDeleteSurface;
			pInfo->nLocationFlags = nLocationFlags;
			pInfo->nExpireTime = nTime + nSeconds;

			return LTTRUE;
		}

		pInfo = pInfo->pNext;
	}

	// now add this one to the list

	pInfo = new DI_INFO;
	if (!pInfo) return LTFALSE;

	pInfo->hSurface = hSurface;
	m_pClientDE->GetSurfaceDims (pInfo->hSurface, &pInfo->szSurface.cx, &pInfo->szSurface.cy);
	pInfo->bDeleteSurface = bDeleteSurface;
	pInfo->nLocationFlags = nLocationFlags;
	pInfo->nExpireTime = nTime + nSeconds;

	if (!m_pInfoList)
	{
		m_pInfoList = pInfo;
	}
	else
	{
		DI_INFO* ptr = m_pInfoList;
		while (ptr->pNext) ptr = ptr->pNext;
		ptr->pNext = pInfo;
	}

	return LTTRUE;
}
