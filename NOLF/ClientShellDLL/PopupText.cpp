// PopupText.cpp: implementation of the CPopupText class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PopupText.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "VarTrack.h"

VarTrack	g_vtPopupAdjust;

#define INVALID_ANI	((HMODELANIM)-1)

namespace
{
    LTVector g_vPos;
	HLTCOLOR hTextColor;
}

static void UpdateModelPos(HOBJECT hObj, int key)
{
    LTFLOAT fIncValue = 0.005f;

	LTVector vPos;
	g_pLTClient->GetObjectPos(hObj, &vPos);

	float fDirOffset = 0.0f;

	// Move model forward or backwards...

	if ((key == VK_LEFT) || (key == VK_RIGHT))
	{
		fIncValue = (key == VK_LEFT ? -fIncValue : fIncValue);
		fDirOffset += fIncValue;
	}


	float fDirUOffset = 0.0f;

	// Move the model up/down

	if ((key == VK_UP) || (key == VK_DOWN))
	{
		fIncValue = (key == VK_DOWN) ? -fIncValue : fIncValue;
		fDirUOffset += fIncValue;
	}

	vPos.z += fDirOffset;
	vPos.y += fDirUOffset;

	// Okay, set the new position...

	if (fDirOffset || fDirUOffset)
	{
		g_pLTClient->SetObjectPos(hObj, &vPos, LTTRUE);
		g_pLTClient->CPrint("Popup pos = %f, %f, %f", vPos.x, vPos.y, vPos.z);
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPopupText::CPopupText()
{
	m_bVisible			= LTFALSE;

    m_hForeSurf         = LTNULL;
    m_pForeFont         = LTNULL;
}



CPopupText::~CPopupText()
{
	Term();
}


void CPopupText::Init()
{
    g_vtPopupAdjust.Init(g_pLTClient, "PopupAdjust", NULL, 0.0f);

	m_rcRect		= g_pLayoutMgr->GetPopupTextRect();

	m_pForeFont		= g_pInterfaceResMgr->GetMsgForeFont();

	m_nLineHeight	= m_pForeFont->GetHeight();

	m_bVisible		= LTFALSE;
	m_dwWidth		= 0;
	m_dwHeight		= 0;

	hTextColor = g_pLayoutMgr->GetPopupTextTint();
}

void CPopupText::Term()
{
	if (m_bVisible)
	{
		Clear();
	}

	if (m_hForeSurf)
	{
        g_pLTClient->DeleteSurface(m_hForeSurf);
        m_hForeSurf = LTNULL;
	}
}

void CPopupText::ClearSurfaces()
{
	uint32 dwWidth  = 0;
	uint32 dwHeight  = 0;
	g_pLTClient->GetSurfaceDims(m_hForeSurf,&dwWidth,&dwHeight);
    LTRect rcFore(0,0, dwWidth, dwHeight);
    g_pLTClient->FillRect(m_hForeSurf, &rcFore, LTNULL);
	g_pLTClient->OptimizeSurface(m_hForeSurf, LTNULL);
}


void CPopupText::ShowText(int nStringId)
{
	HSTRING hText = g_pLTClient->FormatString(nStringId);
	if (!hText) return;

	char* pText = g_pLTClient->GetStringData(hText);
	if (!pText || !*pText) return;

	m_pos.x = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_rcRect.left);
	m_pos.y = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_rcRect.top);
	int width = m_rcRect.right - m_rcRect.left;
	int height = m_rcRect.bottom - m_rcRect.top;
	width = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)width);
	height = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)height);

	m_dwWidth = 0;
	m_dwHeight = 0;
	
	if (m_hForeSurf)
	{
		g_pLTClient->GetSurfaceDims(m_hForeSurf,&m_dwWidth,&m_dwHeight);
	}

	LTIntPt size = m_pForeFont->GetTextExtentsFormat(hText,width);
	if (size.y > height) size.y = height;

	if ((uint32)size.x > m_dwWidth || (uint32)size.y > m_dwHeight)
	{
		if (m_hForeSurf) g_pLTClient->DeleteSurface(m_hForeSurf);
		m_hForeSurf = g_pLTClient->CreateSurface(size.x,size.y);
		m_dwWidth = (uint32)size.x;
		m_dwHeight = (uint32)size.y;
	}

	ClearSurfaces();

	m_pos.x += (width - size.x) / 2;
	m_pos.y += (height - size.y);

	m_pForeFont->DrawFormat(hText,m_hForeSurf,0,0,(uint32)width,kWhite);

	m_bVisible = LTTRUE;
	g_pLTClient->FreeString(hText);

	if (g_pInterfaceMgr)
	{
		g_pGameClientShell->GetWeaponModel()->Disable(LTTRUE);
		g_pInterfaceMgr->EnableCrosshair(LTFALSE);
	}

	m_nKey = GetCommandKey(COMMAND_ID_ACTIVATE);
}

void CPopupText::Clear()
{
	ClearSurfaces();

	while (m_SFXArray.GetSize() > 0)
	{
		CSpecialFX *pSFX = m_SFXArray[0];
		debug_delete(pSFX);
		m_SFXArray.Remove(0);
	}

	m_bVisible = LTFALSE;

	if (g_pInterfaceMgr)
	{
		g_pGameClientShell->GetWeaponModel()->Disable(LTFALSE);
		g_pInterfaceMgr->EnableCrosshair(LTTRUE);
	}
}

void CPopupText::Draw()
{
	if (!m_bVisible) return;

	LTSurfaceBlend oldBlend = LTSURFACEBLEND_ALPHA;

	g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
	g_pLTClient->DrawSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hForeSurf, NULL, m_pos.x+1, m_pos.y+1);
	g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ADD);
	g_pLTClient->SetOptimized2DColor(hTextColor);
	g_pLTClient->DrawSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hForeSurf, NULL, m_pos.x, m_pos.y);
	g_pLTClient->SetOptimized2DColor(kWhite);
	g_pLTClient->SetOptimized2DBlend(oldBlend);
}


void CPopupText::Show(HMESSAGEREAD hMessage)
{
//	g_pLTClient->ClearInput();
    uint32 dwId = g_pLTClient->ReadFromMessageDWord(hMessage);
	ShowText((int)dwId);

    uint8 nSFX = g_pLTClient->ReadFromMessageByte(hMessage);
	int nSFXID = 0;
	char szSFX[128] = "";
	if (!m_bVisible) return;

	HOBJECT hCamera = g_pGameClientShell->GetCamera();
	if (!hCamera) return;
	g_pLTClient->GetObjectPos(hCamera, &g_vPos);
	if (nSFX)
	{
		for (uint8 i = 0; i < nSFX; i++)
		{
			hMessage->ReadStringFL(szSFX, sizeof(szSFX));
			CreateScaleFX(szSFX);
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(szSFX);
			nSFXID = pScaleFX->nId;
		}
	}
	else
	{
		CreateScaleFX("DefaultPopup");
	}

	g_pGameClientShell->GetIntelItemMgr()->AddItem(nSFXID,dwId);

}


void CPopupText::CreateScaleFX(char *szFXName)
{
	CBaseScaleFX *pSFX = LTNULL;
	CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(szFXName);
	if (pScaleFX)
	{
		pSFX = debug_new(CBaseScaleFX);
		// This assumes that flag_reallyclose is always used with
		// these fx.
		g_pFXButeMgr->CreateScaleFX(pScaleFX, g_vPos, LTVector(0, 0, 1),
			LTNULL, LTNULL, pSFX);
		m_SFXArray.Add(pSFX);

		if (pScaleFX->eType == SCALEFX_MODEL)
		{
			HOBJECT hObj = pSFX->GetObject();
			if (hObj)
			{
				uint32 dwAni = g_pLTClient->GetAnimIndex(hObj, "Interface");
				if (dwAni != INVALID_ANI)
				{
					g_pLTClient->SetModelAnimation(hObj, dwAni);
				}
			}
		}
	}
}

void CPopupText::Update()
{
	for (uint32 i = 0; i < m_SFXArray.GetSize(); i++)
	{
		m_SFXArray[i]->Update();
	}
}

LTBOOL CPopupText::OnKeyDown(int key, int rep)
{
	// They pressed escape - close the popup
	if (key == VK_ESCAPE)// || key == m_nKey)
	{
		Clear();
        return LTTRUE;
	}
	else if (g_vtPopupAdjust.GetFloat())
	{
		for (uint32 i = 0; i < m_SFXArray.GetSize(); i++)
		{
			if (g_vtPopupAdjust.GetFloat())
			{
				if (g_pLTClient->GetObjectType(m_SFXArray[i]->GetObject()) == OT_MODEL)
				{
					UpdateModelPos(m_SFXArray[i]->GetObject(), key);
				}
			}
		}
	}
	return LTFALSE;
}
