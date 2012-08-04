// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponChooser.cpp
//
// PURPOSE : In-game popup for choosing weapons
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponChooser.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "WinUtil.h"
#include "SoundMgr.h"
#include "LayoutMgr.h"
#include "ClientWeaponMgr.h"

extern CGameClientShell* g_pGameClientShell;


namespace
{
	const int kLastAmmo = 1;
	const int kCurrWeapon = 1;
	const int kCurrAmmo = 0;
	const float kfDelayTime	= 1.5f;
	VarTrack	g_vtChooserAutoSwitchTime;
	VarTrack	g_vtChooserAutoSwitchFreq;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWeaponChooser::CWeaponChooser()
{
	m_hWeapon = NULL;
	m_nClass = 0;
    m_bIsOpen = false;
}

CWeaponChooser::~CWeaponChooser()
{
}

void CWeaponChooser::Init()
{
	g_vtChooserAutoSwitchTime.Init(g_pLTClient, "ChooserAutoSwitchTime", NULL, 0.175f);
	g_vtChooserAutoSwitchFreq.Init(g_pLTClient, "ChooserAutoSwitchFreq", NULL, 0.1f);

	m_NextWeaponKeyDownTimer.SetEngineTimer( RealTimeTimer::Instance( ));
	m_PrevWeaponKeyDownTimer.SetEngineTimer( RealTimeTimer::Instance( ));
	m_AutoSwitchTimer.SetEngineTimer( RealTimeTimer::Instance( ));
	m_AutoCloseTimer.SetEngineTimer( RealTimeTimer::Instance( ));

}

void CWeaponChooser::Term()
{
	if (m_bIsOpen)
		Close();
}

bool CWeaponChooser::Open(uint8 nClass)
{
	if (m_bIsOpen && nClass == m_nClass)
		return true;

	CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	CClientWeapon const *pClientWeapon = pClientWeaponMgr->GetCurrentClientWeapon();
	if( !pClientWeapon )
		return false;

	if (!m_bIsOpen)
		m_hWeapon = pClientWeapon->GetWeaponRecord();


	HWEAPON hWeapon = m_hWeapon;
	while( true )
	{
		// Get the next weapon.
		hWeapon = pClientWeaponMgr->GetNextWeaponRecord( hWeapon, nClass );

		// If we couldn't find a weapon, we can't go to a next one.
		if( !hWeapon )
		{
			// If we didn't find a weapon and we're staying within our
			// current weapon's class, then just open the chooser.
			if( g_pWeaponDB->GetWeaponClass( m_hWeapon ) != nClass )
				break;

			if (!m_bIsOpen)
				m_hWeapon = NULL;
			g_pHUDMgr->QueueUpdate(kHUDChooser);
			return false;
		}

		if( g_pWeaponDB->GetBool( hWeapon, WDB_WEAPON_bShowChooser ))
		{
			break;
		}
	}

	m_bIsOpen = true;
	m_nClass = nClass;

	m_AutoCloseTimer.Start(kfDelayTime);
	g_pHUDMgr->QueueUpdate(kHUDChooser);
    return true;
}

void CWeaponChooser::Close()
{
	if (!m_bIsOpen)
		return;

    m_bIsOpen = false;
	m_nClass = 0;
	m_AutoCloseTimer.Stop();
	m_AutoSwitchTimer.Stop();
	m_NextWeaponKeyDownTimer.Stop();
	m_PrevWeaponKeyDownTimer.Stop();

	if (g_pHUDMgr)
		g_pHUDMgr->QueueUpdate(kHUDChooser);

}

void CWeaponChooser::NextWeapon(uint8 nClass)
{
	if( !g_pWeaponDB )
		return;

	// Invalid class will just go to the next weapon and ignore classes...

	if( nClass > g_pWeaponDB->GetNumValues( g_pWeaponDB->GetWeaponOrderRecord(), WDB_WEAPONORDER_nEndOfClass ))
		nClass = WDB_INVALID_WEAPONCLASS;

	HWEAPON hWeapon = m_hWeapon;
	while( true )
	{
		// get the next avail weapon
		hWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetNextWeaponRecord( hWeapon, nClass );

		// If we couldn't find a weapon, or we wrapped around, we can't find a next one.
		if( !hWeapon || hWeapon == m_hWeapon )
			return;

		if( g_pWeaponDB->GetBool( hWeapon, WDB_WEAPON_bShowChooser ))
		{
			m_hWeapon = hWeapon;
			break;
		}
	}

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

   	m_AutoCloseTimer.Start(kfDelayTime);
	m_NextWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	g_pHUDMgr->QueueUpdate(kHUDChooser);

}

void CWeaponChooser::PrevWeapon()
{
	HWEAPON hWeapon = m_hWeapon;
	while( true )
	{
		// get the next avail weapon
		hWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetPrevWeaponRecord( hWeapon, 0 );

		// If we couldn't find a weapon, or we wrapped around, we can't find a next one.
		if( !hWeapon || hWeapon == m_hWeapon )
			return;

		if( g_pWeaponDB->GetBool( hWeapon, WDB_WEAPON_bShowChooser ))
		{
			m_hWeapon = hWeapon;
			break;
		}
	}

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

   	m_AutoCloseTimer.Start(kfDelayTime);
	m_PrevWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	g_pHUDMgr->QueueUpdate(kHUDChooser);
}

void CWeaponChooser::EndAutoSwitch(bool bNextWeaponKey)
{
	if (bNextWeaponKey)
		m_NextWeaponKeyDownTimer.Stop();
	else
		m_PrevWeaponKeyDownTimer.Stop();
	m_AutoSwitchTimer.Stop();
}

void CWeaponChooser::Update()
{
	// If Weapon chooser is being drawn, see if we want to change weapons...
	if (!IsOpen()) return;


	// See if we should close ourselves...
	if (m_AutoCloseTimer.IsStarted() && m_AutoCloseTimer.IsTimedOut())
	{
        g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());
		Close();
	}
	else if (m_NextWeaponKeyDownTimer.IsStarted() && m_NextWeaponKeyDownTimer.IsTimedOut())
	{
		// See if we should switch to the next weapon...
		if (m_AutoSwitchTimer.IsStarted())
		{
			if (m_AutoSwitchTimer.IsTimedOut())
			{
				NextWeapon(-1);
				g_pClientWeaponMgr->ChangeWeapon( GetCurrentSelection() );
				m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
			}
		}
		else
		{
			m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
		}
	}
	else if (m_PrevWeaponKeyDownTimer.IsStarted() && m_PrevWeaponKeyDownTimer.IsTimedOut())
	{
		if (m_AutoSwitchTimer.IsTimedOut())
		{
			PrevWeapon();
			g_pClientWeaponMgr->ChangeWeapon( GetCurrentSelection());
			m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
		}
	}

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAmmoChooser::CAmmoChooser()
{
	m_hAmmo = NULL;
    m_bIsOpen = false;
	
}

CAmmoChooser::~CAmmoChooser()
{
}

void CAmmoChooser::Init()
{

}


void CAmmoChooser::Term()
{
	if (m_bIsOpen)
		Close();
}

bool CAmmoChooser::Open()
{
	// Don't allow the chooser to be opened if we're selecting/deselecting a
	// weapon...

	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();

	WeaponState eState = pClientWeapon->GetState();
	if (W_DESELECT == eState || W_SELECT == eState) return false;


	if (m_bIsOpen)
        return true;


	m_hAmmo = pClientWeapon->GetAmmoRecord();

	if( m_hAmmo == pClientWeapon->GetNextAvailableAmmo() )
	{
		m_hAmmo = NULL;
        m_bIsOpen = false;
        return false;
	}
    m_bIsOpen = true;

	m_AutoCloseTimer.Start(kfDelayTime);
	g_pHUDMgr->QueueUpdate(kHUDChooser);
    return true;
}

void CAmmoChooser::Close()
{
	if (!m_bIsOpen)
		return;
    m_bIsOpen = false;
	m_AutoCloseTimer.Stop();
	m_AutoSwitchTimer.Stop();
	m_NextAmmoKeyDownTimer.Stop();
	if (g_pHUDMgr)
		g_pHUDMgr->QueueUpdate(kHUDChooser);

}

void CAmmoChooser::NextAmmo()
{
	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if ( !pClientWeapon )
	{
		return;
	}

	m_hAmmo = pClientWeapon->GetNextAvailableAmmo(m_hAmmo);

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

	m_AutoCloseTimer.Start(kfDelayTime);
	m_NextAmmoKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	g_pHUDMgr->QueueUpdate(kHUDChooser);
}

void CAmmoChooser::EndAutoSwitch()
{
	m_NextAmmoKeyDownTimer.Stop();
	m_AutoSwitchTimer.Stop();
}

void CAmmoChooser::Update()
{
	// If Weapon chooser is being drawn, see if we want to change weapons...
	if (!IsOpen()) return;


	// See if we should close ourselves...

	// See if we should switch to the next ammo type...
	if (m_AutoCloseTimer.IsStarted( ) && m_AutoCloseTimer.IsTimedOut( ))
	{
        g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());
		Close();
	}
	else if (m_NextAmmoKeyDownTimer.On() && m_NextAmmoKeyDownTimer.Stopped())
	{
		if (m_AutoSwitchTimer.IsStarted( ))
		{
			if (m_AutoSwitchTimer.IsTimedOut( ))
			{
				NextAmmo();
				m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
			}
		}
		else
		{
			m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
		}
	}



}
