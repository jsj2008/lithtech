// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCarrying.cpp
//
// PURPOSE : HUDItem to display an icon while carrying a body
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2HUDMgr.h"
#include "TO2PlayerStats.h"
#include "TO2InterfaceMgr.h"
#include "TO2PlayerMgr.h"

//******************************************************************************************
//**
//** HUD Carry Icon display
//**
//******************************************************************************************

CHUDCarrying::CHUDCarrying()
{
	m_UpdateFlags = kHUDCarry | kHUDFrame;
	for (int i = 0; i < kNumCarryIcons; i++)
	{
		m_hIcon[i] = LTNULL;
	};
}


LTBOOL CHUDCarrying::Init()
{
	m_hIcon[eBodyCanDrop] = g_pInterfaceResMgr->GetTexture("interface\\hud\\carrying.dtx");
	m_hIcon[eBodyNoDrop] = g_pInterfaceResMgr->GetTexture("interface\\hud\\nodrop.dtx");
	m_hIcon[eBodyCanCarry] = g_pInterfaceResMgr->GetTexture("interface\\hud\\carry.dtx");

	m_hIcon[eDD_TransmitterCanDrop] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_transc.dtx");
	m_hIcon[eDD_TransmitterCanCarry] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_transp.dtx");

	m_hIcon[eDD_BatteryCanDrop] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_batc.dtx");
	m_hIcon[eDD_BatteryCanCarry] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_batp.dtx");

	m_hIcon[eDD_CoreCanDrop] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_corec.dtx");
	m_hIcon[eDD_CoreCanCarry] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_corep.dtx");

	g_pDrawPrim->SetRGBA(&m_Poly,argbWhite);
	SetupQuadUVs(m_Poly, m_hIcon[0], 0.0f,0.0f,1.0f,1.0f);

	UpdateLayout();

	return LTTRUE;
}

void CHUDCarrying::Term()
{

}

void CHUDCarrying::Render()
{
	uint8 nCarry = g_pPlayerMgr->GetCarryingObject();
	uint8 nCanCarry = g_pPlayerMgr->CanCarryObject();
	if (!nCarry && !nCanCarry) return;

	SetRenderState();

	g_pDrawPrim->SetTexture(NULL);

	if(nCanCarry)
	{
		switch (nCanCarry)
		{
		case CFX_CARRY_BODY:
			g_pDrawPrim->SetTexture(m_hIcon[eBodyCanCarry]);
			break;
		case CFX_CARRY_DD_CORE:
			g_pDrawPrim->SetTexture(m_hIcon[eDD_CoreCanCarry]);
			break;
		case CFX_CARRY_DD_TRAN:
			g_pDrawPrim->SetTexture(m_hIcon[eDD_TransmitterCanCarry]);
			break;
		case CFX_CARRY_DD_BATT:
			g_pDrawPrim->SetTexture(m_hIcon[eDD_BatteryCanCarry]);
			break;
		};
	}
	else if (g_pPlayerMgr->CanDropCarriedObject())
	{
		switch (nCarry)
		{
		case CFX_CARRY_BODY:
			g_pDrawPrim->SetTexture(m_hIcon[eBodyCanDrop]);
			break;
		case CFX_CARRY_DD_CORE:
			g_pDrawPrim->SetTexture(m_hIcon[eDD_CoreCanDrop]);
			break;
		case CFX_CARRY_DD_TRAN:
			g_pDrawPrim->SetTexture(m_hIcon[eDD_TransmitterCanDrop]);
			break;
		case CFX_CARRY_DD_BATT:
			g_pDrawPrim->SetTexture(m_hIcon[eDD_BatteryCanDrop]);
			break;
		};
	}
	else
	{
		g_pDrawPrim->SetTexture(m_hIcon[eBodyNoDrop]);
	}

	g_pDrawPrim->DrawPrim(&m_Poly);
}

void CHUDCarrying::Update()
{
	if (!g_pPlayerMgr->GetCarryingObject() && !g_pPlayerMgr->CanCarryObject()) return;

	float fx = (float)(m_BasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float fy = (float)(m_BasePos.y) * g_pInterfaceResMgr->GetXRatio();
	float fw = (float)(m_nSize) * g_pInterfaceResMgr->GetXRatio();

	g_pDrawPrim->SetXYWH(&m_Poly,fx,fy,fw,fw);

}

void CHUDCarrying::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos		= g_pLayoutMgr->GetCarryIconPos(nCurrentLayout);
	m_nSize			= g_pLayoutMgr->GetCarryIconSize(nCurrentLayout);

	if (!m_nSize)
	{
		m_BasePos		= LTIntPt(40,360);
		m_nSize			= 64;
	}

}



