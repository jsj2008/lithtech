// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDialogue.cpp
//
// PURPOSE : HUDItem to display dialogue icons
//
// CREATED : 05/06/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDDialogue.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "ClientWeaponMgr.h"
#include "PlayerMgr.h"
#include "DialogueDB.h"
#include "ClientInfoMgr.h"
#include "ClientVoteMgr.h"

//******************************************************************************************
//**
//** HUD dialogue display
//**
//******************************************************************************************

CHUDDialogue::CHUDDialogue()
{
	m_UpdateFlags = kHUDNone;
}

bool CHUDDialogue::Init()
{
	UpdateLayout();
	ScaleChanged();

	m_Timer.SetEngineTimer(RealTimeTimer::Instance());
	EnableFade( true );

	return true;
}

void CHUDDialogue::Term()
{
}

void CHUDDialogue::Render()
{
	SetRenderState();

	float fFade = GetFadeLevel();
	if (fFade < MATH_EPSILON)
	{
		if (m_sIcon.empty())
		{
			m_Name.SetText(L"");
			m_Text.SetText(L"");
			m_hIconTexture = NULL;
		}

		return;
	}

	if (m_Timer.IsStarted())
	{
		if (m_Timer.IsTimedOut())
		{
			m_Timer.Stop();
			EndFlicker();
		}
		else
			UpdateFlicker();
	}

	//render icon here
	if (m_hIconTexture)
	{
		g_pDrawPrim->SetTexture(m_hIconTexture);
		SetupQuadUVs(m_IconPoly, m_hIconTexture, 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->DrawPrim(&m_IconPoly,1);

		m_Text.Render();
		m_Name.Render();

		//render insignia here
		if (m_hInsignia != NULL)
		{
			g_pDrawPrim->SetTexture(m_hInsignia);
			SetupQuadUVs(m_Insignia, m_hInsignia, 0.0f, 0.0f, 1.0f, 1.0f);
			g_pDrawPrim->DrawPrim(&m_Insignia,1);
		}
	}

}

void CHUDDialogue::Update()
{
}

HRECORD	CHUDDialogue::GetLayout()
{
	return g_pLayoutDB->GetHUDRecord("HUDDialogue");
}

void CHUDDialogue::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = GetLayout();
	}

	m_fFlicker = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,0,0.25f);
	m_vNameOffset = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,0);

	m_rfInsignia = g_pLayoutDB->GetRectF(m_hLayout,LDB_HUDAddRect, 0);

	CHUDItem::UpdateLayout();

	m_Name.SetFont(m_sTextFont);
	m_Name.SetColor(m_cTextColor);
	m_Name.SetAlignment(m_eTextAlignment);
}

void CHUDDialogue::Show(const char* szIcon, uint32 nClientID)
{
	HRECORD hRec = NULL;
	if (szIcon && szIcon[0] && !LTStrIEquals( szIcon, "<none>" ))
	{
		hRec = DATABASE_CATEGORY( Dialogue ).GetRecordByName( szIcon );
		Show(szIcon, hRec, nClientID);
	}
}

void CHUDDialogue::ShowRecord(HRECORD hRec, uint32 nClientID)
{
	Show(g_pLTDatabase->GetRecordName(hRec), hRec, nClientID);
}

void CHUDDialogue::Show(const char* szIcon, HRECORD hRec, uint32 nClientID)
{
	if (!hRec || !szIcon)
		return;

	//don't show while a vote is in progress
	if (IsMultiplayerGameClient() && ClientVoteMgr::Instance().IsVoteInProgress())
	{
		return;
	}


	m_sIcon = szIcon;
	m_hIconTexture.Load( DATABASE_CATEGORY( Dialogue ).GETRECORDATTRIB(hRec, IconTexture) );
	m_Text.SetText( LoadString( DATABASE_CATEGORY( Dialogue ).GETRECORDATTRIB(hRec, HeaderString ) ) );

	m_hInsignia.Free();
	if (IsMultiplayerGameClient())
	{
		CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
		CLIENT_INFO* pCI = pCIMgr->GetClientByID(nClientID);

		if (pCI)
		{
			m_Name.SetText(pCI->sName.c_str());

			if (!pCI->sInsignia.empty()) 
			{
				m_hInsignia.Load(pCI->sInsignia.c_str());
			}
		}
		else
		{
			m_Name.SetText( L"" );
		}
	}
	else
	{
		m_Name.SetText( LoadString( DATABASE_CATEGORY( Dialogue ).GETRECORDATTRIB(hRec, NameString ) ) );
	}

	m_Text.SetColor(m_cTextColor);
	m_Name.SetColor(m_cTextColor);
	DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
	ResetFade();
	EnableFade(false);

	StartFlicker();
	m_Timer.Start(m_fFlicker);
}

void CHUDDialogue::HideRecord(HRECORD hRec)
{
	Hide(g_pLTDatabase->GetRecordName(hRec));
}

void CHUDDialogue::Hide(const char* szIcon)
{
	if (LTStrIEquals(m_sIcon.c_str(),szIcon))
	{
		ResetFade();
		EnableFade(true);
		m_sIcon = "";
		
	}
}

float CHUDDialogue::GetFadeSpeed() const
{
	// We want to fade, even if persistent hud is enabled.
	const CUserProfile* const pProfile = g_pProfileMgr->GetCurrentProfile();
	return pProfile ? pProfile->m_fHUDFadeSpeed : 1.0f;
}

//called before during update loop to update the fade
void CHUDDialogue::UpdateFade()
{
	CHUDItem::UpdateFade();
	if (FadeLevelChanged())
	{
		float fFade = GetFadeLevel();
		if (fFade < 1.0f)
		{
			m_Name.SetColor(FadeARGB(m_cTextColor,fFade));
			DrawPrimSetRGBA(m_Insignia,FadeARGB(argbWhite,fFade));
		}
		else
		{
			m_Name.SetColor(m_cTextColor);
			DrawPrimSetRGBA(m_Insignia,argbWhite);
		}
	}
}

void CHUDDialogue::EndFlicker()
{
	CHUDItem::EndFlicker();
	float fFade = GetFadeLevel();
	if (fFade < 1.0f)
	{
		m_Name.SetColor(FadeARGB(m_cTextColor,fFade));
		DrawPrimSetRGBA(m_Insignia,FadeARGB(argbWhite,fFade));
	}
	else
	{
		m_Name.SetColor(m_cTextColor);
		DrawPrimSetRGBA(m_Insignia,argbWhite);
	}
}

void CHUDDialogue::UpdateFlicker()
{
	//override the default flicker if our timer is going
	float fFlickerLevel = g_pHUDMgr->GetFlickerLevel();
	float fFadeLevel = GetFadeLevel();
	if (fFlickerLevel > 0.0f)
	{
		CHUDItem::UpdateFlicker();
		if (m_fFlicker < 1.0f)
		{
			m_Name.SetColor(FadeARGB(m_cTextColor,m_fFlicker));
			DrawPrimSetRGBA(m_Insignia,FadeARGB(argbWhite,m_fFlicker));
		}
		else
		{
			m_Name.SetColor(m_cTextColor);
			DrawPrimSetRGBA(m_Insignia,argbWhite);
		}
	}
	else
	{
		if (fFadeLevel > 0.25f)
		{
			float fRange = fFadeLevel - 0.25f;
			m_fFlicker = fFadeLevel - (fRange * GetSinCycle(m_fFlickerFreq));
		}
		else
		{
			m_fFlicker = fFadeLevel;
		}

		if (m_fFlicker < 1.0f)
		{
			m_Text.SetColor(FadeARGB(m_cTextColor,m_fFlicker));
			m_Name.SetColor(FadeARGB(m_cTextColor,m_fFlicker));
			DrawPrimSetRGBA(m_IconPoly,FadeARGB(m_cIconColor,m_fFlicker));
			DrawPrimSetRGBA(m_Insignia,FadeARGB(argbWhite,m_fFlicker));
		}
		else
		{
			m_Text.SetColor(m_cTextColor);
			m_Name.SetColor(m_cTextColor);
			DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
			DrawPrimSetRGBA(m_Insignia,argbWhite);
		}
	}
}



void CHUDDialogue::ScaleChanged()
{
	float x = float(m_vBasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float y = float(m_vBasePos.y) * g_pInterfaceResMgr->GetYRatio();

	LTVector2 vPos;
	vPos.x = x + float(m_vNameOffset.x);
	vPos.y = y + float(m_vNameOffset.y);
	m_Name.SetPos(vPos);

	DrawPrimSetXYWH(m_Insignia,x+m_rfInsignia.Left(),y+m_rfInsignia.Top(),m_rfInsignia.GetWidth(),m_rfInsignia.GetHeight());


}
