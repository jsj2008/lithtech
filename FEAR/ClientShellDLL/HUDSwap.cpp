// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSwap.cpp
//
// PURPOSE : HUDItem to display weapon swap info
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "ClientWeaponMgr.h"
#include "PlayerMgr.h"
#include "CommandIDs.h"
#include "WeaponDB.h"
#include "HUDSwap.h"
#include "sys/win/mpstrconv.h"
#include "PickupItemFX.h"
#include "ProjectileFX.h"
#include "CTFFlagSFX.h"


std::wstring CHUDSwap::m_wsTrigger;

class CTFFlagSFX;

//******************************************************************************************
//**
//** HUD Ammo display
//**
//******************************************************************************************

CHUDSwap::CHUDSwap()
{
	m_UpdateFlags = kHUDSwap;
	m_bDraw = false;
	m_bShowIcon = false;
}

bool CHUDSwap::Init()
{
	UpdateLayout();
	ScaleChanged();

	return true;
}

void CHUDSwap::Term()
{
}

void CHUDSwap::Render()
{
	if (!m_bDraw) return;

	SetRenderState();


	m_Text.Render();

	//render icon here
	if (m_hIconTexture && m_bShowIcon)
	{
		g_pDrawPrim->SetTexture(m_hIconTexture);
		SetupQuadUVs(m_IconPoly, m_hIconTexture, 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->DrawPrim(&m_IconPoly,1);
	}
}

void CHUDSwap::Update()
{
	m_bDraw = false;

	// Get the item we're inspecting.
	HOBJECT hObject = g_pPlayerMgr->GetPickupObjectDetector().GetObject( );
	if( !hObject)
		return;

	wchar_t wsName[64] = L"";

	bool bFound = false;
	bool bSwap = false;

	// See if it's a pickupitem.
	CPickupItemFX* pPickupItemFX = static_cast< CPickupItemFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_PICKUPITEM_ID, hObject ));
	if( pPickupItemFX )
	{

		// Check if we should show text for this item.
		if( !pPickupItemFX->IsMustSwap( ) && !pPickupItemFX->CanPickup())
		{
			return;
		}

		// Get the name of the item.
		if( !pPickupItemFX->GetName( wsName, LTARRAYSIZE( wsName )))
			return;

		bFound = true;
		bSwap = pPickupItemFX->IsMustSwap( );

		m_bShowIcon = true;
		if	(hObject != m_hLastIconTarget)
		{
			m_hLastIconTarget = hObject;
			m_hIconTexture.Load( pPickupItemFX->GetIcon( ));
		}
	}

	// Check if not found yet.
	if( !bFound )
	{
		// See if it's a ctfflag.
		CTFFlagSFX* pCTFFlagSFX = reinterpret_cast< CTFFlagSFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_CTFFLAG_ID, hObject ));
		if( pCTFFlagSFX )
		{
			if( pCTFFlagSFX->CanGrabFlag( ))
			{
				bFound = true;
				LTStrCpy( wsName, LoadString( "CTF_FlagName" ), LTARRAYSIZE( wsName ));
				m_bShowIcon = false;
			}
		}
	}

	// Is it a recoverable projectile?
	if( !bFound )
	{
		// See if it's a projectile
		CProjectileFX* pProjSFX = reinterpret_cast< CProjectileFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_PROJECTILE_ID, hObject ));
		if( pProjSFX )
		{
			bFound = pProjSFX->IsRecoverable();
			if (bFound)
			{
				// Get the name of the item.
				if( !pProjSFX->GetName( wsName, LTARRAYSIZE( wsName )))
					return;

				m_bShowIcon = true;
				if	(hObject != m_hLastIconTarget)
				{
					m_hLastIconTarget = hObject;
					m_hIconTexture.Load( pProjSFX->GetIcon( ));
				}
			}
		}
	}


	if( !bFound )
	{
		return;
	}

	m_bDraw = true;

	if (!m_wsTrigger.size())
		UpdateTriggerName();

	wchar_t wsStr[128];

	if (m_wsTrigger.size())
	{
		if (bSwap)
		{
			FormatString("HUD_SwapWeapon",wsStr,LTARRAYSIZE(wsStr),m_wsTrigger.c_str(),wsName);
		}
		else
		{
			FormatString("HUD_PickupItem",wsStr,LTARRAYSIZE(wsStr),m_wsTrigger.c_str(),wsName);
		}
		
	}
	else
	{
		LTStrCpy(wsStr,wsName,LTARRAYSIZE(wsStr));
	}

	if (!LTStrIEquals(wsStr,m_Text.GetText()))
	{
		m_Text.SetText(wsStr);
	}

	if (bSwap)
	{
		m_cTextColor = m_cSwapColor;
		m_cIconColor = m_cSwapColor;
	}
	else
	{
		m_cTextColor = m_cSavedTextColor;
		m_cIconColor = m_cSavedIconColor;
	}
	m_Text.SetColor(m_cTextColor);
	DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
}

void CHUDSwap::ScaleChanged()
{
	CHUDItem::ScaleChanged();
}

void CHUDSwap::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDSwap");
	}

	CHUDItem::UpdateLayout();

	m_cSwapColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);
	m_cSavedTextColor = m_cTextColor;
	m_cSavedIconColor = m_cIconColor;


}


void CHUDSwap::UpdateTriggerName()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	m_wsTrigger = pProfile->GetTriggerNameFromCommandID(COMMAND_ID_ACTIVATE);
	g_pHUDMgr->QueueUpdate(kHUDSwap);
}


