// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerStats.cpp
//
// PURPOSE : Implementation of PlayerStats class
//
// CREATED : 10/9/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerStats.h"
#include "WeaponMgr.h"
#include <stdio.h>
#include "GameClientShell.h"
#include "iltclient.h"
#include "ClientRes.h"
#include "MsgIDs.h"
#include "MissionData.h"
#include "LayoutMgr.h"
#include "VarTrack.h"
#include "SoundMgr.h"
#include "SurfaceFunctions.h"
#include "CharacterFX.h"

extern CGameClientShell* g_pGameClientShell;


#define CH_LEFT			(1 << 0)
#define CH_RIGHT		(1 << 1)
#define CH_TOP			(1 << 2)
#define CH_BOTTOM		(1 << 3)
#define CH_XLEFT		(1 << 4)
#define CH_XRIGHT		(1 << 5)
#define CH_XTOP			(1 << 6)
#define CH_XBOTTOM		(1 << 7)
#define CH_DOT			(1 << 8)
#define CH_FULLRIGHT	(1 << 9)
#define CH_FULLBOTTOM	(1 << 10)

VarTrack g_vtHUDLayout;

namespace
{
    HSURFACE hCrossHighlight = LTNULL;

	VarTrack g_vtCrosshairGapMin;
	VarTrack g_vtCrosshairGapMax;
	VarTrack g_vtCrosshairBarMin;
	VarTrack g_vtCrosshairBarMax;
	VarTrack g_vtCrosshairStyle;
	VarTrack g_vtCrosshairColorR;
	VarTrack g_vtCrosshairColorG;
	VarTrack g_vtCrosshairColorB;
	VarTrack g_vtCrosshairAlpha;
	VarTrack g_vtCrosshairDynamic;

	VarTrack g_vtScopeLRGap;
	VarTrack g_vtScopeUDGap;
	VarTrack g_vtScopeLRRadius;
	VarTrack g_vtScopeUDRadius;

    LTFLOAT   fLastColorR        = 0.0f;
    LTFLOAT   fLastColorG        = 0.0f;
    LTFLOAT   fLastColorB        = 0.0f;
    LTFLOAT   fLastAlpha         = 0.0f;
    HLTCOLOR hCursorColor       = LTNULL;
    LTFLOAT   fRad               = 0.0f;

	CLTGUIFont *g_pObjForeFont;
	CLTGUIFont *g_pForeFont;

    LTRect      g_rcObjectives;
	int 		g_nObjBmpOffset;
	LTIntPt 	g_ObjTextOffset;
	uint32		g_dwObjHeight;

	HLTCOLOR	hHealthTint = LTNULL;
	HLTCOLOR	hArmorTint = LTNULL;
	HLTCOLOR	hAmmoTint = LTNULL;

	 // Default activation distance

	const LTFLOAT c_ActivationDist = 100.0f;

	char szTriggers[10][2] =
	{
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"0"
	};

}


static LTBOOL DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	// Make sure we hit a surface type we care about...

	SurfaceType eSurfType = GetSurfaceType(hPoly);

	if (eSurfType == ST_INVISIBLE)
	{
        return LTFALSE;
	}

    return LTTRUE;
}

static LTBOOL ActivateFilterFn(HOBJECT hTest, void *pUserData)
{
	CPlayerStats* pStats = (CPlayerStats*)pUserData;
	if (!pStats) return LTFALSE;

	if (pStats->IsGadgetActivatable(hTest)) return LTTRUE;

	// Okay, do normal tests...

    HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(),
		g_pGameClientShell->GetMoveMgr()->GetObject(), LTNULL};

	if (ObjListFilterFn(hTest, (void*) hFilterList))
	{
		// Ignore non-solid objects that don't have the
		// activate user flag set...

		uint32 dwFlags = g_pLTClient->GetObjectFlags(hTest);

		if (!(dwFlags & FLAG_SOLID))
		{
			g_pLTClient->GetObjectUserFlags(hTest, &dwFlags);
			if (!(dwFlags & USRFLG_CAN_ACTIVATE))
			{
				return LTFALSE;
			}
		}
	}
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CPlayerStats()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerStats::CPlayerStats()
{
    LTRect nullrect(0,0,0,0);
    LTIntPt nullPt(0,0);

    m_pnAmmo            = LTNULL;
    m_pbHaveAmmo        = LTNULL;
    m_pbHaveWeapon      = LTNULL;
    m_pbCanUseWeapon    = LTNULL;
    m_pbCanUseAmmo      = LTNULL;
    m_pbHaveMod         = LTNULL;
    m_pbCanUseMod       = LTNULL;
    m_pbHaveGear        = LTNULL;
    m_pbCanUseGear      = LTNULL;
	m_nHealth			= 0;
	m_nDamage			= 0;
	m_nArmor			= 0;
	m_nMaxHealth		= 0;
	m_nMaxArmor			= 0;
	m_nCurrentWeapon	= WMGR_INVALID_ID;
	m_nCurrentAmmo		= WMGR_INVALID_ID;

    m_bHealthChanged = LTFALSE;
    m_bArmorChanged = LTFALSE;
    m_bAmmoTypeChanged = LTFALSE;
    m_bAmmoChanged = LTFALSE;

	m_fAirPercent = 1.0f;

	m_nCrosshairLevel = 2;
    m_bCrosshairEnabled = LTTRUE;

    m_hArmedCrosshair		= LTNULL;
    m_hUnarmedCrosshair		= LTNULL;
    m_hActivateCrosshair	= LTNULL;
	m_hInnocentCrosshair	= LTNULL;
	m_hActivateGadgetCrosshair	= LTNULL;
	m_bDrawingGadgetActivate	= LTFALSE;

    m_hHUDHealth = LTNULL;
    m_hHealthIcon = LTNULL;
    m_hArmorIcon = LTNULL;
    m_hHealthBar = LTNULL;
    m_hArmorBar = LTNULL;
	m_rcHealth = nullrect;
	m_rcHealthBar = nullrect;
	m_rcArmor = nullrect;
	m_rcArmorBar = nullrect;
    m_bHealthInit = LTFALSE;

    m_hHUDAmmo = LTNULL;
    m_hAmmoBar = LTNULL;
    m_hAmmoIcon = LTNULL;
    m_hAmmoFull = LTNULL;
    m_hAmmoEmpty = LTNULL;
	m_AmmoSz = nullPt;
	m_rcAmmo = nullrect;
	m_rcAmmoBar = nullrect;
    m_bAmmoInit = LTFALSE;


    m_hHUDAir = LTNULL;
    m_hAirBar = LTNULL;
    m_hAirStr = LTNULL;
    m_hAirIcon = LTNULL;
	m_rcAir = nullrect;
	m_rcAirBar = nullrect;
    m_bAirInit = LTFALSE;


    m_bDrawAmmo = LTFALSE;

    m_bHealthFlash = LTFALSE;
	m_fCurrHealthFlash = 0.0f;
	m_fTotalHealthFlash = 0.0f;
    m_bArmorFlash = LTFALSE;
	m_fCurrArmorFlash = 0.0f;
	m_fTotalArmorFlash = 0.0f;
	m_fFlashSpeed = 0.0f;
	m_fFlashDuration = 0.0f;

	m_nCurrentLayout = -1;
    m_bBarsChanged   = LTFALSE;

    m_bUseAmmoBar    = LTFALSE;
	m_AmmoBasePos = nullPt;
	m_AmmoClipOffset = nullPt;
	m_AmmoBarOffset = nullPt;
    m_bUseAmmoText = LTFALSE;
	m_AmmoTextOffset = nullPt;
	m_AmmoIconOffset = nullPt;

    m_bUseHealthBar = LTFALSE;
	m_HealthBasePos = nullPt;
	m_HealthBarOffset = nullPt;
	m_ArmorBarOffset = nullPt;
    m_bUseHealthText = LTFALSE;
	m_HealthTextOffset = nullPt;
	m_ArmorTextOffset = nullPt;
    m_bUseHealthIcon = LTFALSE;
	m_HealthIconOffset = nullPt;
	m_ArmorIconOffset = nullPt;

    m_bUseAirIcon = LTFALSE;
	m_AirBasePos = nullPt;
	m_AirIconOffset = nullPt;
    m_bUseAirText = LTFALSE;
	m_AirTextOffset = nullPt;
    m_bUseAirBar = LTFALSE;
	m_AirBarOffset = nullPt;
	m_nBarHeight = 0;
	m_fBarScale = 0;

	m_ModeTextPos = nullPt;
    m_hModeStr = LTNULL;
	m_eLastMode = SUN_NONE;

    m_bObjAdded = LTFALSE;
    m_bObjRemoved = LTFALSE;
    m_bObjCompleted = LTFALSE;
    m_bObjVisible = LTFALSE;

	memset(m_nBoundWeapons,0,sizeof(m_nBoundWeapons));
	memset(m_hWeaponSurf,0,sizeof(m_hWeaponSurf));
	memset(m_hNumberSurf,0,sizeof(m_hNumberSurf));
	m_bLargeNumbers = LTFALSE;
	m_fWeaponAlpha	= 1.0f;

	m_hBleeding = LTFALSE;
	m_hPoisoned = LTFALSE;
	m_hStunned = LTFALSE;
	m_hSleeping = LTFALSE;
	m_hBurning = LTFALSE;
	m_hChoking = LTFALSE;
	m_hElectrocute = LTFALSE;
	m_nDamageIconSize = 0;
	m_DamageBasePos = nullPt;

	m_nTargetNameWidth = 0;
	m_nTargetNameHeight = 0;
	m_hTargetNameSurface = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::~CPlayerStats()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPlayerStats::~CPlayerStats()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerStats::Init()
{
    if (!g_pLTClient) return LTFALSE;

	if (m_pnAmmo)
	{
		debug_deletea(m_pnAmmo);
        m_pnAmmo = LTNULL;
	}

	if (m_pbHaveAmmo)
	{
		debug_deletea(m_pbHaveAmmo);
        m_pbHaveAmmo = LTNULL;
	}

	if (m_pbCanUseAmmo)
	{
		debug_deletea(m_pbCanUseAmmo);
        m_pbCanUseAmmo = LTNULL;
	}

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();
	if (nNumAmmoTypes > 0)
	{
        m_pnAmmo = debug_newa(uint32, nNumAmmoTypes);
        m_pbCanUseAmmo = debug_newa(LTBOOL, nNumAmmoTypes);
        m_pbHaveAmmo = debug_newa(LTBOOL, nNumAmmoTypes);
        memset(m_pnAmmo, 0, sizeof(uint32) * nNumAmmoTypes);
        memset(m_pbHaveAmmo, 0, sizeof(LTBOOL) * nNumAmmoTypes);
        memset(m_pbCanUseAmmo, 0, sizeof(LTBOOL) * nNumAmmoTypes);
	}

	if (m_pbHaveWeapon)
	{
		debug_deletea(m_pbHaveWeapon);
        m_pbHaveWeapon = LTNULL;
	}

	if (m_pbCanUseWeapon)
	{
		debug_deletea(m_pbCanUseWeapon);
        m_pbCanUseWeapon = LTNULL;
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
        m_pbHaveWeapon = debug_newa(LTBOOL, nNumWeapons);
        memset(m_pbHaveWeapon, 0, sizeof(LTBOOL) * nNumWeapons);

        m_pbCanUseWeapon = debug_newa(LTBOOL, nNumWeapons);
        memset(m_pbCanUseWeapon, 0, sizeof(LTBOOL) * nNumWeapons);
	}

	if (m_pbHaveMod)
	{
		debug_deletea(m_pbHaveMod);
        m_pbHaveMod = LTNULL;
	}

	if (m_pbCanUseMod)
	{
		debug_deletea(m_pbCanUseMod);
        m_pbCanUseMod = LTNULL;
	}

	if (m_pbHaveGear)
	{
		debug_deletea(m_pbHaveGear);
        m_pbHaveGear = LTNULL;
	}

	if (m_pbCanUseGear)
	{
		debug_deletea(m_pbCanUseGear);
        m_pbCanUseGear = LTNULL;
	}

	int nNumMods = g_pWeaponMgr->GetNumModTypes();
	if (nNumMods > 0)
	{
        m_pbHaveMod = debug_newa(LTBOOL, nNumMods);
        memset(m_pbHaveMod, 0, sizeof(LTBOOL) * nNumMods);

        m_pbCanUseMod = debug_newa(LTBOOL, nNumMods);
        memset(m_pbCanUseMod, 0, sizeof(LTBOOL) * nNumMods);

	}

	int nNumGear = g_pWeaponMgr->GetNumGearTypes();
	if (nNumGear > 0)
	{
        m_pbHaveGear = debug_newa(LTBOOL, nNumGear);
        memset(m_pbHaveGear, 0, sizeof(LTBOOL) * nNumGear);

        m_pbCanUseGear = debug_newa(LTBOOL, nNumGear);
        memset(m_pbCanUseGear, 0, sizeof(LTBOOL) * nNumGear);

	}

    g_vtHUDLayout.Init(g_pLTClient, "HUDLayout", NULL, 0.0f);

    LTFLOAT fTemp = g_pLayoutMgr->GetCrosshairGapMin();
	if (fTemp <= 0.0f)
		fTemp = 5.0f;
    g_vtCrosshairGapMin.Init(g_pLTClient, "CrosshairGapMin", NULL, fTemp);

	fTemp = g_pLayoutMgr->GetCrosshairGapMax();
	if (fTemp <= 0.0f)
		fTemp = 8.0f;
    g_vtCrosshairGapMax.Init(g_pLTClient, "CrosshairGapMax", NULL, fTemp);

	fTemp = g_pLayoutMgr->GetCrosshairBarMin();
	if (fTemp <= 0.0f)
		fTemp = 5.0f;
    g_vtCrosshairBarMin.Init(g_pLTClient, "CrosshairBarMin", NULL, fTemp);

	fTemp = g_pLayoutMgr->GetCrosshairBarMax();
	if (fTemp <= 0.0f)
		fTemp = 8.0f;
    g_vtCrosshairBarMax.Init(g_pLTClient, "CrosshairBarMax", NULL, fTemp);

    g_vtCrosshairStyle.Init(g_pLTClient, "CrosshairStyle", NULL, 0.0f);

    g_vtCrosshairColorR.Init(g_pLTClient, "CrosshairColorR", NULL, 1.0f);
    g_vtCrosshairColorG.Init(g_pLTClient, "CrosshairColorG", NULL, 1.0f);
    g_vtCrosshairColorB.Init(g_pLTClient, "CrosshairColorB", NULL, 1.0f);
    g_vtCrosshairAlpha.Init(g_pLTClient, "CrosshairAlpha", NULL, 1.0f);
    g_vtCrosshairDynamic.Init(g_pLTClient, "CrosshairDynamic", NULL, 1.0f);

    g_vtScopeLRGap.Init(g_pLTClient, "ScopeLRGap", NULL, 24.0f);
    g_vtScopeUDGap.Init(g_pLTClient, "ScopeUPGap", NULL, 24.0f);
    g_vtScopeLRRadius.Init(g_pLTClient, "ScopeLRRadius", NULL, 0.4845f);
    g_vtScopeUDRadius.Init(g_pLTClient, "ScopeUDRadius", NULL, 0.449f);

	InitPlayerStats();

	g_rcObjectives = g_pLayoutMgr->GetObjectiveRect();

	HSURFACE hTempSurf = g_pInterfaceResMgr->GetSharedSurface("interface\\check-on.pcx");
	uint32 dwBmpHeight, dwBmpWidth;
	g_pLTClient->GetSurfaceDims(hTempSurf, &dwBmpHeight, &dwBmpWidth);

	uint32 dwTextHeight = 0;
	g_pObjForeFont = g_pInterfaceResMgr->GetMsgForeFont();
	g_pForeFont = g_pInterfaceResMgr->GetHUDForeFont();

	if (g_pObjForeFont)
		dwTextHeight = (uint32)g_pObjForeFont->GetHeight();

	g_dwObjHeight = Max(dwBmpHeight,dwTextHeight);

	g_nObjBmpOffset = (int)(g_dwObjHeight - dwBmpHeight) / 2;
	g_ObjTextOffset.x = (int)(dwBmpWidth + 8);
	g_ObjTextOffset.y = (int)(g_dwObjHeight - dwTextHeight) / 2;

	hHealthTint = g_pLayoutMgr->GetHealthTint();
	hArmorTint = g_pLayoutMgr->GetArmorTint();
	hAmmoTint = g_pLayoutMgr->GetAmmoTint();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Term()
//
//	PURPOSE:	Terminate the player stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::Term()
{
    if (!g_pLTClient) return;

	if ( m_hTargetNameSurface )
	{
		g_pLTClient->DeleteSurface(m_hTargetNameSurface);
		m_hTargetNameSurface = LTNULL;
	}

	DeleteCrosshairs();

	if (m_hModeStr)
	{
        g_pLTClient->FreeString(m_hModeStr);
        m_hModeStr = LTNULL;
	}


	DestroyHealthSurfaces();
	DestroyAmmoSurfaces();
	DestroyAirSurfaces();
	DestroyDamageSurfaces();


	m_nHealth		= 0;
	m_nDamage		= 0;
	m_nArmor		= 0;
	m_nMaxHealth	= 0;
	m_nMaxArmor		= 0;

	if (m_pnAmmo)
	{
		debug_deletea(m_pnAmmo);
        m_pnAmmo = LTNULL;
	}

	if (m_pbCanUseAmmo)
	{
		debug_deletea(m_pbCanUseAmmo);
        m_pbCanUseAmmo = LTNULL;
	}

	if (m_pbHaveAmmo)
	{
		debug_deletea(m_pbHaveAmmo);
        m_pbHaveAmmo = LTNULL;
	}

	if (m_pbHaveWeapon)
	{
		debug_deletea(m_pbHaveWeapon);
        m_pbHaveWeapon = LTNULL;
	}

	if (m_pbCanUseWeapon)
	{
		debug_deletea(m_pbCanUseWeapon);
        m_pbCanUseWeapon = LTNULL;
	}

	if (m_pbHaveMod)
	{
		debug_deletea(m_pbHaveMod);
        m_pbHaveMod = LTNULL;
	}

	if (m_pbCanUseMod)
	{
		debug_deletea(m_pbCanUseMod);
        m_pbCanUseMod = LTNULL;
	}

	if (m_pbHaveGear)
	{
		debug_deletea(m_pbHaveGear);
        m_pbHaveGear = LTNULL;
	}

	if (m_pbCanUseGear)
	{
		debug_deletea(m_pbCanUseGear);
        m_pbCanUseGear = LTNULL;
	}

	for (int i = 0; i < 10; i++)
	{
		if (m_hWeaponSurf[i])
		{
			g_pLTClient->DeleteSurface(m_hWeaponSurf[i]);
			m_hWeaponSurf[i] = LTNULL;
		}
		if (m_hNumberSurf[i])
		{
			g_pLTClient->DeleteSurface(m_hNumberSurf[i]);
			m_hNumberSurf[i] = LTNULL;
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::OnEnterWorld()
//
//	PURPOSE:	Handle entering the world
//
// ----------------------------------------------------------------------- //

void CPlayerStats::OnEnterWorld(LTBOOL bRestoringGame)
{
    if (!g_pLTClient || !g_pGameClientShell) return;

	// find out what mode we are in and make sure that mode is set

	ResetStats();

	if (!bRestoringGame)
	{
		// clear the values

		Clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::OnExitWorld()
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CPlayerStats::OnExitWorld()
{
    if (!g_pLTClient) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Draw()
//
//	PURPOSE:	Handle drawing the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::Draw(LTBOOL bShowStats)
{
    if (!g_pLTClient || !g_pGameClientShell || !g_pGameClientShell->GetCamera()) return;

    float m_nCurrentTime = g_pLTClient->GetTime();

	// get the screen size

    uint32 nWidth = 0;
    uint32 nHeight = 0;
    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);


	DrawPlayerStats(hScreen, 0, 0, nWidth, nHeight, bShowStats);

	if (m_bObjVisible)
		DrawObjectives(hScreen);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Clear()
//
//	PURPOSE:	Handle clearing the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::Clear()
{
	UpdateHealth(0);
	m_nDamage = 0;
	UpdateArmor(0);

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();
    for (uint8 i = 0; i < nNumAmmoTypes; i++)
	{
		UpdateAmmo(m_nCurrentWeapon, i, 0);
	}

    m_bHealthFlash  = LTFALSE;
	m_fCurrHealthFlash	= 0.0f;
	m_fTotalHealthFlash	= 0.0f;

    m_bArmorFlash   = LTFALSE;
	m_fCurrArmorFlash	= 0.0f;
	m_fTotalArmorFlash	= 0.0f;


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Update()
//
//	PURPOSE:	Handle updates
//
// ----------------------------------------------------------------------- //

void CPlayerStats::Update()
{
	if (m_nCurrentLayout != (int)g_vtHUDLayout.GetFloat())
		UpdateLayout();

	if	(	fLastColorR != g_vtCrosshairColorR.GetFloat() || fLastColorG != g_vtCrosshairColorG.GetFloat() ||
			fLastColorB != g_vtCrosshairColorB.GetFloat() || fLastAlpha != g_vtCrosshairAlpha.GetFloat() )
	{
		UpdateCrosshairColors();
	}


	// did the player's health change?
	if (m_bHealthChanged || m_bArmorChanged)
	{
		UpdatePlayerHealth();
        m_bHealthChanged = LTFALSE;
        m_bArmorChanged = LTFALSE;
	}

	// did the player's ammo count change?

	if ((m_bAmmoChanged || m_bAmmoTypeChanged) && m_bDrawAmmo)
	{
		UpdatePlayerAmmo();
        m_bAmmoChanged = LTFALSE;
	}

	if (m_bObjAdded || m_bObjRemoved || m_bObjCompleted)
	{
		//use if-else conditional to enforce priority of notification
		if (m_bObjCompleted)
		{
			g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetObjectiveCompletedSound());
		}
		else if (m_bObjAdded)
		{
			g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetObjectiveAddedSound());
		}
		else if (m_bObjRemoved)
		{
			g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetObjectiveRemovedSound());
		}

		if ( GetConsoleInt("ObjectiveMessages",1) > 0 )
			g_pInterfaceMgr->GetMessageMgr()->AddLine(IDS_OBJECTIVES_CHANGED);

        m_bObjAdded = LTFALSE;
        m_bObjRemoved = LTFALSE;
        m_bObjCompleted = LTFALSE;
	}

	m_bObjVisible = (g_pLTClient->IsCommandOn(COMMAND_ID_MISSION) && g_pInterfaceMgr->GetGameState() == GS_PLAYING);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ResetStats()
//
//	PURPOSE:	Reset the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::ResetStats()
{
	// remove all currently loaded surfaces
	DeleteCrosshairs();

	// load new surfaces

	InitPlayerStats();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdatePlayerWeapon()
//
//	PURPOSE:	Update the weapon related stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdatePlayerWeapon(uint8 nWeaponId, uint8 nAmmoId, LTBOOL bForce)
{
	if (!g_pGameClientShell || !g_pWeaponMgr->IsValidWeapon(nWeaponId)) return;

	if (!bForce)
	{
		if (m_nCurrentWeapon == nWeaponId && m_nCurrentAmmo == nAmmoId) return;
	}

	m_nCurrentWeapon = nWeaponId;

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_nCurrentWeapon);
	if (!pWeapon) return;

	if (nAmmoId != m_nCurrentAmmo)
	{
		m_nCurrentAmmo = nAmmoId;
        m_bAmmoTypeChanged = LTTRUE;
	}

    SetDrawAmmo(LTTRUE);

	Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateHealth()
//
//	PURPOSE:	Update the health stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateHealth(uint32 nHealth)
{
	if (m_nHealth == nHealth) return;

	// update the member variable

	if (nHealth < m_nHealth)
	{
        m_bHealthFlash  = LTTRUE;
		m_fCurrHealthFlash	= m_fFlashSpeed;
		m_fTotalHealthFlash	= m_fFlashDuration;
		m_nDamage += (m_nHealth  - nHealth);
	}
	m_nHealth = nHealth;
    m_bHealthChanged = LTTRUE;


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateArmor()
//
//	PURPOSE:	Update the armor stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateArmor(uint32 nArmor)
{
	if (m_nArmor == nArmor) return;

	if (nArmor > m_nMaxArmor)
		nArmor = m_nMaxArmor;
	// update the member variable

	if (nArmor != m_nArmor)
	{
        m_bArmorFlash   = LTTRUE;
		m_fCurrArmorFlash	= m_fFlashSpeed;
		m_fTotalArmorFlash	= m_fFlashDuration;
	}
	m_nArmor = nArmor;
    m_bArmorChanged = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateMaxHealth()
//
//	PURPOSE:	Update the health stat maximum
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateMaxHealth(uint32 nHealth)
{
	if (m_nMaxHealth == nHealth) return;

	// update the member variable
	m_nMaxHealth = nHealth;

	UpdateHealthSurfaces();
    m_bHealthChanged = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateMaxArmor()
//
//	PURPOSE:	Update the armor stat maximum
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateMaxArmor(uint32 nArmor)
{
	if (m_nMaxArmor == nArmor) return;

	// update the member variable
	m_nMaxArmor = nArmor;

	UpdateHealthSurfaces();
    m_bArmorChanged = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateAmmo()
//
//	PURPOSE:	Update the ammo stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateAmmo(uint8 nWeaponId, uint8 nAmmoId,
                              uint32 nAmmo, LTBOOL bPickedup,
                              LTBOOL bDisplayMsg)
{
    if (!g_pLTClient || !g_pWeaponMgr) return;

	int nMissionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(nMissionNum);

	if (bPickedup && g_pWeaponMgr->IsValidWeapon(nWeaponId))
	{
		if (m_pbHaveWeapon)
		{
			if (!m_pbHaveWeapon[nWeaponId] && bDisplayMsg)
			{
				g_pGameClientShell->HandleWeaponPickup(nWeaponId);
			}

            m_pbHaveWeapon[nWeaponId] = LTTRUE;
			if (m_pbCanUseWeapon && pMission && !pMission->IsOneTimeWeapon(nWeaponId) && !pMission->IsOneTimeGadget(nWeaponId))
                m_pbCanUseWeapon[nWeaponId] = LTTRUE;

		}
	}

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);

	if (bPickedup && pWeapon && m_pbHaveWeapon[nWeaponId])
	{
		for (int m = 0; m < pWeapon->nNumModTypes; m++)
		{
			int nModId = pWeapon->aModTypes[m];
			MOD* pMod = g_pWeaponMgr->GetMod(nModId);

			if (pMod && pMod->bIntegrated)
			{
				if (m_pbHaveMod)
				{
                    m_pbHaveMod[nModId] = LTTRUE;
					if (m_pbCanUseMod && pMission)
						m_pbCanUseMod[nModId] = LTTRUE;
				}
			}
		}
	}

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);

    LTBOOL bInfiniteAmmo = pWeapon ? pWeapon->bInfiniteAmmo : LTFALSE;
	int nEquipWeapon = WMGR_INVALID_ID;

	if (pAmmo && !bInfiniteAmmo)
	{
		if (m_pnAmmo)
		{
			if (bPickedup) /*** BL 12/08/2000 Added to equip weapon when you are out of ammo and pick up ammo/weapon ******/
			{
				if ( IsMultiplayerGame() && (LTBOOL)GetConsoleInt("AutoAmmoSwitch",1) )
				{
					// Check to see if we are out of ammo (minus gadgets, infinite ammo weapons like fisticuffs, barette, etc)

					LTBOOL bOutOfAmmo = LTTRUE;
					int nWeaponBest = WMGR_INVALID_ID;

					for ( int32 iWeapon = g_pWeaponMgr->GetNumWeapons()-1 ; iWeapon >= 0 ; iWeapon-- )
					{
						int32 nWeapon = g_pWeaponMgr->GetWeaponId(iWeapon);
						if ( nWeapon != WMGR_INVALID_ID && m_pbHaveWeapon[nWeapon] )
						{
							WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeapon);

							if ( !pWeapon->bInfiniteAmmo )
							{
								for ( int32 iAmmo = 0 ; iAmmo < pWeapon->nNumAmmoTypes ; iAmmo++ )
								{
									if ( (nWeaponBest == WMGR_INVALID_ID) && (pWeapon->aAmmoTypes[iAmmo] == nAmmoId) )
									{
										nWeaponBest = nWeapon;
									}

									AMMO* pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->aAmmoTypes[iAmmo]);

									if ( pAmmo->eType == GADGET )
									{
									}
									else if ( m_pnAmmo[pWeapon->aAmmoTypes[iAmmo]] > 0 )
									{
										//g_pLTClient->CPrint("%s has %d rounds of %s", pWeapon->szName, m_pnAmmo[pWeapon->aAmmoTypes[iAmmo]], pAmmo->szName);
										bOutOfAmmo = LTFALSE;
										break;
									}
									else
									{
										//g_pLTClient->CPrint("%s is out of %s", pWeapon->szName, pAmmo->szName);
									}
								}
							}
						}
					}

					if ( bOutOfAmmo )
					{
						//g_pLTClient->CPrint("Out of ammo!");

						if ( nWeaponId == WMGR_INVALID_ID )
						{
							// Picked up ammo -- equip the best gun for that can use the ammo

							nEquipWeapon = nWeaponBest;
						}
						else
						{
							// Picked up a gun -- equip that gun

							nEquipWeapon = nWeaponId;
						}
					}
					else
					{
						//g_pLTClient->CPrint("Not out of ammo!");
					}
				}
			}  /*** BL 12/08/2000 End changes ******/

            uint32 maxAmmo = pAmmo->GetMaxAmount(LTNULL);
			if (m_pnAmmo[nAmmoId] > maxAmmo)
			{
				m_pnAmmo[nAmmoId] = maxAmmo;
			}

			if (nAmmo > maxAmmo)
			{
				nAmmo = maxAmmo;
			}

			int taken = nAmmo - m_pnAmmo[nAmmoId];

			m_pnAmmo[nAmmoId] = nAmmo;

			if (taken != 0)
			{
				if (bPickedup && bDisplayMsg)
				{
					g_pGameClientShell->HandleAmmoPickup(nAmmoId,taken);
				}

				if (bPickedup)  /*** BL 12/08/2000 Added to auto-switch to new ammo if you pick up better ammo ******/
				{
					if ( IsMultiplayerGame() && (LTBOOL)GetConsoleInt("AutoAmmoSwitch",1) )
					{
						CWeaponModel* pWeapon = g_pGameClientShell->GetWeaponModel();
						
						if (pWeapon)
						{
							if ( (nWeaponId == WMGR_INVALID_ID) || 
								(pWeapon->GetWeaponId() == nWeaponId) )
							{
								if (pWeapon->CanCurrentWeaponUseAmmo(nAmmoId))
								{
									AMMO *pAmmoCurrent = pWeapon->GetAmmo();
									AMMO *pAmmoNew = g_pWeaponMgr->GetAmmo(nAmmoId);
									if ( (pAmmoNew && pAmmoCurrent) && (pAmmoNew->fPriority > pAmmoCurrent->fPriority) )
									{
										pWeapon->ChangeAmmo(nAmmoId, LTTRUE);
										pWeapon->ReloadClip(LTTRUE, -1, LTTRUE);
									}
								}
							}
						}
					}
				} /*** BL 12/08/2000 End changes ******/

				if (pMission)
				{
					if (m_pbHaveAmmo && !pMission->IsOneTimeAmmo(nAmmoId))
					{
                        m_pbHaveAmmo[nAmmoId] = LTTRUE;
						if (m_pbCanUseAmmo && pMission && !pMission->IsOneTimeAmmo(nAmmoId))
							m_pbCanUseAmmo[nAmmoId] = LTTRUE;
					}
				}
			}
		}
	}

	if ( nEquipWeapon != WMGR_INVALID_ID )  /*** BL 12/08/2000 Added to change equip weapon when you are out of ammo and pick up a weapon ******/
	{
		g_pGameClientShell->ChangeWeapon(nEquipWeapon, nAmmoId, nAmmo);
		g_pGameClientShell->GetWeaponModel()->ChangeAmmo(nAmmoId);
		g_pGameClientShell->GetWeaponModel()->ReloadClip(LTFALSE, -1, LTTRUE);
	}  /*** BL 12/08/2000 End changes ******/

	if (m_nCurrentAmmo == nAmmoId)
	{
        m_bAmmoChanged = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateGear()
//
//	PURPOSE:	Update the gear stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateGear(uint8 nGearId)
{
    if (!g_pLTClient || !g_pWeaponMgr) return;

	int nMissionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(nMissionNum);

	if (g_pWeaponMgr->IsValidGearType(nGearId))
	{
        LTBOOL bHadAirSupply = HaveAirSupply();
		if (m_pbHaveGear)
		{
            m_pbHaveGear[nGearId] = LTTRUE;
			if (m_pbCanUseGear && pMission && !pMission->IsOneTimeGear(nGearId))
			{
				GEAR *pGear = g_pWeaponMgr->GetGear(nGearId);
				if (pGear && pGear->bExclusive)
					m_pbCanUseGear[nGearId] = LTTRUE;
			}

		}

		if (g_pGameClientShell->IsUnderwater() && HaveAirSupply() && !bHadAirSupply)
			g_pInterfaceMgr->BeginUnderwater();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateMod()
//
//	PURPOSE:	Update the mod stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateMod(uint8 nModId)
{
	if (g_pWeaponMgr->IsValidModType(nModId))
	{
		if (m_pbHaveMod)
		{
			if (!m_pbHaveMod[nModId])
			{
				m_pbHaveMod[nModId] = LTTRUE;
				m_pbCanUseMod[nModId] = LTTRUE;

				// Make sure the player-view weapon is updated if
				// this mod is used with the weapon...

				MOD* pMod = g_pWeaponMgr->GetMod(nModId);
				if (!pMod) return;

				CWeaponModel* pWeapon = g_pGameClientShell->GetWeaponModel();
				if (pWeapon->GetWeaponId() == pMod->GetWeaponId())
				{
					pWeapon->CreateMods();
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateAir()
//
//	PURPOSE:	Update the air stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateAir(LTFLOAT fPercent)
{
    if (!g_pLTClient) return;

	if (fPercent > 1.0f)
		fPercent = 1.0f;
	if (fPercent < 0.0f)
		fPercent = 0.0f;

	if (m_fAirPercent == fPercent) return;

	m_fAirPercent = fPercent;

	// clear the meter
	ClearSurface(m_hHUDAir, SETRGB(25,50,50));

    LTRect rcPower = m_rcAir;
	rcPower.right = m_rcAir.left + (int)(100.0f * fPercent);
    g_pLTClient->ScaleSurfaceToSurfaceTransparent(m_hHUDAir, m_hAirBar, &rcPower, &m_rcAirBar, kTransBlack);

	// clear the text
	char szStr[5] = "";
	sprintf(szStr,"%d",(int)(100.0f * fPercent));
	ClearSurface(m_hAirStr, kTransBlack);
	g_pInterfaceResMgr->GetAirFont()->Draw(szStr, m_hAirStr, 0, 0, LTF_JUSTIFY_LEFT);
    g_pLTClient->OptimizeSurface(m_hAirStr, kTransBlack);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateObjectives()
//
//	PURPOSE:	Update the objectives
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateObjectives(uint8 nType, uint8 nTeam, uint32 dwId)
{
	if (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT && nTeam != 0)
	{
		CLIENT_INFO* pInfo = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if (!pInfo || nTeam != pInfo->team)
			return;
	}

	switch (nType)
	{
		case OBJECTIVE_ADD_ID:
		{

			if (m_Objectives.Add(dwId))
                m_bObjAdded = LTTRUE;
		}
		break;

		case OBJECTIVE_REMOVE_ID:
		{
			m_bObjRemoved = m_Objectives.Remove(dwId);
			m_bObjRemoved |= m_CompletedObjectives.Remove(dwId);

		}
		break;

		case OBJECTIVE_COMPLETE_ID:
		{
			m_bObjCompleted = m_Objectives.Add(dwId);
			m_bObjCompleted |= m_CompletedObjectives.Add(dwId);

		}
		break;

		case OBJECTIVE_CLEAR_ID:
		{
			if (m_Objectives.nNumObjectives || m_CompletedObjectives.nNumObjectives)
                m_bObjRemoved = LTTRUE;
			m_Objectives.Clear();
			m_CompletedObjectives.Clear();
		}
		break;

		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::SetMultiplayerObjectives()
//
//	PURPOSE:	Override current objectives with new list from server
//
// ----------------------------------------------------------------------- //
void CPlayerStats::SetMultiplayerObjectives(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

	if (m_Objectives.nNumObjectives || m_CompletedObjectives.nNumObjectives)
        m_bObjRemoved = LTTRUE;
	m_Objectives.Clear();
	m_CompletedObjectives.Clear();

	//read number of objectives
    uint8 nNumObj = g_pLTClient->ReadFromMessageByte(hMessage);

	//read list of objectives
    for (uint8 i = 0; i < nNumObj; i++)
	{
        uint32 dwId = g_pLTClient->ReadFromMessageDWord(hMessage);
		m_Objectives.Add(dwId);
	}

	//read number of completed objectives
    uint8 nNumCompObj = g_pLTClient->ReadFromMessageByte(hMessage);

	//read list of completed objectives
	for (i = 0; i < nNumCompObj; i++)
	{
        uint32 dwId = g_pLTClient->ReadFromMessageDWord(hMessage);
		m_Objectives.Add(dwId);
		m_CompletedObjectives.Add(dwId);
	}

	m_bObjAdded = (nNumObj > 0) || (nNumCompObj > 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ToggleCrosshair()
//
//	PURPOSE:	Toggle the crosshairs on/off
//
// ----------------------------------------------------------------------- //

void CPlayerStats::ToggleCrosshair()
{
	int nMaxLevel = 1;

	if (--m_nCrosshairLevel < 0) m_nCrosshairLevel = nMaxLevel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::InitPlayerStats()
//
//	PURPOSE:	More initialization
//
// ----------------------------------------------------------------------- //

void CPlayerStats::InitPlayerStats()
{
	// make sure crosshair level is set correctly

	m_nCrosshairLevel = 1;

	CreateCrosshairs();

	UpdateLayout();

	CreateHealthSurfaces();
	CreateAmmoSurfaces();
	CreateAirSurfaces();
	CreateDamageSurfaces();

	// force updating of the surfaces

	UpdatePlayerHealth();
    UpdatePlayerWeapon(m_nCurrentWeapon, m_nCurrentAmmo, LTTRUE);

    m_bHealthFlash  = LTFALSE;
	m_fCurrHealthFlash	= 0.0f;
	m_fTotalHealthFlash	= 0.0f;

    m_bArmorFlash   = LTFALSE;
	m_fCurrArmorFlash	= 0.0f;
	m_fTotalArmorFlash	= 0.0f;

	m_fFlashSpeed = g_pLayoutMgr->GetFlashSpeed();
	m_fFlashDuration = g_pLayoutMgr->GetFlashDuration();

	memset(m_fIconOffset,0,sizeof(m_fIconOffset));
	UpdateWeaponBindings();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawPlayerStats()
//
//	PURPOSE:	Draw the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DrawPlayerStats(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom, LTBOOL bShowStats)
{
    if (!g_pLTClient) return;

	int nScreenWidth		= nRight - nLeft;
	int nScreenHeight		= nBottom - nTop;

	float xRatio = g_pInterfaceResMgr->GetXRatio();
	float yRatio = g_pInterfaceResMgr->GetYRatio();

	if (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT)
	{
		CLIENT_INFO* pInfo = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		TEAM_INFO* pTeam = g_pInterfaceMgr->GetClientInfoMgr()->GetTeam(pInfo->team);
		if (pInfo)
		{
			int nHeight = (int)(60.0f * yRatio);
			LTRect rcBanner(0,nScreenHeight-nHeight,nScreenWidth,nScreenHeight);
			g_pLTClient->ScaleSurfaceToSurface(hScreen, pTeam->hBanner, &rcBanner, LTNULL);
		}
	}

	// set up the transparent color

    HLTCOLOR hTransColor = LTNULL;

	eOverlayMask eCurrMask = g_pInterfaceMgr->GetCurrentOverlay();


	switch (eCurrMask)
	{
	case OVM_SCOPE:
		{
			if (g_pInterfaceMgr->GetSunglassMode() == SUN_NONE)
				DrawScope(hScreen,nLeft,nTop,nRight,nBottom);
		} break;

	case OVM_SCUBA:
		{
			DrawScuba(hScreen,nLeft,nTop,nRight,nBottom);
		} break;
	}

	eSunglassMode eSunMode = g_pInterfaceMgr->GetSunglassMode();
	if (eSunMode != SUN_NONE)
	{
		// For now no text
		// DrawSunglass(hScreen,nLeft,nTop,nRight,nBottom);
	}

	m_bDrawingGadgetActivate = LTFALSE;

	// Draw the crosshair
	LTBOOL bDrawCrosshair = m_bCrosshairEnabled && m_nCrosshairLevel;

	if (bDrawCrosshair && (eCurrMask != OVM_SCOPE || eSunMode == SUN_CAMERA))
	{
		int cx = nLeft + (nScreenWidth / 2);
		int cy = nTop + (nScreenHeight / 2);
		DrawCrosshair(hScreen,cx,cy);
	}


	DrawBoundWeapons(hScreen);

	CDamageFXMgr *pDFX = g_pGameClientShell->GetDamageFXMgr();
	if (pDFX)
	{
		int nDamageX = (int) ((float)m_DamageBasePos.x * xRatio);
		int nDamageY = (int) ((float)m_DamageBasePos.y * yRatio);
		if (pDFX->IsBleeding())
		{
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hBleeding, LTNULL, nDamageX, nDamageY, hTransColor);
			nDamageY += m_nDamageIconSize;
		}
		if (pDFX->IsPoisoned())
		{
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hPoisoned, LTNULL, nDamageX, nDamageY, hTransColor);
			nDamageY += m_nDamageIconSize;
		}
		if (pDFX->IsStunned())
		{
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hStunned, LTNULL, nDamageX, nDamageY, hTransColor);
			nDamageY += m_nDamageIconSize;
		}
		if (pDFX->IsSleeping())
		{
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hSleeping, LTNULL, nDamageX, nDamageY, hTransColor);
			nDamageY += m_nDamageIconSize;
		}
		if (pDFX->IsBurning())
		{
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hBurning, LTNULL, nDamageX, nDamageY, hTransColor);
			nDamageY += m_nDamageIconSize;
		}
		if (pDFX->IsChoking())
		{
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hChoking, LTNULL, nDamageX, nDamageY, hTransColor);
			nDamageY += m_nDamageIconSize;
		}
		if (pDFX->IsElectrocuted())
		{
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hElectrocute, LTNULL, nDamageX, nDamageY, hTransColor);
			nDamageY += m_nDamageIconSize;
		}

	}


	// draw air meter if we have less than 100% air

	if (m_fAirPercent < 1.0f)
	{
		int nAirX = (int) ((float)m_AirBasePos.x * xRatio);
		int nAirY = (int) ((float)m_AirBasePos.y * yRatio);
		if (m_bUseAirBar)
		{
			int nBarX = nAirX + m_AirBarOffset.x;
			int nBarY = nAirY + m_AirBarOffset.y;
            g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hHUDAir, LTNULL, nBarX, nBarY, hTransColor);
			if (m_fAirPercent < 0.25f)
			{
                LTRect rcPower = m_rcAir;
				rcPower.left += nBarX;
				rcPower.top += nBarY;
				rcPower.right = rcPower.left + (int)(100.0f * m_fAirPercent);
				rcPower.bottom += nBarY;
                g_pLTClient->ScaleSurfaceToSurfaceTransparent(hScreen, m_hAirBar, &rcPower, &m_rcAirBar, kTransBlack);
			}
		}
		if (m_bUseAirIcon)
		{
            g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hAirIcon, LTNULL, nAirX+m_AirIconOffset.x, nAirY+m_AirIconOffset.y, hTransColor);
		}
		if (m_bUseAirText)
		{
            g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hAirStr, LTNULL, nAirX+m_AirTextOffset.x, nAirY+m_AirTextOffset.y, hTransColor);
		}
	}

	// draw the HUD
	if (bShowStats)
	{
		int nHealthX = (int) ((float)m_HealthBasePos.x * xRatio);
		int nHealthY = (int) ((float)m_HealthBasePos.y * yRatio);
		if (m_bUseHealthBar)
		{
			int nBarX = nHealthX + m_HealthBarOffset.x;
			int nBarY = nHealthY + m_HealthBarOffset.y;

            g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hHUDHealth, NULL, nBarX, nBarY, hTransColor);
			// draw flashing health as required
			if (m_bHealthFlash ||  (m_nHealth <= (m_nMaxHealth/10)))
			{
                LTRect rcTemp = m_rcHealth;
				rcTemp.left += nBarX;
				rcTemp.top += nBarY;
				rcTemp.right += nBarX;
				rcTemp.bottom += nBarY;

                g_pLTClient->ScaleSurfaceToSurfaceTransparent(hScreen, m_hHealthBar, &rcTemp, &m_rcHealthBar, kTransBlack);
			}

			// draw flashing Armor as required
			if (m_bArmorFlash )
			{
                LTRect rcTemp = m_rcArmor;
				rcTemp.left += nBarX;
				rcTemp.top += nBarY;
				rcTemp.right += nBarX;
				rcTemp.bottom += nBarY;

                g_pLTClient->ScaleSurfaceToSurfaceTransparent(hScreen, m_hArmorBar, &rcTemp, &m_rcArmorBar, kTransBlack);
			}
			UpdateFlash();
		}
		if (m_bUseHealthIcon)
		{
            g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hHealthIcon, LTNULL, nHealthX + m_HealthIconOffset.x, nHealthY + m_HealthIconOffset.y, hTransColor);
            g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hArmorIcon, LTNULL, nHealthX + m_ArmorIconOffset.x , nHealthY + m_ArmorIconOffset.y, hTransColor);
		}
		if (m_bUseHealthText)
		{
			char szStr[5] = "";
			sprintf(szStr,"%d",m_nHealth);
			g_pForeFont->Draw(szStr, hScreen, nHealthX + m_HealthTextOffset.x+1, nHealthY + m_HealthTextOffset.y+1, LTF_JUSTIFY_LEFT);
			g_pForeFont->Draw(szStr, hScreen, nHealthX + m_HealthTextOffset.x, nHealthY + m_HealthTextOffset.y, LTF_JUSTIFY_LEFT,hHealthTint);

			sprintf(szStr,"%d",m_nArmor);
			g_pForeFont->Draw(szStr, hScreen, nHealthX + m_ArmorTextOffset.x+1, nHealthY + m_ArmorTextOffset.y+1, LTF_JUSTIFY_LEFT);
			g_pForeFont->Draw(szStr, hScreen, nHealthX + m_ArmorTextOffset.x, nHealthY + m_ArmorTextOffset.y, LTF_JUSTIFY_LEFT,hArmorTint);
		}

		// Don't draw the ammo if the weapon is diabled...

		CWeaponModel* pWeapon = g_pGameClientShell->GetWeaponModel();
		if (!pWeapon) return;

		if (m_bDrawAmmo && !pWeapon->IsDisabled())
		{
			// Only display ammo icons for weapons with infinite ammo...

			WEAPON* pW = g_pWeaponMgr->GetWeapon(m_nCurrentWeapon);
			if (!pW) return;

			int nAmmoX = (int) ((float)m_AmmoBasePos.x * xRatio);
			int nAmmoY = (int) ((float)m_AmmoBasePos.y * yRatio);
			if (m_bUseAmmoBar && !pW->bInfiniteAmmo)
			{
                g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hHUDAmmo, NULL, (nAmmoX + m_AmmoBarOffset.x) - m_rcAmmoHUD.right, nAmmoY + m_AmmoBarOffset.y, hTransColor);
			}

			if (m_hAmmoIcon)
			{
				g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hAmmoIcon, LTNULL, nAmmoX+m_AmmoIconOffset.x, nAmmoY+m_AmmoIconOffset.y, hTransColor);
			}

			if (m_bUseAmmoText && !pW->bInfiniteAmmo)
			{
				char str[20];
				int nAmmoInClip = g_pGameClientShell->GetWeaponModel()->GetAmmoInClip();
				int nAmmo = m_pnAmmo[m_nCurrentAmmo] - nAmmoInClip;
				sprintf(str,"%d/%d", nAmmoInClip, nAmmo < 0 ? 0 : nAmmo);
				g_pForeFont->Draw(str, hScreen, nAmmoX+m_AmmoTextOffset.x+1, nAmmoY+m_AmmoTextOffset.y+1, LTF_JUSTIFY_LEFT);
				g_pForeFont->Draw(str, hScreen, nAmmoX+m_AmmoTextOffset.x, nAmmoY+m_AmmoTextOffset.y, LTF_JUSTIFY_LEFT,hAmmoTint);
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdatePlayerHealth()
//
//	PURPOSE:	Update the health
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdatePlayerHealth()
{
    if (!g_pLTClient) return;

	// Clear the surface...

    HLTCOLOR hTransColor = kTransBlack;


	ClearSurface(m_hHUDHealth,hTransColor);

    m_rcHealth.right = (int)((LTFLOAT)m_nHealth * m_fBarScale) + m_rcHealth.left;
    m_rcArmor.right = (int)((LTFLOAT)m_nArmor * m_fBarScale) + m_rcArmor.left;

    g_pLTClient->FillRect(m_hHUDHealth, &m_rcArmorShadow, SETRGB(25,25,50));
    g_pLTClient->FillRect(m_hHUDHealth, &m_rcHealthShadow, SETRGB(50,25,25));
    g_pLTClient->ScaleSurfaceToSurfaceTransparent(m_hHUDHealth, m_hHealthBar, &m_rcHealth, &m_rcHealthBar, kTransBlack);
    g_pLTClient->ScaleSurfaceToSurfaceTransparent(m_hHUDHealth, m_hArmorBar, &m_rcArmor, &m_rcArmorBar, kTransBlack);
    g_pLTClient->OptimizeSurface(m_hHUDHealth, hTransColor);


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdatePlayerAmmo()
//
//	PURPOSE:	Update the ammo
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdatePlayerAmmo()
{
    if (!g_pLTClient || !g_pWeaponMgr || !m_pnAmmo) return;

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_nCurrentWeapon);
	if (!pWeapon) return;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_nCurrentAmmo);
	if (!pAmmo) return;

    SetDrawAmmo(LTTRUE);

	if (m_bAmmoTypeChanged)
	{
		UpdateAmmoSurfaces();
        m_bAmmoTypeChanged = LTFALSE;
	}

	// Clear the surfaces...

    HLTCOLOR hTransColor = kTransBlack;
	ClearSurface(m_hHUDAmmo,hTransColor);

	int nAmmoInClip = g_pGameClientShell->GetWeaponModel()->GetAmmoInClip();
	int nAmmo = m_pnAmmo[m_nCurrentAmmo] - nAmmoInClip;

    LTFLOAT fPercent = 100.0f * (LTFLOAT) (nAmmo + nAmmoInClip) /  (LTFLOAT) pAmmo->GetMaxAmount(LTNULL);

	m_rcAmmo.left = m_rcAmmo.right - (int)(fPercent * m_fBarScale);

    g_pLTClient->FillRect(m_hHUDAmmo, &m_rcAmmoShadow, SETRGB(50,50,25));
    g_pLTClient->ScaleSurfaceToSurfaceTransparent(m_hHUDAmmo, m_hAmmoBar, &m_rcAmmo, &m_rcAmmoBar, kTransBlack);

	int nEmpty = pWeapon->nShotsPerClip - nAmmoInClip;
	int x = (m_rcAmmoHUD.right + m_AmmoClipOffset.x) - m_AmmoSz.x;
	int y = m_AmmoClipOffset.y;

    int i;
    for (i = 0; i < nAmmoInClip; i++)
	{
        g_pLTClient->DrawSurfaceToSurfaceTransparent(m_hHUDAmmo, m_hAmmoFull, LTNULL, x, y, hTransColor);
		x -= m_AmmoSz.x;
	}

	for (i = 0; i < nEmpty; i++)
	{
        g_pLTClient->DrawSurfaceToSurfaceTransparent(m_hHUDAmmo, m_hAmmoEmpty, LTNULL, x, y, hTransColor);
		x -= m_AmmoSz.x;
	}


    g_pLTClient->OptimizeSurface(m_hHUDAmmo, hTransColor);

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Save
//
//	PURPOSE:	Save the player stats info
//
// --------------------------------------------------------------------------- //

void CPlayerStats::Save(HMESSAGEWRITE hWrite)
{
    if (!g_pLTClient || !g_pWeaponMgr) return;

    m_Objectives.Save(g_pLTClient, hWrite);
    m_CompletedObjectives.Save(g_pLTClient, hWrite);

    g_pLTClient->WriteToMessageDWord(hWrite, m_nHealth);
    g_pLTClient->WriteToMessageDWord(hWrite, m_nDamage);
    g_pLTClient->WriteToMessageDWord(hWrite, m_nArmor);
    g_pLTClient->WriteToMessageDWord(hWrite, m_nMaxHealth);
    g_pLTClient->WriteToMessageDWord(hWrite, m_nMaxArmor);
    g_pLTClient->WriteToMessageDWord(hWrite, m_nCrosshairLevel);

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();
    uint8 i;
    for (i = 0; i < nNumAmmoTypes; i++)
	{
        g_pLTClient->WriteToMessageByte(hWrite, m_pbCanUseAmmo[i]);
        g_pLTClient->WriteToMessageByte(hWrite, m_pbHaveAmmo[i]);
        g_pLTClient->WriteToMessageDWord(hWrite, m_pnAmmo[i]);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	for (i = 0; i < nNumWeapons; i++)
	{
        g_pLTClient->WriteToMessageByte(hWrite, m_pbCanUseWeapon[i]);
        g_pLTClient->WriteToMessageByte(hWrite, m_pbHaveWeapon[i]);
	}

	int nNumMods = g_pWeaponMgr->GetNumModTypes();
	for (i = 0; i < nNumMods; i++)
	{
        g_pLTClient->WriteToMessageByte(hWrite, m_pbCanUseMod[i]);
        g_pLTClient->WriteToMessageByte(hWrite, m_pbHaveMod[i]);
	}

	int nNumGear = g_pWeaponMgr->GetNumGearTypes();
	for (i = 0; i < nNumGear; i++)
	{
        g_pLTClient->WriteToMessageByte(hWrite, m_pbCanUseGear[i]);
        g_pLTClient->WriteToMessageByte(hWrite, m_pbHaveGear[i]);
	}

    g_pLTClient->WriteToMessageByte(hWrite, m_nCurrentWeapon);
    g_pLTClient->WriteToMessageByte(hWrite, m_nCurrentAmmo);
    g_pLTClient->WriteToMessageByte(hWrite, m_bCrosshairEnabled);
    g_pLTClient->WriteToMessageByte(hWrite, m_bDrawAmmo);
    g_pLTClient->WriteToMessageFloat(hWrite, m_fAirPercent);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Load
//
//	PURPOSE:	Load the player stats info
//
// --------------------------------------------------------------------------- //

void CPlayerStats::Load(HMESSAGEREAD hRead)
{
    if (!g_pLTClient || !g_pWeaponMgr) return;

    m_Objectives.Load(g_pLTClient, hRead);
    m_CompletedObjectives.Load(g_pLTClient, hRead);

    m_nHealth           = g_pLTClient->ReadFromMessageDWord(hRead);
    m_nDamage           = g_pLTClient->ReadFromMessageDWord(hRead);
    m_nArmor            = g_pLTClient->ReadFromMessageDWord(hRead);
    m_nMaxHealth        = g_pLTClient->ReadFromMessageDWord(hRead);
    m_nMaxArmor         = g_pLTClient->ReadFromMessageDWord(hRead);
    m_nCrosshairLevel   = (int) g_pLTClient->ReadFromMessageDWord(hRead);

    uint8 i;
	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();
    for (i = 0; i < nNumAmmoTypes; i++)
	{
        m_pbCanUseAmmo[i]   = (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
        m_pbHaveAmmo[i] = (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
        m_pnAmmo[i]         = g_pLTClient->ReadFromMessageDWord(hRead);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	for ( i = 0; i < nNumWeapons; i++)
	{
        m_pbCanUseWeapon[i] = (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
        m_pbHaveWeapon[i] = (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
	}

	int nNumMods = g_pWeaponMgr->GetNumModTypes();
	for ( i = 0; i < nNumMods; i++)
	{
        m_pbCanUseMod[i] = (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
        m_pbHaveMod[i] = (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
	}

	int nNumGear = g_pWeaponMgr->GetNumGearTypes();
	for ( i = 0; i < nNumGear; i++)
	{
        m_pbCanUseGear[i] = (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
        m_pbHaveGear[i] = (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
	}

    m_nCurrentWeapon    = g_pLTClient->ReadFromMessageByte(hRead);
    m_nCurrentAmmo      = g_pLTClient->ReadFromMessageByte(hRead);
    m_bCrosshairEnabled = (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
    m_bDrawAmmo         = (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
    m_fAirPercent       = g_pLTClient->ReadFromMessageFloat(hRead);

    UpdatePlayerWeapon(m_nCurrentWeapon, m_nCurrentAmmo, LTTRUE);
}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetMod
//
//	PURPOSE:	Get the id of thefirst mod for the current weapon
//				of the given type
//
// --------------------------------------------------------------------------- //

uint8 CPlayerStats::GetMod(ModType eType)
{
	uint8 nModId	= WMGR_INVALID_ID;
	int nPriority	= -1;

	WEAPON *pWpn = g_pWeaponMgr->GetWeapon(m_nCurrentWeapon);
	if (pWpn)
	{
		for (int i=0; i < pWpn->nNumModTypes; i++)
		{
			if (HaveMod(pWpn->aModTypes[i]))
			{
				MOD* pMod = g_pWeaponMgr->GetMod(pWpn->aModTypes[i]);

				if (pMod && pMod->eType == eType)
				{
					// Get the mod with the hightest priority...

					if (pMod->nPriority > nPriority)
					{
						nPriority = pMod->nPriority;
						nModId = pWpn->aModTypes[i];
					}
				}
			}
		}
	}


	return nModId;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetAmmoCount
//
//	PURPOSE:	Get the ammo count for the passed in ammo id
//
// --------------------------------------------------------------------------- //

uint32 CPlayerStats::GetAmmoCount(uint8 nAmmoId) const
{
	 if (!m_pnAmmo || !g_pWeaponMgr->IsValidAmmoType(nAmmoId))  return 0;

	 return m_pnAmmo[nAmmoId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::HaveWeapon
//
//	PURPOSE:	Do we have the weapon associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerStats::HaveWeapon(uint8 nWeaponId) const
{
     if (!m_pbHaveWeapon || !g_pWeaponMgr->IsValidWeapon(nWeaponId)) return LTFALSE;

	 return m_pbHaveWeapon[nWeaponId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CanUseWeapon
//
//	PURPOSE:	Can we use the weapon associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerStats::CanUseWeapon(uint8 nWeaponId) const
{
     if (!m_pbCanUseWeapon || !g_pWeaponMgr->IsValidWeapon(nWeaponId) || !g_pWeaponMgr->IsPlayerWeapon(nWeaponId)) return LTFALSE;

	 return m_pbCanUseWeapon[nWeaponId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::HaveMod
//
//	PURPOSE:	Do we have the mod associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerStats::HaveMod(uint8 nModId) const
{
     if (!m_pbHaveMod || !g_pWeaponMgr->IsValidModType(nModId)) return LTFALSE;

	 return m_pbHaveMod[nModId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CanUseMod
//
//	PURPOSE:	Can we use the mod associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerStats::CanUseMod(uint8 nModId) const
{
     if (!m_pbCanUseMod || !g_pWeaponMgr->IsValidModType(nModId)) return LTFALSE;

	 return m_pbCanUseMod[nModId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::HaveGear
//
//	PURPOSE:	Do we have the Gear associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerStats::HaveGear(uint8 nGearId) const
{
     if (!m_pbHaveGear || !g_pWeaponMgr->IsValidGearType(nGearId)) return LTFALSE;

	 return m_pbHaveGear[nGearId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CanUseGear
//
//	PURPOSE:	Can we use the Gear associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerStats::CanUseGear(uint8 nGearId) const
{
     if (!m_pbCanUseGear || !g_pWeaponMgr->IsValidGearType(nGearId)) return LTFALSE;

	 return m_pbCanUseGear[nGearId];
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CanUseAmmo
//
//	PURPOSE:	Can we use the ammo associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerStats::CanUseAmmo(uint8 nAmmoId) const
{
     if (!m_pbCanUseAmmo || !g_pWeaponMgr->IsValidAmmoType(nAmmoId)) return LTFALSE;

	 return m_pbCanUseAmmo[nAmmoId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::AddCanUseWeapon
//
//	PURPOSE:	Add a weapon to our can use list
//
// --------------------------------------------------------------------------- //

void CPlayerStats::AddCanUseWeapon(uint8 nWeaponId)
{
	 if (!m_pbCanUseWeapon || !g_pWeaponMgr->IsValidWeapon(nWeaponId) || !g_pWeaponMgr->IsPlayerWeapon(nWeaponId)) return;

     m_pbCanUseWeapon[nWeaponId] = LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::AddCanUseMod
//
//	PURPOSE:	Add a mod to our can use list
//
// --------------------------------------------------------------------------- //

void CPlayerStats::AddCanUseMod(uint8 nModId)
{
	 if (!m_pbCanUseMod || !g_pWeaponMgr->IsValidModType(nModId)) return;

     m_pbCanUseMod[nModId] = LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::AddCanUseGear
//
//	PURPOSE:	Add a Gear to our can use list
//
// --------------------------------------------------------------------------- //

void CPlayerStats::AddCanUseGear(uint8 nGearId)
{
	 if (!m_pbCanUseGear || !g_pWeaponMgr->IsValidGearType(nGearId)) return;

     m_pbCanUseGear[nGearId] = LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::AddCanUseAmmo
//
//	PURPOSE:	Add the ammo to our can use list
//
// --------------------------------------------------------------------------- //

void CPlayerStats::AddCanUseAmmo(uint8 nAmmoId)
{
	 if (!m_pbCanUseAmmo || !g_pWeaponMgr->IsValidAmmoType(nAmmoId)) return;

     m_pbCanUseAmmo[nAmmoId] = LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::PrepareInventory
//
//	PURPOSE:	Setup for inventory selection for a new mission
//
// --------------------------------------------------------------------------- //

void CPlayerStats::PrepareInventory()
{
	// Clear our data...
	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();
	CPlayerSummaryMgr *pPSummary = g_pGameClientShell->GetPlayerSummary();
	if (!pPSummary) return;

	// Make sure the summary file is up-to-date...

    pPSummary->RefreshData(g_pLTClient);

	if (nNumAmmoTypes > 0)
	{
		pPSummary->ReadAmmoData(m_pbCanUseAmmo,nNumAmmoTypes);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
		pPSummary->ReadWeaponData(m_pbCanUseWeapon,nNumWeapons);
	}

	int nNumMods = g_pWeaponMgr->GetNumModTypes();
	if (nNumMods > 0)
	{
		pPSummary->ReadModData(m_pbCanUseMod,nNumMods);
	}

	int nNumGear = g_pWeaponMgr->GetNumGearTypes();
	if (nNumGear > 0)
	{
		pPSummary->ReadGearData(m_pbCanUseGear,nNumGear);
	}


}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::SaveInventory
//
//	PURPOSE:	Save Inventory after completing a mission
//
// --------------------------------------------------------------------------- //

void CPlayerStats::SaveInventory()
{
	//make sure we preserve what's already been saved...
//	PrepareInventory(LTFALSE);
	int nMissionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(nMissionNum);

	if (!pMission) return;

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();
	CPlayerSummaryMgr *pPSummary = g_pGameClientShell->GetPlayerSummary();

	// Make sure the summary file is up-to-date...
    pPSummary->RefreshData(g_pLTClient);


	if (nNumAmmoTypes > 0)
	{
        for (uint8 i = 0; i < nNumAmmoTypes; i++)
		{
			if (m_pbHaveAmmo[i] && !pMission->IsOneTimeAmmo(i))
                m_pbCanUseAmmo[i] = LTTRUE;
		}
		pPSummary->WriteAmmoData(m_pbCanUseAmmo,nNumAmmoTypes);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
        for (uint8 i = 0; i < nNumWeapons; i++)
		{
			WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(i);

			//if we already could use this weapon, or we have it and it's not a one-time,
			//  check for mods
			if (m_pbCanUseWeapon[i] ||  (m_pbHaveWeapon[i] && !pMission->IsOneTimeWeapon(i)))
			{
				for (int m = 0; m < pWeapon->nNumModTypes; m++)
				{
					int nModId = pWeapon->aModTypes[m];
					if (m_pbHaveMod[nModId])
                        m_pbCanUseMod[nModId] = LTTRUE;
				}
			}

			if (m_pbHaveWeapon[i] && !pMission->IsOneTimeWeapon(i) && !pMission->IsOneTimeGadget(i))
                m_pbCanUseWeapon[i] = LTTRUE;

		}
		pPSummary->WriteWeaponData(m_pbCanUseWeapon,nNumWeapons);
	}

	int nNumMods = g_pWeaponMgr->GetNumModTypes();
	if (nNumMods > 0)
	{
		pPSummary->WriteModData(m_pbCanUseMod,nNumMods);
	}

	int nNumGear = g_pWeaponMgr->GetNumGearTypes();
	if (nNumGear > 0)
	{
        for (uint8 i = 0; i < nNumGear; i++)
		{
			if (m_pbHaveGear[i] && !pMission->IsOneTimeGear(i))
			{
				GEAR *pGear = g_pWeaponMgr->GetGear(i);
				if (pGear && pGear->bExclusive)
					m_pbCanUseGear[i] = LTTRUE;
			}
		}
		pPSummary->WriteGearData(m_pbCanUseGear,nNumGear);

	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Setup
//
//	PURPOSE:	Setup the stats based on the CMissionData info
//
// --------------------------------------------------------------------------- //

void CPlayerStats::Setup(CMissionData* pMissionData)
{
	// Only set things up if this is the first level of the mission...

	if (!pMissionData || pMissionData->GetLevelNum() > 0) return;

	if (!m_pnAmmo || !m_pbHaveWeapon || !m_pbCanUseAmmo || !m_pbCanUseWeapon) return;


	// Clear our data...

	DropInventory();

	CPlayerSummaryMgr *pPSummary = g_pGameClientShell->GetPlayerSummary();


	// Give us all the specified weapons...

	CWeaponData *weapons[100];
	int nNum = pMissionData->GetWeapons(weapons, ARRAY_LEN(weapons));

    int i;
    for (i=0; i < nNum; i++)
	{
        UpdateAmmo(weapons[i]->m_nID, -1, 0, LTTRUE);
	}

	// Give us all the specified ammo...

	CAmmoData *ammo[100];
	nNum = pMissionData->GetAmmo(ammo, ARRAY_LEN(ammo));

	for (i=0; i < nNum; i++)
	{
        UpdateAmmo(-1, ammo[i]->m_nID, ammo[i]->m_nCount, LTFALSE);
	}

	// Give us all the specified mods...

	CModData *mods[100];
	nNum = pMissionData->GetMods(mods, ARRAY_LEN(mods));

	for (i=0; i < nNum; i++)
	{
		if (g_pWeaponMgr->IsValidModType(mods[i]->m_nID))
		{
			if (m_pbHaveMod)
			{
                m_pbHaveMod[mods[i]->m_nID] = LTTRUE;
			}

		}

	}

	// Give us all the specified gear...

	CGearData *gear[100];
	nNum = pMissionData->GetGear(gear, ARRAY_LEN(gear));

	for (i=0; i < nNum; i++)
	{
		UpdateGear(gear[i]->m_nID);
	}

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ResetInventory
//
//	PURPOSE:	Reset all inventory items
//
// --------------------------------------------------------------------------- //

void CPlayerStats::ResetInventory()
{
	// Clear our data...

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();
	if (nNumAmmoTypes > 0)
	{
        memset(m_pnAmmo, 0, sizeof(uint32) * nNumAmmoTypes);
        memset(m_pbHaveAmmo, 0, sizeof(LTBOOL) * nNumAmmoTypes);
        memset(m_pbCanUseAmmo, 0, sizeof(LTBOOL) * nNumAmmoTypes);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
        memset(m_pbHaveWeapon, 0, sizeof(LTBOOL) * nNumWeapons);
        memset(m_pbCanUseWeapon, 0, sizeof(LTBOOL) * nNumWeapons);
	}

	int nNumMods = g_pWeaponMgr->GetNumModTypes();
	if (nNumMods > 0)
	{
		memset(m_pbHaveMod, 0, sizeof(LTBOOL) * nNumMods);
		memset(m_pbCanUseMod, 0, sizeof(LTBOOL) * nNumMods);
	}

	int nNumGear = g_pWeaponMgr->GetNumGearTypes();
	if (nNumGear > 0)
	{
		memset(m_pbHaveGear, 0, sizeof(LTBOOL) * nNumGear);
		memset(m_pbCanUseGear, 0, sizeof(LTBOOL) * nNumGear);
	}

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DropInventory
//
//	PURPOSE:	Removes all currently carried weapons and ammo, optionally
//				removes gear and mods
//
// --------------------------------------------------------------------------- //

void CPlayerStats::DropInventory(LTBOOL bRemoveGear)
{
	// Clear our data...

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();
	if (nNumAmmoTypes > 0)
	{
        memset(m_pnAmmo, 0, sizeof(uint32) * nNumAmmoTypes);
        memset(m_pbHaveAmmo, 0, sizeof(LTBOOL) * nNumAmmoTypes);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
        memset(m_pbHaveWeapon, 0, sizeof(LTBOOL) * nNumWeapons);
	}

	if (bRemoveGear)
	{
		int nNumMods = g_pWeaponMgr->GetNumModTypes();
		if (nNumMods > 0)
		{
			memset(m_pbHaveMod, 0, sizeof(LTBOOL) * nNumMods);
		}

		int nNumGear = g_pWeaponMgr->GetNumGearTypes();
		if (nNumGear > 0)
		{
			memset(m_pbHaveGear, 0, sizeof(LTBOOL) * nNumGear);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CreateHealthSurfaces
//
//	PURPOSE:	Create a surfaces for health and armor display
//
// --------------------------------------------------------------------------- //

void CPlayerStats::CreateHealthSurfaces()
{
	// creates surface to draw bars on
	UpdateHealthSurfaces();

    HLTCOLOR hTransColor = kTransBlack;

	if (!m_hHealthBar)
	{
        uint32 dwWidth, dwHeight;
        m_hHealthBar = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\Bar_Health.pcx");
        g_pLTClient->OptimizeSurface (m_hHealthBar, hTransColor);
        g_pLTClient->GetSurfaceDims(m_hHealthBar, &dwWidth, &dwHeight);
		m_rcHealthBar.right = dwWidth;
		m_rcHealthBar.bottom = dwHeight;
	}

	if (!m_hArmorBar)
	{
        uint32 dwWidth, dwHeight;
        m_hArmorBar = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\Bar_Armor.pcx");
        g_pLTClient->OptimizeSurface (m_hArmorBar, hTransColor);
        g_pLTClient->GetSurfaceDims(m_hArmorBar, &dwWidth, &dwHeight);
		m_rcArmorBar.right = dwWidth;
		m_rcArmorBar.bottom = dwHeight;
	}

	if (!m_hHealthIcon)
	{
        m_hHealthIcon = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\Icon_Health.pcx");
        g_pLTClient->OptimizeSurface (m_hHealthIcon, hTransColor);
        g_pLTClient->SetSurfaceAlpha(m_hHealthIcon,0.5f);
	}

	if (!m_hArmorIcon)
	{
        m_hArmorIcon = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\Icon_Armor.pcx");
        g_pLTClient->OptimizeSurface (m_hArmorIcon, hTransColor);
        g_pLTClient->SetSurfaceAlpha(m_hArmorIcon,0.5f);
	}

    m_bHealthInit = LTTRUE;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateHealthSurfaces
//
//	PURPOSE:	Create a surface to draw the health and armor bars on
//
// --------------------------------------------------------------------------- //

void CPlayerStats::UpdateHealthSurfaces()
{
	if (m_hHUDHealth)
        g_pLTClient->DeleteSurface (m_hHUDHealth);
    m_hHUDHealth = LTNULL;

    int nWidth = (int)( Max((LTFLOAT)m_nMaxHealth,(LTFLOAT)m_nMaxArmor) * m_fBarScale ) + abs(m_ArmorBarOffset.x) + 2;
	int nHeight = m_nBarHeight + abs(m_ArmorBarOffset.y) + 2;
    HLTCOLOR hTransColor = kTransBlack;

    m_hHUDHealth = g_pLTClient->CreateSurface(nWidth,nHeight);

    g_pLTClient->OptimizeSurface (m_hHUDHealth, hTransColor);
    g_pLTClient->SetSurfaceAlpha(m_hHUDHealth,0.5f);

	m_rcHealth.left = 1;
	m_rcHealth.top = 1;
    m_rcHealth.right = (int)( (LTFLOAT)m_nMaxHealth * m_fBarScale ) + 1;
	m_rcHealth.bottom = m_nBarHeight + 1;

	m_rcHealthShadow.left = 0;
	m_rcHealthShadow.top = 0;
	m_rcHealthShadow.right = m_rcHealth.right+1;
	m_rcHealthShadow.bottom = m_rcHealth.bottom+1;

	m_rcArmor.left = m_rcHealth.left + m_ArmorBarOffset.x;
	m_rcArmor.top = m_rcHealth.top + m_ArmorBarOffset.y;
    m_rcArmor.right = (int)( (LTFLOAT)m_nMaxArmor * m_fBarScale ) + m_rcArmor.left;
	m_rcArmor.bottom = m_nBarHeight + m_rcArmor.top;

	m_rcArmorShadow.left = m_rcArmor.left - 1;
	m_rcArmorShadow.top = m_rcArmor.top - 1;
	m_rcArmorShadow.right = m_rcArmor.right + 1;
	m_rcArmorShadow.bottom = m_rcArmor.bottom + 1;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DestroyHealthSurfaces
//
//	PURPOSE:	Destroy surfaces for health and armor display
//
// --------------------------------------------------------------------------- //

void CPlayerStats::DestroyHealthSurfaces()
{
	if (m_hHUDHealth)
        g_pLTClient->DeleteSurface (m_hHUDHealth);
    m_hHUDHealth = LTNULL;

	if (m_hHealthBar)
        g_pLTClient->DeleteSurface (m_hHealthBar);
    m_hHealthBar = LTNULL;

	if (m_hHealthIcon)
        g_pLTClient->DeleteSurface (m_hHealthIcon);
    m_hHealthIcon = LTNULL;

	if (m_hArmorBar)
        g_pLTClient->DeleteSurface (m_hArmorBar);
    m_hArmorBar = LTNULL;

	if (m_hArmorIcon)
        g_pLTClient->DeleteSurface (m_hArmorIcon);
    m_hArmorIcon = LTNULL;


    m_bHealthInit = LTFALSE;

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CreateAmmoSurfaces
//
//	PURPOSE:	Create surfaces for ammo display
//
// --------------------------------------------------------------------------- //

void CPlayerStats::CreateAmmoSurfaces()
{
    uint32 dwWidth, dwHeight;

    HLTCOLOR hTransColor = kTransBlack;

	if (!m_hAmmoBar)
	{
        m_hAmmoBar = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\Bar_Ammo.pcx");
        g_pLTClient->OptimizeSurface (m_hAmmoBar, hTransColor);
        g_pLTClient->GetSurfaceDims(m_hAmmoBar, &dwWidth, &dwHeight);
		m_rcAmmoBar.right = dwWidth;
		m_rcAmmoBar.bottom = dwHeight;
	}

	if (!m_hAmmoFull)
	{
        m_hAmmoFull = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\ammo_full.pcx");
        g_pLTClient->OptimizeSurface (m_hAmmoFull, hTransColor);
        g_pLTClient->GetSurfaceDims(m_hAmmoFull, &dwWidth, &dwHeight);
		m_AmmoSz.x = (int)dwWidth;
		m_AmmoSz.y = (int)dwHeight;
	}

	if (!m_hAmmoEmpty)
	{
        m_hAmmoEmpty = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\ammo_Empty.pcx");
        g_pLTClient->OptimizeSurface (m_hAmmoEmpty, hTransColor);
        g_pLTClient->GetSurfaceDims(m_hAmmoEmpty, &dwWidth, &dwHeight);
	}

	// creates surface to draw bars on
	// create surface for icon
	UpdateAmmoSurfaces();

    m_bAmmoInit = LTTRUE;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateAmmoSurfaces
//
//	PURPOSE:	Create a surface to draw the ammo bars on, and an ammo icon
//
// --------------------------------------------------------------------------- //

void CPlayerStats::UpdateAmmoSurfaces()
{
	if (m_hAmmoIcon)
	{
        g_pLTClient->DeleteSurface (m_hAmmoIcon);
        m_hAmmoIcon = LTNULL;
	}

    if (!g_pLTClient || !g_pWeaponMgr || !m_pnAmmo) return;
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_nCurrentWeapon);
	if (!pWeapon) return;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_nCurrentAmmo);
	if (!pAmmo) return;

	if (pAmmo->eInstDamageType == DT_MELEE) return;

	if (pAmmo->szSmallIcon[0])
	{
        m_hAmmoIcon = g_pLTClient->CreateSurfaceFromBitmap(pAmmo->szSmallIcon);
	}
	else
	{
        m_hAmmoIcon = g_pLTClient->CreateSurfaceFromBitmap("interface\\missingslot.pcx");
	}
    HLTCOLOR hTransColor = kTransBlack;
    g_pLTClient->OptimizeSurface (m_hAmmoIcon, hTransColor);
    g_pLTClient->SetSurfaceAlpha(m_hAmmoIcon,0.5f);

	int nClipWidth = pWeapon->nShotsPerClip * m_AmmoSz.x;
	int nBarWidth = (int)( 100.0f * m_fBarScale );


	int nWidth  = Max(nClipWidth,nBarWidth) + abs(m_AmmoClipOffset.y) + 2;
	int nHeight = Max(m_AmmoSz.y,m_nBarHeight) + abs(m_AmmoClipOffset.y) + 2;

	if (nWidth > m_rcAmmoHUD.right || nHeight > m_rcAmmoHUD.bottom)
	{
		if (m_hHUDAmmo)
            g_pLTClient->DeleteSurface (m_hHUDAmmo);
        m_hHUDAmmo = LTNULL;

        m_hHUDAmmo = g_pLTClient->CreateSurface(nWidth,nHeight);

		m_rcAmmoHUD.right = nWidth;
		m_rcAmmoHUD.bottom = nHeight;
	}
	m_rcAmmo.right = m_rcAmmoHUD.right - 1;
	m_rcAmmo.top = 1;
	m_rcAmmo.left = m_rcAmmo.right - (int)( 100.0f * m_fBarScale );
	m_rcAmmo.bottom = m_nBarHeight + 1;

	m_rcAmmoShadow.left = m_rcAmmo.left - 1;
	m_rcAmmoShadow.top = 0;
	m_rcAmmoShadow.right = m_rcAmmo.right + 1;
	m_rcAmmoShadow.bottom = m_nBarHeight + 2;

	ClearSurface(m_hHUDAmmo,hTransColor);

    g_pLTClient->OptimizeSurface (m_hHUDAmmo, hTransColor);
    g_pLTClient->SetSurfaceAlpha(m_hHUDAmmo,0.5f);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DestroyAmmoSurfaces
//
//	PURPOSE:	Destroy surfaces for health and armor display
//
// --------------------------------------------------------------------------- //

void CPlayerStats::DestroyAmmoSurfaces()
{
	if (m_hHUDAmmo)
        g_pLTClient->DeleteSurface (m_hHUDAmmo);
    m_hHUDAmmo = LTNULL;

	if (m_hAmmoBar)
        g_pLTClient->DeleteSurface (m_hAmmoBar);
    m_hAmmoBar = LTNULL;

	if (m_hAmmoIcon)
        g_pLTClient->DeleteSurface (m_hAmmoIcon);
    m_hAmmoIcon = LTNULL;

	if (m_hAmmoFull)
        g_pLTClient->DeleteSurface (m_hAmmoFull);
    m_hAmmoFull = LTNULL;

	if (m_hAmmoEmpty)
        g_pLTClient->DeleteSurface (m_hAmmoEmpty);
    m_hAmmoEmpty = LTNULL;

    m_bAmmoInit = LTFALSE;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CreateAirSurfaces
//
//	PURPOSE:	Create surfaces to draw the air meter on
//
// --------------------------------------------------------------------------- //

void CPlayerStats::CreateAirSurfaces()
{
    HLTCOLOR hTransColor = kTransBlack;

	// creates surface to draw bars on
	UpdateAirSurfaces();

	if (!m_hAirBar)
	{
        uint32 dwWidth, dwHeight;
        m_hAirBar = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\Bar_Air.pcx");
        g_pLTClient->OptimizeSurface (m_hAirBar, hTransColor);
        g_pLTClient->GetSurfaceDims(m_hAirBar, &dwWidth, &dwHeight);
		m_rcAirBar.right = dwWidth;
		m_rcAirBar.bottom = dwHeight;
	}

	if (!m_hAirIcon)
	{
        m_hAirIcon = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\Icon_Air.pcx");
        g_pLTClient->OptimizeSurface (m_hAirIcon, hTransColor);
        g_pLTClient->SetSurfaceAlpha(m_hAirIcon,0.5f);
	}

	if (!m_hAirStr)
	{
        HSTRING hStr = g_pLTClient->CreateString("888");
        LTIntPt txtSize = g_pInterfaceResMgr->GetAirFont()->GetTextExtents(hStr);
        g_pLTClient->FreeString(hStr);

        m_hAirStr = g_pLTClient->CreateSurface(txtSize.x,txtSize.y);
        g_pLTClient->OptimizeSurface (m_hAirStr, hTransColor);
        g_pLTClient->SetSurfaceAlpha(m_hAirStr,0.5f);
	}

    m_bAirInit = LTTRUE;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateAirSurfaces
//
//	PURPOSE:	Create a surface to draw the air bar on
//
// --------------------------------------------------------------------------- //

void CPlayerStats::UpdateAirSurfaces()
{
	if (m_hHUDAir)
        g_pLTClient->DeleteSurface (m_hHUDAir);
    m_hHUDAir = LTNULL;

    HLTCOLOR hTransColor = kTransBlack;
	int nWidth = (int)( 100.0f * m_fBarScale ) + 2;
	int nHeight = m_nBarHeight + 2;

    m_hHUDAir = g_pLTClient->CreateSurface(nWidth,nHeight);

    g_pLTClient->OptimizeSurface (m_hHUDAir, hTransColor);
    g_pLTClient->SetSurfaceAlpha(m_hHUDAir,0.5f);

	m_rcAir.left = 1;
	m_rcAir.top = 1;
	m_rcAir.right = (int)(100.0f * m_fBarScale) + m_rcAir.left;
	m_rcAir.bottom = m_nBarHeight + m_rcAir.top;

}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DestroyAirSurfaces
//
//	PURPOSE:	Destroy surfaces for air display
//
// --------------------------------------------------------------------------- //

void CPlayerStats::DestroyAirSurfaces()
{
	if (m_hHUDAir)
        g_pLTClient->DeleteSurface (m_hHUDAir);
    m_hHUDAir = LTNULL;

	if (m_hAirBar)
        g_pLTClient->DeleteSurface (m_hAirBar);
    m_hAirBar = LTNULL;

	if (m_hAirIcon)
        g_pLTClient->DeleteSurface (m_hAirIcon);
    m_hAirIcon = LTNULL;

	if (m_hAirStr)
        g_pLTClient->DeleteSurface (m_hAirStr);
    m_hAirStr = LTNULL;

    m_bAirInit = LTFALSE;

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawScope()
//
//	PURPOSE:	Draw the mask and crosshairs for the zoomed view
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DrawScope(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom)
{
	// Don't draw the scope if the weapon is diabled...

	CWeaponModel* pWeapon = g_pGameClientShell->GetWeaponModel();
	if (!pWeapon || pWeapon->IsDisabled()) return;


    LTRect rect;
	int nType = 1;
	int nScreenWidth		= nRight - nLeft;
	int nScreenHeight		= nBottom - nTop;

    HLTCOLOR hGray  = SETRGB(128,128,128);
    HLTCOLOR hGold  = SETRGB(140,128,20);

	if (g_pGameClientShell->UsingNightVision())
	{
		hGold  = SETRGB(128,255,128);
	}


	int cx = nLeft + (nScreenWidth) / 2;
	int cy = nTop + (nScreenHeight) / 2;

	int nRadius = (int)( g_vtScopeLRRadius.GetFloat() * (float)nScreenHeight );

    LTRect srect;
	srect.top = 0;
	srect.left = 0;
	srect.bottom = 2;
	srect.right = 2;

	switch (nType)
	{
	case 0:
		{
			int offset = (nScreenHeight) / 4;

			//horizontal hilight
			rect.top = cy - 1;
			rect.left = cx - offset;
			rect.bottom = cy;
			rect.right = cx + offset;
            g_pLTClient->ScaleSurfaceToSurface(hScreen, hCrossHighlight, &rect, &srect);

			//vertical hilight
			rect.top = cy - offset;;
			rect.left = cx -1 ;
			rect.bottom = cy+offset;
			rect.right = cx;
            g_pLTClient->ScaleSurfaceToSurface(hScreen, hCrossHighlight, &rect, &srect);

			//horizontal hair
			rect.top = cy;
			rect.left = cx - nRadius;
			rect.bottom = rect.top + 1;
			rect.right = cx + nRadius;
            g_pLTClient->FillRect(hScreen,&rect,kBlack);

			//vertical hair
			rect.top = cy - nRadius;
			rect.left = cx;
			rect.bottom = cy + nRadius;
			rect.right = rect.left + 1;
            g_pLTClient->FillRect(hScreen,&rect,kBlack);
		}	break;
	case 1:
		{
			int nGap = nScreenHeight / (int)g_vtScopeLRGap.GetFloat();

			//left outline
			rect.top = cy - 2;
			rect.left = cx - nRadius;
			rect.bottom = cy + 2;
			rect.right = cx - nGap;
            g_pLTClient->FillRect(hScreen,&rect,kBlack);

			//left post
			rect.top = cy - 1;
			rect.left = cx - nRadius;
			rect.bottom = cy + 1;
			rect.right = (cx - nGap) - 1;
            g_pLTClient->FillRect(hScreen,&rect,hGold);

			//right outline
			rect.top = cy - 2;
			rect.left = cx + nGap;
			rect.bottom = cy + 2;
			rect.right = cx + nRadius;
            g_pLTClient->FillRect(hScreen,&rect,kBlack);

			//right post
			rect.top = cy - 1;
			rect.left = cx + nGap + 1;
			rect.bottom = cy + 1;
			rect.right = cx + nRadius;
            g_pLTClient->FillRect(hScreen,&rect,hGold);

			nGap = nScreenHeight / (int)g_vtScopeUDGap.GetFloat();
			nRadius = (int)( g_vtScopeUDRadius.GetFloat() * (float)nScreenHeight );

			//top outline
			rect.top = cy - nRadius;
			rect.left = cx - 2;
			rect.bottom = cy - nGap;
			rect.right = cx + 2;
            g_pLTClient->FillRect(hScreen,&rect,kBlack);

			//top post
			rect.top = cy - nRadius;
			rect.left = cx - 1;
			rect.bottom = (cy - nGap) - 1;
			rect.right = cx + 1;
            g_pLTClient->FillRect(hScreen,&rect,hGold);

			//bottom outline
			rect.top = cy + nGap;
			rect.left = cx - 2;
			rect.bottom = cy + nRadius;
			rect.right = cx + 2;
            g_pLTClient->FillRect(hScreen,&rect,kBlack);

			//bottom post
			rect.top = cy + nGap + 1;
			rect.left = cx - 1;
			rect.bottom = cy+ nRadius;
			rect.right = cx + 1;
            g_pLTClient->FillRect(hScreen,&rect,hGold);

			nGap = nScreenHeight / (int)g_vtScopeLRGap.GetFloat();

			//horizontal hair
			rect.top = cy;
			rect.left = cx - nGap;
			rect.bottom = rect.top + 1;
			rect.right = cx + nGap;
            g_pLTClient->FillRect(hScreen,&rect,kBlack);

			nGap = nScreenHeight / (int)g_vtScopeUDGap.GetFloat();

			//vertical hair
			rect.top = cy - nGap;
			rect.left = cx;
			rect.bottom = cy + nGap;
			rect.right = rect.left + 1;
            g_pLTClient->FillRect(hScreen,&rect,kBlack);

			nGap = nScreenHeight / (int)g_vtScopeLRGap.GetFloat();

			//horizontal hair
			rect.top = cy-1;
			rect.left = cx - nGap;
			rect.bottom = rect.top + 1;
			rect.right = cx + nGap;
            g_pLTClient->FillRect(hScreen,&rect,hGold);

			nGap = nScreenHeight / (int)g_vtScopeUDGap.GetFloat();

			//vertical hair
			rect.top = cy - nGap;
			rect.left = cx-1;
			rect.bottom = cy + nGap;
			rect.right = rect.left + 1;
            g_pLTClient->FillRect(hScreen,&rect,hGold);

		}	break;
	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawScuba()
//
//	PURPOSE:	Draw the mask for the underwater view
//
// ----------------------------------------------------------------------- //
void CPlayerStats::DrawScuba(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom)
{
	//add any 2D effects for the scuba gear here
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawSunglass()
//
//	PURPOSE:	Draw 2d effects for the sunglass overlay
//
// ----------------------------------------------------------------------- //
void CPlayerStats::DrawSunglass(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom)
{
	// Don't draw the sunglasses if the weapon is diabled...

	CWeaponModel* pWeapon = g_pGameClientShell->GetWeaponModel();
	if (!pWeapon || pWeapon->IsDisabled()) return;

	float xRatio = g_pInterfaceResMgr->GetXRatio();
	float yRatio = g_pInterfaceResMgr->GetYRatio();

	int nTextX = (int) ((float)m_ModeTextPos.x * xRatio);
	int nTextY = (int) ((float)m_ModeTextPos.y * yRatio);

	eSunglassMode eCurrMode = g_pInterfaceMgr->GetSunglassMode();
	if (m_eLastMode != eCurrMode)
	{
		if (m_hModeStr)
		{
            g_pLTClient->FreeString(m_hModeStr);
            m_hModeStr = LTNULL;
		}
		m_eLastMode = eCurrMode;
		switch (eCurrMode)
		{
		case SUN_CAMERA:
            m_hModeStr = g_pLTClient->FormatString(IDS_GLASS_CAMERA);
			break;
		case SUN_MINES:
            m_hModeStr = g_pLTClient->FormatString(IDS_GLASS_MINES);
			break;
		case SUN_IR:
            m_hModeStr = g_pLTClient->FormatString(IDS_GLASS_IR);
			break;
		case SUN_INVIS_INK:
            m_hModeStr = g_pLTClient->FormatString(IDS_GLASS_INK);
			break;
		}
	}
	if (m_hModeStr)
	{
		CLTGUIFont* pFont = g_pInterfaceResMgr->GetChooserFont();
		if (pFont)
		{
			pFont->Draw(m_hModeStr,hScreen,nTextX,nTextY,LTF_JUSTIFY_LEFT);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::HaveAirSupply
//
//	PURPOSE:	Returns true if current gear provides air supply
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerStats::HaveAirSupply()
{
    LTBOOL bAir = LTFALSE;
    GEAR* pGear = LTNULL;

	int numGear = g_pWeaponMgr->GetNumGearTypes();
	for (int nGearId=0; nGearId < numGear && !bAir; nGearId++)
	{
		if (m_pbHaveGear[nGearId])
		{
			pGear = g_pWeaponMgr->GetGear(nGearId);
			if (pGear)
			{
				bAir = ( (pGear->eProtectionType == DT_CHOKE) && (pGear->fProtection >= 1.0f) );
			}
		}
	}
	return bAir;
}

void CPlayerStats::NextLayout()
{
	m_nCurrentLayout++;
	if (m_nCurrentLayout >= g_pLayoutMgr->GetNumHUDLayouts())
		m_nCurrentLayout = 0;
    g_vtHUDLayout.SetFloat((LTFLOAT)m_nCurrentLayout);
	UpdateLayout();
}

void CPlayerStats::PrevLayout()
{
	m_nCurrentLayout--;
	if (m_nCurrentLayout < 0)
		m_nCurrentLayout =  g_pLayoutMgr->GetNumHUDLayouts() - 1;
    g_vtHUDLayout.SetFloat((LTFLOAT)m_nCurrentLayout);
	UpdateLayout();
}


void CPlayerStats::UpdateLayout()
{
	m_nCurrentLayout = (int)g_vtHUDLayout.GetFloat();

	m_bUseAmmoBar		= g_pLayoutMgr->GetUseAmmoBar(m_nCurrentLayout);
	m_AmmoBasePos		= g_pLayoutMgr->GetAmmoBasePos(m_nCurrentLayout);
	m_AmmoClipOffset	= g_pLayoutMgr->GetAmmoClipOffset(m_nCurrentLayout);
	m_AmmoBarOffset		= g_pLayoutMgr->GetAmmoBarOffset(m_nCurrentLayout);
	m_bUseAmmoText		= g_pLayoutMgr->GetUseAmmoText(m_nCurrentLayout);
	m_AmmoTextOffset	= g_pLayoutMgr->GetAmmoTextOffset(m_nCurrentLayout);
	m_AmmoIconOffset	= g_pLayoutMgr->GetAmmoIconOffset(m_nCurrentLayout);

	m_bUseHealthBar		= g_pLayoutMgr->GetUseHealthBar(m_nCurrentLayout);
	m_HealthBasePos		= g_pLayoutMgr->GetHealthBasePos(m_nCurrentLayout);
	m_HealthBarOffset	= g_pLayoutMgr->GetHealthBarOffset(m_nCurrentLayout);
	m_ArmorBarOffset	= g_pLayoutMgr->GetArmorBarOffset(m_nCurrentLayout);
	m_bUseHealthText	= g_pLayoutMgr->GetUseHealthText(m_nCurrentLayout);
	m_HealthTextOffset	= g_pLayoutMgr->GetHealthTextOffset(m_nCurrentLayout);
	m_ArmorTextOffset	= g_pLayoutMgr->GetArmorTextOffset(m_nCurrentLayout);
	m_bUseHealthIcon	= g_pLayoutMgr->GetUseHealthIcon(m_nCurrentLayout);
	m_HealthIconOffset	= g_pLayoutMgr->GetHealthIconOffset(m_nCurrentLayout);
	m_ArmorIconOffset	= g_pLayoutMgr->GetArmorIconOffset(m_nCurrentLayout);

	m_bUseAirIcon		= g_pLayoutMgr->GetUseAirIcon(m_nCurrentLayout);
	m_AirBasePos		= g_pLayoutMgr->GetAirBasePos(m_nCurrentLayout);
	m_AirIconOffset		= g_pLayoutMgr->GetAirIconOffset(m_nCurrentLayout);
	m_bUseAirText		= g_pLayoutMgr->GetUseAirText(m_nCurrentLayout);
	m_AirTextOffset		= g_pLayoutMgr->GetAirTextOffset(m_nCurrentLayout);
	m_bUseAirBar		= g_pLayoutMgr->GetUseAirBar(m_nCurrentLayout);
	m_AirBarOffset		= g_pLayoutMgr->GetAirBarOffset(m_nCurrentLayout);

	m_ModeTextPos		= g_pLayoutMgr->GetModeTextPos(m_nCurrentLayout);

	m_DamageBasePos		= g_pLayoutMgr->GetDamageBasePos(m_nCurrentLayout);

	int		nOldHeight = m_nBarHeight;
    LTFLOAT  fOldScale = m_fBarScale;
	m_nBarHeight		= g_pLayoutMgr->GetBarHeight(m_nCurrentLayout);
	m_fBarScale			= g_pLayoutMgr->GetBarScale(m_nCurrentLayout);

	if (nOldHeight != m_nBarHeight || fOldScale != m_fBarScale)
	{
		if (m_bHealthInit)
		{
			UpdateHealthSurfaces();
			UpdatePlayerHealth();
		}
		if (m_bAmmoInit)
		{
			UpdateAmmoSurfaces();
			UpdatePlayerAmmo();
		}
		if (m_bAirInit)
		{
			UpdateAirSurfaces();
		}
	}
}


void CPlayerStats::ClearSurface(HSURFACE hSurf, HLTCOLOR hColor)
{
    uint32 dwWidth, dwHeight;
    g_pLTClient->GetSurfaceDims(hSurf, &dwWidth, &dwHeight);
    LTRect rcSrc;
	rcSrc.left = rcSrc.top = 0;
	rcSrc.right = dwWidth;
	rcSrc.bottom = dwHeight;
    g_pLTClient->FillRect(hSurf, &rcSrc, hColor);
}

void CPlayerStats::UpdateFlash()
{
    LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	if (m_fTotalHealthFlash > 0.0f)
	{
		m_fTotalHealthFlash -= fDelta;
		m_fCurrHealthFlash -= fDelta;
		if (m_fCurrHealthFlash < 0.0f)
		{
			m_bHealthFlash = !m_bHealthFlash;
			m_fCurrHealthFlash = m_fFlashSpeed;
		}
	}
	else
        m_bHealthFlash = LTFALSE;

	if (m_fTotalArmorFlash > 0.0f)
	{
		m_fTotalArmorFlash -= fDelta;
		m_fCurrArmorFlash -= fDelta;
		if (m_fCurrArmorFlash < 0.0f)
		{
			m_bArmorFlash = !m_bArmorFlash;
			m_fCurrArmorFlash = m_fFlashSpeed;
		}
	}
	else
        m_bArmorFlash = LTFALSE;
}


void CPlayerStats::UpdateCrosshairColors()
{
	fLastColorR = g_vtCrosshairColorR.GetFloat();
	if (fLastColorR < 0.1f)
	{
		fLastColorR = 0.1f;
		g_vtCrosshairColorR.SetFloat(0.1f);
	}
	else if (fLastColorR > 1.0f)
	{
		fLastColorR = 1.0f;
		g_vtCrosshairColorR.SetFloat(1.0f);
	}

	fLastColorG = g_vtCrosshairColorG.GetFloat();
		if (fLastColorG < 0.1f)
	{
		fLastColorG = 0.1f;
		g_vtCrosshairColorG.SetFloat(0.1f);
	}
	else if (fLastColorG > 1.0f)
	{
		fLastColorG = 1.0f;
		g_vtCrosshairColorG.SetFloat(1.0f);
	}

	fLastColorB = g_vtCrosshairColorB.GetFloat();
	if (fLastColorB < 0.1f)
	{
		fLastColorB = 0.1f;
		g_vtCrosshairColorB.SetFloat(0.1f);
	}
	else if (fLastColorB > 1.0f)
	{
		fLastColorB = 1.0f;
		g_vtCrosshairColorB.SetFloat(1.0f);
	}

	fLastAlpha = g_vtCrosshairAlpha.GetFloat();
	if (fLastAlpha < 0.1f)
	{
		fLastAlpha = 0.1f;
		g_vtCrosshairAlpha.SetFloat(0.1f);
	}
	else if (fLastAlpha > 1.0f)
	{
		fLastAlpha = 1.0f;
		g_vtCrosshairAlpha.SetFloat(1.0f);
	}
    HLTCOLOR hCursorColor = g_pLTClient->SetupColor1(fLastColorR,fLastColorG,fLastColorB,LTFALSE);
    LTRect rect(0,0,2,2);

	if (!m_hArmedCrosshair)
	{
        m_hArmedCrosshair = g_pLTClient->CreateSurface(2,2);
	}

    g_pLTClient->FillRect(m_hArmedCrosshair,&rect,hCursorColor);
    g_pLTClient->OptimizeSurface (m_hArmedCrosshair, kTransBlack);
    g_pLTClient->SetSurfaceAlpha(m_hArmedCrosshair,fLastAlpha);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawCrosshair()
//
//	PURPOSE:	Draw the crosshair
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DrawCrosshair(HSURFACE hScreen, int nCenterX,
								 int nCenterY, LTBOOL bMenuCrosshair)
{
	if (!m_hArmedCrosshair)
	{
		ResetStats();
	}
	// See if we are drawing the menu crosshair, else draw based on
	// the current weapon...

	if (bMenuCrosshair)
	{
		DrawArmedCrosshair(LTNULL, hScreen, nCenterX, nCenterY);
		return;
	}

	// Draw the appropriate cross-hair based on our current weapon,
	// and what is directly in front of us...

	CWeaponModel* pWeapon = g_pGameClientShell->GetWeaponModel();

	if (!pWeapon || pWeapon->IsDisabled()) return;


	// First see if we should draw the activate crosshair, if not
	// draw the armed/unarmed crosshair

	uint32 dwUsrflgs = 0;
	LTFLOAT fDistAway = 100000.0f;

	HOBJECT hObj = TestForActivationObject(dwUsrflgs, fDistAway);
	if (hObj)
	{
		if (DrawActivateCrosshair(hObj, hScreen, nCenterX, nCenterY,
			dwUsrflgs, fDistAway))
		{
			return;
		}
	}

	DrawTargetName();

	// No activation object, so draw normal crosshair...

	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_nCurrentWeapon);
	if (!pWeaponData) return;

	if (pWeaponData->bLooksDangerous)
	{
		DrawArmedCrosshair(pWeapon, hScreen, nCenterX, nCenterY);
	}
	else
	{
		DrawUnArmedCrosshair(hScreen, nCenterX, nCenterY);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawUnArmedCrosshair()
//
//	PURPOSE:	Draw the melee weapon crosshair
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DrawUnArmedCrosshair(HSURFACE hScreen, int nCenterX, int nCenterY)
{
	DrawSurfaceCrosshair(m_hUnarmedCrosshair, hScreen, nCenterX, nCenterY);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawArmedCrosshair()
//
//	PURPOSE:	Draw the weapon crosshair
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DrawArmedCrosshair(CWeaponModel* pWeapon, HSURFACE hScreen,
									   int nCenterX, int nCenterY)
{
	if (g_pInterfaceMgr->GetSunglassMode() == SUN_CAMERA) return;

	LTRect srect(0,0,2,2);
    LTFLOAT  fRad = 0.25;
	if (g_vtCrosshairDynamic.GetFloat() >= 1.0f && pWeapon)
	{
		float fTemp = pWeapon->GetMovementPerturb();
		if (fabs(fRad-fTemp) > 0.1f)
			fRad = fTemp;
	}
	int nSize = (int)(g_vtCrosshairBarMin.GetFloat() + (fRad * (g_vtCrosshairBarMax.GetFloat() - g_vtCrosshairBarMin.GetFloat())));
	int nInside = (int)(g_vtCrosshairGapMin.GetFloat() + (fRad * (g_vtCrosshairGapMax.GetFloat() - g_vtCrosshairGapMin.GetFloat())));
	int nOutside = nInside + nSize;
	int nSlide =  nInside + (int)(fRad * nSize);


    LTRect rect;

	int style = (int)g_vtCrosshairStyle.GetFloat();
    uint32 dwFlags = 0;
	switch (style)
	{
	case 0:
		dwFlags = CH_LEFT | CH_TOP | CH_RIGHT | CH_BOTTOM;
		break;
	case 1:
		dwFlags = CH_LEFT | CH_TOP | CH_RIGHT | CH_BOTTOM | CH_XLEFT | CH_XTOP | CH_XRIGHT | CH_XBOTTOM;
		break;
	case 2:
		dwFlags = CH_LEFT | CH_TOP | CH_RIGHT | CH_BOTTOM | CH_DOT;
		break;
	case 3:
		dwFlags = CH_LEFT | CH_RIGHT | CH_FULLBOTTOM | CH_XLEFT | CH_XRIGHT;
		break;
	case 4:
		dwFlags = CH_DOT;
		break;
	case 5:
		dwFlags = CH_FULLRIGHT | CH_FULLBOTTOM;
		break;
	default:
		dwFlags = CH_LEFT | CH_TOP | CH_RIGHT | CH_BOTTOM;
		break;

	}

	//left
	if (dwFlags & CH_LEFT)
	{
		rect.top = nCenterY - 1;
		rect.left = nCenterX - nOutside;
		rect.bottom = nCenterY + 1;
		rect.right = nCenterX - nInside;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}
	if (dwFlags & CH_XLEFT)
	{
		rect.top = nCenterY - nInside/2;
		rect.left = (nCenterX - nSlide) - 1;
		rect.bottom = nCenterY + nInside/2;
		rect.right = nCenterX - nSlide;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}

	//right
	if (dwFlags & CH_RIGHT)
	{
		rect.top = nCenterY - 1;
		rect.left = nCenterX + nInside;
		rect.bottom = nCenterY + 1;
		rect.right = nCenterX + nOutside;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}
	if (dwFlags & CH_XRIGHT)
	{
		rect.top = nCenterY - nInside/2;
		rect.left = (nCenterX + nSlide);
		rect.bottom = nCenterY + nInside/2;
		rect.right = nCenterX + nSlide + 1;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}
	if (dwFlags & CH_FULLRIGHT)
	{
		rect.top = nCenterY - 1;
		rect.left = nCenterX;
		rect.bottom = nCenterY + 1;
		rect.right = nCenterX + nOutside;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}

	//top
	if (dwFlags & CH_TOP)
	{
		rect.top = nCenterY - nOutside;
		rect.left = nCenterX - 1;
		rect.bottom = nCenterY - nInside;
		rect.right = nCenterX + 1;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}
	if (dwFlags & CH_XTOP)
	{
		rect.top = (nCenterY - nSlide) - 1;
		rect.left = nCenterX - nInside/2;
		rect.bottom = nCenterY - nSlide;
		rect.right = nCenterX + nInside/2;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}

	//bottom
	if (dwFlags & CH_BOTTOM)
	{
		rect.top = nCenterY + nInside;
		rect.left = nCenterX - 1;
		rect.bottom = nCenterY + nOutside;
		rect.right = nCenterX + 1;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}
	if (dwFlags & CH_XBOTTOM)
	{
		rect.top = (nCenterY + nSlide);
		rect.left = nCenterX - nInside/2;
		rect.bottom = nCenterY + nSlide + 1;
		rect.right = nCenterX + nInside/2;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}
	if (dwFlags & CH_FULLBOTTOM)
	{
		rect.top = nCenterY;
		rect.left = nCenterX - 1;
		rect.bottom = nCenterY + nOutside;
		rect.right = nCenterX + 1;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}

	if (dwFlags & CH_DOT)
	{
		rect.top = nCenterY - 1;
		rect.left = nCenterX -1;
		rect.bottom = nCenterY + 1;
		rect.right = nCenterX + 1;
        g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hArmedCrosshair, &rect, &srect);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::IsGadgetActivatable()
//
//	PURPOSE:	Can the passed in object be activated with our
//				currently selected gadget.
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerStats::IsGadgetActivatable(HOBJECT hObj)
{
	if (!hObj) return LTFALSE;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_nCurrentAmmo);
	if (!pAmmo) return LTFALSE;

	DamageType eType = pAmmo->eInstDamageType;

	uint32 dwUserFlags = 0;
    g_pLTClient->GetObjectUserFlags(hObj, &dwUserFlags);

	// If its a character, see if our current weapon is the
	// lighter...

	LTBOOL bIsGadgetActivatable = LTFALSE;

	if (dwUserFlags & USRFLG_CHARACTER)
	{
		if ((pAmmo->eType == GADGET) && (eType == DT_GADGET_LIGHTER))
		{
			bIsGadgetActivatable = LTTRUE;
		}
	}


	if (!bIsGadgetActivatable && (pAmmo->eType == GADGET))
	{
		if (eType == DT_GADGET_CAMERA_DISABLER)
		{
			// Make sure the object can be disabled...

			if (dwUserFlags & USRFLG_GADGET_CAMERA_DISABLER)
				bIsGadgetActivatable = LTTRUE;
		}
		else if (eType == DT_GADGET_CODE_DECIPHERER)
		{
			// Make sure the object can be deciphered...

			if (dwUserFlags & USRFLG_GADGET_CODE_DECIPHERER)
				bIsGadgetActivatable = LTTRUE;
		}
		else if (eType == DT_GADGET_LOCK_PICK)
		{
			// Make sure the object is "pickable"...

			if (dwUserFlags & USRFLG_GADGET_LOCK_PICK)
				bIsGadgetActivatable = LTTRUE;
		}
		else if (eType == DT_GADGET_LIGHTER)
		{
			// Make sure the object is "lightable"...

			if (dwUserFlags & USRFLG_GADGET_LIGHTER)
				bIsGadgetActivatable = LTTRUE;
		}
		else if (eType == DT_GADGET_WELDER)
		{
			// Make sure the object is "weldable"...

			if (dwUserFlags & USRFLG_GADGET_WELDER)
				bIsGadgetActivatable = LTTRUE;
		}
		else if (eType == DT_GADGET_CAMERA)
		{
			// Make sure the object is something we can photograph...

			if (dwUserFlags & USRFLG_GADGET_INTELLIGENCE)
			{
				bIsGadgetActivatable = g_pGameClientShell->InCameraGadgetRange(hObj);
			}
		}
		else if (eType == DT_GADGET_ZIPCORD)
		{
			// Make sure the object is something we can zipcord to...

			if (dwUserFlags & USRFLG_GADGET_ZIPCORD)
				bIsGadgetActivatable = LTTRUE;
		}
	}

	return bIsGadgetActivatable;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawActivateCrosshair()
//
//	PURPOSE:	Draw the activate weapon crosshair
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerStats::DrawActivateCrosshair(HOBJECT hObj, HSURFACE hScreen,
										 int nCenterX, int nCenterY,
										 uint32 dwInitialUsrFlags,
										 LTFLOAT fDistAway)
{
	if (!hObj) return LTFALSE;

	uint32 dwUserFlags = 0;
    g_pLTClient->GetObjectUserFlags(hObj, &dwUserFlags);

	dwUserFlags |= dwInitialUsrFlags;

	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_nCurrentWeapon);
	if (!pWeaponData) return LTFALSE;

	if (IsGadgetActivatable(hObj))
	{
		if (fDistAway <= (LTFLOAT) pWeaponData->nRange)
		{
			// Cool looks like we can use our gadget on it...

			m_bDrawingGadgetActivate = LTTRUE;

			DrawSurfaceCrosshair(m_hActivateGadgetCrosshair, hScreen, nCenterX, nCenterY);
			return LTTRUE;
		}
	}
	else if (dwUserFlags & USRFLG_CHARACTER)
	{
		if (g_pGameClientShell->IsMultiplayerGame()) return LTFALSE;

		// Draw the innocent crosshair if we have a dangerous weapon...

		if (pWeaponData->bLooksDangerous)
		{
			CCharacterFX* pFX = (CCharacterFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_CHARACTER_ID, hObj);
			if (pFX)
			{
				if (pFX->m_cs.eCrosshairCharacterClass != BAD)
				{
					if (fDistAway <= (LTFLOAT) pWeaponData->nRange)
					{
						DrawSurfaceCrosshair(m_hInnocentCrosshair, hScreen, nCenterX, nCenterY);
						return LTTRUE;
					}
				}
			}
		}
		else
		{
			// Draw the activate crosshair if we're close enough...

			if (dwUserFlags & USRFLG_CAN_ACTIVATE)
			{
				if (fDistAway <= c_ActivationDist)
				{
					DrawSurfaceCrosshair(m_hActivateCrosshair, hScreen, nCenterX, nCenterY);
					return LTTRUE;
				}
			}
		}
	}
	else if (dwUserFlags & USRFLG_CAN_ACTIVATE)
	{
		// Draw the activate crosshair

		if (fDistAway <= c_ActivationDist)
		{
			DrawSurfaceCrosshair(m_hActivateCrosshair, hScreen, nCenterX, nCenterY);
			return LTTRUE;
		}
	}


	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawSurfaceCrosshair()
//
//	PURPOSE:	Draw the surface crosshair
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DrawSurfaceCrosshair(HSURFACE hSurf, HSURFACE hScreen,
										 int nCenterX, int nCenterY)
{
	if (g_pInterfaceMgr->GetSunglassMode() == SUN_CAMERA) return;

	LTRect rect;
    LTRect srect(0,0,2,2);

    uint32 dwWidth, dwHeight;
    g_pLTClient->GetSurfaceDims(hSurf, &dwWidth, &dwHeight);
	srect.right = dwWidth;
	srect.bottom = dwHeight;

 	rect.top = nCenterY - (dwHeight/2);
	rect.left = nCenterX - (dwWidth/2);
	rect.bottom = nCenterY + (dwHeight/2);
	rect.right = nCenterX + (dwWidth/2);

	g_pLTClient->ScaleSurfaceToSurfaceTransparent(hScreen, hSurf, &rect,
		&srect, kTransBlack);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::TestForActivationObject()
//
//	PURPOSE:	See if there is an activation object directly in front of
//				the camera.
//
// ----------------------------------------------------------------------- //

HOBJECT CPlayerStats::TestForActivationObject(uint32 & dwUsrFlags, LTFLOAT & fDistAway)
{
	HOBJECT hObj = LTNULL;
	fDistAway = 1000000.0f;

	// Cast ray from the camera to see if there is an object to activate...

	HOBJECT hCamera = g_pGameClientShell->GetCamera();
	if (!hCamera) return LTNULL;

    LTRotation rRot;
    LTVector vU, vR, vF, vPos;

    g_pLTClient->GetObjectPos(hCamera, &vPos);
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

  	HOBJECT hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return LTNULL;

	LTVector vDims;
    g_pLTClient->Physics()->GetObjectDims(hPlayerObj, &vDims);

	// Use our current weapon's range if it is a gadget...

	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_nCurrentWeapon);
	if (!pWeaponData) return LTNULL;

    LTFLOAT fDist = (vDims.x + vDims.z)/2.0f + c_ActivationDist;
	LTFLOAT fMaxDist = fDist;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_nCurrentAmmo);
	if (!pAmmo) return LTNULL;

	// If we're using a gadget, use the gadget's range as the distance...

	//if (pAmmo->eType == GADGET)
	//{
	if (pWeaponData->nRange > fDist)
	{
		fDist = (LTFLOAT) pWeaponData->nRange;
	}
	//}


	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To   = IQuery.m_From + (vF * fDist);;

	IQuery.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;

	IQuery.m_FilterFn	  = ActivateFilterFn;
	IQuery.m_pUserData	  = this;
	IQuery.m_PolyFilterFn = DoVectorPolyFilterFn;

	if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
	{
        if (IsMainWorld(IInfo.m_hObject))
		{
			if (IInfo.m_hPoly != INVALID_HPOLY)
			{
				SurfaceType eType = GetSurfaceType(IInfo.m_hPoly);
				SURFACE *pSurf = g_pSurfaceMgr->GetSurface(eType);

				// See if the surface we tried to activate has an activation
				// sound...If so, the user can activate it...

				if (pSurf && pSurf->szActivationSnd[0] && pSurf->fActivationSndRadius > 0)
				{
					dwUsrFlags |= USRFLG_CAN_ACTIVATE;
				}
			}
		}
		else
		{
			uint32 dwUserFlags = 0;
		    g_pLTClient->GetObjectUserFlags(IInfo.m_hObject, &dwUserFlags);

			// If its an activatable object, only show the activate
			// crosshair if we are close enough...

			if (!(dwUserFlags & USRFLG_CHARACTER))
			{
				if (dwUserFlags & USRFLG_CAN_ACTIVATE)
				{
					LTVector vObjPos = IInfo.m_Point;

					vObjPos -= vPos;
					if (vObjPos.Mag() > fMaxDist)
					{
						return LTNULL;
					}
				}
			}
		}

		// Calculate how far away the object is...

		LTVector vDist = IInfo.m_Point - vPos;
		fDistAway = vDist.Mag();

		hObj = IInfo.m_hObject;
	}

	return hObj;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ShowObjectives()
//
//	PURPOSE:	Draw the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::ShowObjectives()
{

	m_bObjVisible = LTTRUE;
}

void CPlayerStats::HideObjectives()
{
	m_bObjVisible = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawObjectives()
//
//	PURPOSE:	Draw the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DrawObjectives(HSURFACE hScreen)
{
    if (!g_pLTClient || !m_bObjVisible) return;
	if (m_Objectives.nNumObjectives < 1) return;


	float xRatio = g_pInterfaceResMgr->GetXRatio();
	float yRatio = g_pInterfaceResMgr->GetYRatio();
	int nTeam = 0;
	HLTCOLOR hTeamColor = LTNULL;

	if (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT)
	{
		CLIENT_INFO* pInfo = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if (!pInfo) return;
		TEAM_INFO* pTeam = g_pInterfaceMgr->GetClientInfoMgr()->GetTeam(pInfo->team);

		if (pInfo->team == 1)
		{
			nTeam = IDS_PLAYER_UNITY;
			hTeamColor = pTeam->hColor;
		}
		else
		{
			nTeam = IDS_PLAYER_HARM;
			hTeamColor = pTeam->hColor;
		}
	}

   	int nWidth = (int)(xRatio * (float)(g_rcObjectives.right - g_rcObjectives.left));
	int nHeight = (int)(yRatio * (float)(g_rcObjectives.bottom - g_rcObjectives.top));


	m_ObjectivePos.x = (int)(xRatio * (float)g_rcObjectives.left);
	m_ObjectivePos.y = (int)(yRatio * (float)g_rcObjectives.top);

	int nTextWidth = nWidth - g_ObjTextOffset.x;
	int x = m_ObjectivePos.x;
	int y = m_ObjectivePos.y;

	if (nTeam)
	{
		LTRect rcBanner(x,y,x+nWidth,y+g_pObjForeFont->GetHeight());
		g_pLTClient->FillRect(hScreen,&rcBanner,hTeamColor);

		HSTRING hTeam = g_pLTClient->FormatString(nTeam);
		g_pObjForeFont->Draw(hTeam,hScreen,x+nWidth/2,y,LTF_JUSTIFY_CENTER,kWhite);
		g_pLTClient->FreeString(hTeam);
		y += g_pObjForeFont->GetHeight();
	}

	for (int i = m_Objectives.nNumObjectives-1; i >= 0 ; i--)
	{

        uint32 objID = m_Objectives.dwObjectives[i];
		HSURFACE hCheck = LTNULL;
		HLTCOLOR hColor = kWhite;

		int nIndex;
		if (m_CompletedObjectives.Have(objID,nIndex))
		{
			hCheck = g_pInterfaceResMgr->GetSharedSurface("interface\\HUDcheck-on.pcx");
			hColor = SETRGB(128,128,128);
		}
		else
		{
			hCheck = g_pInterfaceResMgr->GetSharedSurface("interface\\HUDcheck-off.pcx");
		}


		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
        g_pLTClient->DrawSurfaceToSurface(hScreen, hCheck, LTNULL, x + 2, y + g_nObjBmpOffset + 2);

		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASKADD);
		g_pLTClient->SetOptimized2DColor(hColor);
        g_pLTClient->DrawSurfaceToSurface(hScreen, hCheck, LTNULL, x, y + g_nObjBmpOffset);

		g_pLTClient->SetOptimized2DColor(kWhite);
		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ALPHA);



		HSTRING hTxt = g_pLTClient->FormatString(objID);

		LTIntPt txtSz = g_pObjForeFont->GetTextExtentsFormat(hTxt,nTextWidth);
		//using different fore and back fonts can cause problems when word-wrapping
		//safer to use the same font for both front aand back
		g_pObjForeFont->DrawFormat(hTxt,hScreen,x+g_ObjTextOffset.x+1,y+g_ObjTextOffset.y+1,nTextWidth,kBlack);
		g_pObjForeFont->DrawFormat(hTxt,hScreen,x+g_ObjTextOffset.x,y+g_ObjTextOffset.y,nTextWidth,hColor);
		y += Max((int)g_dwObjHeight,txtSz.y);
		g_pLTClient->FreeString(hTxt);
	}

	if (nTeam)
	{
		y += 16;
		int iy = m_ObjectivePos.y;
		LTRect rcBanner(x,iy,x+2,y);
		g_pLTClient->FillRect(hScreen,&rcBanner,hTeamColor);

		rcBanner = LTRect(x+nWidth-2,iy,x+nWidth,y);
		g_pLTClient->FillRect(hScreen,&rcBanner,hTeamColor);

		rcBanner = LTRect(x,y-2,x+nWidth,y);
		g_pLTClient->FillRect(hScreen,&rcBanner,hTeamColor);

	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DeleteCrosshairs()
//
//	PURPOSE:	Free crosshair surfaces
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DeleteCrosshairs()
{
    if (m_hArmedCrosshair) g_pLTClient->DeleteSurface(m_hArmedCrosshair);
    if (m_hUnarmedCrosshair) g_pLTClient->DeleteSurface(m_hUnarmedCrosshair);
    if (m_hActivateCrosshair) g_pLTClient->DeleteSurface(m_hActivateCrosshair);
    if (m_hInnocentCrosshair) g_pLTClient->DeleteSurface(m_hInnocentCrosshair);
    if (m_hActivateGadgetCrosshair) g_pLTClient->DeleteSurface(m_hActivateGadgetCrosshair);

	if (hCrossHighlight) g_pLTClient->DeleteSurface(hCrossHighlight);


    m_hArmedCrosshair			= LTNULL;
    m_hUnarmedCrosshair			= LTNULL;
    m_hActivateCrosshair		= LTNULL;
	m_hInnocentCrosshair		= LTNULL;
	m_hActivateGadgetCrosshair	= LTNULL;
    hCrossHighlight				= LTNULL;

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CreateCrosshairs()
//
//	PURPOSE:	Create crosshair surfaces
//
// ----------------------------------------------------------------------- //

void CPlayerStats::CreateCrosshairs()
{

	// load the surfaces

    m_hUnarmedCrosshair = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\Unarmed.pcx");
    HLTCOLOR hTransColor = kTransBlack;
    g_pLTClient->OptimizeSurface (m_hUnarmedCrosshair, hTransColor);

    m_hActivateCrosshair = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\Activate.pcx");
    hTransColor = kTransBlack;
    g_pLTClient->OptimizeSurface(m_hActivateCrosshair, hTransColor);

    m_hInnocentCrosshair = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\Innocent.pcx");
    hTransColor = kTransBlack;
    g_pLTClient->OptimizeSurface(m_hInnocentCrosshair, hTransColor);

    m_hActivateGadgetCrosshair = g_pLTClient->CreateSurfaceFromBitmap("StatBar\\Art\\ActivateGadget.pcx");
    hTransColor = kTransBlack;
    g_pLTClient->OptimizeSurface(m_hActivateGadgetCrosshair, hTransColor);


    hCrossHighlight     = g_pLTClient->CreateSurface(2,2);
    LTRect rect(0,0,2,2);
    g_pLTClient->FillRect(hCrossHighlight,&rect,SETRGB(128,128,128));
    g_pLTClient->OptimizeSurface (hCrossHighlight, hTransColor);
    g_pLTClient->SetSurfaceAlpha(hCrossHighlight,0.5f);

	if (!m_hArmedCrosshair)
	{
		UpdateCrosshairColors();
	}


}

void CPlayerStats::UpdateWeaponBindings()
{
    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (DEVICETYPE_KEYBOARD);
	if (!pBindings)
	{
		return;
	}
	m_fWeaponAlpha = GetConsoleFloat("BindingIconAlpha",0.7f);
	LTBOOL bLarge = (GetConsoleInt("BindingIconSize",0) > 0);
	for (int i = 0; i < 10; i++)
	{
		if (m_hWeaponSurf[i])
		{
			g_pLTClient->DeleteSurface(m_hWeaponSurf[i]);
			m_hWeaponSurf[i] = LTNULL;
		}
		int nCommand = COMMAND_ID_WEAPON_BASE;
        LTBOOL bFound = LTFALSE;
		DeviceBinding* ptr = pBindings;
		while (!bFound && ptr)
		{
			if (stricmp(ptr->strTriggerName,szTriggers[i])==0)
			{
				GameAction* pAction = ptr->pActionHead;
				if (pAction && pAction->nActionCode >= COMMAND_ID_WEAPON_BASE && pAction->nActionCode <= COMMAND_ID_WEAPON_MAX)
				{
					nCommand = pAction->nActionCode;
					bFound = LTTRUE;
				}

			}

			ptr = ptr->pNext;
		}

		m_nBoundWeapons[i] = g_pWeaponMgr->GetWeaponId(nCommand);

		if (m_fWeaponAlpha < 0.1f) continue;

		WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(m_nBoundWeapons[i]);
		m_nIconSize = 0;
		if (pWeapon)
		{
			if (bLarge)
			{
				m_hWeaponSurf[i] = g_pLTClient->CreateSurfaceFromBitmap(pWeapon->szIcon);
			}
			else
			{
				m_hWeaponSurf[i] = g_pLTClient->CreateSurfaceFromBitmap(pWeapon->szSmallIcon);
			}
			if (m_hWeaponSurf[i])
			{
				if (!m_nIconSize)
				{
					uint32 h;
					g_pLTClient->GetSurfaceDims(m_hWeaponSurf[i],&m_nIconSize,&h);
				}
				g_pLTClient->OptimizeSurface(m_hWeaponSurf[i],SETRGB(255,0,255));
				g_pLTClient->SetSurfaceAlpha(m_hWeaponSurf[i],m_fWeaponAlpha);
			}

		}

		if (m_hNumberSurf[i] && bLarge != m_bLargeNumbers)
		{
			g_pLTClient->DeleteSurface(m_hNumberSurf[i]);
			m_hNumberSurf[i] = LTNULL;
		}
		if (!m_hNumberSurf[i])
		{
			if (bLarge)
				m_hNumberSurf[i] = g_pInterfaceResMgr->CreateSurfaceFromString(g_pInterfaceResMgr->GetMediumFont(),szTriggers[i],kTransBlack);
			else
				m_hNumberSurf[i] = g_pInterfaceResMgr->CreateSurfaceFromString(g_pInterfaceResMgr->GetSmallFont(),szTriggers[i],kTransBlack);
		}

	}
	m_bLargeNumbers = bLarge;
    g_pLTClient->FreeDeviceBindings (pBindings);

}

void CPlayerStats::DrawBoundWeapons(HSURFACE hScreen)
{
	if (m_fWeaponAlpha < 0.1f) return;

	int w = (int)g_pInterfaceResMgr->GetScreenWidth();
	int y = (int)g_pInterfaceResMgr->GetScreenHeight()/2 - 5*m_nIconSize;

	HLTCOLOR hTextColor = kWhite;
	if (m_fWeaponAlpha < 1.0f)
	{
		uint8 fade = (uint8)(255.0f * m_fWeaponAlpha);
		hTextColor = SETRGB(fade,fade,fade);
	}

	int nTextOffset = m_nIconSize - 8;
	if (m_bLargeNumbers)
		nTextOffset -= 8;
	for (int i = 0; i < 10; i++)
	{
		if (m_pbHaveWeapon && m_pbHaveWeapon[m_nBoundWeapons[i]] && m_hWeaponSurf[i])
		{
			if (m_fIconOffset[i] < (LTFLOAT)m_nIconSize)
			{
				m_fIconOffset[i] += (72.0f*g_pLTClient->GetFrameTime());
			}
			if (m_fIconOffset[i] > (LTFLOAT)m_nIconSize) 
				m_fIconOffset[i] = (LTFLOAT)m_nIconSize;
			int x = w-(int)m_fIconOffset[i];
			g_pLTClient->DrawSurfaceToSurface(hScreen,m_hWeaponSurf[i],LTNULL,x,y);
			if (GetConsoleInt("BindingNumbers",1) > 0)
			{
				
				g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
				g_pLTClient->SetOptimized2DColor(hTextColor);
				g_pLTClient->DrawSurfaceToSurface(hScreen,m_hNumberSurf[i],LTNULL,x+nTextOffset+1,y+1);
				g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ADD);
				g_pLTClient->DrawSurfaceToSurface(hScreen,m_hNumberSurf[i],LTNULL,x+nTextOffset,y);
				g_pLTClient->SetOptimized2DColor(kWhite);
				g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ALPHA);
			}
		}
		else if (m_fIconOffset[i] > 0.0f) 
			m_fIconOffset[i] = 0.0f;
		y += m_nIconSize;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CreateDamageSurfaces
//
//	PURPOSE:	Create surfaces for displaying current progressive damage
//
// --------------------------------------------------------------------------- //

void CPlayerStats::CreateDamageSurfaces()
{
	m_hBleeding = g_pLTClient->CreateSurfaceFromBitmap("statbar\\art\\icon_bleed.pcx");
	if (m_hBleeding)
	{
		g_pLTClient->OptimizeSurface(m_hBleeding,kTransBlack);
		g_pLTClient->SetSurfaceAlpha(m_hBleeding,0.7f);
		uint32 h,w;
		g_pLTClient->GetSurfaceDims(m_hBleeding,&w,&h);
		m_nDamageIconSize = (int)w;
	}
	m_hPoisoned = g_pLTClient->CreateSurfaceFromBitmap("statbar\\art\\icon_poison.pcx");
	if (m_hPoisoned)
	{
		g_pLTClient->OptimizeSurface(m_hPoisoned,kTransBlack);
		g_pLTClient->SetSurfaceAlpha(m_hPoisoned,0.7f);
	}
	m_hStunned = g_pLTClient->CreateSurfaceFromBitmap("statbar\\art\\icon_stun.pcx");
	if (m_hStunned)
	{
		g_pLTClient->OptimizeSurface(m_hStunned,kTransBlack);
		g_pLTClient->SetSurfaceAlpha(m_hStunned,0.7f);
	}
	m_hSleeping = g_pLTClient->CreateSurfaceFromBitmap("statbar\\art\\icon_sleep.pcx");
	if (m_hSleeping)
	{
		g_pLTClient->OptimizeSurface(m_hSleeping,kTransBlack);
		g_pLTClient->SetSurfaceAlpha(m_hSleeping,0.7f);
	}
	m_hBurning = g_pLTClient->CreateSurfaceFromBitmap("statbar\\art\\icon_burn.pcx");
	if (m_hBurning)
	{
		g_pLTClient->OptimizeSurface(m_hBurning,kTransBlack);
		g_pLTClient->SetSurfaceAlpha(m_hBurning,0.7f);
	}
	m_hChoking = g_pLTClient->CreateSurfaceFromBitmap("statbar\\art\\icon_choke.pcx");
	if (m_hChoking)
	{
		g_pLTClient->OptimizeSurface(m_hChoking,kTransBlack);
		g_pLTClient->SetSurfaceAlpha(m_hChoking,0.7f);
	}
	m_hElectrocute = g_pLTClient->CreateSurfaceFromBitmap("statbar\\art\\icon_electrocute.pcx");
	if (m_hElectrocute)
	{
		g_pLTClient->OptimizeSurface(m_hElectrocute,kTransBlack);
		g_pLTClient->SetSurfaceAlpha(m_hElectrocute,0.7f);
	}


}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DestroyDamageSurfaces
//
//	PURPOSE:	Destroy surfaces for displaying current progressive damage
//
// --------------------------------------------------------------------------- //

void CPlayerStats::DestroyDamageSurfaces()
{
	if (m_hBleeding)
        g_pLTClient->DeleteSurface (m_hBleeding);
    m_hBleeding = LTNULL;
	if (m_hPoisoned)
        g_pLTClient->DeleteSurface (m_hPoisoned);
    m_hPoisoned = LTNULL;
	if (m_hStunned)
        g_pLTClient->DeleteSurface (m_hStunned);
    m_hStunned = LTNULL;
	if (m_hSleeping)
        g_pLTClient->DeleteSurface (m_hSleeping);
    m_hSleeping = LTNULL;
	if (m_hBurning)
        g_pLTClient->DeleteSurface (m_hBurning);
    m_hBurning = LTNULL;
	if (m_hChoking)
        g_pLTClient->DeleteSurface (m_hChoking);
    m_hChoking = LTNULL;
	if (m_hElectrocute)
        g_pLTClient->DeleteSurface (m_hElectrocute);
    m_hElectrocute = LTNULL;
};


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawTargetName
//
//	PURPOSE:	Draws the name of whatever we're targetting
//
// --------------------------------------------------------------------------- //

void CPlayerStats::DrawTargetName()
{
	if ( !g_pGameClientShell->IsMultiplayerGame() ) return;

	// Cast ray from the camera to see if there is a target

	HOBJECT hCamera = g_pGameClientShell->GetCamera();
	if (!hCamera) return;

    LTRotation rRot;
    LTVector vU, vR, vF, vPos;

    g_pLTClient->GetObjectPos(hCamera, &vPos);
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

  	HOBJECT hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

	LTVector vDims;
    g_pLTClient->Physics()->GetObjectDims(hPlayerObj, &vDims);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To   = IQuery.m_From + (vF * 10000000.0f);

	IQuery.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;

	IQuery.m_FilterFn	  = ActivateFilterFn;
	IQuery.m_pUserData	  = this;
	IQuery.m_PolyFilterFn = DoVectorPolyFilterFn;

	if ( g_pLTClient->IntersectSegment(&IQuery, &IInfo) && IInfo.m_hObject )
	{
		uint32 dwUserFlags = 0;
		g_pLTClient->GetObjectUserFlags(IInfo.m_hObject, &dwUserFlags);

		uint32 dwFlags = g_pLTClient->GetObjectFlags(IInfo.m_hObject);

		// If its an activatable object, only show the activate
		// crosshair if we are close enough...

		if ( (dwUserFlags & USRFLG_CHARACTER) && (dwFlags & FLAG_VISIBLE) )
		{
			// Draw their name if it's multiplayer
			CCharacterFX* pCharacterFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(IInfo.m_hObject);
			if ( pCharacterFX )
			{
				HLTCOLOR hColor = kWhite;
				char* szName = "";

				if ( !pCharacterFX->m_cs.bIsPlayer ) return;

				CLIENT_INFO* pInfo = g_pInterfaceMgr->GetClientInfoMgr()->GetClientByID(pCharacterFX->m_cs.nClientID);
				if ( !pInfo ) return;

				szName = g_pLTClient->GetStringData(pInfo->hstrName);

				TEAM_INFO* pTeam = g_pInterfaceMgr->GetClientInfoMgr()->GetTeam(pInfo->team);
				if ( pTeam )
				{
					hColor = pTeam->hColor;
				}

				CLTGUIFont* pFont;
				switch ( (int)GetConsoleFloat("TargetNameSize", 0.0f) )
				{
					case 2: pFont = g_pInterfaceResMgr->GetLargeFont(); break;
					case 1: pFont = g_pInterfaceResMgr->GetMediumFont(); break;
					default:
					case 0: pFont = g_pInterfaceResMgr->GetSmallFont(); break;
				}

				if (pFont)
				{
					LTFLOAT fAlpha = GetConsoleFloat("TargetNameTransparency", 1.0f);
					uint32 r, g, b;
					GETRGB(hColor, r, g, b);
					hColor = SETRGB(r*fAlpha, g*fAlpha, b*fAlpha);

					HSTRING hstrName = g_pLTClient->CreateString(szName);
					LTRect rcText = LTRect(0, 0, pFont->GetTextExtents(hstrName).x, pFont->GetTextExtents(hstrName).y);

					if ( !m_hTargetNameSurface || (rcText.right > m_nTargetNameWidth) || (rcText.bottom > m_nTargetNameHeight) )
					{
						if ( m_hTargetNameSurface )
						{
							g_pLTClient->DeleteSurface(m_hTargetNameSurface);
						}

						m_hTargetNameSurface = g_pLTClient->CreateSurface(rcText.right, rcText.bottom);
						m_nTargetNameWidth = rcText.right;
						m_nTargetNameHeight = rcText.bottom;
					}

					int nTest = (int)(g_pLTClient->GetTime()*10.0f)%255;
					g_pLTClient->FillRect(m_hTargetNameSurface, &LTRect(0,0,m_nTargetNameWidth,m_nTargetNameHeight), kBlack);
					pFont->Draw(hstrName, m_hTargetNameSurface, 0, 0, LTF_JUSTIFY_LEFT, kWhite);
					g_pLTClient->OptimizeSurface(m_hTargetNameSurface, kBlack);
					g_pLTClient->FreeString(hstrName);

					HSURFACE hScreen = g_pLTClient->GetScreenSurface();
		
					int x = (g_pInterfaceResMgr->GetScreenWidth() - rcText.right)/2;
					int y = (g_pInterfaceResMgr->GetScreenHeight() - rcText.bottom)/2 - 16;
					g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
					g_pLTClient->SetOptimized2DColor(hColor);
					g_pLTClient->DrawSurfaceToSurface(hScreen,m_hTargetNameSurface,LTNULL,x+1,y+1);
					g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ADD);
					g_pLTClient->DrawSurfaceToSurface(hScreen,m_hTargetNameSurface,LTNULL,x,y);
					g_pLTClient->SetOptimized2DColor(kWhite);
					g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ALPHA);
				}
			}	
		}
	}
}
