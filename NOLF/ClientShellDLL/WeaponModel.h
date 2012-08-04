// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponModel.h
//
// PURPOSE : Generic client-side WeaponModel wrapper class - Definition
//
// CREATED : 9/27/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_MODEL_H__
#define __WEAPON_MODEL_H__

#include "iclientshell.h"
#include "SurfaceMgr.h"
#include "WeaponMgr.h"
#include "MuzzleFlashFX.h"
#include "LaserBeam.h"
#include "PVFXMgr.h"

#define WM_MAX_FIRE_ANIS		WMGR_MAX_WEAPONANI_FIRE
#define WM_MAX_ALTFIRE_ANIS		WM_MAX_FIRE_ANIS
#define WM_MAX_IDLE_ANIS		WMGR_MAX_WEAPONANI_IDLE
#define WM_MAX_ALTIDLE_ANIS		WM_MAX_IDLE_ANIS

enum FireType
{
	FT_NORMAL_FIRE=0,
	FT_ALT_FIRE,
};

class CWeaponModel
{
	public :

		CWeaponModel();
		virtual ~CWeaponModel();

        LTBOOL	Init();
        LTBOOL	Create(ILTClient* pClientDE, uint8 nWeaponId, uint8 nAmmoId, uint32 dwAmmo);
        void	ChangeWeapon(uint8 nCommandId, LTBOOL bCanDeselect=LTTRUE);

		void	SetupModel();

		void	ToggleHolster(LTBOOL bPlayDeselect=LTTRUE);
		void	SetHolster(uint8 nWeaponId);
		uint8	GetHolster() { return m_nHolsterWeaponId; }

        void	ChangeAmmo(uint8 nNewAmmoId, LTBOOL bForce=LTFALSE);

        void	ReloadClip(LTBOOL bPlayReload=LTTRUE, int nNewAmmo=-1, LTBOOL bForce=LTFALSE);
        LTVector GetShellEjectPos(LTVector & vOriginalPos);

        WeaponState UpdateWeaponModel(LTRotation rRot, LTVector vPos,
            LTBOOL bFire, FireType eFireType=FT_NORMAL_FIRE);

		void Reset();
		HLOCALOBJ GetHandle() const { return m_hObject; }

        void UpdateBob(LTFLOAT fWidth, LTFLOAT fHeight);

		void	UpdateMovementPerturb();
        LTFLOAT GetMovementPerturb() {return m_fMovementPerturb;}

        void SetVisible(LTBOOL bVis=LTTRUE);
        void Disable(LTBOOL bDisable=LTTRUE);
        void GadgetDisable(LTBOOL bDisable);

        LTVector GetModelPos() const;
        LTRotation GetModelRot() const;

        void SetWeaponOffset(LTVector v);
        void SetMuzzleOffset(LTVector v);

        LTVector GetWeaponOffset();
        LTVector GetMuzzleOffset();

		int GetWeaponId()	const { return m_nWeaponId; }
		int GetAmmoId()		const { return m_nAmmoId; }
		int GetAmmoInClip() const { return m_nAmmoInClip; }

		LTBOOL CanCurrentWeaponUseAmmo(uint8 nAmmoId);

		WEAPON* GetWeapon() const { return m_pWeapon; }
		AMMO*	GetAmmo()	const { return m_pAmmo; }

        LTBOOL	IsDisabled() const { return m_bDisabled; }

        uint8   PrevWeapon(uint8 nCurrWeaponId = -1);
        uint8   NextWeapon(uint8 nCurrWeaponId = -1);
        uint8   NextAmmo(uint8 nCurrAmmoId = -1);
        LTBOOL  IsOutOfAmmo(uint8 nWeaponId);
		uint8	MeleeWeapon();

		LTBOOL	IsMeleeWeapon()	{ return (m_pAmmo && m_pAmmo->eInstDamageType == DT_MELEE); }


		void  OnModelKey(HLOCALOBJ hObj, ArgList* pArgs);

		void CreateMods();

		inline WeaponState GetState() const { return m_eState; }

	protected :

		void CreateFlash();
		void CreateModel();
		void RemoveModel();
		void StartFlash();
		void UpdateFlash(WeaponState eState);

		void RemoveMods();
		void UpdateMods();
		void CreateSilencer();
		void CreateLaser();
		void CreateScope();

        WeaponState Fire(LTBOOL bUpdateAmmo=LTTRUE);
        WeaponState UpdateModelState(LTBOOL bFire);

		void	SendFireMsg();
		void	UpdateFiring();
		void	UpdateNonFiring();
        LTBOOL  PlaySelectAnimation();
        LTBOOL  PlayDeselectAnimation();
        LTBOOL  PlayFireAnimation(LTBOOL bResetAni=LTTRUE);
        LTBOOL  PlayReloadAnimation();
        LTBOOL  PlayIdleAnimation();
		void	InitAnimations(LTBOOL bAllowSelectOverride=LTFALSE);
		void	SetAmmoOverrideAnis(LTBOOL bAllowSelectOverride);
		void	ResetWeaponData();

		void	Deselect();
		void	Select();

        void    ClientFire(LTVector & vPath, LTVector & vFirePos);
		void	DoProjectile();
		void	DoVector();
		void	DoGadget();
		void	GadgetReload();
		void	HandleSunglassMode();
		LTBOOL	HandleZipCordFire();

		HLOCALOBJ CreateServerObj();

        void    AddImpact(HLOCALOBJ hObj, LTVector & vInpactPoint,
                          LTVector & vNormal, SurfaceType eType);
		void	HandleVectorImpact(IntersectQuery & qInfo, IntersectInfo & iInfo);
        void    HandleGadgetImpact(HOBJECT hObj, LTVector vImpactPoint);

        LTBOOL  GetBestAvailableAmmoType(uint8 nWeaponId, int & nAmmoType);
        LTBOOL  GetFirstAvailableAmmoType(uint8 nWeaponId, int & nAmmoType);
		void	DecrementAmmo();

        void    HandleInternalWeaponChange(uint8 nWeaponId);
        void    DoWeaponChange(uint8 nWeaponId);
        LTBOOL  CanChangeToWeapon(uint8 nCommandId);
		void	AutoSelectWeapon();
		void	ChangeToNextRealWeapon();
		LTBOOL	IsGadgetAmmo(AMMO* pAmmo);


		HOBJECT CreateModelObject(HOBJECT hOldObj, ObjectCreateStruct & createStruct);

		void	DoSpecialFire();
		void	DoSpecialEndFire();
		void	DoSpecialCreateModel();
		void	DoSpecialWeaponChange();
        void    SpecialShowPieces(LTBOOL bShow=LTTRUE, LTBOOL bForce=LTFALSE);

        LTFLOAT GetNextIdleTime();

        LTBOOL  IsFireAni(uint32 dwAni);
        uint32  GetFireAni(FireType eFireType=FT_NORMAL_FIRE);

        LTBOOL  IsIdleAni(uint32 dwAni);
        uint32  GetIdleAni();
        uint32  GetSubtleIdleAni();

        LTBOOL  IsReloadAni(uint32 dwAni);
        uint32  GetReloadAni();

        LTBOOL  IsSelectAni(uint32 dwAni);
        uint32  GetSelectAni();

        LTBOOL  IsDeselectAni(uint32 dwAni);
        uint32  GetDeselectAni();

        LTBOOL  CanUseAltFireAnis();

		void	HandleFireKeyDown();
		void	HandleFireKeyUp();

        uint8   GetLastSndFireType();

        LTBOOL  GetFireInfo(LTVector & vU, LTVector & vR, LTVector & vF,
            LTVector & vFirePos);

		WeaponState	SetState(WeaponState eNewState);

		HOBJECT			m_hObject;			// Handle of WeaponModel model

		HMODELSOCKET	m_hBreachSocket;	// Handle of breach socket

		HOBJECT			m_hSilencerModel;	// Handle of silencer mod model
		HOBJECT			m_hLaserModel;		// Handle of laser mod model
		HOBJECT			m_hScopeModel;		// Handle of scope mod model

		HMODELSOCKET	m_hSilencerSocket;	// Handle of silencer mod socket
		HMODELSOCKET	m_hLaserSocket;		// Handle of laser mod socket
		HMODELSOCKET	m_hScopeSocket;		// Handle of scope mod socket

        LTBOOL           m_bHaveSilencer;    // Do we have a silencer mod
        LTBOOL           m_bHaveLaser;       // Do we have a laser mod
        LTBOOL           m_bHaveScope;       // Do we have a scope mod

		CLaserBeam		m_LaserBeam;		// Laser beam

		CPVFXMgr		m_PVFXMgr;			// Player-view fx mgr

		int			m_nWeaponId;
		int			m_nHolsterWeaponId;
		int			m_nAmmoId;

		WEAPON*		m_pWeapon;
		AMMO*		m_pAmmo;

		LTVector	m_vFlashPos;
		LTVector	m_vFlashOffset;
		LTFLOAT		m_fFlashStartTime;	// When did flash start

		CMuzzleFlashFX m_MuzzleFlash;

        LTFLOAT     m_fBobHeight;
        LTFLOAT     m_fBobWidth;

        LTFLOAT     m_fMovementPerturb;

		FireType	m_eLastFireType;	// How did we last fire
        LTBOOL      m_bCanSetLastFire;  // Can we set m_eLastFireType

        LTFLOAT     m_fNextIdleTime;
        LTBOOL      m_bFire;
		int			m_nAmmoInClip;
		int			m_nNewAmmoInClip;
		WeaponState m_eState;			// What are we currently doing
		WeaponState	m_eLastWeaponState;

		HMODELANIM	m_nSelectAni;		// Select weapon
		HMODELANIM	m_nDeselectAni;		// Deselect weapon
		HMODELANIM	m_nReloadAni;		// Reload weapon

		HMODELANIM	m_nIdleAnis[WM_MAX_IDLE_ANIS];	// Idle animations
		HMODELANIM	m_nFireAnis[WM_MAX_FIRE_ANIS];	// Fire animations

		HMODELANIM	m_nAltSelectAni;		// Alt-Fire Select weapon
		HMODELANIM	m_nAltDeselectAni;		// Alt-Fire Deselect weapon (back to normal weapon)
		HMODELANIM	m_nAltDeselect2Ani;		// Alt-Fire Deselect weapon (to new weapon)
		HMODELANIM	m_nAltReloadAni;		// Alt-Fire Reload weapon

		HMODELANIM	m_nAltIdleAnis[WM_MAX_ALTIDLE_ANIS];	// Alt-Fire Idle animations
		HMODELANIM	m_nAltFireAnis[WM_MAX_ALTFIRE_ANIS];	// Alt-Fire animations

        LTBOOL      m_bUsingAltFireAnis;        // Use the m_nAltXXX anis?
        LTBOOL      m_bFireKeyDownLastUpdate;   // Was the fire key down last update?

        LTVector	m_vPath;        // Path of current vector/projectile
        LTVector    m_vFirePos;     // Fire position of current vector/projectile
        LTVector    m_vEndPos;      // Impact location of current vector
        uint16      m_wIgnoreFX;    // FX to ignore for current vector/projectile

        uint8       m_nRequestedWeaponId;   // Id of weapon to select
        LTBOOL      m_bWeaponDeselected;    // Did we just deselect the weapon

        LTBOOL      m_bDisabled;    // Is the weapon disabled
        LTBOOL      m_bVisible;     // Is the weapon visible (should it be)

        LTRotation  m_rCamRot;
        LTVector    m_vCamPos;
};

inline 
LTBOOL CWeaponModel::CanCurrentWeaponUseAmmo(uint8 nAmmoId)
{
	if (!m_pWeapon || !g_pWeaponMgr->IsValidAmmoType(nAmmoId)) return LTFALSE;

	for (int i=0; i < m_pWeapon->nNumAmmoTypes; i++)
	{
		if (m_pWeapon->aAmmoTypes[i] == nAmmoId) return LTTRUE;
	}

    return LTFALSE;
}

#endif // __WEAPON_MODEL_H__