// ----------------------------------------------------------------------- //
//
// MODULE  : HUDRespawn.cpp
//
// PURPOSE : HUDItem to display hiding icon
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDRespawn.h"
#include "PlayerStats.h"
#include "PlayerMgr.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "GameModeMgr.h"


static char s_aTagName[30] = "HUDRespawn";
static const int kMaxValue = 200;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRespawnBar::CHUDRespawnBar
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDRespawnBar::CHUDRespawnBar()
:	CHUDMeter	()
{
	m_UpdateFlags = kHUDNone;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRespawnBar::Update
//
//  PURPOSE:	Update the values to display the bar...
//
// ----------------------------------------------------------------------- //

void CHUDRespawnBar::Update()
{
	CHUDMeter::Update();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRespawnBar::UpdateLayout
//
//  PURPOSE:	Get the values for displaying the bar...
//
// ----------------------------------------------------------------------- //

void CHUDRespawnBar::UpdateLayout()
{

	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDRespawn");
	}

	CHUDItem::UpdateLayout();

	LTSNPrintF(m_szMeterTex,LTARRAYSIZE(m_szMeterTex),"%s",g_pLayoutDB->GetString(m_hLayout,"IconTexture"));

		

	m_bCentered = true;

}


//******************************************************************************************
//**
//** HUD Respawn display
//**
//******************************************************************************************

CHUDRespawn::CHUDRespawn()
{
	m_UpdateFlags = kHUDRespawn;
	m_eLevel = kHUDRenderText;
	m_bDraw = false;
	m_fDuration = 0.0f;
	m_Text.SetDropShadow(2);
	m_TimerText.SetDropShadow(2);
	m_PenaltyText.SetDropShadow(2);
	m_hTimerSourceString = NULL;

}


bool CHUDRespawn::Init()
{

	UpdateLayout();

	if (m_hTimerSourceString)
	{
		LTRESULT res = g_pTextureString->RecreateTextureString(m_hTimerSourceString,LoadString("HUD_Timer_Chars"),m_sTextFont);
		if (res != LT_OK)
		{
			g_pTextureString->ReleaseTextureString(m_hTimerSourceString);
			m_hSourceString = NULL;
		}
	}
	else
	{
		m_hTimerSourceString = g_pTextureString->CreateTextureString(LoadString("HUD_Timer_Chars"),m_sTextFont);
	}
	m_TimerText.SetSourceString(m_hTimerSourceString);



	// Init our bar...
	
	m_RespawnBar.Init();
	m_RespawnBar.SetMaxValue( kMaxValue );

	ScaleChanged();

	return true;
}

void CHUDRespawn::Term()
{
	m_Text.FlushTexture();
	m_TimerText.FlushTexture();
	m_PenaltyText.FlushTexture();
}

void CHUDRespawn::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	uint8 nVal = kMaxValue;
	if( !g_pPlayerMgr->GetRespawnTime( ).IsTimedOut( ))
	{
		nVal = uint8( (1.0f - (g_pPlayerMgr->GetRespawnTime( ).GetTimeLeft( ) / m_fDuration)) * kMaxValue );
		//		Update range text

		wchar_t szTmp[16];
		LTSNPrintF(szTmp,LTARRAYSIZE(szTmp),L"%d",uint32(g_pPlayerMgr->GetRespawnTime( ).GetTimeLeft( )+0.99f));
		if (!LTStrIEquals(szTmp,m_TimerText.GetText()) )
			m_TimerText.SetText(szTmp);
		m_TimerText.Render();
		if (g_pPlayerMgr->GetRespawnPenalty())
		{
			m_PenaltyText.Render();
		}

	}


	m_Text.Render();

	
	m_RespawnBar.SetValue( nVal );
	m_RespawnBar.Update();
	m_RespawnBar.Render();

}

void CHUDRespawn::Update()
{
	m_bDraw = IsMultiplayerGameClient() && g_pPlayerMgr->GetRespawnTime().IsStarted() && GameModeMgr::Instance().m_grbAllowRespawnFromDeath;
	if (!m_bDraw)
	{
		m_fDuration = -1.0f;
		return;
	}

	m_fDuration = (float)g_pPlayerMgr->GetRespawnTime( ).GetDuration();
	if (GameModeMgr::Instance().m_grbUseRespawnWaves)
	{
		m_Text.SetText( CreateHelpString("IDS_RESPAWN_WAVE"));
		m_Text.SetColor(m_cTextColor);
	}
	else if (g_pPlayerMgr->WillRespawnOnTimeOut())
	{
		m_Text.SetText( LoadString( "IDS_RESPAWN" ));
		m_Text.SetColor(m_cDisabledColor);
	}
	else
	{
		m_Text.SetText( CreateHelpString("IDS_RESPAWN_FIRE"));
		m_Text.SetColor(m_cTextColor);
	}
	
	if (g_pPlayerMgr->GetRespawnPenalty() > 0.0f)
	{
		wchar_t wsTmp[64] = L"";
		FormatString("HUD_Respawn_TKPenalty",wsTmp,LTARRAYSIZE(wsTmp),(uint32)g_pPlayerMgr->GetRespawnPenalty());
		m_PenaltyText.SetText( wsTmp);
	}

	m_RespawnBar.Update();
}

void CHUDRespawn::ScaleChanged()
{
	CHUDItem::ScaleChanged();
	m_RespawnBar.ScaleChanged();

	float x = float(m_vBasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float y = float(m_vBasePos.y) * g_pInterfaceResMgr->GetYRatio();

	LTVector2 vPos;
	vPos.x = x + float(m_vTimerOffset.x);
	vPos.y = y + float(m_vTimerOffset.y);
	m_TimerText.SetPos(vPos);

	vPos.x = x + float(m_vPenaltyOffset.x);
	vPos.y = y + float(m_vPenaltyOffset.y);
	m_PenaltyText.SetPos(vPos);


}

void CHUDRespawn::UpdateLayout()
{

	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDRespawn");
	}

	CHUDItem::UpdateLayout();

	m_cDisabledColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);
	m_cPenaltyColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,1);
	m_vTimerOffset = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,0);
	m_vPenaltyOffset = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,1);
	
	std::string sFont = g_pLayoutDB->GetFont(m_hLayout,"Font");
	if (sFont.empty())
		sFont = g_pLayoutDB->GetHUDFont();
	uint32 nTextSize = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);
	m_sTimerFont = CFontInfo(sFont.c_str(),nTextSize);

	nTextSize = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,1);
	m_sPenaltyFont = CFontInfo(sFont.c_str(),nTextSize);

	m_TimerText.SetFont(m_sTimerFont);
	m_TimerText.SetColor(m_cTextColor);
	m_TimerText.SetAlignment(kCenter);

	m_PenaltyText.SetFont(m_sPenaltyFont);
	m_PenaltyText.SetColor(m_cPenaltyColor);
	m_PenaltyText.SetAlignment(kCenter);

	m_RespawnBar.UpdateLayout();
}

