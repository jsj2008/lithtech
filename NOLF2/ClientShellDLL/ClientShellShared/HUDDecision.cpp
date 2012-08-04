// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDecision.cpp
//
// PURPOSE : Implementation of CHUDDecision to display messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDDecision.h"
#include "InterfaceResMgr.h"
#include "LayoutMgr.h"
#include "PlayerMgr.h"
#include "MsgIDs.h"
#include "ClientUtilities.h"
#include "CMoveMgr.h"


static const char* szOpenSound = "interface\\snd\\dialogueopen.wav";
static const char* szCloseSound = "interface\\snd\\dialogueClose.wav";

CHUDDecision::CHUDDecision()
{
	m_UpdateFlags = kHUDFrame;
	m_bVisible = LTFALSE;
	for (int i = 0; i < MAX_DECISION_CHOICES; i++)
	{
		m_pText[i] = LTNULL;
	}
	m_hObject = LTNULL;
	m_fRadius = 0.0f;
}
	

LTBOOL CHUDDecision::Init()
{
	UpdateLayout();

	return LTTRUE;

}
void CHUDDecision::Term()
{
	m_Dlg.Destroy();
}

void CHUDDecision::Render()
{
	if (!m_bVisible) return;
	m_Dlg.Render();
}

void CHUDDecision::Update()
{
	// Sanity checks...
	if (!IsVisible()) return;

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		SetScale(g_pInterfaceResMgr->GetXRatio());

	LTVector vPos;
	g_pLTClient->GetObjectPos(g_pPlayerMgr->GetMoveMgr()->GetObject(), &vPos);
	float fDist = vPos.Dist(m_vObjPos);
	if (m_fRadius > 0.0f && fDist > m_fRadius)
	{
		Hide();
	}


}

void CHUDDecision::Hide()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_DECISION);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(-1);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	m_hObject = LTNULL;

	if (m_bVisible)
		g_pClientSoundMgr->PlayInterfaceSound(szCloseSound);
	m_bVisible = LTFALSE;
	


}

void CHUDDecision::Show(ILTMessage_Read *pMsg)
{
	HOBJECT hObj = LTNULL;
	float	fRad = 0.0f;
	uint32  nID[MAX_DECISION_CHOICES];
	memset(nID,0,sizeof(nID));

	LTBOOL bShow = (LTBOOL)pMsg->Readuint8();
	if (bShow)
	{
		LTBOOL bForce = (LTBOOL)pMsg->Readuint8();
		g_pLTClient->CPrint("CHUDDecision::Show() : show");
		//read in the message
		for (int i=0; i < MAX_DECISION_CHOICES; i++)
		{
			nID[i] = pMsg->Readuint32();
		}

		hObj = pMsg->ReadObject();
		fRad = pMsg->Readfloat();

		g_pLTClient->GetObjectPos(hObj, &m_vObjPos);

		LTVector vPos;
		g_pLTClient->GetObjectPos(g_pPlayerMgr->GetMoveMgr()->GetObject(), &vPos);
		float fDist = vPos.Dist(m_vObjPos);

		m_hObject = hObj;
		m_fRadius = fRad;

		//if we're out of range, bail right away
		if (!bForce && fRad > 0.0f && fDist > fRad)
		{
			g_pLTClient->CPrint("CHUDDecision::Show() : out of range");
			Hide();
			return;
		}

		//if a decision window is open and we get a new one, hide (and abort) the old one
		if (m_bVisible && m_hObject)
		{
			m_bVisible = LTFALSE; //set here so close soun doesn't play
			Hide();
		}

		//fill the new window
		m_Dlg.SetScale(1.0f);
		LTIntPt offset = m_Offset;
		for (int i=0; i < MAX_DECISION_CHOICES; i++)
		{
			char szStr[256] = "";
			if (nID[i])
			{
				LoadString(nID[i],szStr,sizeof(szStr));
			}
			if (strlen(szStr))
			{
				if (m_pText[i])
				{
					m_pText[i]->Show(LTTRUE);
					m_pText[i]->GetColumn(1)->SetString(szStr);
					m_Dlg.SetControlOffset(m_pText[i],offset);
					offset.y += m_pText[i]->GetHeight() + 4;
				}
			}
			else if (m_pText[i])
			{
				m_pText[i]->Show(LTFALSE);
			}



		}

		m_Dlg.SetSize(m_nWidth,(offset.y+m_Offset.y));
		m_Dlg.SetScale(m_fScale);

		if (!m_bVisible)
			g_pClientSoundMgr->PlayInterfaceSound(szOpenSound);

		m_bVisible = LTTRUE;
	}
	else
	{
		HOBJECT hObject = pMsg->ReadObject();
		if (m_hObject == hObject)
		{
			g_pLTClient->CPrint("CHUDDecision::Show() : hide");
			m_hObject = LTNULL;
			if (m_bVisible)
				g_pClientSoundMgr->PlayInterfaceSound(szCloseSound);
			m_bVisible = LTFALSE;
		}

	}





}

void CHUDDecision::Choose(uint8 nChoice)
{
	m_bVisible = LTFALSE;
	if (!m_hObject) return;

	if (nChoice > MAX_DECISION_CHOICES)
		nChoice = MAX_DECISION_CHOICES;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_DECISION);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(nChoice);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	g_pClientSoundMgr->PlayInterfaceSound(szCloseSound);
	m_hObject = LTNULL;
}

void CHUDDecision::SetScale(float fScale)
{
	m_Dlg.SetScale(fScale);
	m_fScale = fScale;
}

void CHUDDecision::UpdateLayout()
{

	char *pTag = "DecisionWindow";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag,"Pos");

	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");
	m_pFont = g_pInterfaceResMgr->GetFont(nFont);
	m_nFontSize = m_nBaseFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");

	uint16 nHeaderWidth = 2 * m_nFontSize;

	m_nWidth = (uint16)g_pLayoutMgr->GetInt(pTag,"Width");

	m_Offset = g_pLayoutMgr->GetPoint(pTag,"TextOffset");
	LTIntPt offset = m_Offset;

	uint16 nTextWidth = (m_nWidth - 2 * offset.x) - nHeaderWidth;
	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	uint32 color = SET_ARGB(0xFF,nR,nG,nB);

	char szFrame[128] = "";
	g_pLayoutMgr->GetString(pTag,"Frame",szFrame,sizeof(szFrame));

	m_Dlg.Create(g_pInterfaceResMgr->GetTexture(szFrame),m_nWidth,m_nWidth);
	m_Dlg.Show(LTTRUE);
	m_Dlg.SetScale(1.0f);

	for (int i = 0; i < MAX_DECISION_CHOICES; i++)
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
			m_pText[i]->AddColumn("X",nTextWidth);

			m_Dlg.AddControl(m_pText[i],offset);
		}

		m_pText[i]->SetColors(color,color,color);
//		m_pText[i]->SetFixedWidth(nTextWidth);

		offset.y += m_pText[i]->GetBaseHeight() + 4;

	}

	m_Dlg.SetBasePos(m_BasePos);

	m_Dlg.SetSize(m_nWidth,(offset.y+m_Offset.y));

	m_Dlg.SetScale(g_pInterfaceResMgr->GetXRatio());
}



void CHUDDecision::OnObjectRemove(HLOCALOBJ hObj)
{
	if (!hObj || m_hObject != hObj) return;

	m_hObject = LTNULL;
	m_bVisible = LTFALSE;

}
