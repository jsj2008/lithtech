// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponBase.h
//
// PURPOSE : Generic interface for the client-side weapon 
//
// CREATED : 4/09/02
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef  _ClientWeaponBase_h_INCLUDED_
#define  _ClientWeaponBase_h_INCLUDED_


//
// Includes
//

#include "WeaponMgr.h"


//
// Typedefs
//

// Perhaps this should be in the CClientWeaponMgr.h file, but
// to avoid creating header files that depend on each other
// (and in all actuality this class should have no clue what
// a CClientWeaponMgr actually is) I'm sticking it here and
// placing a commented out version in the other header file.
typedef void (*ClientWeaponCallBackFn)( int nWeaponId, void *pData );


//
// Enumerations
//
enum FireType
{
	FT_NORMAL_FIRE=0,

	// this appears to have been broken for quite some time...
	FT_ALT_FIRE,
};


///
// class IClientWeaponBase
//
class IClientWeaponBase
{
public:

	// This pure virtual destructor maintains this class
	// as an abstract class.  It should not be instantiated,
	// but all public functions can have default behaviours.
	virtual       ~IClientWeaponBase() = 0 {}
	IClientWeaponBase() {}

	// handle messages
	virtual bool  OnMessage( uint8 messageID, ILTMessage_Read *pMsg ) { return true; }

	// callbacks from the animation system
	virtual bool  OnModelKey( HLOCALOBJ hObj, ArgList* pArgs ) { return true; }

	// enter/exit world functionality
	virtual void          OnEnterWorld() {}
	virtual void          OnExitWorld() {}

	// standard init/term functions
	virtual bool          Init( WEAPON const &pWeapon ) { return true; }
	virtual void          Term() {}

	// main update function
	virtual WeaponState   Update( bool bFire, FireType eFireType = FT_NORMAL_FIRE ) { return W_IDLE; }

	// external interfaces to change the ammo
	virtual void          ChangeAmmoWithReload( uint8 nNewAmmoId, bool bForce= false ) {}
	virtual void          ChangeAmmoImmediate( uint8 nNewAmmoId, int nAmmoAmount = -1, bool bForce= false ) {}
	virtual void          ReloadClip( bool bPlayReload = true, int nNewAmmo=-1, bool bForce = false, bool bNotifyServer = false ) {}
	virtual void          DecrementAmmo() {}

	// set the information about the camera that the weapon needs to know
	virtual void          SetCameraInfo( LTRotation const &rCamRot, LTVector const &vCamPos ) {}

	// Update the weapon model filenames, particularly the skins.
	// This is for cases where the base player model changes and 
	// the playerview model needs to change too (i.e. sleeves
	// look different on new player model).
	virtual void          ResetWeaponFilenames() {}

	// to update the weapon bob
	virtual void          UpdateBob( LTFLOAT fWidth, LTFLOAT fHeight ) {}

	// dynanic perturb
	virtual LTFLOAT       GetDynamicPerturb()  const { return 0.0f; }

	// Do NOT call these functions directly.  If you want to
	// enable/disable or show/hide the weapons, call the functions
	// in CClientWeaponMgr.
	virtual void          SetVisible( bool bVis = true ) {}
	virtual void          SetDisable( bool bDisable = true ) {}

	// Do NOT call these directly.  They are used CClientWeaponMgr.
	virtual void          Select() {}
	virtual bool          Deselect( ClientWeaponCallBackFn cbFn, void *pData ) { return false; }
	virtual bool          Activate() { return false; }   // Create the weapon's resources
	virtual void          Deactivate() {} // Shut down the weapon (destroy resources)

	// weapon info
	virtual WEAPON const *GetWeapon()       const { return 0; }
	virtual int           GetWeaponId()     const { return 0; }

	// ammo info
	virtual AMMO const   *GetAmmo()         const { return 0; }
	virtual int           GetAmmoId()       const { return WMGR_INVALID_ID; }
	virtual int           GetAmmoInClip()   const { return WMGR_INVALID_ID; }
	virtual bool          HasAmmo() const { return true; }
	virtual bool          CanUseAmmo( uint8 nAmmoId ) const { return false; }
	virtual bool          CanChangeToAmmo( uint8 nAmmoId ) const { return false; };
	virtual uint8         GetNextAvailableAmmo( uint8 nGivenAmmoId = WMGR_INVALID_ID ) { return WMGR_INVALID_ID; };
	virtual bool          GetBestAvailableAmmoId( int *nAmmoId ) const { *nAmmoId = WMGR_INVALID_ID; return false; };

	// state info
	virtual WeaponState   GetState()        const { return W_IDLE; }

	// misc functions
	virtual bool          IsMeleeWeapon() const { return false; }
	virtual bool          IsFireButtonDown() const { return false; }
	virtual void          GetShellEjectPos( LTVector *vOriginalPos ) { vOriginalPos->Init( 0.0f, 0.0f, 0.0f ); }
	virtual bool          WeaponNeedsCrosshair() const { return false; }

	// used with the cheat codes to adjust the proper position of the weapon
	virtual LTVector      GetMuzzleOffset() const { return LTVector( 0.0f, 0.0f, 0.0f ); }

	virtual void          SetMuzzleOffset( LTVector const &v ) {}
	
	virtual LTVector      GetBreachOffset() const { return LTVector( 0.0f, 0.0f, 0.0f ); }

	virtual void          SetBreachOffset( LTVector const &v ) {}
	
	virtual LTVector      GetWeaponOffset() const { return LTVector( 0.0f, 0.0f, 0.0f ); }
	
	virtual void          SetWeaponOffset( LTVector const &v ) {}

	// This interface is only used for the camera shake.  This
	// is privledged information, and the code to shake the
	// player view weapon should eventually be moved to this class.
	// Please don't make use of this function, and if you do
	// comment its use here.
	virtual HLOCALOBJ     GetHandle() const { return LTNULL; }

	// This needs to be exposed for the PlayerStats.  Eventually
	// this should be moved so creation/deletion of the mods
	// are transparent to the outside system (not all games
	// need mods).
	virtual void          CreateMods() {}

	// load/save functionality
	virtual void          Load( ILTMessage_Read *pMsg ) {}
	virtual void          Save( ILTMessage_Write *pMsg ) {}

	virtual void		  ResetWeapon() {}
	
	virtual void		SetPaused( bool bPaused ) {}
	
	virtual void		ClearFiring() {}
};



#endif //_ClientWeaponBase_h_INCLUDED_