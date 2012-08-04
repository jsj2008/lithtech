
// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponMgr.cpp
//
// PURPOSE : Manager of client-side weapons
//
// (c) 2002-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "PlayerMgr.h"
#include "PlayerStats.h"
#include "CommandIDs.h"
#include "MsgIDs.h"
#include "ClientWeaponMgr.h"
#include "PlayerBodyMgr.h"
#include "CMoveMgr.h"
#include "TurretFX.h"
#include "ClientDB.h"
#include "VehicleMgr.h"
#include "PlayerLureFX.h"

//
// Defines
//

#define CWM_NO_WEAPON -1

// checks if weapon is in range, the pointer array exists
// and the pointer is valid
#define CWM_WEAPON_INDEX_IS_VALID( _wep ) \
	( ( ( 0 <= ( _wep ) ) && ( ( _wep ) < m_nMaxWeapons ) ) && \
	( ( 0 != m_apClientWeapon ) && ( 0 != m_apClientWeapon[ ( _wep ) ] ) ) )

#define DEFAULT_COLLECTION_TOOL  "CollectionToolBase"

//
// Static functions
//
void CClientWeaponMgr::CallbackHook( HWEAPON hWeapon, void *pData )
{
	ASSERT( 0 != pData );
	CClientWeaponMgr *pClientWeaponMgr = static_cast< CClientWeaponMgr* >( pData );
	pClientWeaponMgr->DeselectCallback( hWeapon );
}

//
// Globals...
//

VarTrack	g_vtKeepCurrentAmmo;

CClientWeaponMgr	*g_pClientWeaponMgr = NULL;

// PLAYER_BODY
extern VarTrack g_vtPlayerBodyWeapons;


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::CClientWeaponMgr()
//
//	PURPOSE:	Constructor for client weapon manager class	
//
// ----------------------------------------------------------------------- //

CClientWeaponMgr::CClientWeaponMgr():
	m_apClientWeapon		( NULL ),
	m_nMaxWeapons			( 0 ),
	m_nCurClientWeaponIndex	( CWM_NO_WEAPON ),
	m_pCurrentWeapon		( NULL ),
	m_bWeaponsEnabled		( true ),
	m_bWeaponsVisible		( true ),
	m_hDefaultWeapon		( NULL ),
	m_hHolsterWeapon		( NULL ),
	m_hRequestedWeapon		( NULL ),
	m_hLastWeapon			( NULL ),
	m_hRequestedAmmo		( NULL ),
	m_bWeaponsPaused		( false ),
	m_pVisibleCustomWeapon( NULL )
{
	g_pClientWeaponMgr = this;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::~CClientWeaponMgr()
//
//	PURPOSE:	Desructor for the client weapon mgr class
//
// ----------------------------------------------------------------------- //

CClientWeaponMgr::~CClientWeaponMgr()
{
	// Destruct
	Term();

	g_pClientWeaponMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::Init()
//
//	PURPOSE:	Initialize the client weapon mgr system and create the weapons
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::Init()
{
	// Determine m_nMaxWeapons
	
	m_nMaxWeapons = g_pWeaponDB->GetNumPlayerWeapons();

	// Allocate space for our array...

	m_apClientWeapon = debug_newa( CClientWeapon*, m_nMaxWeapons );
	if( !m_apClientWeapon )
	{
		ASSERT( !"CClientWeaponMgr::Init.  Could not allocate clientweapon array." );
		return false;
	}

	// Clear us out.
	memset( m_apClientWeapon, 0, m_nMaxWeapons * sizeof( CClientWeapon* ));

	for( uint8 nWeapon = 0; nWeapon < m_nMaxWeapons; ++nWeapon )
	{
		// Get the weapon datadase record...
		
		HWEAPON hWeapon = g_pWeaponDB->GetPlayerWeapon( nWeapon );
		if( !hWeapon )
		{
			ASSERT( !"CClientWeaponMgr::Init.  Could not get HWEAPON." );
			return false;
		}

		m_apClientWeapon[nWeapon] = debug_new( CClientWeapon );
		if( !m_apClientWeapon[nWeapon] )
		{
			ASSERT( !"CClientWeaponMgr::Init.  Could not create clientweapon." );
			return false;
		}

		// init the weapon
		if( !m_apClientWeapon[nWeapon]->Init( hWeapon ))
		{
			ASSERT( !"CClientWeaponMgr::Init.  Could not init clientweapon." );
			return false;
		}
	}

	m_pCurrentWeapon		= NULL;
	m_nCurClientWeaponIndex	= CWM_NO_WEAPON;
	m_hLastWeapon			= NULL;

	g_vtKeepCurrentAmmo.Init( g_pLTClient, "KeepCurrentAmmo", NULL, 1.0f );


	HRECORD hShared = ClientDB::Instance( ).GetClientSharedRecord( );

	// for now, always succeed
	return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::Term()
//
//	PURPOSE:	Initialize the client weapon mgr system and create the weapons
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::Term()
{
	if ( 0 != m_apClientWeapon )
	{
		// destroy all the array entries
		for ( int i = 0; i < m_nMaxWeapons; ++i )
		{
			if ( 0 != m_apClientWeapon[ i ] )
			{
				m_apClientWeapon[ i ]->Term();
				debug_delete( m_apClientWeapon[ i ] );
				m_apClientWeapon[ i ] = 0;
			}
		}

		// destroy the array
		debug_deletea( m_apClientWeapon );
		m_apClientWeapon = 0;
	}

	m_pCurrentWeapon		= NULL;
	m_nCurClientWeaponIndex	= CWM_NO_WEAPON;
	m_hLastWeapon			= NULL;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::OnEnterWorld()
//
//	PURPOSE:	What to do when the world is entered
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::OnEnterWorld()
{
	// this system is not designed to work without weapons
	ASSERT( 0 != m_apClientWeapon );

	// destroy all the array entries
	for ( int i = 0; i < m_nMaxWeapons; ++i )
	{
		if ( 0 != m_apClientWeapon[ i ] )
		{
			m_apClientWeapon[ i ]->OnEnterWorld();
		}
	}

	// We should not have a current weapon at this point...

	m_pCurrentWeapon		= NULL;
	m_nCurClientWeaponIndex	= CWM_NO_WEAPON;
	m_hLastWeapon			= NULL;

}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::OnExitWorld()
//
//	PURPOSE:	What to do when the world is exited
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::OnExitWorld()
{
	// [kml] 1/30/02
	// We need to return here because the engine can
	// call GameClientShell::OnEngineTerm before GameClientShell::OnExitWorld.
	// Since OnEngineTerm() terminates this class, the array might have already
	// been cleared here.
	if(!m_apClientWeapon)
	{
		return;
	}

	// destroy all the array entries
	for ( int i = 0; i < m_nMaxWeapons; ++i )
	{
		if ( 0 != m_apClientWeapon[ i ] )
		{
			m_apClientWeapon[ i ]->OnExitWorld();
		}
	}

	// When leaving a world there is no need to keep track of the current weapon...

	m_pCurrentWeapon		= NULL;
	m_nCurClientWeaponIndex	= CWM_NO_WEAPON;
	m_hLastWeapon			= NULL;

}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::OnMessage()
//
//	PURPOSE:	Handle messages
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::OnMessage( uint8 messageId, ILTMessage_Read *pMsg )
{
	// See if the message is strickly for our current active weapon...

	if( m_pCurrentWeapon )
	{
		if( m_pCurrentWeapon->OnMessage( messageId, pMsg ))
		{
			return true;
		}
	}
	
	switch ( messageId )
	{
		case MID_WEAPON_CHANGE :	HandleMsgWeaponChange( pMsg );	break;
		case MID_WEAPON_LAST :		HandleMsgLastWeapon( pMsg );	break;
		case MID_WEAPON_BREAK_WARN:	HandleWeaponBreakWarning( pMsg ); break;

		default:					return false;					break;
	};

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::HandleMsgWeaponChange()
//
//	PURPOSE:	Handle a message recieved to change to a weapon...
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::HandleMsgWeaponChange (ILTMessage_Read *pMsg)
{
	if( !pMsg )
		return;

	HWEAPON	hWeapon	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	bool	bForce	= pMsg->Readbool();
	bool	bPlaySelect	= pMsg->Readbool();
	bool	bPlayDeselect	= pMsg->Readbool();
	HAMMO	hAmmo	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );

//	const char* pszName = g_pLTDatabase->GetRecordName(hWeapon);

	if( !hWeapon )
		return;
	

	bool bChange = bForce;
	if (!bForce)
	{
		if (IsMultiplayerGameClient())
			bChange = GetConsoleBool( "MPAutoWeaponSwitch",true );
		else
			bChange = GetConsoleBool( "SPAutoWeaponSwitch",true );

	}


	// See what ammo the weapon should start with...
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
	if (g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bIsGrenade))
		return;

	HAMMO hDefaultAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
	if( hAmmo )
	{
		hDefaultAmmo = hAmmo;
	}

	if( bChange )
	{
		// Force a change to the appropriate weapon...
		if( g_pPlayerStats )
		{
			//if we're forcing a weapon change, do not honor any old weapon requests
			if (!bPlaySelect)
			{
				m_hRequestedWeapon = NULL;
				m_hRequestedAmmo = NULL;
			}
			ChangeWeapon(hWeapon, hDefaultAmmo, g_pPlayerStats->GetAmmoCount( hDefaultAmmo ), bPlaySelect, bPlayDeselect);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::HandleMsgLastWeapon()
//
//	PURPOSE:	Handle a message recieved to change back to our previous weapon...
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::HandleMsgLastWeapon (ILTMessage_Read *pMsg)
{
	LastWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::HandleWeaponBreakWarning()
//
//	PURPOSE:	Handle when the current weapon is close to breaking
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::HandleWeaponBreakWarning( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	HWEAPON	hWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	bool bWillBreak = pMsg->Readbool();

	CClientWeapon* pWeapon = GetClientWeapon(hWeapon);
	if( pWeapon )
	{
		pWeapon->SetWeaponAboutToBreak(bWillBreak);
	}
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::OnModelKey()
//
//	PURPOSE:	Pass model key messages to the current weapon, returns true
//				if the message was handled
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::OnModelKey( HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList *pArgs )
{
	// pass the model key to the current weapon
	if ( m_pCurrentWeapon )
	{
		ASSERT( CWM_WEAPON_INDEX_IS_VALID( m_nCurClientWeaponIndex ) );
		ASSERT( m_pCurrentWeapon == m_apClientWeapon[ m_nCurClientWeaponIndex ] );

		return m_pCurrentWeapon->OnModelKey( hObj, hTrackerID, pArgs );
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::OnCommandOn()
//
//	PURPOSE:	Handle a command turning on...
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::OnCommandOn( int nCmd )
{
	if (COMMAND_ID_HOLSTER == nCmd) 
	{
		ToggleHolster(true);
		return true;
	}
	
	if( (COMMAND_ID_WEAPON_BASE <= nCmd) && (nCmd <= COMMAND_ID_WEAPON_MAX) )
	{
		uint8 nIndex = nCmd - COMMAND_ID_WEAPON_BASE;

		HWEAPON hWeapon = g_pPlayerStats->GetWeaponInSlot(nIndex);

		return ChangeWeapon( hWeapon );
	}

	if( (COMMAND_ID_GRENADE_BASE <= nCmd) && (nCmd <= COMMAND_ID_GRENADE_MAX) )
	{
		uint8 nIndex = nCmd - COMMAND_ID_GRENADE_BASE;
		
		//get grenade for this slot
		HWEAPON hGrenade = g_pWeaponDB->GetPlayerGrenade(nIndex);
		if (hGrenade)
		{
			g_pPlayerStats->UpdatePlayerGrenade( hGrenade, true );
		}
		
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::Load()
//
//	PURPOSE:	Load the pertanant information from the message
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::Load( ILTMessage_Read *pMsg )
{
	ASSERT( 0 != pMsg );

	// load the holster information
	m_hHolsterWeapon	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );

	// load the request info
	m_hRequestedWeapon	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	m_hRequestedAmmo	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );

	uint32 nWeapons = pMsg->Readint32();

	for( uint32 i = 0; i < nWeapons; ++i )
	{
		if( NULL != m_apClientWeapon[i] )
		{
			m_apClientWeapon[i]->Load( pMsg );
		}
	}

	m_hDefaultWeapon		= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	m_bWeaponsVisible		= pMsg->Readbool( );
	m_bWeaponsEnabled		= pMsg->Readbool( );
	m_bWeaponsPaused		= pMsg->Readbool( );
	m_nCurClientWeaponIndex	= pMsg->Readuint32( );

	// Request a weapon change with either the saved requested weapon or the saved current weapon...
	if( !m_hRequestedWeapon && m_nCurClientWeaponIndex < nWeapons )
		m_hRequestedWeapon = m_apClientWeapon[ m_nCurClientWeaponIndex ]->GetWeaponRecord( );

	m_pCurrentWeapon		= NULL;
	m_nCurClientWeaponIndex	= CWM_NO_WEAPON;

	//set this after changing weapons
	m_hLastWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );

	if (m_bWeaponsEnabled)
		EnableWeapons();
	else
		DisableWeapons();

	if (m_bWeaponsVisible)
		ShowWeapons();
	else
		HideWeapons();


	PauseWeapons( m_bWeaponsPaused );
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	void CClientWeaponMgr::Save(ILTMessage_Write *pMsg)
//
//	PURPOSE:	Save the pertanant information to the message
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::Save( ILTMessage_Write *pMsg )
{
	ASSERT( 0 != pMsg );

	// holster information
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hHolsterWeapon );

	// requst info
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hRequestedWeapon );
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hRequestedAmmo );

	pMsg->Writeint32( m_nMaxWeapons );

	for( int i = 0; i < m_nMaxWeapons; ++i )
	{
		if( NULL != m_apClientWeapon[i] )
		{
			m_apClientWeapon[i]->Save( pMsg );
		}
	}

	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hDefaultWeapon );
	pMsg->Writebool( m_bWeaponsVisible );
	pMsg->Writebool( m_bWeaponsEnabled );
	pMsg->Writebool( m_bWeaponsPaused );
	pMsg->Writeuint32( m_nCurClientWeaponIndex );
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hLastWeapon );
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::UpdateWeaponModel()
//
//	PURPOSE:	Update the current weapon
//
// ----------------------------------------------------------------------- //

WeaponState CClientWeaponMgr::Update( LTRotation const &rRot, LTVector const &vPos )
{

	if ( ( CWM_NO_WEAPON == m_nCurClientWeaponIndex ) && ( m_pCurrentWeapon ) )
	{
		// the weapon is out of sync with itself, check calling order
		ASSERT( 0 );
	}

	// update the current weapon 
	WeaponState eWeaponState = W_IDLE;
	if ( m_pCurrentWeapon )
	{
		// Set the weapon that the player is using so the PlayerBody plays the correct animations...
		CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Weapon, m_pCurrentWeapon->GetAnimationProperty() );

		m_pCurrentWeapon->SetCameraInfo( rRot, vPos );
		eWeaponState = m_pCurrentWeapon->Update( );
	}

	// Check to see if we should auto-switch to a new weapon...

	if ( W_AUTO_SWITCH == eWeaponState )
	{
		AutoSelectWeapon();
	}

	// if we received a request to change the weapon during
	// the update, do it now
	if( (CWM_NO_WEAPON == m_nCurClientWeaponIndex) || CPlayerBodyMgr::Instance( ).IsPlayingSpecial( ))
	{
		// no current weapon, see if we are trying to change
		// to another weapon
		if( m_hRequestedWeapon )
		{
			// select the requested weapon
			ChangeWeapon( m_hRequestedWeapon, m_hRequestedAmmo, -1, sk_bPlaySelect, !sk_bPlayDeselect );

			m_hRequestedWeapon = NULL;
			m_hRequestedAmmo = NULL;
		}
	}

	// update the state of specialty weapons (forensics)
	UpdateCustomWeapon();

	return eWeaponState;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::UpdateCustomWeapon()
//
//	PURPOSE:	Update the current custom weapon
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::UpdateCustomWeapon()
{
	if(!m_pVisibleCustomWeapon)
		return;

	CPlayerBodyMgr &PlayerBodyMgr = CPlayerBodyMgr::Instance( );

	// Hide custom model when no longer in use
	if(!PlayerBodyMgr.HandlingAnimationStimulusGroup(m_sCustomStimulusGroup.c_str()) &&
		!PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kCustomContext)->IsTransitioning())
	{
		HideCustomWeapon();
		return;
	}

	// Update custom weapon model position to attached position
	m_pVisibleCustomWeapon->UpdateWeaponPosition(LTVector::GetIdentity());

	HOBJECT hPlayerBody = PlayerBodyMgr.GetObject();
	if(!hPlayerBody)
		return;

	// Update the custom weapon animation

	CAnimationContext *pContext = PlayerBodyMgr.GetAnimationContext( PlayerBodyMgr.GetWeaponContext() );
	LTASSERT( pContext, "ERROR - CClientWeaponMgr::UpdateCustomWeapon: Invalid weapon context!" );
	if( !pContext )
		return;

	uint32 dwAnimIndex = INVALID_MODEL_ANIM;
	g_pModelLT->GetCurAnim( hPlayerBody, pContext->GetTrackerID( ), dwAnimIndex );

	// Try and play the custom weapon animation if the PlayerBody animation changed...
	if( m_dwLastCustomWeaponContextAnim != dwAnimIndex )
	{
		char szAnimName[64] = {0};
		g_pModelLT->GetAnimName( hPlayerBody, dwAnimIndex, szAnimName, LTARRAYSIZE(szAnimName) );

		m_pVisibleCustomWeapon->SetWeaponModelAnimation( szAnimName );

		m_dwLastCustomWeaponContextAnim = dwAnimIndex;
	}

	m_pVisibleCustomWeapon->UpdateWeaponDisplay();
}

//-----------------------------------------------------------------------------

void CClientWeaponMgr::SelectCustomWeapon(HWEAPON hWeapon)
{
	if (hWeapon)
	{
		// find the common prefix of the custom weapon select and deselect enums
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		std::string sSelect(  g_pWeaponDB->GetString( hWpnData, "CustomSelectStimulus" ));
		std::string sDeselect(  g_pWeaponDB->GetString( hWpnData, "CustomDeselectStimulus" ));
		std::pair<std::string::iterator,std::string::iterator> prCommonPrefix;
		prCommonPrefix = std::mismatch(sSelect.begin(), sSelect.end(), sDeselect.begin());
		std::string sStimulusGroup(sSelect.begin(),prCommonPrefix.first);

		// Play an appropriate animation...
		CPlayerBodyMgr& PlayerBodyMgr = CPlayerBodyMgr::Instance();

		PlayerBodyMgr.HandleAnimationStimulus(sSelect.c_str());
		if(PlayerBodyMgr.ActiveAnimationStimulus())
		{
			CClientWeapon* pWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetClientWeapon(hWeapon);
			if (pWeapon)
			{
				// Attach the weapon model
				g_pPlayerMgr->GetClientWeaponMgr()->ShowCustomWeapon(pWeapon, sStimulusGroup.c_str());
			}
		}
	}		
}

//-----------------------------------------------------------------------------

void CClientWeaponMgr::DeselectCustomWeapon()
{
	CPlayerBodyMgr::Instance().HandleAnimationStimulus(GetCustomWeaponDeselectStimulus());
}

//-----------------------------------------------------------------------------

const char* CClientWeaponMgr::GetCustomWeaponDeselectStimulus()
{
	CClientWeapon* pWeapon = GetVisibleCustomWeapon();
	if (pWeapon)
	{
		HWEAPON hWeapon = pWeapon->GetWeaponRecord();
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		return g_pWeaponDB->GetString( hWpnData, "CustomDeselectStimulus" );
	}

	return "";
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::GetClientWeapon()
//
//	PURPOSE:	Finds a client weapon corresponding to the specified weapon record
//
// ----------------------------------------------------------------------- //

CClientWeapon* CClientWeaponMgr::GetClientWeapon( HWEAPON hDesiredWeapon ) const
{
	for( uint32 iWeapon=0; iWeapon < m_nMaxWeapons; ++iWeapon )
	{
		CClientWeapon* pCur = m_apClientWeapon[iWeapon];
		if( pCur->GetWeaponRecord() == hDesiredWeapon )
			return m_apClientWeapon[iWeapon];
	}
	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::GetCurrentClientWeapon()
//
//	PURPOSE:	Return a pointer to the current weapon
//
// ----------------------------------------------------------------------- //

CClientWeapon *CClientWeaponMgr::GetCurrentClientWeapon() const
{
	if ( CWM_NO_WEAPON == m_nCurClientWeaponIndex )
	{
		return 0;
	}

	// we should always have a current weapon
	ASSERT( CWM_WEAPON_INDEX_IS_VALID( m_nCurClientWeaponIndex ) );
	ASSERT( m_pCurrentWeapon == m_apClientWeapon[ m_nCurClientWeaponIndex ] );

	return m_pCurrentWeapon;
}

//-----------------------------------------------------------------------------

void CClientWeaponMgr::InitCustomWeapons()
{
	for ( int i = 0; i < m_nMaxWeapons; ++i )
	{
		CClientWeapon* pWeapon = m_apClientWeapon[ i ];
		if ( pWeapon )
		{
			// check to see if this is a custom weapon by check for a custom select stimulus
			// which is used to bring the custom weapon up.
			HWEAPON hWeapon = pWeapon->GetWeaponRecord();
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
			const char* pszCustomSelect = g_pWeaponDB->GetString(hWpnData, WDB_WEAPON_CustomSelectStimulus);
			bool bCustomWeapon = !LTStrEmpty(pszCustomSelect) && LTStrCmp(pszCustomSelect, "None")!=0;
			if (bCustomWeapon)
			{
				pWeapon->Activate();
				pWeapon->SetDisable(false);
				pWeapon->SetVisible(false);
			}
		}
	}
}

//-----------------------------------------------------------------------------

CClientWeapon *CClientWeaponMgr::GetVisibleCustomWeapon() const
{
	return m_pVisibleCustomWeapon;
}

//-----------------------------------------------------------------------------

void CClientWeaponMgr::ShowCustomWeapon(CClientWeapon* pWeapon, const char* pszStimulusGroup)
{
	if( pWeapon == m_pVisibleCustomWeapon )
		return;

	HideCustomWeapon();		// just in case

	m_pVisibleCustomWeapon = pWeapon;
	m_sCustomStimulusGroup = pszStimulusGroup;

	if (m_pVisibleCustomWeapon)
	{
		m_pVisibleCustomWeapon->ShowCustomWeapon();
		m_pVisibleCustomWeapon->UpdateWeaponDisplay();	// force an update not that its being shown
	}
}

//-----------------------------------------------------------------------------

void CClientWeaponMgr::HideCustomWeapon()
{
	if (m_pVisibleCustomWeapon)
	{
		m_pVisibleCustomWeapon->HideCustomWeapon();
		m_pVisibleCustomWeapon = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::GetSequentialWeaponId()
//
//	PURPOSE:	Find the next/prev available weapon within the same class
//				This function will get either the next or the previous
//				available weapon based on the 'next' flag passed in.
//
// ----------------------------------------------------------------------- //

HWEAPON CClientWeaponMgr::GetSequentialWeaponRecord( HWEAPON hWeapon, bool bNext ) const
{
	ASSERT( 0 != g_pPlayerStats );
	ASSERT( 0 != g_pWeaponDB );

	// check that there are other weapons to try
	if ( 1 >= m_nMaxWeapons )
	{
		return NULL;
	}


	// make sure a valid weapon was specified
	if( !hWeapon )
	{
		if( CWM_NO_WEAPON == m_nCurClientWeaponIndex )
		{
			// no current weapon, get the first player weapon record...

			hWeapon = g_pWeaponDB->GetWeaponFromDefaultPriority( 0 );
		}
		else
		{
			// if an invalid weapon is specified, use the current

			hWeapon = m_apClientWeapon[m_nCurClientWeaponIndex]->GetWeaponRecord();
		}
	}

	//if we still don't have a weapon...
	if (!hWeapon) 
	{
		return NULL;
	}

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	// Find where in the order list the specified weapon is located...
    uint8 nCurPlayerWeaponPriority = pProfile->GetWeaponPriority(hWeapon);
	
	if( nCurPlayerWeaponPriority == WDB_INVALID_WEAPON_INDEX )
	{
		// Current weapon isn't in the priority list check if it's single version is...
		HWEAPONDATA hCurWeaponData = g_pWeaponDB->GetWeaponData( hWeapon, !USE_AI_DATA );
		HWEAPON hSingleWeapon = g_pWeaponDB->GetRecordLink( hCurWeaponData, WDB_WEAPON_rSingleWeapon );
		if( hSingleWeapon )
			nCurPlayerWeaponPriority = pProfile->GetWeaponPriority( hSingleWeapon );
	}

	// Now find the next weapon with ammo...
	HWEAPON hReturnWeapon = NULL;
	uint8 nNumPri = pProfile->m_vecWeapons.size();

	if( nNumPri > 0 )
	{
		uint8 nNextPlayerWeaponPriority = nCurPlayerWeaponPriority + ( bNext ? 1 : -1 );
		while( (nNextPlayerWeaponPriority != nCurPlayerWeaponPriority) && (hReturnWeapon == NULL) )
		{
			// Check for wrap around...

			if( nNextPlayerWeaponPriority >= nNumPri )
			{
				nNextPlayerWeaponPriority = (bNext ? 0 : nNumPri - 1);
				continue;
			}


			HWEAPON hNextPlayerWeapon = pProfile->m_vecWeapons[nNextPlayerWeaponPriority];
			
			if( !g_pPlayerStats->HaveWeapon( hNextPlayerWeapon ))
			{
				// Use the dual version of the weapon if specified...
				HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hNextPlayerWeapon, !USE_AI_DATA);
				HWEAPON hDualWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rDualWeapon );
				if( hDualWeapon )
					hNextPlayerWeapon = hDualWeapon;
			}
			
			uint8 nWpnIndex = g_pWeaponDB->GetPlayerWeaponIndex( hNextPlayerWeapon );

			if( g_pPlayerStats->HaveWeapon( hNextPlayerWeapon ) &&
				m_apClientWeapon[nWpnIndex]->HasAmmo( ))
			{
				// Potential match...

				HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hNextPlayerWeapon, !USE_AI_DATA);
				bool bIsGrenade = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bIsGrenade);	

				if( !bIsGrenade)
				{
					hReturnWeapon = hNextPlayerWeapon;
				}
			}
			
			nNextPlayerWeaponPriority += (bNext ? 1 : -1 );
		}
	}

	return hReturnWeapon;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::ChangeWeapon()
//
//	PURPOSE:	Change to a different weapon
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::ChangeWeapon( HWEAPON hWeapon,
									HAMMO hAmmo /* = NULL */,
                                     int nAmmoAmount /*=-1*/,
                                     bool bPlaySelect /*=true*/,
									 bool bPlayDeselect /*=true*/ )
{

	// Check if the weapon has a dual version that can be switched to...
	HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData( hWeapon, !USE_AI_DATA );
	HWEAPON hDualWeapon = g_pWeaponDB->GetRecordLink( hWeaponData, WDB_WEAPON_rDualWeapon );
	if( CanChangeToWeapon( hDualWeapon ))
	{
		// Change to dual version of weapon...
		hWeapon = hDualWeapon;
	}
	else if( !CanChangeToWeapon( hWeapon ))
		return false;

//	const char* pszName = g_pLTDatabase->GetRecordName(hWeapon);
//	DebugCPrint(0,"%s - %s",__FUNCTION__,pszName);

	// assume we'll be changing weapon and ammo
	bool bChangeWeapon = true;
	bool bChangeAmmo = true;

	if ( m_pCurrentWeapon )
	{
		// If there is a current weapon, determine if we
		// have to switch the weapon, the ammo, or both.
		HWEAPON hCurWeapon = m_pCurrentWeapon->GetWeaponRecord();

		// Do we need to change the weapon?
		if( hWeapon == hCurWeapon )
		{
			bChangeWeapon = false;

			// Make sure a weapon change in progress is aborted...
			if( NULL != m_hRequestedWeapon )
			{
				m_hRequestedWeapon = hCurWeapon;
			}
		}

	}

	// if the specified hAmmo is INVALID, we'll try to find
	// the best ammo to switch to
	HAMMO hNewAmmo = NULL;
	int nNewAmmoAmount = -1;

	// get the best new ammo type
	uint8 nNewWeaponIndex = g_pWeaponDB->GetPlayerWeaponIndex( hWeapon );
	if( (NULL == hAmmo) && (CWM_NO_WEAPON != nNewWeaponIndex) )
	{
		// If we don't want to keep the current ammo get the highest priority ammo...

		if( g_vtKeepCurrentAmmo.GetFloat() < 1.0f )
		{
			hNewAmmo = m_apClientWeapon[ nNewWeaponIndex ]->GetBestAvailableAmmo( );
		}
		else
		{
			hNewAmmo = m_apClientWeapon[nNewWeaponIndex]->GetAmmoRecord();

			// [KLS 5/9/02] Since we're using the ammo record stored in the weapon, make sure
			// that the weapon actually has ammo associated with this record...If not,
			// change to the best available ammo...

			if( g_pPlayerStats->GetAmmoCount( hNewAmmo ) <= 0 )
			{
				hNewAmmo = m_apClientWeapon[ nNewWeaponIndex ]->GetBestAvailableAmmo( );
			}
		}

		if (!hNewAmmo)
		{
			hNewAmmo = g_pWeaponDB->GetRecordLink( hWeaponData, WDB_WEAPON_rAmmoName );
		}

	}
	else
	{
		hNewAmmo = hAmmo;
	}



	// get the amount of ammo
	if ( -1 == nAmmoAmount )
	{
		nNewAmmoAmount = g_pPlayerStats->GetAmmoCount( hNewAmmo );
	}
	else
	{
		nNewAmmoAmount = nAmmoAmount;
	}

	// Do we need to change the ammo?
	if ( m_pCurrentWeapon )
	{
		// If there is a current weapon, determine if we
		// have to switch the weapon, the ammo, or both.
		HAMMO hCurAmmo = m_pCurrentWeapon->GetAmmoRecord();
		if ( hNewAmmo == hCurAmmo )
		{
			bChangeAmmo = false;

			// Make sure a ammo change in progress is aborted...
			if (m_hRequestedAmmo != NULL)
			{
				m_hRequestedAmmo = hCurAmmo;
			}
		}
	}

	bool bSwitchInstantly = bChangeWeapon;


	if ( bChangeWeapon )
	{
		// Handle deselection of current weapon...
		if ( bPlayDeselect )
		{
			if ( CWM_NO_WEAPON != m_nCurClientWeaponIndex )
			{
				// change weapons

				// save the new weapon id.
				m_hRequestedWeapon = hWeapon;
				m_hRequestedAmmo = hNewAmmo;
				if (m_pCurrentWeapon && CanLastWeapon(m_pCurrentWeapon->GetWeaponRecord()))
				{
					m_hLastWeapon = m_pCurrentWeapon->GetWeaponRecord();
				}

				// deselect the weapon
				bool bResult = m_apClientWeapon[ m_nCurClientWeaponIndex ]->Deselect( CallbackHook, this );
				if ( bResult )
				{
					bSwitchInstantly = false;
				}
			}

			//check to see if the our old weapon is linked to a grenade-type...
			if (m_pCurrentWeapon)
			{
				HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_pCurrentWeapon->GetWeaponRecord(), !USE_AI_DATA);
				HWEAPON hLinkedWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rLinkedWeapon  );
				bool bSelect = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bSelectLinked );
				if( bSelect && hLinkedWeapon)
				{
					//OK, it is linked, so deselect that grenade too...
					HWEAPONDATA hLnkWpnData = g_pWeaponDB->GetWeaponData(hLinkedWeapon, !USE_AI_DATA);
					if (g_pWeaponDB->GetBool( hLnkWpnData, WDB_WEAPON_bIsGrenade ) &&
						g_pPlayerStats->GetLastGrenadeRecord() != hLinkedWeapon)
					{
						g_pPlayerStats->LastGrenade();
					}
				}
			}

		}

		if ( bSwitchInstantly )
		{

			if ( m_pCurrentWeapon )
			{
				// deactivate the old weapon, if any
				m_pCurrentWeapon->Deactivate();

			}


			// no current weapon, no need to deselect
			ASSERT( 0 != m_apClientWeapon );
			if (m_pCurrentWeapon && CanLastWeapon(m_pCurrentWeapon->GetWeaponRecord()))
			{
				m_hLastWeapon = m_pCurrentWeapon->GetWeaponRecord();
			}

			m_nCurClientWeaponIndex = g_pWeaponDB->GetPlayerWeaponIndex( hWeapon );
			if( m_nCurClientWeaponIndex != CWM_NO_WEAPON )
			{
				m_pCurrentWeapon = m_apClientWeapon[ m_nCurClientWeaponIndex ];
				m_pCurrentWeapon->Activate();
				m_pCurrentWeapon->Select(!bPlaySelect);

				// instantly switch to the new ammo
				m_pCurrentWeapon->ChangeAmmoImmediate( hNewAmmo , nNewAmmoAmount );

				if( !m_bWeaponsEnabled )
					m_pCurrentWeapon->SetDisable( true );
			}
			else
			{
				m_pCurrentWeapon = NULL;
			}
		}
	}
	else if ( bChangeAmmo && m_pCurrentWeapon )
	{
		// don't need to change the weapon, but we do
		// need to change the ammo
		m_pCurrentWeapon->ChangeAmmoWithReload( hNewAmmo );
	}
	
	// We succesfully started the weapon change process, so let the player mgr handle
	// it...

	//jrg- 9/2/02 - this is getting called both when we start switching and when we finish switching
	//		PlayerMgr is using this to track if we're in mid switch
	g_pPlayerMgr->HandleWeaponChanged( hWeapon, hNewAmmo, (!bChangeWeapon || bSwitchInstantly) );


	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDerWeaponModel::CanChangeToWeapon()
//
//	PURPOSE:	See if we can change to this weapon
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::CanChangeToWeapon( HWEAPON hWeapon )
{
	ASSERT( 0 != g_pPlayerMgr );
	ASSERT( 0 != g_pPlayerStats );

	// Make sure this is a valid weapon for us to switch to...
	if( (NULL == hWeapon) || !g_pPlayerStats->HaveWeapon( hWeapon ))
	{
		return false;
	}

	// no changing if the player is dead or in spectator mode or the CharacterFX is not yet initialized...
	if( !g_pPlayerMgr->IsPlayerAlive() || g_pPlayerMgr->IsSpectating() ||
		(g_pPlayerMgr->GetMoveMgr( )->GetCharacterFX( ) == NULL))
	{
		return false;
	}

	// Don't allow weapon changes if we are playing a special animation.
	if( CPlayerBodyMgr::Instance().IsPlayingSpecial() )
	{
		return false;
	}

	// Determine if the PlayerLure will allow switching weapons...
	if( g_pMoveMgr->GetVehicleMgr( )->IsVehiclePhysics( ))
	{
		PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( g_pMoveMgr->GetVehicleMgr( )->GetPlayerLureId( ));
		if( pPlayerLureFX && !pPlayerLureFX->GetAllowSwitchWeapon( ))
			return false;
	}

	if( g_pPlayerMgr->IsOperatingTurret( ))
	{
		CTurretFX *pTurret = g_pPlayerMgr->GetTurret( );
		if( pTurret )
		{
			HWEAPON hTurretWeapon = g_pWeaponDB->GetRecordLink( pTurret->GetTurretRecord(), WDB_TURRET_rWeapon );
			if( hTurretWeapon != hWeapon )
				return false;
		}
	}
	
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::CanLastWeapon()
//
//	PURPOSE:	See if we can change to this weapon using LastWeapon
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::CanLastWeapon( HWEAPON hWeapon )
{
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
	return g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bCanLastWeapon );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::SetDefaultWepon()
//
//	PURPOSE:	Set the default weapon
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::SetDefaultWeapon( HWEAPON hWeapon )
{
	if( !g_pWeaponDB->IsPlayerWeapon( hWeapon ))
		return false;

	m_hDefaultWeapon = hWeapon;

	if (NULL == GetCurrentClientWeapon())
		ChangeWeapon(m_hDefaultWeapon);
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::ToggleHolster()
//
//	PURPOSE:	Holster or unholster our weapon
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::ToggleHolster( bool bPlayDeselect )
{
	bPlayDeselect = bPlayDeselect && WeaponsEnabled();

	// [kml] 3/26/02 Sanity check because we entered a world
	// without a current weapon
	if( m_pCurrentWeapon )
	{
		// when bPlayDeselect == FALSE, ToggleHolster will force 
		// the weapon to change without the deselect animation
		if ( g_pWeaponDB->GetUnarmedRecord( ) == m_pCurrentWeapon->GetWeaponRecord() )
		{
			// the current weapon is the default, get out the holstered weapon
			ChangeWeapon( m_hHolsterWeapon, NULL, -1, sk_bPlaySelect, bPlayDeselect );
			m_hHolsterWeapon = NULL;
		}
		else
		{
			// put the current weapon away and switch to the default
			m_hHolsterWeapon = m_pCurrentWeapon->GetWeaponRecord();
			// Can only do this if a holster weapon was specified in weapons.txt.
			if( g_pWeaponDB->GetUnarmedRecord( ))
			{
				ChangeWeapon( g_pWeaponDB->GetUnarmedRecord( ), NULL, -1, sk_bPlaySelect, bPlayDeselect );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::LastWeapon()
//
//	PURPOSE:	Swap to our previously used weapon.
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::LastWeapon()
{
	if (!WeaponsEnabled()) return;

	uint8 nWeaponIndex = g_pWeaponDB->GetPlayerWeaponIndex( m_hLastWeapon );

	if( CWM_NO_WEAPON != nWeaponIndex )
	{
		if ( ( g_pPlayerStats->HaveWeapon( m_hLastWeapon ) ) &&  // have the weapon
			( m_apClientWeapon[ nWeaponIndex ]->HasAmmo() ) )  // weapon has ammo
		{
			ChangeWeapon( m_hLastWeapon, NULL, -1 );
		}
	}
}
	

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	void CClientWeaponMgr::EnableWeapons()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::EnableWeapons()
{
	// Don't allow the enabling of weapons while dead...
	if( !g_pPlayerMgr->IsPlayerAlive() || !g_pDamageFXMgr->AllowWeapons( ))
		return;

	// Don't allow the enabling of weapons while swimming...
	if( g_pPlayerMgr->IsUnderwater( ) && g_pPlayerMgr->IsSwimmingAllowed( ) )
		return;

	// I'd rather not have to check the consistancy of these two variables,
	// (the only time they are inconsistant is when we are changing weapons)
	// but because of some circular depentancies in the the old weapon code,
	// the playermgr and a few other files, its too involved to fix right now.

	if ( ( CWM_NO_WEAPON != m_nCurClientWeaponIndex ) && ( m_pCurrentWeapon ) )
	{
		ASSERT( CWM_WEAPON_INDEX_IS_VALID( m_nCurClientWeaponIndex ) );
		ASSERT( m_pCurrentWeapon == m_apClientWeapon[ m_nCurClientWeaponIndex ] );

		// enable the current weapon
		// the rest will be enabled/disabled as necessary
		m_pCurrentWeapon->SetDisable( false );
	}

	m_bWeaponsEnabled = true;

	if (m_pCurrentWeapon && !m_pCurrentWeapon->HasAmmo())
	{
		AutoSelectWeapon();
	}
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	void CClientWeaponMgr::DisableWeapons()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::DisableWeapons()
{
	ASSERT( 0 != m_apClientWeapon );

	// disable each weapon
	for ( int i = 0; i < m_nMaxWeapons; ++i )
	{
		if ( m_apClientWeapon[ i ] )
		{
			m_apClientWeapon[ i ]->SetDisable( true );
		}
	}

	m_bWeaponsEnabled = false;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	void CClientWeaponMgr::ShowWeapons()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::ShowWeapons(bool bShadow /* = true */)
{
	// I'd rather not have to check the consistancy of these two variables,
	// (the only time they are inconsistant is when we are changing weapons)
	// but because of some circular depentancies in the the old weapon code,
	// the playermgr and a few other files its too involved to fix right now.

	if ( ( CWM_NO_WEAPON != m_nCurClientWeaponIndex ) && ( m_pCurrentWeapon ) )
	{
		ASSERT( CWM_WEAPON_INDEX_IS_VALID( m_nCurClientWeaponIndex ) );
		ASSERT( m_pCurrentWeapon == m_apClientWeapon[ m_nCurClientWeaponIndex ] );

		m_pCurrentWeapon->SetVisible( true );
	}

	// show the current weapon
	// the rest will be shown/hidden as necessary

	m_bWeaponsVisible = true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	void CClientWeaponMgr::HideWeapons()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::HideWeapons(bool bShadow /* = true */)
{
	ASSERT( 0 != m_apClientWeapon );

	// disable each weapon
	for ( int i = 0; i < m_nMaxWeapons; ++i )
	{
		if ( m_apClientWeapon[ i ] )
		{
			m_apClientWeapon[ i ]->SetVisible( false, bShadow );
		}
	}

	m_bWeaponsVisible = false;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	void CClientWeaponMgr::PauseWeapons()
//
//	PURPOSE:	Pause/UnPause all the weapons.  When in a cinematic we don't 
//				want any selected weapons to be updated.
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::PauseWeapons( bool bPause )
{
	ASSERT( 0 != m_apClientWeapon );

	for( int i = 0; i < m_nMaxWeapons; ++i )
	{
		if( m_apClientWeapon[i] )
		{
			m_apClientWeapon[i]->SetPaused( bPause );
		}
	}

	m_bWeaponsPaused = bPause;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	void CClientWeaponMgr::DeselectCallback()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::DeselectCallback( HWEAPON hWeapon )
{
	ASSERT( m_pCurrentWeapon && (m_pCurrentWeapon->GetWeaponRecord() == hWeapon) );

	// set the current weapon index to NO_WEAPON and 
	// we'll start the switch to the next weapon (if any)
	// during the next update
	m_nCurClientWeaponIndex = CWM_NO_WEAPON;
	m_pCurrentWeapon = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapomMgr::AutoSelectWeapon()
//
//	PURPOSE:	Determine what weapon to switch to, and switch
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::AutoSelectWeapon()
{
	// [KLS 4/25/02] First see if we can just change ammo types...

	if (m_pCurrentWeapon)
	{
		// Get the best new ammo type
		HAMMO hNewAmmo = m_pCurrentWeapon->GetBestAvailableAmmo( );

		if( hNewAmmo )
		{
			m_pCurrentWeapon->ChangeAmmoWithReload( hNewAmmo );
			return;
		}

		// Okay, need to change, find the next weapon
		ChangeToNextRealWeapon();
	}
	else
	{
		// No current weapon so find the best...

		HWEAPON hBestWeapon = NULL;

		uint8 nNumPlayerWeapons = g_pWeaponDB->GetNumPlayerWeapons( );
		for( uint8 nPlayerWeaponIndex = 0; nPlayerWeaponIndex < nNumPlayerWeapons; ++nPlayerWeaponIndex )
		{
			HWEAPON hCurWeapon = g_pWeaponDB->GetPlayerWeapon( nPlayerWeaponIndex );

			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hCurWeapon, !USE_AI_DATA);
			HWEAPON hDualWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rDualWeapon );
			if( hDualWeapon && 
				g_pPlayerStats->HaveWeapon( hDualWeapon ) && 
				g_pWeaponDB->IsBetterWeapon( hDualWeapon, hBestWeapon ))
			{
				hBestWeapon = hDualWeapon;
			}
			else if( g_pPlayerStats->HaveWeapon( hCurWeapon ) && g_pWeaponDB->IsBetterWeapon( hCurWeapon, hBestWeapon ))
				hBestWeapon = hCurWeapon;
		}

		if( hBestWeapon )
			ChangeWeapon( hBestWeapon );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::ChangeToNextRealWeapon()
//
//	PURPOSE:	Change to the next weapon that does damage.
//				(used by the auto weapon switching)
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::ChangeToNextRealWeapon()
{
	// If we're supposed to hide the weapon when it is empty 
	// (i.e., it doesn't make sense to see it) then we don't 
	// want to play the deselect animation...

	if (!m_pCurrentWeapon) return;

	HWEAPON hCurWeapon = m_pCurrentWeapon->GetWeaponRecord();
	if( !hCurWeapon )
		return;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hCurWeapon, !USE_AI_DATA);
	bool bCanDeselect = !g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bHideWhenEmpty );

	// Find the next available weapon on the weapon selection list...
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	int32 nNumPriorities = ( int32 )pProfile->m_vecWeapons.size();

	for( int32 nWeapon = ( nNumPriorities - 1 ); nWeapon >= 0; --nWeapon )
	{
		HWEAPON hWeapon = pProfile->m_vecWeapons[nWeapon];
		if( hWeapon )
		{
			uint8 nWeaponIndex = g_pWeaponDB->GetPlayerWeaponIndex( hWeapon );

			if( CWM_NO_WEAPON != nWeaponIndex )
			{
				HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
				HWEAPON hDualWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rDualWeapon );
				if( hDualWeapon && 
					( g_pPlayerStats->HaveWeapon( hDualWeapon ) ) &&  // have the weapon
					( m_apClientWeapon[ nWeaponIndex ]->HasAmmo() ) )  // weapon has ammo
				{
					ChangeWeapon( hDualWeapon, NULL, -1, sk_bPlaySelect, bCanDeselect);
					return;
				}
				else if ( ( g_pPlayerStats->HaveWeapon( hWeapon ) ) &&  // have the weapon
					 ( m_apClientWeapon[ nWeaponIndex ]->HasAmmo() ) )  // weapon has ammo
				{
					ChangeWeapon( hWeapon, NULL, -1, sk_bPlaySelect, bCanDeselect);
					return;
				}
			}
		}
	}

	//couldn't find any regular weapons... go to melee
	ChangeWeapon( g_pWeaponDB->GetUnarmedRecord( ), NULL, -1, sk_bPlaySelect, bCanDeselect);
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	ClientWeaponMgr::ResetWeapons()
//
//	PURPOSE:	Reset all the weapons
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::ResetWeapons()
{
	ASSERT( 0 != m_apClientWeapon );

	m_hRequestedWeapon	= NULL;
	m_hRequestedAmmo	= NULL;

	// Reset each weapon

	for( int i = 0; i < m_nMaxWeapons; ++i )
	{
		if( m_apClientWeapon[i] )
		{
			m_apClientWeapon[i]->ResetWeapon();
		}
	}

	// Don't have a current weapon any longer.
	m_pCurrentWeapon		= NULL;
	m_nCurClientWeaponIndex	= CWM_NO_WEAPON;
	m_hLastWeapon			= NULL;

	//get grenade for the first slot
	HWEAPON hGrenade = g_pWeaponDB->GetPlayerGrenade(0);
	if (hGrenade)
	{
		g_pPlayerStats->UpdatePlayerGrenade( hGrenade, false );
	}


}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	ClientWeaponMgr::OnPlayerDead()
//
//	PURPOSE:	Handle when the player goes to a dead state...
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::OnPlayerDead()
{
	// See if we are trying to throw a grenade...
	if( (!g_pPlayerMgr->IsSpectating( )) &&
		(g_pInterfaceMgr->GetGameState() == GS_PLAYING)  &&
		(g_pPlayerStats->GetCurrentGrenadeRecord( ) != NULL) &&
		(g_pPlayerStats->GetCurrentGrenadeCount( ) > 0) &&
		(m_pCurrentWeapon) && (m_pCurrentWeapon->GetState() == W_GREN_THROWING))
	{
		// Determine if the grenade may be dropped on death...
		HWEAPONDATA hGrenadeData = g_pWeaponDB->GetWeaponData( g_pPlayerStats->GetCurrentGrenadeRecord( ), !USE_AI_DATA );
		bool bDropGrenade = g_pWeaponDB->GetBool( hGrenadeData, WDB_WEAPON_bDropGrenadeOnDeath );
		
		CClientWeapon *pGrenade = CPlayerBodyMgr::Instance( ).GetGrenadeWeapon( );
		if( pGrenade && bDropGrenade )
		{
			pGrenade->DropGrenade( );
		}
	}

	// Since the player is dead they should no longer fire...

	if( m_pCurrentWeapon )
		m_pCurrentWeapon->ClearFiring();

	DisableWeapons();
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	ClientWeaponMgr::OnPlayerAlive()
//
//	PURPOSE:	Handle when the player comes alive...
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::OnPlayerAlive()
{
	EnableWeapons();
	if( m_hRequestedWeapon )
	{
		// select the requested weapon immediately
		ChangeWeapon( m_hRequestedWeapon, m_hRequestedAmmo, -1, !sk_bPlaySelect, !sk_bPlayDeselect );

		m_hRequestedWeapon = NULL;
		m_hRequestedAmmo = NULL;
	}

	if( m_pCurrentWeapon )
		m_pCurrentWeapon->ClearFiring();



}


// GRENADE PROTOTYPE
WeaponState	CClientWeaponMgr::GetCurrentWeaponState() const
{
	CClientWeapon* pWpn = GetCurrentClientWeapon();

	if (!pWpn) return W_INACTIVE;

	return pWpn->GetState();
}


//-----------------------------------------------------------------------------

HWEAPON CClientWeaponMgr::GetDefaultCollectionTool()
{
	return g_pWeaponDB->GetWeaponRecord(DEFAULT_COLLECTION_TOOL);
}

//-----------------------------------------------------------------------------

void CClientWeaponMgr::SetWeaponLODDistanceBias( float fLODDistBias )
{
	CClientWeapon* pWpn;

	pWpn = GetCurrentClientWeapon();
	if (pWpn) pWpn->SetWeaponLODDistanceBias(fLODDistBias);

	pWpn = GetVisibleCustomWeapon();
	if (pWpn) pWpn->SetWeaponLODDistanceBias(fLODDistBias);

	pWpn = CPlayerBodyMgr::Instance().GetGrenadeWeapon();
	if (pWpn) pWpn->SetWeaponLODDistanceBias(fLODDistBias);
}

//-----------------------------------------------------------------------------

void CClientWeaponMgr::SetWeaponDepthBiasTableIndex( ERenderLayer eRenderLayer )
{
	CClientWeapon* pWpn;

	pWpn = GetCurrentClientWeapon();
	if (pWpn) pWpn->SetWeaponDepthBiasTableIndex(eRenderLayer);

	pWpn = GetVisibleCustomWeapon();
	if (pWpn) pWpn->SetWeaponDepthBiasTableIndex(eRenderLayer);

	pWpn = CPlayerBodyMgr::Instance().GetGrenadeWeapon();
	if (pWpn) pWpn->SetWeaponDepthBiasTableIndex(eRenderLayer);
}

