
// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponMgr.h
//
// PURPOSE : Manager of client-side weapons
//
// (c) 2002-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_WEAPON_MGR_H__
#define __CLIENT_WEAPON_MGR_H__


#include "ClientWeapon.h"

extern CClientWeaponMgr *g_pClientWeaponMgr;

class CClientWeaponMgr
{
public:	// Methods...

	// constructor/destructor
	CClientWeaponMgr();
	~CClientWeaponMgr();

	// standard init/term functions
	bool			Init();
	void			Term();

	// on enter/exit world functionality
	void			OnEnterWorld();
	void			OnExitWorld();

	// handle messages for the mgr and pass messages to the weapons
	bool			OnMessage( uint8 messageId, ILTMessage_Read *pMsg );
	bool			OnModelKey( HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList *pArgs );

	bool			OnCommandOn( int nCmd );

	// load/save functionality
	void			Load( ILTMessage_Read *pMsg );
	void			Save( ILTMessage_Write *pMsg );

	// main update function
	WeaponState		Update( LTRotation const &rRot, LTVector const &vPos );

	// GRENADE PROTOTYPE
	WeaponState		GetCurrentWeaponState() const;

	// accessor functions
	CClientWeapon	*GetClientWeapon( HWEAPON hDesiredWeapon ) const;
	CClientWeapon	*GetCurrentClientWeapon() const;
	HWEAPON			GetNextWeaponRecord( HWEAPON hWeapon ) const { return GetSequentialWeaponRecord( hWeapon, true ); }
	HWEAPON			GetPrevWeaponRecord( HWEAPON hWeapon ) const { return GetSequentialWeaponRecord( hWeapon, false ); }
	HWEAPON			GetSequentialWeaponRecord(  HWEAPON hWeapon, bool bNext ) const;
	HWEAPON			GetRequestedWeaponRecord( ) const { return m_hRequestedWeapon; }

	bool			CanChangeToWeapon( HWEAPON hWeapon );
	bool			CanLastWeapon( HWEAPON hWeapon );

	static const bool sk_bPlaySelect = true;
	static const bool sk_bPlayDeselect = true;
	bool			ChangeWeapon( HWEAPON hWeapon, HAMMO hAmmo = NULL, 	int nAmmoAmount = -1, bool bPlaySelect = true, bool bPlayDeselect = true );
	void			ToggleHolster( bool bPlayDeselet = true );
	void			LastWeapon();

	// When the weapon is holstered, draw this weapon as the default.
	bool			SetDefaultWeapon( HWEAPON hWeapon /* = NULL */ );
	HWEAPON			GetDefaultWeapon() { return m_hDefaultWeapon;}


	// Enable/Disable all the weapons
	void			EnableWeapons();
	void			DisableWeapons();
	bool			WeaponsEnabled() const { return m_bWeaponsEnabled; }

	// Hide/Show all the weapons
	void			ShowWeapons(bool bShadow = true);
	void			HideWeapons(bool bShadow = true);
	bool			WeaponsVisible() const { return m_bWeaponsVisible; }
	
	// Pause/UnPause all weapons
	void			PauseWeapons( bool bPause );
	
	// Do NOT call these directly, they are for the ClientWeapon to
	// callback when its done with a deselect (so we know when to change
	// weapons).
	static void		CallbackHook( HWEAPON hWeapon, void *pData );
	void			DeselectCallback( HWEAPON hWeapon );
	
	void			ResetWeapons();

	// Change to the next logical ammo type / weapon (called by ClientWeapon)
	void			AutoSelectWeapon();

	// Custom weapon support (eg: forensics and other off-handed weapons, etc.)
	void			InitCustomWeapons();
	void			UpdateCustomWeapon();
	void			ShowCustomWeapon( CClientWeapon* pWeapon, const char* pszStimulusGroup );
	void			HideCustomWeapon();
	CClientWeapon *	GetVisibleCustomWeapon() const;

	void			SelectCustomWeapon(HWEAPON hWeapon);
	void			DeselectCustomWeapon();
	const char*		GetCustomWeaponDeselectStimulus();

	// Default collection tool to use if no specific forensic tools are specified
	HWEAPON			GetDefaultCollectionTool();

	// Set the LOD distance bias on the visible weapons...
	void			SetWeaponLODDistanceBias( float fLODDistBias );
	void			SetWeaponDepthBiasTableIndex( ERenderLayer eRenderLayer );

	// Functionality for when the player changes state to dead/dying or alive
	void			OnPlayerDead();
	void			OnPlayerAlive();

	
private:	// Methods...
	
	void			ChangeToNextRealWeapon();

	// Message handlers...
	void			HandleMsgWeaponChange( ILTMessage_Read *pMsg );
	void			HandleMsgLastWeapon( ILTMessage_Read *pMsg );
	void			HandleWeaponBreakWarning( ILTMessage_Read *pMsg );
	
private:	// Members...

	// array of pointers to the client weapons
	CClientWeapon	**m_apClientWeapon;

	// maximum number of weapons
	uint8			m_nMaxWeapons;

	// this is an index into m_apClientWeapon, NOT a weapon id!
	uint32			m_nCurClientWeaponIndex;

	// easier to write, should ALWAYS match the m_nCurrentWeapon index
	CClientWeapon	*m_pCurrentWeapon;

	// true if all weapons are enabled
	bool			m_bWeaponsEnabled;

	// true if all weapons are visible
	bool			m_bWeaponsVisible;

	// Default weapon to switch to when holstering weapons
	// or there is nothing else to switch to...
	HWEAPON			m_hDefaultWeapon;

	// Weapon that is currently holstered...
	HWEAPON			m_hHolsterWeapon;

	// Weapon that we will switch to...
	HWEAPON			m_hRequestedWeapon;

	// Last weapon that we were using...
	HWEAPON			m_hLastWeapon;

	// Ammo that we will switch to...
	HAMMO			m_hRequestedAmmo;

	bool			m_bWeaponsPaused;

	// custom weapon (forensics & off-handed weapons)
	CClientWeapon*	m_pVisibleCustomWeapon;
	std::string		m_sCustomStimulusGroup;
	uint32			m_dwLastCustomWeaponContextAnim;

};


#endif //__CLIENT_WEAPON_MGR_H__
