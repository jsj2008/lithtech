// ----------------------------------------------------------------------- //
//
// MODULE  : HUDArmor.cpp
//
// PURPOSE : Implementation of HUD Armor display
//
// CREATED : 01/26/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDArmor.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"

VarTrack	g_vtHUDArmorRender;

extern CGameClientShell* g_pGameClientShell;

//******************************************************************************************
//**
//** HUD Armor display
//**
//******************************************************************************************

CHUDArmor::CHUDArmor()
{
	m_UpdateFlags = kHUDArmor | kHUDHealth;
}

bool CHUDArmor::Init()
{
	g_vtHUDArmorRender.Init( g_pLTClient, "HUDArmorRender", NULL, 1.0f );

	m_nFadeThreshold = 0;
	m_nLastValue = 0;
	UpdateLayout();
	ScaleChanged();
	SetSourceString( LoadString("HUD_Number_Chars"));

	return true;
}

void CHUDArmor::Term()
{
}

void CHUDArmor::Render()
{
	if( g_vtHUDArmorRender.GetFloat( ) < 1.0f )
		return;

	m_Text.Render();
	g_pDrawPrim->SetTexture(m_hIconTexture);
	g_pDrawPrim->DrawPrim(&m_IconPoly,1);
	if (m_hBarTexture)
	{
		g_pDrawPrim->SetTexture(m_hBarTexture);
		g_pDrawPrim->DrawPrim(&m_BarPoly,1);
	}

}

void CHUDArmor::Update()
{
	uint8 nValue = g_pPlayerStats->GetArmor();
	if (nValue > m_nLastValue)
	{
		Flash("Pickup");
	}
	m_nLastValue = nValue;

	wchar_t szTmp[16] = L"";
	swprintf(szTmp,L"%d",nValue);
	m_Text.SetText(szTmp);

	ResetFade();
	EnableFade( (m_nFadeThreshold == 0) || g_pPlayerStats->GetArmor() > m_nFadeThreshold );

	if (!m_bFadeEnabled)
	{
		m_Text.SetColor(m_cTextColor);
		DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
	}
	UpdateBar();
}

void CHUDArmor::UpdateBar()
{
	float fPercent = 1.0f;
	if (g_pPlayerStats->GetMaxArmor() > 0)
	{
		fPercent = float(g_pPlayerStats->GetArmor()) / float(g_pPlayerStats->GetMaxArmor());
	}
	fPercent *= m_fBarWidth;
	SetupQuadUVs(m_BarPoly, m_hBarTexture, 0.0f, 0.0f, fPercent, 1.0f);

	float x = (float)(m_vBasePos.x * g_pInterfaceResMgr->GetXRatio()) + float(m_vIconOffset.x);
	float y = (float)(m_vBasePos.y * g_pInterfaceResMgr->GetYRatio()) + float(m_vIconOffset.y);

	float fPartialWidth = fPercent * float(m_vIconSize.x);
	DrawPrimSetXYWH(m_BarPoly,x,y, fPartialWidth ,float(m_vIconSize.y));
}

void CHUDArmor::ScaleChanged()
{
	CHUDItem::ScaleChanged();
	UpdateBar();
}


void CHUDArmor::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDArmor");
	}

	m_nFadeThreshold = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt);
	m_hBarTexture.Load(g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,0));
	m_fBarWidth = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,0);


	CHUDItem::UpdateLayout();


}


void CHUDArmor::UpdateFade()
{
	CHUDItem::UpdateFade();
	if (FadeLevelChanged())
	{
		float	fFade = GetFadeLevel();
		if (fFade < 1.0f)
		{
			DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
		}
		else
		{
			DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
		}
	}
}

void CHUDArmor::UpdateFlicker()
{
	CHUDItem::UpdateFlicker();
	if (m_fFlicker > 0.0f)
	{
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,m_fFlicker));
	}
	else
	{
		DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
	}
}

void CHUDArmor::UpdateFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.GetEngineTimer().IsValid() || !m_FlashTimer.IsStarted())
		return;

	CHUDItem::UpdateFlash();
	//still flashing, is the flash on or off?
	if (m_FlashTimer.IsStarted() && m_FlashTimer.GetTimeLeft() < (m_FlashTimer.GetDuration() / 2.0f))
	{
		DrawPrimSetRGBA(m_BarPoly,m_cFlashColor);
	}
	else
	{
		float fFade = GetFadeLevel();
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
	}
}


void CHUDArmor::EndFlicker()
{
	CHUDItem::EndFlicker();
	float	fFade = GetFadeLevel();
	if (fFade < 1.0f)
	{
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
	}
	else
	{
		DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
	}
}

