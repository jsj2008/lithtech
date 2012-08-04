// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHealth.cpp
//
// PURPOSE : Implementation of HUD Health display
//
// CREATED : 01/26/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDHealth.h"
#include "HUDMessageQueue.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "sys/win/mpstrconv.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack g_vtHUDHealthRender;

//******************************************************************************************
//**
//** HUD Health display
//**
//******************************************************************************************

CHUDHealth::CHUDHealth()
{
	m_UpdateFlags = kHUDArmor | kHUDHealth;
}

bool CHUDHealth::Init()
{
	g_vtHUDHealthRender.Init( g_pLTClient, "HUDHealthRender", NULL, 1.0f );

	m_nFadeThreshold = 0;
	m_nLastValue = 0;
	m_nLastMaxValue = 0;
	UpdateLayout();
	ScaleChanged();

	SetSourceString( LoadString("HUD_Number_Chars"));


	return true;
}

void CHUDHealth::Term()
{
}

void CHUDHealth::Render()
{
	if( g_vtHUDHealthRender.GetFloat( ) < 1.0f )
		return;

	SetRenderState();

	m_Text.Render();
	g_pDrawPrim->SetTexture(m_hIconTexture);
	g_pDrawPrim->DrawPrim(&m_IconPoly,1);
	if (m_hBarTexture)
	{
		g_pDrawPrim->SetTexture(m_hBarTexture);
		g_pDrawPrim->DrawPrim(&m_BarPoly,1);
	}

	if (g_pGameClientShell->GetDifficulty() == GD_EASY 
		&& (g_pPlayerStats->GetHealth() < m_nFadeThreshold)
		)
	{
		m_HelpText.Render();
	}


}

void CHUDHealth::Reset()
{
	m_cTextColor = m_cNormalTextColor;
	m_cIconColor = m_cNormalIconColor;
	CHUDItem::Reset();

}
void CHUDHealth::Update()
{
	uint8 nValue = g_pPlayerStats->GetHealth();
	uint8 nMaxValue = g_pPlayerStats->GetMaxHealth();

	if (nValue < m_nFlashThreshold)
	{
		Flash("LowHealth");
	}
	else if (m_nLastMaxValue > 0 && nMaxValue > m_nLastMaxValue)
	{
		g_pGameMsgs->AddMessage("HUD_HealthMax",kMsgDefault);
		Flash("HealthMax");
	}
	else if (m_nLastValue > 0 && nValue > m_nLastValue && !IsFlashing())
	{
		Flash("Pickup");
	}

	m_nLastValue = nValue;
	m_nLastMaxValue = nMaxValue;

	wchar_t szTmp[16] = L"";
	swprintf(szTmp,L"%d",nValue);
	m_Text.SetText(szTmp);

	ResetFade();
	EnableFade( (m_nFadeThreshold == 0) || g_pPlayerStats->GetHealth() > m_nFadeThreshold );

	if (g_pPlayerStats->GetHealth() >= m_nFadeThreshold)
	{
		m_cTextColor = m_cNormalTextColor;
		m_cIconColor = m_cNormalIconColor;
	}
	else
	{
		m_cTextColor = m_cThresholdColor;
		m_cIconColor = m_cThresholdColor;
	}

	if (!m_bFadeEnabled)
	{
		m_Text.SetColor(m_cTextColor); 
		m_HelpText.SetColor(m_cTextColor);
		DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
	}

	UpdateBar();

	m_HelpText.SetText(CreateHelpString("HUD_Health_Hint"));


}

void CHUDHealth::OnExitWorld()
{
	m_nLastValue = 0;
	m_nLastMaxValue = 0;
};

void CHUDHealth::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDHealth");
	}

	m_nFadeThreshold = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);
	m_cThresholdColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);

	m_nFlashThreshold = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,1);

	m_hBarTexture.Load(g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,0));
	m_fBarWidth = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,0);

	m_vHelpOffset = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,0);

	CHUDItem::UpdateLayout();

	m_cNormalTextColor = m_cTextColor;
	m_cNormalIconColor = m_cIconColor;

	CFontInfo Font(g_pLayoutDB->GetHelpFont(),g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,2));
	m_HelpText.SetFont(Font);
	m_HelpText.SetColor(m_cTextColor);
	m_HelpText.SetAlignment(kLeft);

}

void CHUDHealth::UpdateFade()
{
	CHUDItem::UpdateFade();
	if (FadeLevelChanged())
	{
		float	fFade = GetFadeLevel();
		if (fFade < 1.0f)
		{
			m_HelpText.SetColor(FadeARGB(m_cTextColor,fFade));
			DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
		}
		else
		{
			m_HelpText.SetColor(m_cTextColor);
			DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
		}
	}
}

void CHUDHealth::UpdateFlicker()
{
	CHUDItem::UpdateFlicker();
	if (m_fFlicker > 0.0f)
	{
		m_HelpText.SetColor(FadeARGB(m_cTextColor,m_fFlicker));
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,m_fFlicker));
	}
	else
	{
		m_HelpText.SetColor(m_cTextColor);
		DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
	}
}

void CHUDHealth::UpdateFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.GetEngineTimer().IsValid() || !m_FlashTimer.IsStarted())
		return;

	if (g_pPlayerStats->GetHealth() < m_nFlashThreshold && m_nFlashCount < 2)
	{
		m_nFlashCount = 2;
	}
		

	CHUDItem::UpdateFlash();
	//still flashing, is the flash on or off?
	if (m_FlashTimer.IsStarted() && m_FlashTimer.GetTimeLeft() < (m_FlashTimer.GetDuration() / 2.0f))
	{
		DrawPrimSetRGBA(m_BarPoly,m_cFlashColor);
		m_HelpText.SetColor(m_cFlashColor);
	}
	else
	{
		float fFade = GetFadeLevel();
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
		m_HelpText.SetColor(FadeARGB(m_cTextColor,fFade));
	}
}



void CHUDHealth::EndFlicker()
{
	CHUDItem::EndFlicker();
	float	fFade = GetFadeLevel();
	if (fFade < 1.0f)
	{
		m_HelpText.SetColor(FadeARGB(m_cTextColor,fFade));
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
	}
	else
	{
		m_HelpText.SetColor(m_cTextColor);
		DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
	}
}

void CHUDHealth::UpdateBar()
{
	float fPercent = 1.0f;
	if (g_pPlayerStats->GetMaxHealth() > 0)
	{
		fPercent = float(g_pPlayerStats->GetHealth()) / float(g_pPlayerStats->GetMaxHealth());
	}
	fPercent *= m_fBarWidth;
	SetupQuadUVs(m_BarPoly, m_hBarTexture, 0.0f, 0.0f, fPercent, 1.0f);

	float x = (float)(m_vBasePos.x * g_pInterfaceResMgr->GetXRatio()) + float(m_vIconOffset.x);
	float y = (float)(m_vBasePos.y * g_pInterfaceResMgr->GetYRatio()) + float(m_vIconOffset.y);

	float fPartialWidth = fPercent * float(m_vIconSize.x);
	DrawPrimSetXYWH(m_BarPoly,x,y, fPartialWidth ,float(m_vIconSize.y));
}

void CHUDHealth::ScaleChanged()
{
	CHUDItem::ScaleChanged();
	UpdateBar();

	float x = float(m_vBasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float y = float(m_vBasePos.y) * g_pInterfaceResMgr->GetYRatio();

	LTVector2 vPos;
	vPos.x = x + float(m_vHelpOffset.x);
	vPos.y = y + float(m_vHelpOffset.y);
	m_HelpText.SetPos(vPos);

}
