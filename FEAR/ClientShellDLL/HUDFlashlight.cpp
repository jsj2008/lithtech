// ----------------------------------------------------------------------- //
//
// MODULE  : HUDFlashlight.cpp
//
// PURPOSE : HUDItem to show flashlight bar
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDFlashlight.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "GameModeMgr.h"
#include "FlashLight.h"


//******************************************************************************************
//**
//** HUD Flashlight display
//**
//******************************************************************************************

CHUDFlashlight::CHUDFlashlight()
{
	m_UpdateFlags = kHUDFrame;
	m_bDraw = false;
	m_fDuration = 0.0f;
	m_fLastValue = 0.0f;
}



bool CHUDFlashlight::Init()
{
	UpdateLayout();
	ScaleChanged();
	EnableFade(false);
	ResetFade();

	return true;
}

void CHUDFlashlight::Term()
{
}

void CHUDFlashlight::Render()
{
	if (!m_bDraw) return;
	g_pDrawPrim->SetTexture(m_hIconTexture);
	g_pDrawPrim->DrawPrim(&m_IconPoly,1);

	m_Bar.Render();

}

void CHUDFlashlight::Update()
{
	if( IsMultiplayerGameClient() || (g_pPlayerMgr->GetFlashlightMaxCharge() <= 0.0f) )
	{
		m_bDraw = false;
		return;
	}
	m_bDraw = true;


	if (g_pPlayerMgr->GetFlashLight()->IsOn())
	{
		if (m_bFadeEnabled) 
		{
			ResetFade();
			EnableFade(false);
		}
	}
	else
	{
		if (!m_bFadeEnabled) 
		{
			EnableFade(true);
			ResetFade();
		}
	}
	UpdateBar();
}

void CHUDFlashlight::UpdateBar()
{

	float fPercent = 0.0f;
	if (g_pPlayerMgr) 
	{
		float x = float(m_vBasePos.x) * g_pInterfaceResMgr->GetXRatio();
		float y = float(m_vBasePos.y) * g_pInterfaceResMgr->GetYRatio();

		float fMax = (float)g_pPlayerMgr->GetFlashlightMaxCharge();
		float fCur = (float)g_pPlayerMgr->GetFlashlightCharge();

		m_Bar.Update(x + m_rfBarRect.Left(),
					 y + m_rfBarRect.Top(),
					 fCur * m_fBarWidth,
					 fMax * m_fBarWidth,
					 m_rfBarRect.GetHeight());
		m_bDraw = true;
	}
	else
	{
		m_bDraw = false;		
	}

}

void CHUDFlashlight::ResetBar()
{
	UpdateBar();
}


void CHUDFlashlight::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	UpdateBar();
}


void CHUDFlashlight::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDFlashlight");
	}

	TextureReference hBar(g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,0));
	m_Bar.Init(hBar);

	m_rfBarRect = g_pLayoutDB->GetRectF(m_hLayout,LDB_HUDAddRect,0);
	m_fBarWidth = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,0);

	CHUDItem::UpdateLayout();

	m_Bar.SetColor(m_cIconColor);

}


void CHUDFlashlight::UpdateFade()
{
	CHUDItem::UpdateFade();
	if (FadeLevelChanged())
	{
		float fFade = GetFadeLevel();
		if (fFade < 1.0f)
		{
			m_Bar.SetAlpha(fFade);
		}
		else
		{
			m_Bar.SetAlpha(1.0f);
		}
	}
}

void CHUDFlashlight::UpdateFlicker()
{
	CHUDItem::UpdateFlicker();
	if (m_fFlicker > 0.0f)
	{
		m_Bar.SetAlpha(m_fFlicker);
	}
	else
	{
		m_Bar.SetAlpha(1.0f);
	}
}

void CHUDFlashlight::EndFlicker()
{
	CHUDItem::EndFlicker();
	float	fFade = GetFadeLevel();
	m_Bar.SetAlpha(fFade);
}

void CHUDFlashlight::UpdateFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.GetEngineTimer().IsValid() || !m_FlashTimer.IsStarted())
		return;

	CHUDItem::UpdateFlash();
	//still flashing, is the flash on or off?
	if (m_FlashTimer.IsStarted() && m_FlashTimer.GetTimeLeft() < (m_FlashTimer.GetDuration() / 2.0f))
	{
		m_Bar.SetColor(m_cFlashColor);
	}
	else
	{
		m_Bar.SetColor(m_cIconColor);
	}
}

