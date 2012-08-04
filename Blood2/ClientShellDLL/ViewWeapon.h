#ifndef __VIEWWEAPON_H__
#define __VIEWWEAPON_H__


#include "cpp_clientshell_de.h"
#include "SharedDefs.h"
#include "WeaponDefs.h"

#define MAX_CS_FILENAME_LEN 100

#define FIREMSG_SPECIAL_DWORD	0x04
#define FIREMSG_SPECIAL_FLOAT	0x08

class CViewWeapon
{
    public:

        CViewWeapon();
        ~CViewWeapon();

		DBOOL		Create(CClientDE* pClientDE, DBYTE byWeaponID, DBOOL bLeftHand);
		void		CreateFlash(DFLOAT fScale);
		void		Term();
		void		Update(DFLOAT fPitch, DFLOAT fYaw, DVector *pos);
		void		UpdateBob(DFLOAT nHeight, DFLOAT nWidth);
		void		Hide();
		void		Show();

		DVector&	GetGunOffset() { return m_vViewModelOffset; }
		void		SetGunOffset(DVector& offset) { VEC_COPY(m_vViewModelOffset, offset); }
		char*		GetWeaponName() { return m_szWeaponName; }
		DBYTE		GetWeaponID() { return m_nWeaponID; }
		DVector&	GetFlash() { return m_vAdjFlashPos; }
		DFLOAT		GetViewKick() { return m_fViewKick; }
		DBOOL		IsCumulativeKick() { return m_bCumulativeKick; }
		DBOOL		IsLeftHand() { return m_bLeftHand; }
		DVector&	GetFlashOffset() { return m_vFlashPos; }
		void		SetFlashOffset(DVector& offset) { VEC_COPY(m_vFlashPos, offset); }
		DVector&	GetMuzzlePos() { return m_vMuzzleOffset; }
		HLOCALOBJ	GetWeapObj() { return m_hObject; }
		virtual void	UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);
		virtual void	CancelFiringState();
		DBOOL		IsAltFireZoom()			{ return m_bAltFireZoom; }
		void		OnModelKey(HLOCALOBJ hObj, ArgList* pArgList);
		void		SendFireMsg(DVector* pvPos, DVector* pvFire, DBYTE byRandomSeed, DBOOL bAltFire);
		void		SendSoundMsg(char *szSound, DBYTE byFlags);

		virtual DBYTE	GetAmmoType(DBOOL bAltFire = DFALSE)	{ return m_nAmmoType; }
 
    protected:

		void		AdjustAngleAndSetPos(DFLOAT fPitch, DFLOAT fYaw, DVector *pos);
		void		UpdateFlash(DVector vFlashPos);

		void		SendFireMsg();
		virtual DBOOL	FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags)	{ return DFALSE; }

/*************************************************************/
		DBOOL		IsChanging();
		void		Draw();
		DFLOAT		CheckAmmo(DBOOL bAltFire);
		HSOUNDDE	PlayFireSound(DBOOL bAltFire);
		HSOUNDDE	PlayEmptyWeaponSound(DBOOL bAltFire);
		void		KillSounds();
		void		UpdateWeaponFX(DDWORD &nFX, DDWORD &nExtras, WeaponFXExtras *ext, SurfaceType eType, DBYTE nAmmoType, DFLOAT fDamage);
		virtual DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);
		virtual DBOOL	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
		virtual DDWORD	Fire();
		virtual void	FireSpread(Spread *spread, DDWORD shots, DFLOAT range, DBOOL bAltFire, DVector *rDir);
		DBOOL		FiringTooClose(DVector *vFire, DFLOAT fDist, DVector *vNewPos);
		void		AlignFireVector(DVector *vFire, DFLOAT fDist);
		void		SetFirePosRot(DVector *firedPos, DRotation *rotP, DBOOL bAltFire);
		void		SetupViewModel();
		void		ShowMuzzleFlash(DVector *vPos);
		void		UpdateMuzzleFlash();
		DBOOL		PlayAnimation(DDWORD dwNewAni);
		void		AddImpact(DVector *vSource, DVector *vDest, DVector *vForward, DVector *vNormal, DDWORD nFX, DDWORD nExtras, WeaponFXExtras *ext);
		void		AddWorldModelMark(DVector vPos, DVector vNormal, HOBJECT hObject, SurfaceType eType);

/*************************************************************/


	protected:
//		char		m_szWeaponName[MAX_CS_FILENAME_LEN];
//		char		m_szFlashSprite[MAX_CS_FILENAME_LEN];
//		char		m_szAltFlashSprite[MAX_CS_FILENAME_LEN];
//		DFLOAT		m_fFlashDuration;
//		DFLOAT		m_fFlashStartTime;
		DBYTE		m_nFlashVisible;
		DBYTE		m_nWeaponID;
//		DVector		m_MuzzleOffset;		// Muzzle Offset
//		DVector		m_Recoil;			// Gun's recoil
		DVector		m_vFlashPos;		// Flash position
		DVector		m_vAdjFlashPos;		// Adjusted flash position
//		DFLOAT		m_fViewKick;		// How much to adjust perspective when firing
//		DBOOL		m_bCumulativeKick;	// Kick effect is Cumulative

		DFLOAT		m_fBobHeight;		// Amount of gun bobbing
		DFLOAT		m_fBobWidth;		// Current bobbing position

		DFLOAT		m_fLastEjectedTime;	// When a shell was last ejected

		HLOCALOBJ	m_hFlashSprite;		// Handle for the flash sprite
		HLOCALOBJ	m_hAltFlashSprite;	// Handle for the flash sprite
		HLOCALOBJ	m_hObject;
		DFLOAT		m_fLowerOffset;
		DBOOL		m_bOkToFire;
		DDWORD		m_nFiredType;
		DDWORD		m_nType;
		DBOOL		m_bLeftHand;		// Weapon is carried in the left hand
		CClientDE * m_pClientDE;


		char* 		m_pViewModelFilename;
		char*		m_pLeftViewModelFilename;
		char* 		m_pViewModelSkinname;
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
		DVector		m_vViewModelOffset;		// Gun offset in relation to player
		DVector		m_vMuzzleOffset;		// Muzzle Offset
		DVector		m_vRecoil;				// Gun's recoil
//		DVector		m_vFlash;				// Flash position
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
		HSOUNDDE	m_hLoopSound;
		DFLOAT		m_fCurFireSoundsEndTime[MAX_FIRE_SOUNDS];	// End the sound past this time..

		DBOOL		m_bLastFiring;			// Was firing last time through UpdateFiringState?
		DBOOL		m_bAccuracyCheck;		// Should we check and improve the accuracy?

		int			m_nUpdateWait;
		DDWORD		m_nFlags;				// Extra flags when creating the weapon

		DFLOAT		m_fChromeValue;			// Value to set the alpha for the chrome texture

		HSTRING		m_szPic;				// Regular icon for the weapon
		HSTRING		m_szPicH;				// Highlighted icon for the weapon

		DDWORD		m_nIgnoreFX;
};



enum SHELL_TYPES {
	SHELL_NONE = 0,
	SHELL_SHOTGUN
};

extern HLOCALOBJ g_hWeaponModel;
extern DRotation g_rotGun;

#endif __VIEWWEAPON_H__