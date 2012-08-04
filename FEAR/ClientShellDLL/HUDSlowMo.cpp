// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSlowMo.cpp
//
// PURPOSE : HUDItem to slow mo recharge
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDSlowMo.h"
#include "HUDMessageQueue.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "GameModeMgr.h"


//******************************************************************************************
//**
//** HUD SlowMo display
//**
//******************************************************************************************

CHUDSlowMo::CHUDSlowMo()
{
	m_UpdateFlags = kHUDFrame;
	m_bDraw = false;
	m_fDuration = 0.0f;
	m_fCurrentMax = 0.0f;
	m_fLastValue = 0.0f;
	m_hRechargeTexture = NULL;
}



bool CHUDSlowMo::Init()
{
	UpdateLayout();
	ScaleChanged();
	EnableFade(false);
	ResetFade();

	return true;
}

void CHUDSlowMo::Term()
{
}

void CHUDSlowMo::Render()
{
	if (!m_bDraw) return;
	g_pDrawPrim->SetTexture(m_hIconTexture);
	g_pDrawPrim->DrawPrim(&m_IconPoly,1);

	m_Bar.Render();

	if (IsMultiplayerGameClient() && (g_pPlayerMgr->GetSlowMoRecharge() > 0.0f))
	{
		g_pDrawPrim->SetTexture( m_hRechargeTexture );
		g_pDrawPrim->DrawPrim( &m_RechargePoly );
	}

}

void CHUDSlowMo::Update()
{

	if( !GameModeMgr::Instance( ).m_grbUseSlowMo || (g_pPlayerMgr->GetSlowMoMaxCharge() <= 0.0f) )
	{
		m_bDraw = false;
		return;
	}
	m_bDraw = true;

	float fValue = g_pPlayerMgr->GetSlowMoMaxCharge();
	if (fValue > m_fLastValue)
	{
		m_fCurrentMax = m_fLastValue;
		if (m_fLastValue > 0.0f)
		{
			g_pGameMsgs->AddMessage("HUD_SlowMoMax",kMsgDefault);
			Flash("SlowMoMax");
		}
	}
	m_fLastValue = fValue;
	if (fValue < m_fCurrentMax)
	{
		m_fCurrentMax = fValue;
	}


	if (g_pPlayerMgr->GetSlowMoCurCharge() > g_pPlayerMgr->GetSlowMoMinCharge())
	{
		if (!m_bFadeEnabled) 
		{
			EnableFade(true);
			m_cIconColor = argbWhite;
			m_cTextColor = m_cSavedTextColor;
			DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
			m_Bar.SetColor(m_cIconColor);
			ResetFade();
		}
	}
	else
	{
		if (m_bFadeEnabled) 
		{
			EnableFade(false);
			ResetFade();
			m_cIconColor = m_cDisabledColor;
			m_cTextColor = m_cDisabledColor;
			DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
			m_Bar.SetColor(m_cIconColor);
		}

	}

	UpdateBar();
}

void CHUDSlowMo::UpdateBar()
{
	float fPercent = 0.0f;
	if (g_pPlayerMgr) 
	{
		float x = float(m_vBasePos.x) * g_pInterfaceResMgr->GetXRatio();
		float y = float(m_vBasePos.y) * g_pInterfaceResMgr->GetYRatio();

		float fMax = (float)g_pPlayerMgr->GetSlowMoMaxCharge();
		float fCur = (float)g_pPlayerMgr->GetSlowMoCurCharge();
		if (fMax > m_fCurrentMax)
		{
			m_fCurrentMax = LTMIN(fMax, m_fCurrentMax + float(m_fGrowth * RealTimeTimer::Instance().GetTimerElapsedS()));
		}
		fCur = LTMIN(fCur,m_fCurrentMax);

		m_Bar.Update(x + m_rfBarRect.Left(),
					 y + m_rfBarRect.Top(),
					 fCur * m_fBarWidth,
					 m_fCurrentMax * m_fBarWidth,
					 m_rfBarRect.GetHeight());
		m_bDraw = true;
	}
	else
	{
		m_bDraw = false;		
	}

}

void CHUDSlowMo::ResetBar()
{
	m_fCurrentMax = (float)g_pPlayerMgr->GetSlowMoMaxCharge();

	if (g_pPlayerMgr->GetSlowMoCurCharge() > g_pPlayerMgr->GetSlowMoMinCharge())
	{
		EnableFade(true);
		m_cIconColor = argbWhite;
		m_cTextColor = m_cSavedTextColor;
		DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
		m_Bar.SetColor(m_cIconColor);
		ResetFade();
	}
	else
	{
		EnableFade(false);
		ResetFade();
		m_cIconColor = m_cDisabledColor;
		m_cTextColor = m_cDisabledColor;
		DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
		m_Bar.SetColor(m_cIconColor);
	}

	UpdateBar();

}


void CHUDSlowMo::OnExitWorld()
{
	m_fLastValue = 0.0f;
};



void CHUDSlowMo::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	float x = (float)(m_vBasePos.x * g_pInterfaceResMgr->GetXRatio()) + float(m_vRechargeBasePos.x);
	float y = (float)(m_vBasePos.y * g_pInterfaceResMgr->GetYRatio()) + float(m_vRechargeBasePos.y);
	DrawPrimSetXYWH( m_RechargePoly,   x,  y,   float(m_vRechargeSize.x),   float(m_vRechargeSize.y)   );

	UpdateBar();
}


void CHUDSlowMo::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDSlowMo2");
	}

	TextureReference hBar(g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,0));
	m_Bar.Init(hBar);

	m_rfBarRect = g_pLayoutDB->GetRectF(m_hLayout,LDB_HUDAddRect,0);
	m_cDisabledColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);
	m_cRechargeColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,1);
	m_fBarWidth = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,0);
	m_fGrowth = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,1);

	CHUDItem::UpdateLayout();

	m_cSavedTextColor = m_cTextColor;
	m_cTextColor = m_cDisabledColor;

	m_cIconColor = m_cDisabledColor;
	DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
	m_Bar.SetColor(m_cDisabledColor);

	InitAdditionalTextureData( m_hLayout, 0, m_hRechargeTexture,   m_vRechargeBasePos,   m_vRechargeSize,   m_RechargePoly   );
	DrawPrimSetRGBA(m_RechargePoly,m_cRechargeColor);

}


void CHUDSlowMo::UpdateFade()
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

void CHUDSlowMo::UpdateFlicker()
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

void CHUDSlowMo::EndFlicker()
{
	CHUDItem::EndFlicker();
	float	fFade = GetFadeLevel();
	m_Bar.SetAlpha(fFade);
}

void CHUDSlowMo::UpdateFlash()
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

