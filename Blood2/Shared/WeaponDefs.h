#ifndef __WEAPONDEFS_H__
#define __WEAPONDEFS_H__

// Includes....
#include "SharedDefs.h"

// Weapon spread structure
typedef struct Spread_t
{
	DFLOAT	h;
	DFLOAT	v;
} Spread;

struct VectorInit
{
	DFLOAT x;
	DFLOAT y;
	DFLOAT z;
};


// Weapon flags
#define WEAPFLAG_SEMIAUTO		1
#define WEAPFLAG_ALTZOOM		0
#define	DUAL_HAND_MASK			128
#define MAX_FIRE_SOUNDS			4

#define WEAPON_SOUND_DRYFIRE	1
#define WEAPON_SOUND_KEY		2
#define WEAPON_SOUND_FIRE		3


#define INVALID_ANI		((DDWORD)-1)


// Structure to hold initial values for weapons
typedef struct WeaponData_t {
		char*		m_szViewModelFilename;
		char*		m_szLeftViewModelFilename;
		char*		m_szViewModelSkin;
		char*		m_szHandModelFilename;
		char*		m_szHandModelSkin;
		DBYTE		m_nType;			// Weapon type
		DBYTE		m_nFireType;		// Weapon animation
		DBYTE		m_nAmmoType;		// Type of ammo
		D_WORD		m_nAmmoUse;			// Amount of ammo to use per shot
		D_WORD		m_nAltAmmoUse;		// Amount of ammo to use per Alt shot
		DFLOAT		m_fMinDamage;		// minimum amount of damage per shot vector
		DFLOAT		m_fMaxDamage;		// maximum amount of damage per shot vector
		DFLOAT		m_fMinAltDamage;	// Alt minimum fire damage
		DFLOAT		m_fMaxAltDamage;	// Alt maximum fire damage
		DFLOAT		m_fReloadTime;		// How long to reload?
		DFLOAT		m_fAltReloadTime;	// How long to reload for alt fire?
		Spread		m_Spread;			// Spread;
		Spread		m_AltSpread;
		DFLOAT		m_fProjVelocity;
		DFLOAT		m_fAltProjVelocity;
		DFLOAT		m_fRange;
		DFLOAT		m_fAltRange;
		DDWORD		m_dwShotsPerFire;
		DDWORD		m_dwAltShotsPerFire;
		DDWORD		m_dwStrengthReq;
		DDWORD		m_dwTwoHandStrengthReq;
		int			m_nDamageRadius;
		int			m_nAltDamageRadius;
		DBOOL		m_bAltFireZoom;			// Alt fire for this weapon is a zoom function.
		DBOOL		m_bSemiAuto;			// Weapon has semi-auto firing
		char*		m_szFireSound;			// Sound made when weapon fires.
		char*		m_szAltFireSound;		// Sound made when weapon fires.
		char*		m_szEmptyWeaponSound;	// Sound made when weapon is empty
		char*		m_szAltEmptyWeaponSound;// Sound made when weapon is empty
		char*		m_szProjectileClass;
		char*		m_szAltProjectileClass;
		DFLOAT		m_fFlashRadius;
		VectorInit	m_vFlashColor;
		// Info for client
		char*		m_szWeaponName;
		int			m_nWeaponNameID;
		char*		m_szFlashSprite;
		char*		m_szAltFlashSprite;
		DFLOAT		m_fFlashDuration;
		DFLOAT		m_fFlashScale;
		VectorInit	m_vHandModelOffset;	// Offset for the 3rd person view
		VectorInit	m_vViewModelOffset;	// Gun offset in relation to player
		VectorInit	m_vMuzzleOffset;	// Muzzle Offset
		VectorInit	m_vRecoil;			// Gun's recoil
		VectorInit	m_vFlash;			// Flash position
		DFLOAT		m_fEjectInterval;	// How often to eject shells
		DFLOAT		m_fViewKick;		// Amount to adjust view when firing.
		DBOOL		m_bCumulativeKick;	// Kick is Cumulative
		DBOOL		m_bLoopAnim;		// Anim is looping
		DBOOL		m_bAltLoopAnim;		// Alt Anim is looping
		char*		m_szStatusIcon;		// Icon displayed on the status bar
		char*		m_szStatusIconH;	// Highlighed icon displayed on the status bar
} WeaponData;


// Weapon states
enum WeaponState { 
	WS_REST,
	WS_IDLE, 
	WS_DRAW,
	WS_HOLSTER,
	WS_START_FIRING, 
	WS_FIRING, 
	WS_STOP_FIRING, 
	WS_START_ALT_FIRING,
	WS_ALT_FIRING,
	WS_STOP_ALT_FIRING,
	WS_HOLSTERED			// holstered/inactive state (Greg 9/18)
};

typedef struct WeaponFXExtras
{
	WeaponFXExtras::WeaponFXExtras();

	DBYTE		nAmmo;
	DBYTE		nSurface;
	DBYTE		nExp;
	DFLOAT		fDamage;
	DFLOAT		fDensity;
	DVector		vColor1;
	DVector		vColor2;
	DVector		vLightColor1;
	DVector		vLightColor2;
}	WeaponFXExtras;

// ----------------------------------------------------------------------- //

inline WeaponFXExtras::WeaponFXExtras()
{
	memset(this, 0, sizeof(WeaponFXExtras));
}

// ----------------------------------------------------------------------- //



// initial values for standard weapons
extern WeaponData g_WeaponDefaults[WEAP_MAXWEAPONTYPES];

#endif __WEAPONDEFS_H__