
// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponMgr.cpp
//
// PURPOSE : Manager of client-side weapons
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "PlayerMgr.h"
#include "PlayerStats.h"
#include "MsgIDs.h"
#include "ClientWeaponMgr.h"
#include "ClientWeaponAllocator.h"

//
// Defines
//

#define CWM_NO_WEAPON -1

// checks if weapon is in range, the pointer array exists
// and the pointer is valid
#define CWM_WEAPON_INDEX_IS_VALID( _wep ) \
	( ( ( 0 <= ( _wep ) ) && ( ( _wep ) < m_nMaxWeapons ) ) && \
	( ( 0 != m_apClientWeapon ) && ( 0 != m_apClientWeapon[ ( _wep ) ] ) ) )


//
// Static functions
//
void CClientWeaponMgr::CallbackHook( int nWeaponId, void *pData )
{
	ASSERT( 0 != pData );
	CClientWeaponMgr *pClientWeaponMgr = static_cast< CClientWeaponMgr* >( pData );
	pClientWeaponMgr->DeselectCallback( nWeaponId );
}

//
// Globals...
//

VarTrack	g_vtKeepCurrentAmmo;


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::CClientWeaponMgr()
//
//	PURPOSE:	Constructor for client weapon manager class	
//
// ----------------------------------------------------------------------- //

CClientWeaponMgr::CClientWeaponMgr():
	  m_apClientWeapon( 0 )
	, m_nMaxWeapons( 0 )
	, m_iCurrentWeapon( CWM_NO_WEAPON )
	, m_pCurrentWeapon( 0 )
	, m_bWeaponsEnabled( true )
	, m_bWeaponsVisible( true )
	, m_nDefaultWeaponId( 0 )  // default to the first weapon
	, m_nHolsterWeaponId( WMGR_INVALID_ID )
	, m_nRequestedWeaponId( WMGR_INVALID_ID )
	, m_nRequestedAmmoId( WMGR_INVALID_ID )
	, m_bWeaponsPaused( false )
{
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
	// loop vars
	int i;
	int nCommandId;
	
	// get first and last weapon indices
	int nFirstWeaponCommandId =g_pWeaponMgr->GetFirstWeaponCommandId();
	int nLastWeaponCommandId = g_pWeaponMgr->GetLastWeaponCommandId();

	// determine m_nMaxWeapons
	// note: add 1 because the first and last are inclusive
	m_nMaxWeapons = nLastWeaponCommandId - nFirstWeaponCommandId + 1;

	// allocate space for our array
	m_apClientWeapon = debug_newa( IClientWeaponBase*, m_nMaxWeapons );
	if( !m_apClientWeapon )
	{
		ASSERT( !"CClientWeaponMgr::Init.  Could not allocate clientweapon array." );
		return false;
	}

	for ( i = 0, nCommandId = nFirstWeaponCommandId;
	      i < m_nMaxWeapons;
	      ++i, ++nCommandId )
	{
		// get the weapon data struct
		uint8 nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);
		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( nWeaponId );
		if( !pWeapon )
		{
			ASSERT( !"CClientWeaponMgr::Init.  Could not get WEAPON data." );
			return false;
		}

		// allocate the appropiate type of client weapon
		CClientWeaponAllocator const *pClientWeaponAllocator = 
		        g_pGameClientShell->GetClientWeaponAllocator();
		m_apClientWeapon[ i ] = pClientWeaponAllocator->New( pWeapon->nClientWeaponType );
		if( !m_apClientWeapon[ i ] )
		{
			ASSERT( !"CClientWeaponMgr::Init.  Could not create clientweapon." );
			return false;
		}

		// init the weapon
		if( !m_apClientWeapon[ i ]->Init( *pWeapon ))
		{
			ASSERT( !"CClientWeaponMgr::Init.  Could not init clientweapon." );
			return false;
		}
	}

	// set the current weapon to t
	m_pCurrentWeapon = 0;
	m_iCurrentWeapon = CWM_NO_WEAPON;
	m_nLastWeaponId = WMGR_INVALID_ID;

	g_vtKeepCurrentAmmo.Init( g_pLTClient, "KeepCurrentAmmo", LTNULL, 1.0f );

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

	m_pCurrentWeapon = LTNULL;
	m_iCurrentWeapon = CWM_NO_WEAPON;
	m_nLastWeaponId = WMGR_INVALID_ID;
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

	m_pCurrentWeapon = LTNULL;
	m_iCurrentWeapon = CWM_NO_WEAPON;
	m_nLastWeaponId = WMGR_INVALID_ID;

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

	m_pCurrentWeapon = LTNULL;
	m_iCurrentWeapon = CWM_NO_WEAPON;
	m_nLastWeaponId = WMGR_INVALID_ID;

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
	// first see if the message is for us
/*
	switch ( messageId )
	{
		// currently we don't have any messages for the ClientWeaponMgr
		default:
		{
		}
		break;
	};
*/

	// otherwise pass the message to the active weapon
	if ( m_pCurrentWeapon )
	{
		return m_pCurrentWeapon->OnMessage( messageId, pMsg );
	}
	else
	{
		return false;
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

bool CClientWeaponMgr::OnModelKey( HLOCALOBJ hObj, ArgList *pArgs )
{
	// pass the model key to the current weapon
	if ( m_pCurrentWeapon )
	{
		ASSERT( CWM_WEAPON_INDEX_IS_VALID( m_iCurrentWeapon ) );
		ASSERT( m_pCurrentWeapon == m_apClientWeapon[ m_iCurrentWeapon ] );

		return m_pCurrentWeapon->OnModelKey( hObj, pArgs );
	}
	else
	{
		return false;
	}
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
	m_nHolsterWeaponId = pMsg->Readuint8();

	// load the request info
	m_nRequestedWeaponId = pMsg->Readuint8();
	m_nRequestedAmmoId = pMsg->Readuint8();

	int nWeapons = (int)pMsg->Readint32();

	for( int i = 0; i < nWeapons; ++i )
	{
		if( LTNULL != m_apClientWeapon[i] )
		{
			m_apClientWeapon[i]->Load( pMsg );
		}
	}

	m_nDefaultWeaponId = pMsg->Readuint8( );
	m_bWeaponsVisible = pMsg->Readbool( );
	m_bWeaponsEnabled = pMsg->Readbool( );
	m_bWeaponsPaused = pMsg->Readbool( );
	m_iCurrentWeapon = pMsg->Readuint8( );

	m_pCurrentWeapon = NULL;

	// Change to a requested weapon/ammo if available, otherwise just go back to what we had.
	int nChangeToWeaponId = ( m_nRequestedWeaponId != WMGR_INVALID_ID ) ? m_nRequestedWeaponId : 
		IndexToWeaponId( m_iCurrentWeapon );

	// Just pass the requested ammo since it will be valid or WMGR_INVALID_ID.
	ChangeWeapon( nChangeToWeaponId, m_nRequestedAmmoId, -1, false );

	//set this after changing weapons
	m_nLastWeaponId = pMsg->Readuint8( );

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
	pMsg->Writeuint8( m_nHolsterWeaponId );

	// requst info
	pMsg->Writeuint8( m_nRequestedWeaponId );
	pMsg->Writeuint8( m_nRequestedAmmoId );

	pMsg->Writeint32( m_nMaxWeapons );

	for( int i = 0; i < m_nMaxWeapons; ++i )
	{
		if( LTNULL != m_apClientWeapon[i] )
		{
			m_apClientWeapon[i]->Save( pMsg );
		}
	}

	pMsg->Writeuint8( m_nDefaultWeaponId );
	pMsg->Writebool( m_bWeaponsVisible );
	pMsg->Writebool( m_bWeaponsEnabled );
	pMsg->Writebool( m_bWeaponsPaused );
	pMsg->Writeuint8( m_iCurrentWeapon );
	pMsg->Writeuint8( m_nLastWeaponId );
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::UpdateWeaponModel()
//
//	PURPOSE:	Update the current weapon
//
// ----------------------------------------------------------------------- //

WeaponState CClientWeaponMgr::Update( LTRotation const &rRot, LTVector const &vPos,
                                      bool bFire, FireType eFireType /* = FT_NORMAL_FIRE */ )
{

	if ( ( CWM_NO_WEAPON == m_iCurrentWeapon ) && ( m_pCurrentWeapon ) )
	{
		// the weapon is out of sync with itself, check calling order
		ASSERT( 0 );
	}

	// update the current weapon 
	WeaponState eWeaponState = W_IDLE;
	if ( m_pCurrentWeapon )
	{
		m_pCurrentWeapon->SetCameraInfo( rRot, vPos );
		eWeaponState = m_pCurrentWeapon->Update( bFire, eFireType );
	}

	// Check to see if we should auto-switch to a new weapon...

	if ( W_AUTO_SWITCH == eWeaponState )
	{
		AutoSelectWeapon();
	}

	// if we received a request to change the weapon during
	// the update, do it now
	if ( CWM_NO_WEAPON == m_iCurrentWeapon )
	{
		// no current weapon, see if we are trying to change
		// to another weapon
		if ( WMGR_INVALID_ID != m_nRequestedWeaponId )
		{
			// select the requested weapon
			ChangeWeapon( m_nRequestedWeaponId, m_nRequestedAmmoId, -1, false );

			m_nRequestedWeaponId = WMGR_INVALID_ID;
			m_nRequestedAmmoId = WMGR_INVALID_ID;
		}
	}

	return eWeaponState;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::GetCurrentWeapon()
//
//	PURPOSE:	Return a pointer to the current weapon
//
// ----------------------------------------------------------------------- //

IClientWeaponBase *CClientWeaponMgr::GetCurrentClientWeapon() const
{
	if ( CWM_NO_WEAPON == m_iCurrentWeapon )
	{
		return 0;
	}

	// we should always have a current weapon
	ASSERT( CWM_WEAPON_INDEX_IS_VALID( m_iCurrentWeapon ) );
	ASSERT( m_pCurrentWeapon == m_apClientWeapon[ m_iCurrentWeapon ] );

	return m_pCurrentWeapon;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeaponMgr::GetCurrentWeaponId()
//
//	PURPOSE:	Return the index of the current weapon
//
// ----------------------------------------------------------------------- //

uint8 CClientWeaponMgr::GetCurrentWeaponId() const
{
	if ( CWM_NO_WEAPON == m_iCurrentWeapon )
	{
		// case where we are changing weapons...
		return WMGR_INVALID_ID;
	}

	// we should always have a current weapon
	ASSERT( CWM_WEAPON_INDEX_IS_VALID( m_iCurrentWeapon ) );

	return m_pCurrentWeapon->GetWeaponId();
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

uint8 CClientWeaponMgr::GetSequentialWeaponId( uint8 nWeapon, uint8 nClass, bool bNext ) const
{
	ASSERT( 0 != g_pPlayerStats );
	ASSERT( 0 != g_pWeaponMgr );

	// check that there are other weapons to try
	if ( 1 >= m_nMaxWeapons )
	{
		return WMGR_INVALID_ID;
	}


	// make sure a valid weapon was specified
	if ( !g_pWeaponMgr->IsValidWeaponId( nWeapon ) )
	{
		if ( CWM_NO_WEAPON == m_iCurrentWeapon )
		{
			// no current weapon, get the first weapon id
			nWeapon = g_pWeaponMgr->GetWeaponId( g_pWeaponMgr->GetFirstWeaponCommandId() );
		}
		else
		{
			// if an invalid weapon is specified, use the current
			nWeapon = IndexToWeaponId( m_iCurrentWeapon );
		}
	}


	// init all the loop indices
	int iMin	= 0;
	int iStart	= WeaponIdToIndex( nWeapon );
	int iCur	= iStart + ( bNext ? 1 : -1 );
	int iMax	= m_nMaxWeapons;
	int iFound	= CWM_NO_WEAPON;


	// wrap the index if necessary
	if ( bNext )
	{
		if ( iCur >= iMax ) iCur = iMin;
	}
	else  // prev
	{
		if ( 0 > iCur ) iCur = iMax - 1;
	}

	// check every weapon until we wrap around to where we started
	while ( iCur != iStart && ( CWM_NO_WEAPON == iFound ))
	{
		uint8 nWeaponId = m_apClientWeapon[ iCur ]->GetWeaponId();
		uint8 nWeaponClass = g_pWeaponMgr->GetWeaponClass(nWeaponId);

		if ( ( g_pPlayerStats->HaveWeapon( nWeaponId ) ) &&  // have the weapon
		     ( m_apClientWeapon[ iCur ]->HasAmmo() ) )  // weapon has ammo
		{
			// potential match

			// if a class was specified, make sure it matches the weapon class
			if ( 0 < nClass )
			{
				if ( nClass == nWeaponClass )
				{
					// class specified and it matches weapon class, its OK
					iFound = iCur;
				}
			}
			else
			{
				// no class specified, its OK
				iFound = iCur;
			}
		}

		// Increment/decrement the weapon counter and check for array bounds...
		if ( bNext )
		{
			++iCur;

			// wrap the index if necessary
			if ( iCur >= iMax ) iCur = iMin;
		}
		else  // Prev
		{
			--iCur;

			// wrap the index if necessary
			if ( 0 > iCur ) iCur = iMax - 1;
		}
	}

	if ( CWM_NO_WEAPON != iFound )
	{
		return m_apClientWeapon[ iFound ]->GetWeaponId();
	}
	else
	{
		return WMGR_INVALID_ID;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::ChangeWeapon()
//
//	PURPOSE:	Change to a different weapon
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::ChangeWeapon( uint8 nWeaponId,
                                     uint8 nAmmoId /*=WMGR_INVALID_ID*/,
                                     int nAmmoAmount /*=-1*/,
                                     bool bPlayDeselect /*=true*/ )
{
	// check to see if weapon is a valid weapon to switch to
	if ( m_pCurrentWeapon && !CanChangeToWeapon( nWeaponId ) )
	{
		return false;
	}

	// assume we'll be changing weapon and ammo
	bool bChangeWeapon = true;
	bool bChangeAmmo = true;

	if ( m_pCurrentWeapon )
	{
		// If there is a current weapon, determine if we
		// have to switch the weapon, the ammo, or both.
		uint8 nCurrentWeaponId = m_pCurrentWeapon->GetWeaponId();
		uint8 nCurrentAmmoId = m_pCurrentWeapon->GetAmmoId();
		
		// Do we need to change the weapon?
		if ( nWeaponId == nCurrentWeaponId )
		{
			bChangeWeapon = false;

			// Make sure a weapon change in progress is aborted...
			if (WMGR_INVALID_ID != m_nRequestedWeaponId)
			{
				m_nRequestedWeaponId = nCurrentWeaponId;
			}
		}

		// Do we need to change the ammo?
		if ( nAmmoId == nCurrentAmmoId )
		{
			bChangeAmmo = false;

			// Make sure a ammo change in progress is aborted...
			if (WMGR_INVALID_ID != m_nRequestedAmmoId)
			{
				m_nRequestedAmmoId = nCurrentAmmoId;
			}
		}
	}

	// if the specified nAmmoId is INVALID, we'll try to find
	// the best ammo to switch to
	int nNewAmmoId = WMGR_INVALID_ID;
	int nNewAmmoAmount = -1;

	// get the best new ammo type
	int iNewWeapon;
	iNewWeapon = WeaponIdToIndex( nWeaponId );
	if ( ( WMGR_INVALID_ID == nAmmoId ) && ( CWM_NO_WEAPON != iNewWeapon ) )
	{
		// If we don't want to keep the current ammo get the highest priority ammo...
		
		if( g_vtKeepCurrentAmmo.GetFloat() < 1.0f )
		{
			m_apClientWeapon[ iNewWeapon ]->GetBestAvailableAmmoId( &nNewAmmoId );
		}
		else
		{
			nNewAmmoId = m_apClientWeapon[iNewWeapon]->GetAmmoId();

			// [KLS 5/9/02] Since we're using the ammo id stored in the weapon, make sure
			// that the weapon actually has ammo associated with this id...If not,
			// change to the best available ammo id...

			if( g_pPlayerStats->GetAmmoCount( nNewAmmoId ) <= 0 )
			{
				m_apClientWeapon[ iNewWeapon ]->GetBestAvailableAmmoId( &nNewAmmoId );
			}
		}
	}
	else
	{
		nNewAmmoId = nAmmoId;
	}

	// get the amount of ammo
	if ( -1 == nAmmoAmount )
	{
		nNewAmmoAmount = g_pPlayerStats->GetAmmoCount( nNewAmmoId );
	}
	else
	{
		nNewAmmoAmount = nAmmoAmount;
	}

	bool bSwitchInstantly = bChangeWeapon;
	if ( bChangeWeapon )
	{
		
		// Handle deselection of current weapon...
		if ( bPlayDeselect )
		{
			if ( CWM_NO_WEAPON != m_iCurrentWeapon )
			{
				// change weapons

				// save the new weapon id.
				m_nRequestedWeaponId = nWeaponId;
				m_nRequestedAmmoId = nNewAmmoId;
				if (m_pCurrentWeapon)
				{
					m_nLastWeaponId = m_pCurrentWeapon->GetWeaponId();
				}

				// deselect the weapon
				bool bResult = m_apClientWeapon[ m_iCurrentWeapon ]->Deselect( CallbackHook, this );
				if ( bResult )
				{
					bSwitchInstantly = false;
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
			if (m_pCurrentWeapon)
			{
				m_nLastWeaponId = m_pCurrentWeapon->GetWeaponId();
			}

			m_iCurrentWeapon = WeaponIdToIndex( nWeaponId );
			m_pCurrentWeapon = m_apClientWeapon[ m_iCurrentWeapon ];
			m_pCurrentWeapon->Activate();
			m_pCurrentWeapon->Select();

			// instantly switch to the new ammo
			m_pCurrentWeapon->ChangeAmmoImmediate( nNewAmmoId , nNewAmmoAmount );
		}
	}
	else if ( bChangeAmmo )
	{
		// don't need to change the weapon, but we do
		// need to change the ammo
		m_pCurrentWeapon->ChangeAmmoWithReload( nNewAmmoId );
	}


	// We succesfully started the weapon change process, so let the player mgr handle
	// it...

	//jrg- 9/2/02 - this is getting called both when we start switching and when we finish switching
	//		PlayerMgr is using this to track if we're in mid switch
	g_pPlayerMgr->HandleWeaponChanged(nWeaponId, nNewAmmoId, (!bChangeWeapon || bSwitchInstantly));


	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDerWeaponModel::CanChangeToWeapon()
//
//	PURPOSE:	See if we can change to this weapon
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::CanChangeToWeapon( uint8 nWeaponId )
{
	ASSERT( 0 != g_pPlayerMgr );
	ASSERT( 0 != g_pPlayerStats );

	// make sure the weapon is valid
	if ( WMGR_INVALID_ID == nWeaponId )
	{
		return false;
	}

	// no changing if the player is dead or in spectator mode
	if ( g_pPlayerMgr->IsPlayerDead() || g_pPlayerMgr->IsSpectatorMode() )
	{
		return false;
	}

	// make sure we have data for this weapon
	int iWeapon = WeaponIdToIndex( nWeaponId );
	if ( CWM_NO_WEAPON == iWeapon )
	{
		return false;
	}

	// Make sure this is a valid weapon for us to switch to...
	if ( !g_pPlayerStats->HaveWeapon( nWeaponId ) )
	{
		return false;
	}

	// If this weapon has no ammo, let user know...
	if ( !m_apClientWeapon[ iWeapon ]->HasAmmo() )
	{
		// this should be handled elsewhere already
		//g_pInterfaceMgr->UpdatePlayerStats( IC_OUTOFAMMO_ID, nWeaponId, 0, 0.0f );
//		ASSERT( 0 );
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::SetDefaultWepon()
//
//	PURPOSE:	Set the default weapon
//
// ----------------------------------------------------------------------- //

bool CClientWeaponMgr::SetDefaultWeapon( uint8 nWeaponId )
{
	if( !g_pWeaponMgr->IsPlayerWeapon( nWeaponId ))
		return false;

	m_nDefaultWeaponId = nWeaponId;
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
	if (!WeaponsEnabled()) return;

	// [kml] 3/26/02 Sanity check because we entered a world
	// without a current weapon
	if( m_pCurrentWeapon )
	{
		// when bPlayDeselect == FALSE, ToggleHolster will force 
		// the weapon to change without the deselect animation
		if ( m_nDefaultWeaponId == m_pCurrentWeapon->GetWeaponId() )
		{
			// the current weapon is the default, get out the holstered weapon
			ChangeWeapon( m_nHolsterWeaponId, WMGR_INVALID_ID, -1, bPlayDeselect );
			m_nHolsterWeaponId = WMGR_INVALID_ID;
		}
		else
		{
			// put the current weapon away and switch to the default
			m_nHolsterWeaponId = m_pCurrentWeapon->GetWeaponId();
			ChangeWeapon( m_nDefaultWeaponId, WMGR_INVALID_ID, -1, bPlayDeselect );
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

	ChangeWeapon( m_nLastWeaponId, WMGR_INVALID_ID, -1, true );
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
	// Don't allow the enabling of weapons while dead or carrying a body...

	if( g_pPlayerMgr->IsCarryingHeavyObject() || g_pPlayerMgr->IsPlayerDead() || 
		!g_pDamageFXMgr->AllowWeapons( ))
		return;

	// I'd rather not have to check the consistancy of these two variables,
	// (the only time they are inconsistant is when we are changing weapons)
	// but because of some circular depentancies in the the old weapon code,
	// the playermgr and a few other files, its too involved to fix right now.

	if ( ( CWM_NO_WEAPON != m_iCurrentWeapon ) && ( m_pCurrentWeapon ) )
	{
		ASSERT( CWM_WEAPON_INDEX_IS_VALID( m_iCurrentWeapon ) );
		ASSERT( m_pCurrentWeapon == m_apClientWeapon[ m_iCurrentWeapon ] );

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

void CClientWeaponMgr::ShowWeapons()
{
	// I'd rather not have to check the consistancy of these two variables,
	// (the only time they are inconsistant is when we are changing weapons)
	// but because of some circular depentancies in the the old weapon code,
	// the playermgr and a few other files its too involved to fix right now.

	if ( ( CWM_NO_WEAPON != m_iCurrentWeapon ) && ( m_pCurrentWeapon ) )
	{
		ASSERT( CWM_WEAPON_INDEX_IS_VALID( m_iCurrentWeapon ) );
		ASSERT( m_pCurrentWeapon == m_apClientWeapon[ m_iCurrentWeapon ] );

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

void CClientWeaponMgr::HideWeapons()
{
	ASSERT( 0 != m_apClientWeapon );

	// disable each weapon
	for ( int i = 0; i < m_nMaxWeapons; ++i )
	{
		if ( m_apClientWeapon[ i ] )
		{
			m_apClientWeapon[ i ]->SetVisible( false );
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

void CClientWeaponMgr::DeselectCallback( int nWeaponId )
{
	ASSERT( CWM_WEAPON_INDEX_IS_VALID( WeaponIdToIndex( nWeaponId ) ) );
	ASSERT( m_pCurrentWeapon && (m_pCurrentWeapon->GetWeaponId() == nWeaponId) );

	// set the current weapon index to NO_WEAPON and 
	// we'll start the switch to the next weapon (if any)
	// during the next update
	m_iCurrentWeapon = CWM_NO_WEAPON;
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
		int nNewAmmoId = WMGR_INVALID_ID;
		m_pCurrentWeapon->GetBestAvailableAmmoId( &nNewAmmoId );

		if (WMGR_INVALID_ID != nNewAmmoId)
		{
			m_pCurrentWeapon->ChangeAmmoWithReload( nNewAmmoId );
			return;
		}

		// Okay, need to change, find the next weapon
		ChangeToNextRealWeapon();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponMgr::ChangeToNextRealWeapon()
//
//	PURPOSE:	Change to the next weapon/gadget that does damage.
//				(used by the auto weapon switching)
//
// ----------------------------------------------------------------------- //

void CClientWeaponMgr::ChangeToNextRealWeapon()
{
	// If we're supposed to hide the weapon when it is empty 
	// (i.e., it doesn't make sense to see it) then we don't 
	// want to play the deselect animation...

	if (!m_pCurrentWeapon) return;

	const WEAPON* pCurWeapon = m_pCurrentWeapon->GetWeapon();
	if (!pCurWeapon) return;

	bool bCanDeselect = !pCurWeapon->bHideWhenEmpty;

	// Find the next available weapon on the weapon selection list...

	uint8 nWeaponPriorities[254]; 
	int nNumPriorities = g_pWeaponMgr->GetWeaponPriorities(nWeaponPriorities, ARRAY_LEN(nWeaponPriorities));

	for (uint8 i = 0; i < nNumPriorities; i++)
	{
		uint8 nWeaponId = nWeaponPriorities[i];
		if (WMGR_INVALID_ID != nWeaponId)
		{
			uint8 iCur = WeaponIdToIndex(nWeaponId);

			if (CWM_NO_WEAPON != iCur)
			{
				if ( ( g_pPlayerStats->HaveWeapon( nWeaponId ) ) &&  // have the weapon
					 ( m_apClientWeapon[ iCur ]->HasAmmo() ) )  // weapon has ammo
				{
					ChangeWeapon(nWeaponId, WMGR_INVALID_ID, -1, bCanDeselect);
					return;
				}
			}
		}
	}

/* [KLS 8/26/02] Old class-based system...Just use the hardcoded list now...

	// First try to switch to the next available weapon in the current class...

	int nCurClass = g_pWeaponMgr->GetWeaponClass(pCurWeapon->nId);

	uint8 nMatch = GetNextWeaponId(pCurWeapon->nId, nCurClass);
	if (WMGR_INVALID_ID != nMatch)
	{
		ChangeWeapon(nMatch, WMGR_INVALID_ID, -1, bCanDeselect);
        return;
	}


	// Okay, no available weapons in the current class...switch based on
	// our class priority list...

	uint8 nClassPriorities[10]; 
	int nNumPriorities = g_pWeaponMgr->GetWeaponClassPriorities(nClassPriorities, ARRAY_LEN(nClassPriorities));

	for (int i=0; i < nNumPriorities; i++)
	{
		// Don't bother trying our current class, we already tried that...

		if (nClassPriorities[i] != nCurClass)
		{
			nMatch = GetNextWeaponId(pCurWeapon->nId, nClassPriorities[i]);
			
			if (WMGR_INVALID_ID != nMatch)
			{
				// Switch to this weapon...
				ChangeWeapon(nMatch, WMGR_INVALID_ID, -1, bCanDeselect);
				return;
			}		
		}
	}
*/

}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	ClientWeaponMgr::IndexToWeaponId()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

uint8 CClientWeaponMgr::IndexToWeaponId( int iIndex ) const
{
	if ( CWM_NO_WEAPON == m_iCurrentWeapon )
	{
		return WMGR_INVALID_ID;
	}

	ASSERT( CWM_WEAPON_INDEX_IS_VALID( iIndex ) );

	return m_apClientWeapon[ iIndex ]->GetWeaponId();
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	ClientWeaponMgr::WeaponIdToIndex()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

int CClientWeaponMgr::WeaponIdToIndex( uint8 nWeaponId ) const
{
	int iWeapon;
	for ( iWeapon = 0; iWeapon < m_nMaxWeapons; ++iWeapon )
	{
		// find the specified weapon
		if ( nWeaponId == m_apClientWeapon[ iWeapon ]->GetWeaponId() )
		{
			break;
		}
	}

	if ( iWeapon >= m_nMaxWeapons )
	{
		// couldn't find the weapon to change to
		return CWM_NO_WEAPON;
	}
	else
	{
		return iWeapon;
	}
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

	m_nRequestedWeaponId = WMGR_INVALID_ID;
	m_nRequestedAmmoId = WMGR_INVALID_ID;

	// Reset each weapon

	for( int i = 0; i < m_nMaxWeapons; ++i )
	{
		if( m_apClientWeapon[i] )
		{
			m_apClientWeapon[i]->ResetWeapon();
		}
	}

	// Don't have a current weapon any longer.
	m_pCurrentWeapon = 0;
	m_iCurrentWeapon = CWM_NO_WEAPON;
	m_nLastWeaponId = WMGR_INVALID_ID;
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
}