
// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponMgr.h
//
// PURPOSE : Manager of client-side weapons
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponMgr_h_INCLUDED_
#define  _ClientWeaponMgr_h_INCLUDED_


#include "ClientWeaponBase.h"


//
// Typedefs
//

// Perhaps this should be declared in the this file, but
// to avoid creating header files that depend on each other
// (the IClientWeaponBase class should have no clue what a
// CClientWeaponMgr actually is) I'm sticking it here and
// placing a commented out version in the other header file.
//
// typedef void (*ClientWeaponCallBackFn)( int nWeaponId, void *pData );


class CClientWeaponMgr
{
public:
	// constructor/destructor
	CClientWeaponMgr();
	~CClientWeaponMgr();

	// standard init/term functions
	bool            Init();
	void            Term();

	// on enter/exit world functionality
	void            OnEnterWorld();
	void            OnExitWorld();

	// handle messages for the mgr and pass messages to the weapons
	bool            OnMessage( uint8 messageId, ILTMessage_Read *pMsg );
	bool            OnModelKey( HLOCALOBJ hObj, ArgList *pArgs );

	// load/save functionality
	void            Load( ILTMessage_Read *pMsg );
	void            Save( ILTMessage_Write *pMsg );

	// main update function
	WeaponState     Update( LTRotation const &rRot, LTVector const &vPos,
	                        bool bFire, FireType eFireType = FT_NORMAL_FIRE );

	// accessor functions
	IClientWeaponBase  *GetCurrentClientWeapon() const;
	uint8           GetCurrentWeaponId() const;
	uint8           GetNextWeaponId( uint8 nWeapon, uint8 nClass ) const { return GetSequentialWeaponId(nWeapon, nClass, true); }
	uint8           GetPrevWeaponId( uint8 nWeapon, uint8 nClass ) const { return GetSequentialWeaponId(nWeapon, nClass, false); }
	uint8           GetSequentialWeaponId( uint8 nWeapon, uint8 nClass, bool bNext ) const;

	bool            CanChangeToWeapon( uint8 nWeaponId );

	// Do NOT call ChangeWeapon, LastWeapon or ToggleHolster directly.  Call
	// PlayerMgr's corresponding functions (conviently named the same)
	// because it will also handle things outside the scope of the client 
	// weapon system, like zoom changes and screen overlays.
	void            ToggleHolster( bool bPlayDeselet = true );
	bool            ChangeWeapon( uint8 nWeaponId,
	                              uint8 nAmmoId = WMGR_INVALID_ID,
	                              int nAmmoAmount = -1,
	                              bool bPlayDeselect = true );
	void			LastWeapon();

	// When the weapon is holstered, draw this weapon as the default.
	bool            SetDefaultWeapon( uint8 nWeaponId = 0 );


	// Enable/Disable all the weapons
	void            EnableWeapons();
	void            DisableWeapons();
	bool            WeaponsEnabled() const { return m_bWeaponsEnabled; }

	// Hide/Show all the weapons
	void            ShowWeapons();
	void            HideWeapons();
	bool            WeaponsVisible() const { return m_bWeaponsVisible; }
	
	// Pause/UnPause all weapons
	void			PauseWeapons( bool bPause );
	
	// Do NOT call these directly, they are for the ClientWeapon to
	// callback when its done with a deselect (so we know when to change
	// weapons).
	static void     CallbackHook( int nWeaponId, void *pData );
	void            DeselectCallback( int nWeaponId );
	
	void			ResetWeapons();

	// Change to the next logical ammo type / weapon (called by ClientWeapon)
	void            AutoSelectWeapon();
	
	// Functionality for when the player changes state to dead/dying or alive
	void			OnPlayerDead();
	void			OnPlayerAlive();

private:
	//
	// Private routines
	//		
	void			ChangeToNextRealWeapon();

	// class utilities
	uint8           IndexToWeaponId( int iIndex ) const;
	int             WeaponIdToIndex( uint8 nWeaponId ) const;

	// array of pointers to the client weapons
	IClientWeaponBase  **m_apClientWeapon;

	// maximum number of weapons
	int              m_nMaxWeapons;

	// this is an index into m_apClientWeapon, NOT a weapon id!
	int              m_iCurrentWeapon;

	// easier to write, should ALWAYS match the m_nCurrentWeapon index
	IClientWeaponBase   *m_pCurrentWeapon;

	// true if all weapons are enabled
	bool             m_bWeaponsEnabled;

	// true if all weapons are visible
	bool             m_bWeaponsVisible;

	// default weapon to switch to when holstering weapons
	// or there is nothing else to switch to
	int              m_nDefaultWeaponId;

	// weapon id that is currently holstered
	int              m_nHolsterWeaponId;

	// weapon id that we will switch to
	int              m_nRequestedWeaponId;
	int              m_nRequestedAmmoId;

	// id of last weapon that we were using
	int              m_nLastWeaponId;

	bool			m_bWeaponsPaused;

};


#endif //_ClientWeaponMgr_h_INCLUDED_
