#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "cpp_server_de.h"
#include "cpp_engineobjects_de.h"
#include "SharedDefs.h"
#include "Projectile.h"
#include "ObjectUtilities.h"
#include "ViewWeaponModel.h"
#include "HandWeaponModel.h"
#include "WeaponDefs.h"
#include "ClientWeaponSFX.h"

class CInventoryMgr;

// class to contain weapon info
class CWeapon
{
	public:

		CWeapon(DBYTE dwWeapType);
		~CWeapon();
		virtual void	Init(HOBJECT hOwner, CInventoryMgr *pInventoryMgr, DBOOL bLeftHand = DFALSE, HOBJECT hMuzzleFlash = DNULL);
		virtual void	SetClient(HCLIENT hClient, CViewWeaponModel *pViewWeaponModel);
		virtual DDWORD	Fire();
		virtual void	FireSpread(Spread *spread, DDWORD shots, DFLOAT range, DBOOL bAltFire, DVector *rDir);
		virtual DBOOL	FiringTooClose(DVector *vFire, DFLOAT fDist, DVector *vNewPos);
		virtual void	AlignFireVector(DVector *vFire, DFLOAT fDist);
		virtual void	Update();
		virtual void	UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);

		virtual void	SetSpecialData(DDWORD dwSpecial, DFLOAT fSpecial)	{ return; }

		void		Term();
		DBOOL		IsAltFireZoom()			{ return m_bAltFireZoom; }
		DBOOL		IsDualHanded()			{ return m_bDualHanded; } 

		DBYTE		GetType()				{ return m_nType; }
		DBYTE		GetFireType()			{ return m_nFireType; }
		DFLOAT		GetReloadTime()			{ return m_fReloadTime; }
		DDWORD		GetStrengthReq()		{ return m_dwStrengthReq; }
		DDWORD		GetTwoHandStrengthReq()	{ return m_dwTwoHandStrengthReq; }

        DFLOAT		GetWeaponRange()		{ return m_fRange; }
        DFLOAT		GetAltWeaponRange()		{ return m_fAltRange; }
        
        DDWORD		GetWeaponDamage()		{ return (DDWORD)m_fMaxDamage; }
		DFLOAT		GetWeaponDmgRange()		{ return (DFLOAT)m_nDamageRadius; }
        DDWORD		GetAltWeaponDamage()	{ return (DDWORD)m_fMaxAltDamage; }
        
		D_WORD		GetAmmoUse()			{ return (D_WORD)m_nAmmoUse; }
		D_WORD		GetAltAmmoUse()			{ return (D_WORD)m_nAltAmmoUse; }

		DVector*	GetPosition()			{ return &m_vPosition; }

		DBOOL		IsInitialized()			{ return m_bInitialized; }
		DBOOL		IsClientNotified()		{ return m_bClientNotified; }
		DBOOL		IsOwnerAPlayer()		{ return (m_hClient != DNULL); }
		DBOOL		IsIdle()				{ return (m_eState == WS_REST) || (m_eState == WS_IDLE) || (m_eState == WS_DRAW) ||
													 (m_eState == WS_HOLSTER) || (m_eState == WS_HOLSTERED); }
		HOBJECT		GetOwner( ) const		{ return m_hOwner; }

		DBOOL		IsChanging();
		void		Draw();
		void		Holster()				{ m_eState = WS_HOLSTER; }

		virtual DBYTE	GetAmmoType(DBOOL bAltFire = DFALSE)	{ return m_nAmmoType; }
		virtual void	SetNotFiring()							{ m_eState = WS_REST; }

		void		OnFireKey();
		void		SendClientInfo(DBYTE slot);
		void		SetFirePosRot(DVector *firedPos, DRotation *rotP, DBOOL bAltFire);
		DBOOL		GetHandModelPos(DVector *vPos, DRotation *rRot);
		void		ShowHandModel(DBOOL bShow);
		HOBJECT		GetViewModel() { return (m_pViewModel) ? m_pViewModel->m_hObject : DNULL; }
		HOBJECT		GetHandModel() { return (m_pHandModel) ? m_pHandModel->m_hObject : DNULL; }
		void		SetHandModel( CHandWeaponModel *pHandWeaponModel ) { m_pHandModel = pHandWeaponModel; }
		void		DropHandModel();
		void		SetupMuzzleFlash();

	protected:

		virtual	DBOOL		FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer = DFALSE);
		virtual	CProjectile	*FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
        virtual	DBOOL		SendDamageMsg(HOBJECT firedTo, DVector vPoint, DVector *vFire, DFLOAT fDamage);
		virtual DFLOAT		GetDamageMultiplier();
        
		DFLOAT      CheckAmmo(DBOOL bAltFire = DFALSE);
		void		PlayFireSound(DBOOL bAltFire);
		void		PlayEmptyWeaponSound(DBOOL bAltFire);
		void		KillSounds();
		void		SetupViewModel();
		void		CreateHandModel();
		void		SetupMuzzleFlash(DFLOAT fRadius, DFLOAT fRed, DFLOAT fGreen, DFLOAT fBlue);
		void		ShowMuzzleFlash(DVector *vPos);
		void		UpdateMuzzleFlash();
		void		SendClientFiring(DBYTE type);

		DBOOL		PlayAnimation(DDWORD dwNewAni);

	public:
		void		UpdateWeaponFX(DDWORD &nFX, DDWORD &nExtras, WeaponFXExtras *ext, SurfaceType eType, DBYTE nAmmoType, DFLOAT fDamage);

		void		AddSplash(DVector vPos, DVector vNormal, HOBJECT hObject, DVector vColor1, DVector vColor2);
		void		AddSparks(DVector vPos, DVector vNormal, DFLOAT fDamage, HOBJECT hObject, SurfaceType eType);
		void		AddWorldModelMark(DVector vPos, DVector vNormal, HOBJECT hObject, SurfaceType eType);
		virtual void AddImpact(DVector *vSource, DVector *vDest, DVector *vForward, DVector *vNormal, DDWORD nFX, DDWORD nExtras, WeaponFXExtras *ext);

	protected:

		CViewWeaponModel*	m_pViewModel;	// View model
		CInventoryMgr*		m_pInventoryMgr;// Inventory mgr for ammo usage
		CHandWeaponModel*	m_pHandModel;	// Hand view model
//		HOBJECT		m_hHandModel;			// The model held by the character
		HATTACHMENT	m_hHandAttachment;		// Attachment handle to the character hand
		HOBJECT		m_hOwner;				// The character holding this weapon
		HCLIENT		m_hClient;				// The client of the owner if it's a player
		DBOOL		m_bInitialized;			// The weapon is initialized
		DBOOL		m_bClientNotified;		// The owner's client has been notified
		DBOOL		m_bLeftHand;			// This weapon is in the player's left hand

		char* 		m_pViewModelFilename;
		char*		m_pLeftViewModelFilename;
		char* 		m_pViewModelSkinname;
		char*		m_pHandModelFilename;
		char*		m_pHandModelSkinname;
		DBYTE		m_nType;				// Weapon type
		DBYTE		m_nFireType;			// Weapon animation
		DBYTE		m_nAmmoType;			// Type of ammo
		D_WORD		m_nAmmoUse;				// Amount of ammo to use per shot
		D_WORD		m_nAltAmmoUse;			// Amount of ammo to use per Alt shot
		DFLOAT		m_fDamage;				// Current amount of damage per shot vector
		DFLOAT		m_fMinDamage;			// minimum amount of damage
		DFLOAT		m_fMaxDamage;			// maximum amount of damage
		DFLOAT		m_fMinAltDamage;		// Alt minimum fire damage
		DFLOAT		m_fMaxAltDamage;		// Alt maximum fire damage
		DFLOAT		m_fReloadTime;			// How long to reload?
		DFLOAT		m_fAltReloadTime;		// How long to reload for alt fire?
		Spread		m_Spread;				// Width and height range for random fire locations
		Spread		m_AltSpread;			// Width and height range for random fire locations
		DFLOAT		m_fProjVelocity;		// Speed of the Primary Fire projectile
		DFLOAT		m_fAltProjVelocity;		// Speed of the Alt Fire projectile
		DFLOAT		m_fRange;				// Distance the Primary Fire can shoot
		DFLOAT		m_fAltRange;			// Distance the Alt Fire can shoot
		DDWORD		m_dwShotsPerFire;		// Number of vectors to shoot for Primary Fire
		DDWORD		m_dwAltShotsPerFire;	// Number of vectors to shoot for Alt Fire
		DDWORD		m_dwStrengthReq;		// Amount of strength required for this weapon
		DDWORD		m_dwTwoHandStrengthReq;	// Amount of strength required for two of this weapon
		int			m_nDamageRadius;		// An area of effect damage radius for primary projectile
		int			m_nAltDamageRadius;		// An area of effect damage radius for alt projectile
		DBOOL		m_bAltFireZoom;			// Alt fire for this weapon is a zoom function.
		DBOOL		m_bDualHanded;			// 2 of these can be carried simulataneously
		DBOOL		m_bSemiAuto;			// Weapon has semi-auto firing
		HOBJECT		m_hFlash;				// Object with the muzzle flash light
		DFLOAT		m_fFlashStartTime;		// Start time of the flash
		DBOOL		m_bFlashShowing;		// Is the muzzle flash light showing

		char*		m_szFireSound;			// Sound made when weapon fires.
		char*		m_szAltFireSound;		// Sound made when weapon fires.
		char*		m_szEmptyWeaponSound;	// Sound made when weapon is empty
		char*		m_szAltEmptyWeaponSound;// Sound made when weapon is empty
		char*		m_szProjectileClass;	// Class of projectile used in Primary Fire
		char*		m_szAltProjectileClass;	// Class of projectile used in Alt Fire

		char*		m_szWeaponName;			// Name of the weapon
		int			m_nWeaponNameID;		// Resource number of weapon name
		char*		m_szFlashSprite;		// Sprite to use for flashes
		char*		m_szAltFlashSprite;		// Sprite to use for alternate flashes
//		char*		m_szShellCasingFilename;// Model of shell casing to use
//		char*		m_szShellCasingSkinName;// Skin to place on the shell casing
//		char*		m_szShellCasingSound;	// Sound played when shell casing hits something
		DBOOL		m_bEjectShell;			// Should we eject a shell?
		DBOOL		m_bSplash;				// Should we cause another splash
		DFLOAT		m_fFlashDuration;		// Length to display the flash sprite
		DFLOAT		m_fFlashScale;			// Scale of the flash sprite
		DVector		m_vHandModelOffset;		// Offset for the 3rd person view
		DVector		m_vViewModelOffset;		// Gun offset in relation to player
		DVector		m_vMuzzleOffset;		// Muzzle Offset
		DVector		m_vRecoil;				// Gun's recoil
		DVector		m_vFlash;				// Flash position
		DFLOAT		m_fEjectInterval;		// How often to eject shells
		DFLOAT		m_fViewKick;			// Amount to adjust view when firing.
		DBOOL		m_bCumulativeKick;		// Kick is Cumulative
		DBOOL		m_bLoopAnim;			// Anim is looping
		DBOOL		m_bAltLoopAnim;			// Alt Anim is looping
		DBOOL		m_bLoopStatic;			// Loop the static_model animation

		DBOOL		m_bLastFireAlt;			// Was last fire an alt fire?
		DFLOAT		m_fFireTime;			// Time of last fire sound
		DFLOAT		m_fLastShotTime;		// temp to store when the last legal shot was fired

		DDWORD		m_nRestAnim;			// Rest animation
		DDWORD		m_nIdleAnim;			// Idle animation
		DDWORD		m_nDrawAnim;			// Unholstering the weapon animation
		DDWORD		m_nDrawDuelAnim;		// Unholstering in duel weapon mode
		DDWORD		m_nHolsterAnim;			// Holstering the weapon animation
		DDWORD		m_nHolsterDuelAnim;		// Holstering the weapons in duel mode
		DDWORD		m_nStartFireAnim;		// Firing spin-up animation
		DDWORD		m_nFireAnim;			// Firing animation
		DDWORD		m_nStopFireAnim;		// Firing spin-down animation
		DDWORD		m_nStartAltFireAnim;	// Firing spin-up animation
		DDWORD		m_nAltFireAnim;			// Firing animation
		DDWORD		m_nStopAltFireAnim;		// Firing spin-down animation

		DFLOAT		m_fIdleStartTime;		// Starting time of an Idle delay
		DFLOAT		m_fIdleDelay;			// Current time to delay for idle animations
		DFLOAT		m_fMinIdleDelay;		// Minimum amount of time to delay for idle state
		DFLOAT		m_fMaxIdleDelay;		// Maximum amount of time to delay for idle state

		WeaponState	m_eState;				// Weapon state
		DVector		m_vPosition;			// View weapon position
		DRotation	m_rRotation;			// View weapon rotation

		DBYTE		m_nDamageType;
		HSOUNDDE	m_hCurFireSounds[MAX_FIRE_SOUNDS];	// Keep track of current looping
		DFLOAT		m_fCurFireSoundsEndTime[MAX_FIRE_SOUNDS];	// End the sound past this time..

		DBOOL		m_bLastFiring;			// Was firing last time through UpdateFiringState?
		DBOOL		m_bAccuracyCheck;		// Should we check and improve the accuracy?
		DBOOL		m_bMultiDamageBoost;	// Should we double the damage of this weapon for multiplay?

		int			m_nUpdateWait;
		DDWORD		m_nFlags;				// Extra flags when creating the weapon

		DFLOAT		m_fChromeValue;			// Value to set the alpha for the chrome texture

		HSTRING		m_szPic;				// Regular icon for the weapon
		HSTRING		m_szPicH;				// Highlighted icon for the weapon
};

#endif // __WEAPON_H__