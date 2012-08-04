// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDecision.cpp
//
// PURPOSE : Implementation of CHUDDecision to display messages
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDDecision.h"
#include "InterfaceResMgr.h"
#include "PlayerMgr.h"
#include "MsgIDs.h"
#include "ClientUtilities.h"
#include "CMoveMgr.h"


static const char* szOpenSound = "DialogueOpen";
static const char* szCloseSound = "DialogueClose";

VarTrack g_vtHUDDecisionRender;

CHUDDecision::CHUDDecision()
{
	m_UpdateFlags = kHUDFrame;
	m_bVisible = false;
	for (int i = 0; i < MAX_DECISION_CHOICES; i++)
	{
		m_pText[i] = NULL;
	}
	m_hObject = NULL;
	m_fRadius = 0.0f;
}
	

bool CHUDDecision::Init()
{
	g_vtHUDDecisionRender.Init( g_pLTClient, "HUDDecisionRender", NULL, 1.0f );

	UpdateLayout();
	ScaleChanged();

	return true;

}
void CHUDDecision::Term()
{
	m_Dlg.Destroy();
}

void CHUDDecision::Render()
{
	if( g_vtHUDDecisionRender.GetFloat( ) < 1.0f )
		return;

	if (!m_bVisible) return;
	m_Dlg.Render();
}

void CHUDDecision::Update()
{
	// Sanity checks...
	if (!IsVisible()) return;

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
	cMsg.Writeuint8((uint8)-1);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	m_hObject = NULL;

	if (m_bVisible)
		g_pClientSoundMgr->PlayInterfaceSound(szCloseSound);
	m_bVisible = false;
	


}

void CHUDDecision::Show(ILTMessage_Read *pMsg)
{
	HOBJECT hObj = NULL;
	float	fRad = 0.0f;
	uint32  nID[MAX_DECISION_CHOICES];
	memset(nID,0,sizeof(nID));

	bool bShow = !!pMsg->Readuint8();
	if (bShow)
	{
		bool bForce = !!pMsg->Readuint8();
//		DebugCPrint(1,"CHUDDecision::Show() : show");
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
			Hide();
			return;
		}

		//if a decision window is open and we get a new one, hide (and abort) the old one
		if (m_bVisible && m_hObject)
		{
			m_bVisible = false; //set here so close soun doesn't play
			Hide();
		}

		//fill the new window
		LTVector2n offset = m_vTextOffset;
		for (i=0; i < MAX_DECISION_CHOICES; i++)
		{
			const wchar_t* wszStr = NULL;
			if (nID[i])
			{
				wszStr = LoadString( nID[i] );
			}
			if( wszStr && (wszStr[0] != '\0') )
			{
				if (m_pText[i])
				{
					m_pText[i]->Show(true);
					m_pText[i]->GetColumn(1)->SetString(wszStr);
					m_Dlg.SetControlOffset(m_pText[i],offset);
					offset.y += m_pText[i]->GetBaseHeight() + 4;
				}
			}
			else if (m_pText[i])
			{
				m_pText[i]->Show(false);
			}



		}

		m_Dlg.SetSize( LTVector2n(m_nWidth,(offset.y+m_vTextOffset.y)));

		if (!m_bVisible)
			g_pClientSoundMgr->PlayInterfaceDBSound(szOpenSound);

		m_bVisible = true;
	}
	else
	{
		HOBJECT hObject = pMsg->ReadObject();
		if (m_hObject == hObject)
		{
			m_hObject = NULL;
			if (m_bVisible)
				g_pClientSoundMgr->PlayInterfaceDBSound(szCloseSound);
			m_bVisible = false;
		}

	}





}

void CHUDDecision::Choose(uint8 nChoice)
{
	m_bVisible = false;
	if (!m_hObject) return;

	if (nChoice > MAX_DECISION_CHOICES)
		nChoice = MAX_DECISION_CHOICES;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_DECISION);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(nChoice);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	g_pClientSoundMgr->PlayInterfaceSound(szCloseSound);
	m_hObject = NULL;
}

void CHUDDecision::ScaleChanged()
{

	LTVector2n vPos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(vPos);
	m_Dlg.SetBasePos(vPos);
}

void CHUDDecision::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDDecision");
	}

	CHUDItem::UpdateLayout();


	uint32 nHeaderWidth = 2 * m_sTextFont.m_nHeight;

	ASSERT( m_vIconSize.x == ( uint16 )m_vIconSize.x );
	m_nWidth = (uint16)m_vIconSize.x;

	LTVector2n offset = m_vTextOffset;

	uint32 nTextWidth = (m_nWidth - 2 * offset.x) - nHeaderWidth;

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMax = m_vIconSize;
	m_Dlg.Create(m_hIconTexture,cs);
	m_Dlg.Show(true);

	for (int i = 0; i < MAX_DECISION_CHOICES; i++)
	{
		if (m_pText[i])
		{
			m_Dlg.SetControlOffset(m_pText[i],offset);
			m_pText[i]->SetFont(m_sTextFont);
		}
		else
		{
			wchar_t wszTmp[4];
			swprintf(wszTmp,L"%d.",i+1);
			cs.rnBaseRect.Right() = nTextWidth+nHeaderWidth;
			cs.rnBaseRect.Bottom() = m_sTextFont.m_nHeight;
			m_pText[i] = debug_new(CLTGUIColumnCtrl);
			m_pText[i]->Create(m_sTextFont,cs);
			m_pText[i]->AddColumn(wszTmp,nHeaderWidth,true);
			m_pText[i]->AddColumn(L"X",nTextWidth,true);

			m_Dlg.AddControl(m_pText[i],offset);
		}

		m_pText[i]->SetColor(m_cTextColor);

		offset.y += m_pText[i]->GetBaseHeight() + 4;

	}

	m_Dlg.SetBasePos(m_vBasePos);
	m_Dlg.SetSize( LTVector2n(m_nWidth,(offset.y+m_vTextOffset.y)) );

}



void CHUDDecision::OnObjectRemove(HLOCALOBJ hObj)
{
	if (!hObj || m_hObject != hObj) return;

	m_hObject = NULL;
	m_bVisible = false;

}
