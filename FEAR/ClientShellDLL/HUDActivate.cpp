// ----------------------------------------------------------------------- //
//
// MODULE  : HUDActivate.cpp
//
// PURPOSE : HUD Item to display activation icon
//
// CREATED : 01/03/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDActivate.h"
#include "LadderMgr.h"
#include "TargetMgr.h"
#include "PlayerMgr.h"


//******************************************************************************************
//**
//** HUD Activate display
//**
//******************************************************************************************

CHUDActivate::CHUDActivate() :
	m_bDraw (false)
{
	EnableFade(false);
	m_UpdateFlags = kHUDFrame;
}

bool CHUDActivate::Init()
{
	UpdateLayout();
	ScaleChanged();
	return true;
}


void CHUDActivate::Render()
{
	if( !m_bDraw )
		return;

	g_pDrawPrim->SetTexture(m_hIconTexture);
	g_pDrawPrim->DrawPrim(&m_IconPoly,1);
}

void CHUDActivate::Update()
{
	m_bDraw = false;

	if (!g_pPlayerMgr->GetTargetMgr()->GetTargetObject())
		return;

	CActivationData data = g_pPlayerMgr->GetTargetMgr()->GetActivationData();

	// Don't draw anything if they don't have a target object and for surface sound activations.
	m_bDraw = (data.m_hTarget != NULL && data.m_nType != MID_ACTIVATE_SURFACESND );
}



void CHUDActivate::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDActivate");
	}

	CHUDItem::UpdateLayout();

}
