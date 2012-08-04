// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerStats.h
//
// PURPOSE : Definition of PlayerStats class
//
// CREATED : 10/9/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYERSTATS_H
#define __PLAYERSTATS_H

#include "ltbasedefs.h"
#include "weaponmgr.h"
#include "overlays.h"
#include "objectives.h"

class CGameClientShell;
class CMissionData;

class CPlayerStats
{
public:

	CPlayerStats();
	~CPlayerStats();

	ObjectivesList* GetObjectives()			{ return &m_Objectives; }
	ObjectivesList* GetCompletedObjectives()	{ return &m_CompletedObjectives; }

    LTBOOL      Init();
	void		Term();

    void        OnEnterWorld(LTBOOL bRestoringGame=LTFALSE);
	void		OnExitWorld();

    void        UpdatePlayerWeapon(uint8 nWeaponId, uint8 nAmmoId, LTBOOL bForce=LTFALSE);
    void        Draw(LTBOOL bShowStats);
	void		ResetStats();
    void        DrawObjectives();

	void		Clear();
	void		Update();
    void        UpdateWeapon(uint8 nWeaponId, LTBOOL bForce=LTFALSE);
    void        UpdateHealth(uint32 nHealth);
    void        UpdateArmor(uint32 nArmor);
    void        UpdateMaxHealth(uint32 nHealth);
    void        UpdateMaxArmor(uint32 nArmor);
    void        UpdateAmmo(uint8 nWeaponId, uint8 nAmmoId, uint32 nAmmo, LTBOOL bPickedup=LTFALSE, LTBOOL bDisplayMsg=LTTRUE);
    void        UpdateGear(uint8 nGearId);
    void        UpdateMod(uint8 nModId);
    void        UpdateAir(LTFLOAT nPercent);
    void        UpdateObjectives(uint8 nType, uint8 nTeam, uint32 dwId);
	
	void		ResetInventory();						 //clears all inventory data
	void		DropInventory(LTBOOL bResetGear=LTTRUE); //drops are currently carried weapons and ammo
														 // also drops gear and mods if bResetGear is set

	void		SetMultiplayerObjectives(HMESSAGEREAD hMessage);

    void        UpdateWeaponBindings();


	void		ShowObjectives();
	void		HideObjectives();

	void		ClearMissionDamage()	{m_nDamage = 0;}
	int			GetMissionDamage()		{return m_nDamage;}

	void		Save(HMESSAGEWRITE hWrite);
	void		Load(HMESSAGEREAD hRead);

	void		ToggleCrosshair();
    void        EnableCrosshair(LTBOOL b=LTTRUE) { m_bCrosshairEnabled = b; }
    LTBOOL      CrosshairEnabled() const { return m_bCrosshairEnabled; }
    LTBOOL      CrosshairOn() const { return (LTBOOL)(m_nCrosshairLevel != 0); }

	LTBOOL		DrawingActivateGadget() const { return m_bDrawingGadgetActivate; }

    void        SetDrawAmmo(LTBOOL bVal=LTTRUE) { m_bDrawAmmo = bVal; }
    uint8       GetCurWeapon() const { return m_nCurrentWeapon; }

    uint8           GetMod(ModType eType);
    inline uint8    GetSilencer()   { return GetMod(SILENCER); }
    inline uint8    GetLaser()      { return GetMod(LASER); }
    inline uint8    GetScope()      { return GetMod(SCOPE); }

    inline LTBOOL    CanBeSilenced() { return (GetSilencer() != WMGR_INVALID_ID); }
    inline LTBOOL    CanHaveLaser()  { return (GetLaser() != WMGR_INVALID_ID); }
    inline LTBOOL    CanHaveScope()  { return (GetScope() != WMGR_INVALID_ID); }



    uint32       GetAmmoCount(uint8 nAmmoId) const;
    LTBOOL       HaveWeapon(uint8 nWeaponId) const;
    LTBOOL       HaveMod(uint8 nModId) const;
    LTBOOL       HaveGear(uint8 nGearId) const;

    LTBOOL       CanUseWeapon(uint8 nWeaponId) const;
    LTBOOL       CanUseAmmo(uint8 nAmmoId) const;
    LTBOOL       CanUseMod(uint8 nModId) const;
    LTBOOL       CanUseGear(uint8 nGearId) const;


	void		Setup(CMissionData* pMissionData);

	void		PrepareInventory();
	void		SaveInventory();

    LTBOOL      HaveAirSupply();

	void		NextLayout();
	void		PrevLayout();

    void        DrawCrosshair(HSURFACE hScreen, int nCenterX, int nCenterY, LTBOOL bMenuCrosshair = LTFALSE);
	void		UpdateCrosshairColors();

	LTBOOL		IsGadgetActivatable(HOBJECT hObj);

    void        DrawBoundWeapons(HSURFACE hScreen);

protected:

	void		CreateCrosshairs();
	void		DeleteCrosshairs();

	void		DrawTargetName();
	void		DrawUnArmedCrosshair(HSURFACE hScreen, int nCenterX, int nCenterY);
	void		DrawArmedCrosshair(CWeaponModel* pWeapon, HSURFACE hScreen, int nCenterX, int nCenterY);
	void		DrawSurfaceCrosshair(HSURFACE hSurf, HSURFACE hScreen, int nCenterX, int nCenterY);

	HOBJECT		TestForActivationObject(uint32 & dwUsrFlags, LTFLOAT & fDistAway);
	LTBOOL		DrawActivateCrosshair(HOBJECT hObj, HSURFACE hScreen, int nCenterX, int nCenterY, 
		uint32 dwInitialUsrFlags, LTFLOAT fDistAway);

	void		InitPlayerStats();

    void        DrawPlayerStats(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom, LTBOOL bShowStats);
	void		DrawScope(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom);
	void		DrawScuba(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom);
	void		DrawSunglass(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom);
    void        DrawObjectives(HSURFACE hScreen);

	void		UpdatePlayerHealth();
	void		UpdatePlayerAmmo();

	// create HUD surfaces
	void		CreateHealthSurfaces();
	void		CreateAmmoSurfaces();
	void		CreateAirSurfaces();
	void		CreateDamageSurfaces();

	// replace HUD surfaces as necessary
	void		UpdateHealthSurfaces();
	void		UpdateAmmoSurfaces();
	void		UpdateAirSurfaces();

	// destroy HUD surfaces
	void		DestroyHealthSurfaces();
	void		DestroyAmmoSurfaces();
	void		DestroyAirSurfaces();
	void		DestroyDamageSurfaces();

    void        ClearSurface(HSURFACE hSurf, HLTCOLOR hColor);
	void		UpdateFlash();

	void		UpdateLayout();

    void        AddCanUseWeapon(uint8 nWeaponId);
    void        AddCanUseAmmo(uint8 nAmmoId);
    void        AddCanUseMod(uint8 nModId);
    void        AddCanUseGear(uint8 nGearId);


protected:

    uint32      m_nHealth;                      // current health
    uint32      m_nArmor;                       // current armor
    uint32      m_nMaxHealth;                   // current maximum health
    uint32      m_nMaxArmor;                    // current maximum armor
    LTFLOAT      m_fAirPercent;
    uint32*     m_pnAmmo;                       // All ammo values
    LTBOOL*      m_pbHaveAmmo;                   // ammos that player had during mission
    LTBOOL*      m_pbHaveWeapon;                 // Weapon status
    uint8       m_nCurrentWeapon;               // current weapon
    uint8       m_nCurrentAmmo;                 // current ammo
    LTBOOL*      m_pbHaveMod;                    // Mod status
    LTBOOL*      m_pbHaveGear;                   // Gear status

    LTBOOL*      m_pbCanUseAmmo;                 // Can we use this ammo
    LTBOOL*      m_pbCanUseWeapon;               // Can we carry this Weapon
    LTBOOL*      m_pbCanUseMod;                  // Can we carry this Mod
    LTBOOL*      m_pbCanUseGear;                 // Can we carry this Gear

	int			m_nDamage;						// Damage taken during this mission

	int			m_nBoundWeapons[10];

	ObjectivesList	m_Objectives;
	ObjectivesList	m_CompletedObjectives;

    LTBOOL       m_bHealthChanged;
    LTBOOL       m_bArmorChanged;
    LTBOOL       m_bAmmoTypeChanged;
    LTBOOL       m_bAmmoChanged;
    LTBOOL       m_bShowNumbers;

    LTBOOL       m_bObjAdded;
    LTBOOL       m_bObjRemoved;
    LTBOOL       m_bObjCompleted;
    LTBOOL       m_bObjVisible;
	LTIntPt		 m_ObjectivePos;

	int			m_nCrosshairLevel;
    LTBOOL      m_bCrosshairEnabled;
	HSURFACE	m_hArmedCrosshair;
	HSURFACE	m_hUnarmedCrosshair;
	HSURFACE	m_hActivateCrosshair;
	HSURFACE	m_hInnocentCrosshair;
	HSURFACE	m_hActivateGadgetCrosshair;

	LTBOOL		m_bDrawingGadgetActivate;

	HSURFACE	m_hHUDHealth;
	HSURFACE	m_hHealthStr;
	HSURFACE	m_hHealthIcon;
	HSURFACE	m_hArmorIcon;
	HSURFACE	m_hHealthBar;
	HSURFACE	m_hArmorBar;
    LTRect       m_rcHealth;
    LTRect       m_rcHealthShadow;
    LTRect       m_rcHealthBar;
    LTRect       m_rcArmor;
    LTRect       m_rcArmorShadow;
    LTRect       m_rcArmorBar;
    LTBOOL       m_bHealthInit;

	HSURFACE	m_hHUDAmmo;
	HSURFACE	m_hAmmoBar;
	HSURFACE	m_hAmmoIcon;
	HSURFACE	m_hAmmoFull;
	HSURFACE	m_hAmmoEmpty;
    LTIntPt      m_AmmoSz;
    LTRect       m_rcAmmo;
    LTRect       m_rcAmmoShadow;
    LTRect       m_rcAmmoBar;
    LTRect       m_rcAmmoHUD;
    LTBOOL       m_bAmmoInit;


	HSURFACE	m_hHUDAir;
	HSURFACE	m_hAirBar;
	HSURFACE	m_hAirStr;
	HSURFACE	m_hAirIcon;
    LTRect       m_rcAir;
    LTRect       m_rcAirBar;
    LTBOOL       m_bAirInit;

	HSTRING			m_hModeStr;
	eSunglassMode	m_eLastMode;

    LTBOOL       m_bDrawAmmo;

    LTBOOL       m_bHealthFlash;
    LTFLOAT      m_fCurrHealthFlash;
    LTFLOAT      m_fTotalHealthFlash;
    LTBOOL       m_bArmorFlash;
    LTFLOAT      m_fCurrArmorFlash;
    LTFLOAT      m_fTotalArmorFlash;
    LTFLOAT      m_fFlashSpeed;
    LTFLOAT      m_fFlashDuration;

	//layout info
	int			m_nCurrentLayout;
    LTBOOL       m_bBarsChanged;

    LTBOOL       m_bUseAmmoBar;
    LTIntPt      m_AmmoBasePos;
    LTIntPt      m_AmmoClipOffset;
    LTIntPt      m_AmmoBarOffset;
    LTBOOL       m_bUseAmmoText;
    LTIntPt      m_AmmoTextOffset;
    LTIntPt      m_AmmoIconOffset;

    LTBOOL       m_bUseHealthBar;
    LTIntPt      m_HealthBasePos;
    LTIntPt      m_HealthBarOffset;
    LTIntPt      m_ArmorBarOffset;
    LTBOOL       m_bUseHealthText;
    LTIntPt      m_HealthTextOffset;
    LTIntPt      m_ArmorTextOffset;
    LTBOOL       m_bUseHealthIcon;
    LTIntPt      m_HealthIconOffset;
    LTIntPt      m_ArmorIconOffset;

    LTBOOL       m_bUseAirIcon;
    LTIntPt      m_AirBasePos;
    LTIntPt      m_AirIconOffset;
    LTBOOL       m_bUseAirText;
    LTIntPt      m_AirTextOffset;
    LTBOOL       m_bUseAirBar;
    LTIntPt      m_AirBarOffset;
	int			m_nBarHeight;
    LTFLOAT      m_fBarScale;

    LTIntPt      m_ModeTextPos;

	HSURFACE	m_hWeaponSurf[10];
	HSURFACE	m_hNumberSurf[10];
	LTFLOAT		m_fIconOffset[10];
	uint32		m_nIconSize;
	LTBOOL		m_bLargeNumbers;
	LTFLOAT		m_fWeaponAlpha;


    LTIntPt     m_DamageBasePos;
	int			m_nDamageIconSize;
	HSURFACE	m_hBleeding;
	HSURFACE	m_hPoisoned;
	HSURFACE	m_hStunned;
	HSURFACE	m_hSleeping;
	HSURFACE	m_hBurning;
	HSURFACE	m_hChoking;
	HSURFACE	m_hElectrocute;

	HSURFACE	m_hTargetNameSurface;
	int			m_nTargetNameWidth;
	int			m_nTargetNameHeight;
};

#endif