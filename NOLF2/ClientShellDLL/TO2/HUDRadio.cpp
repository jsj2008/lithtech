// ----------------------------------------------------------------------- //
//
// MODULE  : HUDRadio.cpp
//
// PURPOSE : Implementation of CHUDRadio to display messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDRadio.h"
#include "InterfaceMgr.h"
#include "ClientRes.h"
#include "ClientMultiplayerMgr.h"
#include "GameClientShell.h"

static int g_CoopRadioArray[] =
{
	IDS_DIALOGUE_33575,
	IDS_DIALOGUE_33579,
	IDS_DIALOGUE_33582,
	IDS_DIALOGUE_33584,
	IDS_DIALOGUE_33590,
	IDS_DIALOGUE_33596
};

static int g_TDMRadioArray[] =
{
	IDS_DIALOGUE_33579,
	IDS_DIALOGUE_33582,
	IDS_DIALOGUE_33584,
	IDS_DIALOGUE_33588,
	IDS_DIALOGUE_33593,
	IDS_DIALOGUE_33596
};

static int g_DDRadioArray[] =
{
	IDS_DIALOGUE_33575,
	IDS_DIALOGUE_33579,
	IDS_DIALOGUE_33582,
	IDS_DIALOGUE_33588,
	IDS_DIALOGUE_33590,
	IDS_DIALOGUE_33596
};

static GameType eGameType = eGameTypeSingle;



CHUDRadio::CHUDRadio()
{
	m_eLevel		= kHUDRenderDead;
	m_UpdateFlags	= kHUDNone;
	m_bVisible		= false;

	for (int i = 0; i < MAX_RADIO_CHOICES; i++)
	{
		m_pText[i] = LTNULL;
	}
	m_nNumChoices = 0;
}
	

LTBOOL CHUDRadio::Init()
{
	m_nNumChoices = ARRAY_LEN(g_CoopRadioArray);
	if (m_nNumChoices > MAX_RADIO_CHOICES)
		m_nNumChoices = MAX_RADIO_CHOICES;

	UpdateLayout();

	return LTTRUE;

}
void CHUDRadio::Term()
{
	m_Dlg.Destroy();
}

void CHUDRadio::Render()
{
	if (!m_bVisible) return;
	m_Dlg.Render();
}

void CHUDRadio::Update()
{
	// Sanity checks...
	if (!IsVisible()) return;

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		SetScale(g_pInterfaceResMgr->GetXRatio());

}


void CHUDRadio::Show(bool bShow)
{
	m_bVisible = bShow;

	if (eGameType != g_pGameClientShell->GetGameType())
	{
		eGameType = g_pGameClientShell->GetGameType();

		for (int i = 0; i < m_nNumChoices; i++)
		{
			if (m_pText[i])
			{
				switch (g_pGameClientShell->GetGameType())
				{
				case eGameTypeTeamDeathmatch:
					m_pText[i]->GetColumn(1)->SetString(LoadTempString(g_TDMRadioArray[i]));
					break;
				case eGameTypeDoomsDay:
					m_pText[i]->GetColumn(1)->SetString(LoadTempString(g_DDRadioArray[i]));
					break;
				default:
					m_pText[i]->GetColumn(1)->SetString(LoadTempString(g_CoopRadioArray[i]));
					break;
				}

			}
		}

	}

}

void CHUDRadio::Choose(uint8 nChoice)
{

	if (nChoice > m_nNumChoices)
		return;

	m_bVisible = false;

	uint32 nLocalID;
	g_pLTClient->GetLocalClientID(&nLocalID);
	switch (g_pGameClientShell->GetGameType())
	{
	case eGameTypeTeamDeathmatch:
		g_pClientMultiplayerMgr->DoTaunt(nLocalID,g_TDMRadioArray[nChoice]);
		break;
	case eGameTypeDoomsDay:
		g_pClientMultiplayerMgr->DoTaunt(nLocalID,g_DDRadioArray[nChoice]);
		break;
	default:
		g_pClientMultiplayerMgr->DoTaunt(nLocalID,g_CoopRadioArray[nChoice]);
		break;
	}
	

}

void CHUDRadio::SetScale(float fScale)
{
	m_Dlg.SetScale(fScale);
	m_fScale = fScale;
}

void CHUDRadio::UpdateLayout()
{

	char *pTag = "RadioWindow";

	LTIntPt offset;
	uint16 nTextWidth, nHeaderWidth;
	uint32 color = argbWhite;
	char szFrame[128] = "interface\\menu\\sprtex\\frame.dtx";

	if (g_pLayoutMgr->Exist(pTag))
	{
		m_BasePos = g_pLayoutMgr->GetPoint(pTag,"Pos");
		uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");
		m_pFont = g_pInterfaceResMgr->GetFont(nFont);
		m_nFontSize = m_nBaseFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");

		nHeaderWidth = 2 * m_nFontSize;

		m_nWidth = (uint16)g_pLayoutMgr->GetInt(pTag,"Width");

		m_Offset = g_pLayoutMgr->GetPoint(pTag,"TextOffset");
		offset = m_Offset;
		nTextWidth = (m_nWidth - 2 * offset.x) - nHeaderWidth;

		LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
		uint8 nR = (uint8)vCol.x;
		uint8 nG = (uint8)vCol.y;
		uint8 nB = (uint8)vCol.z;
		color = SET_ARGB(0xFF,nR,nG,nB);

		g_pLayoutMgr->GetString(pTag,"Frame",szFrame,sizeof(szFrame));
	}
	else
	{
		m_BasePos = LTIntPt(220,100);
		uint8 nFont = 0;
		m_pFont = g_pInterfaceResMgr->GetFont(nFont);
		m_nFontSize = m_nBaseFontSize = 12;

		nHeaderWidth = 2 * m_nFontSize;

		m_nWidth = 200;

		m_Offset = LTIntPt(8,8);
		offset = m_Offset;		
		nTextWidth = (m_nWidth - 2 * offset.x) - nHeaderWidth;

	}

	m_Dlg.Create(g_pInterfaceResMgr->GetTexture(szFrame),m_nWidth,m_nWidth);
	m_Dlg.Show(LTTRUE);
	m_Dlg.SetScale(1.0f);

	for (int i = 0; i < m_nNumChoices; i++)
	{
		if (m_pText[i])
		{
			m_Dlg.SetControlOffset(m_pText[i],offset);
			m_pText[i]->SetFont(m_pFont,m_nFontSize);
		}
		else
		{
			char szTmp[4];
			sprintf(szTmp,"%d.",i+1);
			m_pText[i] = debug_new(CLTGUIColumnCtrl);
			m_pText[i]->Create(LTNULL,LTNULL,m_pFont,m_nFontSize,LTNULL);
			m_pText[i]->AddColumn(szTmp,nHeaderWidth);
			m_pText[i]->AddColumn("x",nTextWidth);

			m_Dlg.AddControl(m_pText[i],offset);
		}

		m_pText[i]->Show(LTTRUE);
		m_Dlg.SetControlOffset(m_pText[i],offset);
		offset.y += m_pText[i]->GetBaseHeight() + 4;

		m_pText[i]->SetColors(color,color,color);
//		m_pText[i]->SetFixedWidth(nTextWidth);


	}

	m_Dlg.SetBasePos(m_BasePos);

	m_Dlg.SetSize(m_nWidth,(offset.y+m_Offset.y));

	m_Dlg.SetScale(g_pInterfaceResMgr->GetXRatio());
}


