// ----------------------------------------------------------------------- //
//
// MODULE  : Weapon.h
//
// PURPOSE : Weapon class - definition
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "basedefs_de.h"
#include "serverobj_de.h"
#include "Projectile.h"
#include "Weapondefs.h"
#include "DamageTypes.h"
#include "RiotObjectUtilities.h"
#include "ModelFuncs.h"

class CServerDE;
class CPVWeaponModel;

#define W_SOUND_RADIUS	2000.0f


class CWeapon
{
	public :

		CWeapon();
		virtual ~CWeapon();

		DBOOL Init(HOBJECT hObj, ModelSize eSize=MS_NORMAL);

		void Aquire() { m_bHave = DTRUE; }
		void Drop()	  { m_bHave = DFALSE; }
		DBOOL Have() const { return m_bHave; }

		void HandleStateChange(HMESSAGEREAD hMessage);
		
		DBOOL IsZoomed()			const { return m_bIsZoomed; }
		DBOOL CanLockOnTarget()		const { return m_bCanLockOnTarget; }

		DVector GetLastFirePath()	const { return m_vLastFirePath; }
		DFLOAT	GetDamage()			const; 
		DFLOAT	GetRadius()			const { return (DFLOAT)GetWeaponDamageRadius(m_nId, m_eModelSize); }
		ModelSize GetSize()			const { return m_eModelSize; }
		RiotWeaponId GetId()		const { return m_nId; }
		DFLOAT	GetRange()			const { return GetWeaponRange(m_nId) * m_fRangeAdjust; }
		DFLOAT	GetLifeTime()		const { return m_fLifeTime < 0.0f ? GetWeaponLifeTime(m_nId) : m_fLifeTime; }
		DamageType GetDamageType()	const { return m_eDamageType; }
		DFLOAT	GetDamageFactor()	const { return m_fDamageFactor; }
		DFLOAT	GetRangeAdjust()	const { return m_fRangeAdjust; }

		DFLOAT	GetMinFireDuration() const { return m_fMinFireDuration; }
		DFLOAT	GetMaxFireDuration() const { return m_fMaxFireDuration; }		
		DFLOAT	GetMinFireRest()	 const { return m_fMinFireRest;	}		
		DFLOAT	GetMaxFireRest()	 const { return m_fMaxFireRest;	}	

		void	SetSize(ModelSize eSize)		{ m_eModelSize = eSize; }
		void	SetMinFireDuration(DFLOAT fVal) { m_fMinFireDuration = fVal; }
		void	SetMaxFireDuration(DFLOAT fVal) { m_fMaxFireDuration = fVal; }		
		void	SetMinFireRest(DFLOAT fVal)	    { m_fMinFireRest = fVal; }		
		void	SetMaxFireRest(DFLOAT fVal)     { m_fMaxFireRest = fVal; }	
		void	SetDamageFactor(DFLOAT fVal)	{ m_fDamageFactor = fVal; }
		void	SetRangeAdjust(DFLOAT fVal)		{ m_fRangeAdjust = fVal; }
		void	SetLifetime(DFLOAT fLife)		{ m_fLifeTime = fLife; }
		void	SetCanLockOnTarget(DBOOL b)		{ m_bCanLockOnTarget = b; }
		void	SetZoom(DBOOL b)				{ m_bIsZoomed = b; }

		HOBJECT GetModelObject();
		void SetModelObject(HOBJECT hObj);

		virtual void InitAnimations();

		char* GetHandWeaponName()		const { return m_pHandWeaponName; }
		char* GetHandWeaponSkin()		const { return m_pHandWeaponSkin; }

		DVector GetHandModelFlashOffset();

		void AddAmmo(int nAmount);
		void SetAmmo(int nAmount);

		int GetAmmoCount() { return m_nAmmo; }
		int GetMaxAmmo( ) { return GetWeaponMaxAmmo(m_nId); }

		virtual WeaponState UpdateWeapon(HOBJECT hFiredFrom, DVector path, 
										 DVector vFirePos, DBOOL bFire, DBOOL bZoomed);
		
		void WriteClientInfo(HMESSAGEWRITE hMessage);

		// Weapon model calls this to actually fire the weapon (i.e., on
		// next call to UpdateWeapon() with bFire == TRUE, Fire(...) gets called...

		virtual DBOOL Fire() { m_bFire = DTRUE; return DTRUE; }
		DBOOL	ShotPending() const { return m_bFire; }

		virtual void Deselect();
		virtual void Select();

		virtual WeaponState Fire(HOBJECT hFiredFrom, DVector path, 
							     DVector vFirePos, DBYTE nRandomSeed=0);

		void Save(HMESSAGEWRITE hWrite, DBYTE nType);
		void Load(HMESSAGEREAD hRead, DBYTE nType);

		WeaponState GetState() const { return m_eState; }
		HOBJECT GetObject() const { return m_hObject; }

		void CacheFiles( );

	protected : 

		virtual DBOOL PlaySelectAnimation();
		virtual DBOOL PlayDeselectAnimation();
		virtual DBOOL PlayStartFireAnimation();
		virtual DBOOL PlayFireAnimation();
		virtual DBOOL PlayReloadAnimation();
		virtual DBOOL PlayStopFireAnimation();
		virtual DBOOL PlayIdleAnimation();

		virtual void UpdateFiring();
		virtual void UpdateNonFiring();
		virtual CProjectile* CreateProjectile(ObjectCreateStruct & theStruct);


		HOBJECT		m_hObject;			// Who owns us
		HOBJECT		m_hModelObject;		// Weapon Model

		ModelSize	m_eModelSize;		// Size of weapon (relative to player)
		DBOOL		m_bCanIdle;			// Can we play idle animations
		DBOOL		m_bIsZoomed;		// Are we zoomed in?

		HMODELANIM	m_nSelectAni;		// Select weapon
		HMODELANIM	m_nDeselectAni;		// Deselect weapon
		HMODELANIM	m_nStartFireAni;	// Start firing
		HMODELANIM	m_nFireAni;			// Fire
		HMODELANIM	m_nFireAni2;		// Fire2
		HMODELANIM	m_nFireZoomAni;		// Fire when zoomed in
		HMODELANIM	m_nLastFireAni;		// What fire ani was played last
		HMODELANIM	m_nStopFireAni;		// Stop firing
		HMODELANIM	m_nIdleAni1;		// Idle one
		HMODELANIM	m_nIdleAni2;		// Idle two
		HMODELANIM	m_nReloadAni;		// Reload weapon
	
		DFLOAT	m_fLastIdleTime;		// Last idle animation time
		DFLOAT	m_fTimeBetweenIdles;	// Current time between idle animations
	
		DFLOAT	m_fMinFireDuration;		// Minimum time we fire our weapon
		DFLOAT	m_fMaxFireDuration;		// Maximum time we fire our weapon
		DFLOAT	m_fMinFireRest;			// Minimum time between firing
		DFLOAT	m_fMaxFireRest;			// Maximum time between firing

		DBOOL		m_bFire;			// Should we fire?
		WeaponState m_eState;			// What are we currently doing

		DVector m_vLastFirePath;		// Last direction fired

		DBOOL	m_bHave;				// Do we have this weapon
		int		m_nAmmo;				// How much ammo do we currently have
		int		m_nAmmoInClip;			// How much ammo is in our clip

		DBOOL	m_bInfiniteAmmo;		// For use with melee weapons
		DFLOAT	m_fDamageFactor;		// How much damage is adjusted
		DFLOAT	m_fRangeAdjust;			// How much the range is adjusted
	
		DBOOL	m_bCanLockOnTarget;		// Can this weapon lock on targets...

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		char*		m_pHandWeaponName;  // Hand held weapon model filename
		char*		m_pHandWeaponSkin;  // Hand held weapon model skin

		RiotWeaponId	m_nId;			// What kind of weapon are we?
		DFLOAT			m_fLifeTime;	// How long projectile stays around

		DFLOAT			m_fZoomDamageMult;

		DamageType m_eDamageType;		// Type of damage done by this weapon
};


#endif // __WEAPON_H__