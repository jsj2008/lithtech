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
#include "TO2HUDMgr.h"
#include "HUDRespawn.h"
#include "TO2PlayerStats.h"
#include "TO2InterfaceMgr.h"
#include "GameClientShell.h"


extern VarTrack g_vtMultiplayerRespawnWaitTime;
extern VarTrack g_vtDoomsdayRespawnWaitTime;

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
	m_MeterBasePos.x	= 320 - (uint16)((float)m_dwMaxValue * m_fMeterScale/2.0f);

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

	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_MeterBasePos.y	= g_pLayoutMgr->GetInt(s_aTagName,"BarPosY");
	m_nMeterHeight		= g_pLayoutMgr->GetInt(s_aTagName,"BarHeight");
	m_fMeterScale		= g_pLayoutMgr->GetFloat(s_aTagName,"BarScale");
	
	g_pLayoutMgr->GetString(s_aTagName,"BarTexture", m_szMeterTex, ARRAY_LEN( m_szMeterTex ));

	
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
	m_bDraw = LTFALSE;
	m_fDuration = 0.0f;
	m_pString = NULL;
	m_bReady = false;
	m_bCancelRevive = false;
}


LTBOOL CHUDRespawn::Init()
{
	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_StrPos = LTIntPt(320,320);
	m_nStrSz = 12;
	m_StrColor = argbWhite;
	m_StrDisColor = 0x80808080;

	m_pString = g_pFontManager->CreateFormattedPolyString(pFont,LoadTempString(IDS_RESPAWN),0.0f,0.0f);
	m_pString->SetColor(m_StrColor);
	m_pString->SetAlignmentH(CUI_HALIGN_CENTER);

	UpdateLayout();

	// Init our bar...
	
	m_RespawnBar.Init();
	m_RespawnBar.SetMaxValue( kMaxValue );

	return LTTRUE;
}

void CHUDRespawn::Term()
{
 	if (m_pString)
	{
		g_pFontManager->DestroyPolyString(m_pString);
        m_pString=LTNULL;
	}
}

void CHUDRespawn::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	uint8 nVal = kMaxValue;
	float fTimeLeft = g_pPlayerMgr->GetRespawnTime() - g_pLTClient->GetTime();
	if( fTimeLeft > 0.0f)
	{
		nVal = uint8( (1.0f - (fTimeLeft / m_fDuration)) * kMaxValue );

		// If this game allows the player to be revived, check if they cancelled their
		// revive.  If they did, then change to disabled respawn.
		if( IsRevivePlayerGameType( ))
		{
			if( !m_bCancelRevive )
			{
				if( g_pPlayerMgr->GetCancelRevive( ))
				{
					m_bCancelRevive = true;
					m_pString->SetText( LoadTempString( IDS_RESPAWN ));
					m_pString->SetColor(m_StrDisColor);
					g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
				}
			}
		}
	}
	else
	{
		if (!m_bReady)
		{
			m_bReady = true;
			m_pString->SetText( LoadTempString( IDS_RESPAWN ));
			m_pString->SetColor(m_StrColor);
			g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\pressanykey.wav");
		}
	}


	m_pString->Render();	

	
	m_RespawnBar.SetValue( nVal );
	m_RespawnBar.Update();
	m_RespawnBar.Render();

}

void CHUDRespawn::Update()
{
	m_bDraw = IsMultiplayerGame() && g_pPlayerMgr->IsPlayerDead();
	if (!m_bDraw)
	{
		m_fDuration = -1.0f;
		return;
	}
	
	float x = (float)m_StrPos.x * g_pInterfaceResMgr->GetXRatio();
	float y = (float)m_StrPos.y * g_pInterfaceResMgr->GetYRatio();
	uint8 nTextSize = (uint8)((float)m_nStrSz * g_pInterfaceResMgr->GetYRatio());
	m_pString->SetCharScreenHeight(nTextSize);

	m_pString->SetPosition(x,y);

	if (m_fDuration < 0.0f)
	{
		m_bReady = false;
		m_bCancelRevive = false;
		if (IsCoopMultiplayerGameType( ))
		{
			m_fDuration = g_vtMultiplayerRespawnWaitTime.GetFloat();
		}
		else
		{
			m_fDuration = g_vtDoomsdayRespawnWaitTime.GetFloat();
		}

		// If the player can be revived, then allow them to cancel revive.
		if( IsRevivePlayerGameType( ))
		{
			m_pString->SetText( LoadTempString( IDS_CANCELREVIVE ));
			m_pString->SetColor(m_StrColor);
		}
		else
		{
			m_pString->SetText( LoadTempString( IDS_RESPAWN ));
			m_pString->SetColor(m_StrDisColor);
		}

		m_RespawnBar.SetValue(0);
	}
	m_RespawnBar.Update();
}

void CHUDRespawn::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	int pos = g_pLayoutMgr->GetInt(s_aTagName,"TextPosY");
	if (pos > 0)
		m_StrPos.y = pos;

	uint8 nTmp = (uint8)g_pLayoutMgr->GetInt(s_aTagName,"TextSize");
	if (nTmp > 0)
		m_nStrSz = nTmp;


	{ //set string color
		float fAlpha = g_pLayoutMgr->GetFloat(s_aTagName, "TextAlpha");
		uint8 nA = (uint8)(255.0f * fAlpha);

		LTVector vColor = g_pLayoutMgr->GetVector(s_aTagName, "TextColor");

		uint8 nR = (uint8)vColor.x;
		uint8 nG = (uint8)vColor.y;
		uint8 nB = (uint8)vColor.z;

		uint32 color = SET_ARGB(nA,nR,nG,nB);
		if (color > 0)
			m_StrColor = color;
	}

	{ //set string disabled color
		float fAlpha = g_pLayoutMgr->GetFloat(s_aTagName, "TextDisAlpha");
		uint8 nA = (uint8)(255.0f * fAlpha);

		LTVector vColor = g_pLayoutMgr->GetVector(s_aTagName, "TextDisColor");

		uint8 nR = (uint8)vColor.x;
		uint8 nG = (uint8)vColor.y;
		uint8 nB = (uint8)vColor.z;

		uint32 color = SET_ARGB(nA,nR,nG,nB);
		if (color > 0)
			m_StrDisColor = color;
	}

	m_RespawnBar.UpdateLayout();
}

