// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAir.cpp
//
// PURPOSE : HUDItem to display player air meter
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDAir.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"

//******************************************************************************************
//**
//** HUD Air display
//**
//******************************************************************************************

CHUDAir::CHUDAir()
{
	m_UpdateFlags = kHUDAir;
}


bool CHUDAir::Init()
{
	UpdateLayout();

	ScaleChanged();
	return true;
}

void CHUDAir::Term()
{
}

void CHUDAir::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	if (FadeLevelChanged())
	{
		float	fFade = GetFadeLevel();
		if (fFade < 1.0f)
		{
			m_Bar.SetAlpha(fFade);
		}
		else
		{
			m_Bar.SetAlpha(1.0f);
		}
	}

	m_Bar.Render();
	m_Text.Render();
	g_pDrawPrim->SetTexture(m_hIconTexture);
	g_pDrawPrim->DrawPrim(&m_IconPoly,1);

}

void CHUDAir::Update()
{
	float fPercent = g_pPlayerStats->GetAirPercent();

	m_bDraw = (fPercent < 1.0f);
	if (!m_bDraw) return;

	UpdateBar();

	wchar_t wszTmp[16] = L"";
	swprintf(wszTmp,L"%d",(int)fPercent);

	m_Text.SetText(wszTmp);


}

void CHUDAir::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDAir");
	}

	CHUDItem::UpdateLayout();

	m_rBar = g_pLayoutDB->GetRect(m_hLayout,LDB_HUDAddRect,0);

	m_Bar.Init(g_pInterfaceResMgr->GetTexture(g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,0)));

}

void CHUDAir::ScaleChanged()
{
	CHUDItem::ScaleChanged();
	UpdateBar();
}


void CHUDAir::UpdateBar()
{
	float fPercent = g_pPlayerStats->GetAirPercent();
	float x = (float)(m_vBasePos.x + m_rBar.Left()) * g_pInterfaceResMgr->GetXRatio();
	float y = (float)(m_vBasePos.y + m_rBar.Top()) * g_pInterfaceResMgr->GetYRatio();

	float maxW = float(m_rBar.GetWidth());
	float w = fPercent * maxW;
	float h = float(m_rBar.GetHeight());

	m_Bar.Update(x,y,w,maxW,h);
}

