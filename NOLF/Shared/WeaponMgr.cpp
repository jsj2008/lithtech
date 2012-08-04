// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponMgr.cpp
//
// PURPOSE : WeaponMgr implementation - Controls attributes of all weapons
//
// CREATED : 12/02/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponMgr.h"
#include "CommonUtilities.h"
#include "WeaponFXTypes.h"

#include "CRC32.h"

#ifdef _CLIENTBUILD
// **************** Client only includes
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;
#else
// **************** Server only includes
#include "GameServerShell.h"
#include "PlayerObj.h"
#endif

#define WMGR_WEAPONORDER_TAG			"WeaponOrder"
#define WMGR_WEAPONORDER_WEAPON			"Weapon"
#define WMGR_WEAPONORDER_FIRSTPLAYER	"FirstPlayerWeapon"
#define WMGR_WEAPONORDER_LASTPLAYER		"LastPlayerWeapon"

#define WMGR_WEAPON_TAG					"Weapon"

#define WMGR_WEAPON_NAME				"Name"
#define WMGR_WEAPON_NAMEID				"NameId"
#define WMGR_WEAPON_DESCRIPTIONID		"DescriptionId"
#define WMGR_WEAPON_GADGET				"Gadget"
#define WMGR_WEAPON_ANITYPE				"AniType"
#define WMGR_WEAPON_ICON				"Icon"
#define WMGR_WEAPON_SM_ICON				"SmallIcon"
#define WMGR_WEAPON_PHOTO				"Photo"
#define WMGR_WEAPON_POS					"Pos"
#define WMGR_WEAPON_POS2				"Pos2"
#define WMGR_WEAPON_MUZZLEPOS			"MuzzlePos"
#define WMGR_WEAPON_MINPERTURB			"MinPerturb"
#define WMGR_WEAPON_MAXPERTURB			"MaxPerturb"
#define WMGR_WEAPON_RANGE				"Range"
#define WMGR_WEAPON_RECOIL				"Recoil"
#define WMGR_WEAPON_VECTORSPERROUND		"VectorsPerRound"
#define WMGR_WEAPON_PVMODEL				"PVModel"
#define WMGR_WEAPON_PVSKIN				"PVSkin"
#define WMGR_WEAPON_PVMUZZLEFXNAME		"PVMuzzleFXName"
#define WMGR_WEAPON_HHMODEL				"HHModel"
#define WMGR_WEAPON_HHSKIN				"HHSkin"
#define WMGR_WEAPON_HHSCALE				"HHModelScale"
#define WMGR_WEAPON_HHBREACH			"HHBreachOffset"
#define WMGR_WEAPON_HHMUZZLEFXNAME		"HHMuzzleFXName"
#define WMGR_WEAPON_INTMODEL			"InterfaceModel"
#define WMGR_WEAPON_INTSKIN				"InterfaceSkin"
#define WMGR_WEAPON_INTSCALE			"InterfaceModelScale"
#define WMGR_WEAPON_INTPOS				"InterfaceOffset"
#define WMGR_WEAPON_SILENCEDFIRESND		"SilencedFireSnd"
#define WMGR_WEAPON_ALTFIRESND			"AltFireSnd"
#define WMGR_WEAPON_FIRESND				"FireSnd"
#define WMGR_WEAPON_DRYFIRESND			"DryFireSnd"
#define WMGR_WEAPON_RELOADSND			"ReloadSnd"
#define WMGR_WEAPON_SELECTSND			"SelectSnd"
#define WMGR_WEAPON_DESELECTSND			"DeselectSnd"
#define WMGR_WEAPON_FIRESNDRADIUS		"FireSndRadius"
#define WMGR_WEAPON_AIFIRESNDRADIUS		"AIFireSndRadius"
#define WMGR_WEAPON_ENVMAP				"EnvironmentMap"
#define WMGR_WEAPON_INFINITEAMMO		"InfiniteAmmo"
#define WMGR_WEAPON_LOOKSDANGEROUS		"LooksDangerous"
#define WMGR_WEAPON_CANBEDEFAULT		"CanBeDefault"
#define WMGR_WEAPON_CANBEMAPPED			"CanBeMapped"
#define WMGR_WEAPON_HIDEWHENEMPTY		"HideWhenEmpty"
#define WMGR_WEAPON_SHOTSPERCLIP		"ShotsPerClip"
#define WMGR_WEAPON_AMMOMULT			"AmmoMultiplier"
#define WMGR_WEAPON_AMMONAME			"AmmoName"
#define WMGR_WEAPON_MODNAME				"ModName"
#define WMGR_WEAPON_PVFXNAME			"PVFXName"
#define WMGR_WEAPON_AIMINBURSTSHOTS		"AIMinBurstShots"
#define WMGR_WEAPON_AIMAXBURSTSHOTS		"AIMaxBurstShots"
#define WMGR_WEAPON_AIMINBURSTINTERVAL	"AIMinBurstInterval"
#define WMGR_WEAPON_AIMAXBURSTINTERVAL	"AIMaxBurstInterval"
#define WMGR_WEAPON_FIRERECOILPITCH		"FireRecoilPitch"
#define WMGR_WEAPON_FIRERECOILDECAY		"FireRecoilDecay"
#define WMGR_WEAPON_FIREDELAY			"FireDelay"

#define WMGR_AMMO_TAG				"Ammo"

#define WMGR_AMMO_NAME				"Name"
#define WMGR_AMMO_NAMEID			"NameId"
#define WMGR_AMMO_DESCID			"DescId"
#define WMGR_AMMO_TYPE				"Type"
#define WMGR_AMMO_PRIORITY			"Priority"
#define WMGR_AMMO_ICON				"Icon"
#define WMGR_AMMO_SM_ICON			"SmallIcon"
#define WMGR_AMMO_MAXAMOUNT			"MaxAmount"
#define WMGR_AMMO_SPAWNEDAMOUNT		"SpawnedAmount"
#define WMGR_AMMO_SELECTIONAMOUNT	"SelectionAmount"
#define WMGR_AMMO_INSTDAMAGE		"InstDamage"
#define WMGR_AMMO_INSTDAMAGETYPE	"InstDamageType"
#define WMGR_AMMO_AREADAMAGE		"AreaDamage"
#define WMGR_AMMO_AREADAMAGETYPE	"AreaDamageType"
#define WMGR_AMMO_AREADAMAGERADIUS	"AreaDamageRadius"
#define WMGR_AMMO_PROGDAMAGE		"ProgDamage"
#define WMGR_AMMO_PROGDAMAGETYPE	"ProgDamageType"
#define WMGR_AMMO_PROGDAMAGEDUR		"ProgDamageDuration"
#define WMGR_AMMO_PROGDAMAGERADIUS	"ProgDamageRadius"
#define WMGR_AMMO_PROGDAMAGELIFE	"ProgDamageLifetime"
#define WMGR_AMMO_FIRERECOILMULT	"FireRecoilMult"
#define WMGR_AMMO_IMPACTFX			"ImpactFXName"
#define WMGR_AMMO_UWIMPACTFX		"UWImpactFXName"
#define WMGR_AMMO_FIREFX			"FireFXName"
#define WMGR_AMMO_TRACERFX			"TracerFXName"
#define WMGR_AMMO_PROJECTILEFX		"ProjectileFXName"
#define WMGR_AMMO_ANIOVERRIDE		"WeaponAniOverride"

#define WMGR_MOD_TAG				"Mod"

#define WMGR_MOD_DESCRIPTIONID		"DescriptionId"
#define WMGR_MOD_NAMEID				"NameId"
#define WMGR_MOD_NAME				"Name"
#define WMGR_MOD_SOCKET				"Socket"
#define WMGR_MOD_ICON				"Icon"
#define WMGR_MOD_SM_ICON			"SmallIcon"
#define WMGR_MOD_TYPE				"Type"
#define WMGR_MOD_ATTACHMODEL		"AttachModel"
#define WMGR_MOD_ATTACHSKIN			"AttachSkin"
#define WMGR_MOD_ATTACHSCALE		"AttachScale"
#define WMGR_MOD_ZOOMLEVEL			"ZoomLevel"
#define WMGR_MOD_ZOOMINSND			"ZoomInSound"
#define WMGR_MOD_ZOOMOUTSND			"ZoomOutSound"
#define WMGR_MOD_INTEGRATED			"Integrated"
#define WMGR_MOD_PRIORITY			"Priority"
#define WMGR_MOD_NIGHTVISION		"NightVision"
#define WMGR_MOD_INTMODEL			"InterfaceModel"
#define WMGR_MOD_INTSKIN			"InterfaceSkin"
#define WMGR_MOD_INTSCALE			"InterfaceModelScale"
#define WMGR_MOD_INTPOS				"InterfaceOffset"
#define WMGR_MOD_PICKUPSND			"PickUpSound"
#define WMGR_MOD_RESPAWNSND			"RespawnSound"
#define WMGR_MOD_POWERUPMODEL		"PowerupModel"
#define WMGR_MOD_POWERUPSKIN		"PowerupSkin"
#define WMGR_MOD_POWERUPSCALE		"PowerupModelScale"
#define WMGR_MOD_TINT_COLOR			"TintColor"
#define WMGR_MOD_TINT_TIME			"TintTime"

#define WMGR_GEAR_TAG				"Gear"

#define WMGR_GEAR_NAME				"Name"
#define WMGR_GEAR_NAMEID			"NameId"
#define WMGR_GEAR_DESCRIPTIONID		"DescriptionId"
#define WMGR_GEAR_MODEL				"Model"
#define WMGR_GEAR_SKIN				"Skin"
#define WMGR_GEAR_ICON				"Icon"
#define WMGR_GEAR_SM_ICON			"SmallIcon"
#define WMGR_GEAR_PROTECTTYPE		"ProtectionType"
#define WMGR_GEAR_PROTECTION		"Protection"
#define WMGR_GEAR_ARMOR				"Armor"
#define WMGR_GEAR_STEALTH			"Stealth"
#define WMGR_GEAR_SELECTABLE		"Selectable"
#define WMGR_GEAR_EXCLUSIVE			"Exclusive"
#define WMGR_GEAR_TINT_COLOR		"TintColor"
#define WMGR_GEAR_TINT_TIME			"TintTime"
#define WMGR_GEAR_INTMODEL			"InterfaceModel"
#define WMGR_GEAR_INTSKIN			"InterfaceSkin"
#define WMGR_GEAR_PICKUPSND			"PickUpSound"
#define WMGR_GEAR_RESPAWNSND		"RespawnSound"
#define WMGR_GEAR_INTSCALE			"InterfaceModelScale"
#define WMGR_GEAR_INTPOS			"InterfaceOffset"

#define WMGR_WEAPONANI_TAG			"WeaponAnis"

#define WMGR_WEAPONANI_NAME			"Name"
#define WMGR_WEAPONANI_SELECT		"Select"
#define WMGR_WEAPONANI_DESELECT		"Deselect"
#define WMGR_WEAPONANI_RELOAD		"Reload"
#define WMGR_WEAPONANI_IDLE			"Idle"
#define WMGR_WEAPONANI_FIRE			"Fire"

static char s_aTagName[30];
static char s_aAttName[100];
static char s_FileBuffer[MAX_CS_FILENAME_LEN];

CWeaponMgr* g_pWeaponMgr = LTNULL;

#ifndef _CLIENTBUILD

// Plugin statics

LTBOOL CWeaponMgrPlugin::sm_bInitted = LTFALSE;
CWeaponMgr CWeaponMgrPlugin::sm_ButeMgr;

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::CWeaponMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CWeaponMgr::CWeaponMgr()
{
    m_WeaponList.Init(LTTRUE);
    m_AmmoList.Init(LTTRUE);
    m_ModList.Init(LTTRUE);
    m_GearList.Init(LTTRUE);

    m_pWeaponOrder = LTNULL;
	m_nFirstPlayerWeapon = 0;
	m_nLastPlayerWeapon = 0;

	m_nFileCRC = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::~CWeaponMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CWeaponMgr::~CWeaponMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pWeaponMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;
	
	m_nFileCRC = CRC32::CalcRezFileCRC(szAttributeFile);


	// Set up global pointer...

	g_pWeaponMgr = this;


	// Read in the properties for each mod type...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", WMGR_MOD_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		MOD* pMod = debug_new(MOD);

		if (pMod && pMod->Init(m_buteMgr, s_aTagName))
		{
			pMod->nId = nNum;
			m_ModList.AddTail(pMod);
		}
		else
		{
			debug_delete(pMod);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", WMGR_MOD_TAG, nNum);
	}


	// Read in the properties for each gear type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", WMGR_GEAR_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		GEAR* pGear = debug_new(GEAR);

		if (pGear && pGear->Init(m_buteMgr, s_aTagName))
		{
			pGear->nId = nNum;
			m_GearList.AddTail(pGear);
		}
		else
		{
			debug_delete(pGear);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", WMGR_GEAR_TAG, nNum);
	}


	// Read in the properties for each weapon anis type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", WMGR_WEAPONANI_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		WEAPONANIS* pWeaponAni = debug_new(WEAPONANIS);

		if (pWeaponAni && pWeaponAni->Init(m_buteMgr, s_aTagName))
		{
			pWeaponAni->nId = nNum;
			m_WeaponAnisList.AddTail(pWeaponAni);
		}
		else
		{
			debug_delete(pWeaponAni);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", WMGR_WEAPONANI_TAG, nNum);
	}


	// Read in the properties for each ammo type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", WMGR_AMMO_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		AMMO* pAmmo = debug_new(AMMO);

		if (pAmmo && pAmmo->Init(m_buteMgr, s_aTagName))
		{
			pAmmo->nId = nNum;
			m_AmmoList.AddTail(pAmmo);
		}
		else
		{
			debug_delete(pAmmo);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", WMGR_AMMO_TAG, nNum);
	}


	// Read in the properties for each weapon...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", WMGR_WEAPON_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		WEAPON* pWeapon = debug_new(WEAPON);

		if (pWeapon && pWeapon->Init(m_buteMgr, s_aTagName))
		{
			pWeapon->nId = nNum;
			m_WeaponList.AddTail(pWeapon);
		}
		else
		{
			debug_delete(pWeapon);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", WMGR_WEAPON_TAG, nNum);
	}


	// Read in the order of the weapons...

	if (m_pWeaponOrder)
	{
		debug_deletea(m_pWeaponOrder);
	}

	m_pWeaponOrder = debug_newa(int, m_WeaponList.GetLength());
    if (!m_pWeaponOrder) return LTFALSE;

	nNum = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPONORDER_WEAPON, nNum);

	CString strWeaponName;
    WEAPON* pWeapon = LTNULL;

	while (m_buteMgr.Exist(WMGR_WEAPONORDER_TAG, s_aAttName) && nNum < m_WeaponList.GetLength())
	{
		strWeaponName = m_buteMgr.GetString(WMGR_WEAPONORDER_TAG, s_aAttName);

		if (!strWeaponName.IsEmpty())
		{
			pWeapon = GetWeapon((char*)(LPCSTR)strWeaponName);
		}
		else
		{
            return LTFALSE;
		}

		if (pWeapon)
		{
			m_pWeaponOrder[nNum] = pWeapon->nId;
		}
		else
		{
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPONORDER_WEAPON, nNum);
	}

	m_nFirstPlayerWeapon = m_buteMgr.GetInt(WMGR_WEAPONORDER_TAG, WMGR_WEAPONORDER_FIRSTPLAYER);
	m_nLastPlayerWeapon  = m_buteMgr.GetInt(WMGR_WEAPONORDER_TAG, WMGR_WEAPONORDER_LASTPLAYER);

	// Free up the bute mgr's memory...

	m_buteMgr.Term();


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::Term()
{
    g_pWeaponMgr = LTNULL;

	m_WeaponList.Clear();
	m_AmmoList.Clear();
	m_ModList.Clear();
	m_GearList.Clear();
	m_WeaponAnisList.Clear();

	if (m_pWeaponOrder)
	{
		debug_deletea(m_pWeaponOrder);
        m_pWeaponOrder = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::WriteFile()
//
//	PURPOSE:	Write necessary data back out to bute file
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponMgr::WriteFile(ILTCSBase *pInterface)
{
	// Re-init our bute mgr...

	m_buteMgr.Init(GBM_DisplayError);
    Parse(pInterface, WEAPON_DEFAULT_FILE);


	// Save the necessary weapon data...

    WEAPON** pCur  = LTNULL;

	pCur = m_WeaponList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		WEAPON* pWeapon = *pCur;
		if (pWeapon)
		{
			pWeapon->Save(m_buteMgr);
		}

		pCur = m_WeaponList.GetItem(TLIT_NEXT);
	}


	// Save the file...

    LTBOOL bRet = m_buteMgr.Save();
	m_buteMgr.Term();


	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::Reload()
//
//	PURPOSE:	Reload data from the bute file
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::Reload(ILTCSBase *pInterface)
{
	Term();
    Init(pInterface);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::CalculateWeaponPath()
//
//	PURPOSE:	Utility function used to calculate a path based on the
//				perturb of the weapon
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::CalculateWeaponPath(WeaponPath & wp)
{
	WEAPON* pWeapon = GetWeapon(wp.nWeaponId);
	if (!pWeapon) return;

	// Make sure the path has been normalized...

	wp.vPath.Norm();

	int nMinPerturb = pWeapon->nMinPerturb;
	int nMaxPerturb = pWeapon->nMaxPerturb;

	if ((nMinPerturb <= nMaxPerturb) && nMaxPerturb > 0)
	{
		float fPerturbRange = float(nMaxPerturb - nMinPerturb);

		int nPerturb = nMinPerturb + (int) (fPerturbRange * wp.fPerturbR);
        LTFLOAT fRPerturb = ((LTFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;

		nPerturb = nMinPerturb + (int) (fPerturbRange * wp.fPerturbU);
        LTFLOAT fUPerturb = ((LTFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;

        // pInterface->CPrint("Weapon Path RPerturb : %.2f", wp.fPerturbR);
        // pInterface->CPrint("Calculated RPerturb : %.2f", fRPerturb);
        // pInterface->CPrint("Weapon Path UPerturb : %.2f", wp.fPerturbU);
        // pInterface->CPrint("Calculated UPerturb : %.2f", fUPerturb);

		wp.vPath += (wp.vR * fRPerturb);
		wp.vPath += (wp.vU * fUPerturb);
	}

	wp.vPath.Norm();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetWeapon
//
//	PURPOSE:	Get the specified weapon struct
//
// ----------------------------------------------------------------------- //

WEAPON* CWeaponMgr::GetWeapon(int nWeaponId)
{
    WEAPON** pCur  = LTNULL;

	pCur = m_WeaponList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nWeaponId)
		{
			return *pCur;
		}

		pCur = m_WeaponList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetWeapon
//
//	PURPOSE:	Get the specified weapon struct
//
// ----------------------------------------------------------------------- //

WEAPON* CWeaponMgr::GetWeapon(char* pWeaponName)
{
    if (!pWeaponName) return LTNULL;

    WEAPON** pCur  = LTNULL;

	pCur = m_WeaponList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pWeaponName) == 0))
		{
			return *pCur;
		}

		pCur = m_WeaponList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetAmmo
//
//	PURPOSE:	Get the specified ammo struct
//
// ----------------------------------------------------------------------- //

AMMO* CWeaponMgr::GetAmmo(int nAmmoId)
{
    AMMO** pCur  = LTNULL;

	pCur = m_AmmoList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nAmmoId)
		{
			return *pCur;
		}

		pCur = m_AmmoList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetAmmo
//
//	PURPOSE:	Get the specified ammo struct
//
// ----------------------------------------------------------------------- //

AMMO* CWeaponMgr::GetAmmo(char* pAmmoName)
{
    if (!pAmmoName) return LTNULL;

    AMMO** pCur  = LTNULL;

	pCur = m_AmmoList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pAmmoName) == 0))
		{
			return *pCur;
		}

		pCur = m_AmmoList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetMod
//
//	PURPOSE:	Get the specified mod struct
//
// ----------------------------------------------------------------------- //

MOD* CWeaponMgr::GetMod(int nModId)
{
    MOD** pCur  = LTNULL;

	pCur = m_ModList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nModId)
		{
			return *pCur;
		}

		pCur = m_ModList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetMod
//
//	PURPOSE:	Get the specified mod struct
//
// ----------------------------------------------------------------------- //

MOD* CWeaponMgr::GetMod(char* pModName)
{
    if (!pModName) return LTNULL;

    MOD** pCur  = LTNULL;

	pCur = m_ModList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pModName) == 0))
		{
			return *pCur;
		}

		pCur = m_ModList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetGear
//
//	PURPOSE:	Get the specified gear struct
//
// ----------------------------------------------------------------------- //

GEAR* CWeaponMgr::GetGear(int nGearId)
{
    GEAR** pCur  = LTNULL;

	pCur = m_GearList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nGearId)
		{
			return *pCur;
		}

		pCur = m_GearList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetGear
//
//	PURPOSE:	Get the specified gear struct
//
// ----------------------------------------------------------------------- //

GEAR* CWeaponMgr::GetGear(char* pGearName)
{
    if (!pGearName) return LTNULL;

    GEAR** pCur  = LTNULL;

	pCur = m_GearList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pGearName) == 0))
		{
			return *pCur;
		}

		pCur = m_GearList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetWeaponAnis
//
//	PURPOSE:	Get the specified weapon anis struct
//
// ----------------------------------------------------------------------- //

WEAPONANIS* CWeaponMgr::GetWeaponAnis(char* pAnisName)
{
    if (!pAnisName) return LTNULL;

    WEAPONANIS** pCur  = LTNULL;

	pCur = m_WeaponAnisList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pAnisName) == 0))
		{
			return *pCur;
		}

		pCur = m_WeaponAnisList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

/////////////////////////////////////////////////////////////////////////////
//
//	W E A P O N  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON::WEAPON
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

WEAPON::WEAPON()
{
	nId				= WMGR_INVALID_ID;

	nNameId			= 0;
	nDescriptionId	= 0;
    bGadget         = LTFALSE;
	nAniType		= 0;

	szName[0]		= '\0';
	szIcon[0]		= '\0';
	szSmallIcon[0]	= '\0';
	szPhoto[0]		= '\0';

	vPos.Init();
	vMuzzlePos.Init();

	szPVModel[0]	= '\0';
	szPVSkin[0]		= '\0';
	szHHModel[0]	= '\0';
	szHHSkin[0]		= '\0';

	szInterfaceModel[0]		= '\0';
	szInterfaceSkin[0]		= '\0';
    fInterfaceScale			=	1.0f;
    vInterfaceOffset.Init();

	vHHScale.Init();
	fHHBreachOffset	= 0.0f;

	szSilencedFireSound[0]	= '\0';
	szAltFireSound[0]		= '\0';
	szFireSound[0]		= '\0';
	szDryFireSound[0]	= '\0';

    int i;
    for (i=0; i < WMGR_MAX_RELOAD_SNDS; i++)
	{
		szReloadSounds[i][0] = '\0';
	}

	szSelectSound[0]	= '\0';
	szDeselectSound[0]	= '\0';

	nFireSoundRadius	= 0;
	nAIFireSoundRadius	= 0;

    bEnvironmentMap     = LTFALSE;
    bInfiniteAmmo       = LTFALSE;
    bLooksDangerous     = LTFALSE;
	bCanBeDefault		= LTFALSE;
	bCanBeMapped		= LTFALSE;
	bHideWhenEmpty		= LTFALSE;

	nShotsPerClip		= 0;

	nNumAmmoTypes		= 0;
	for (i=0; i < WMGR_MAX_AMMO_TYPES; i++)
	{
		aAmmoTypes[i] = WMGR_INVALID_ID;
	}

	nDefaultAmmoType	= 0;
	nAmmoMultiplier		= 1;

	nNumModTypes		= 0;
	for (i=0; i < WMGR_MAX_MOD_TYPES; i++)
	{
		aModTypes[i] = WMGR_INVALID_ID;
	}

	nNumPVFXTypes		= 0;
	for (i=0; i < WMGR_MAX_PVFX_TYPES; i++)
	{
		aPVFXTypes[i] = WMGR_INVALID_ID;
	}

	vRecoil.Init();

	nMinPerturb			= 0;
	nMaxPerturb			= 0;

	nRange				= 0;
	nVectorsPerRound	= 0;

	nAIMinBurstShots	= 0;
	nAIMaxBurstShots	= 0;
	fAIMinBurstInterval	= 1.0f;
	fAIMaxBurstInterval	= 1.0f;

	fFireRecoilPitch	= 0.0f;
	fFireRecoilDecay	= 0.0f;

    pPVMuzzleFX         = LTNULL;
    pHHMuzzleFX         = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON::Init
//
//	PURPOSE:	Build the weapon struct
//
// ----------------------------------------------------------------------- //

LTBOOL WEAPON::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nNameId					= buteMgr.GetInt(aTagName, WMGR_WEAPON_NAMEID);
	nDescriptionId			= buteMgr.GetInt(aTagName, WMGR_WEAPON_DESCRIPTIONID);
    bGadget                 = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_GADGET);
	nAniType				= buteMgr.GetInt(aTagName, WMGR_WEAPON_ANITYPE);

	vPos					= buteMgr.GetVector(aTagName, WMGR_WEAPON_POS);
	vMuzzlePos				= buteMgr.GetVector(aTagName, WMGR_WEAPON_MUZZLEPOS);
	vHHScale				= buteMgr.GetVector(aTagName, WMGR_WEAPON_HHSCALE);
	vRecoil					= buteMgr.GetVector(aTagName, WMGR_WEAPON_RECOIL);
	fInterfaceScale			= (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_WEAPON_INTSCALE);
	vInterfaceOffset		= buteMgr.GetVector(aTagName, WMGR_WEAPON_INTPOS);

	CString str = buteMgr.GetString(aTagName, WMGR_WEAPON_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_ICON);
	if (!str.IsEmpty())
	{
		strncpy(szIcon, (char*)(LPCSTR)str, ARRAY_LEN(szIcon));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_SM_ICON);
	if (!str.IsEmpty())
	{
		strncpy(szSmallIcon, (char*)(LPCSTR)str, ARRAY_LEN(szSmallIcon));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_PHOTO);
	if (!str.IsEmpty())
	{
		strncpy(szPhoto, (char*)(LPCSTR)str, ARRAY_LEN(szPhoto));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_PVMODEL);
	if (!str.IsEmpty())
	{
		strncpy(szPVModel, (char*)(LPCSTR)str, ARRAY_LEN(szPVModel));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_PVSKIN);
	if (!str.IsEmpty())
	{
		strncpy(szPVSkin, (char*)(LPCSTR)str, ARRAY_LEN(szPVSkin));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_HHMODEL);
	if (!str.IsEmpty())
	{
		strncpy(szHHModel, (char*)(LPCSTR)str, ARRAY_LEN(szHHModel));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_HHSKIN);
	if (!str.IsEmpty())
	{
		strncpy(szHHSkin, (char*)(LPCSTR)str, ARRAY_LEN(szHHSkin));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_INTMODEL);
	if (!str.IsEmpty())
	{
		strncpy(szInterfaceModel, (char*)(LPCSTR)str, ARRAY_LEN(szInterfaceModel));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_INTSKIN);
	if (!str.IsEmpty())
	{
		strncpy(szInterfaceSkin, (char*)(LPCSTR)str, ARRAY_LEN(szInterfaceSkin));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_ALTFIRESND);
	if (!str.IsEmpty())
	{
		strncpy(szAltFireSound, (char*)(LPCSTR)str, ARRAY_LEN(szAltFireSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_SILENCEDFIRESND);
	if (!str.IsEmpty())
	{
		strncpy(szSilencedFireSound, (char*)(LPCSTR)str, ARRAY_LEN(szSilencedFireSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_FIRESND);
	if (!str.IsEmpty())
	{
		strncpy(szFireSound, (char*)(LPCSTR)str, ARRAY_LEN(szFireSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_DRYFIRESND);
	if (!str.IsEmpty())
	{
		strncpy(szDryFireSound, (char*)(LPCSTR)str, ARRAY_LEN(szDryFireSound));
	}

	for (int i=0; i < WMGR_MAX_RELOAD_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", WMGR_WEAPON_RELOADSND, i+1);

		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			strncpy(szReloadSounds[i], (char*)(LPCSTR)str, WMGR_MAX_FILE_PATH);
		}
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_SELECTSND);
	if (!str.IsEmpty())
	{
		strncpy(szSelectSound, (char*)(LPCSTR)str, ARRAY_LEN(szSelectSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_DESELECTSND);
	if (!str.IsEmpty())
	{
		strncpy(szDeselectSound, (char*)(LPCSTR)str, ARRAY_LEN(szDeselectSound));
	}

    fHHBreachOffset     = (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_WEAPON_HHBREACH);

	nFireSoundRadius	= buteMgr.GetInt(aTagName, WMGR_WEAPON_FIRESNDRADIUS);
    nAIFireSoundRadius  = buteMgr.GetInt(aTagName, WMGR_WEAPON_AIFIRESNDRADIUS);
    bEnvironmentMap     = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_ENVMAP);
    bInfiniteAmmo       = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_INFINITEAMMO);
    bLooksDangerous     = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_LOOKSDANGEROUS);
    bCanBeDefault		= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_CANBEDEFAULT);
    bCanBeMapped		= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_CANBEMAPPED);
    bHideWhenEmpty      = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_HIDEWHENEMPTY);

    nShotsPerClip       = buteMgr.GetInt(aTagName, WMGR_WEAPON_SHOTSPERCLIP);

	nNumAmmoTypes		= 0;


	// Build our ammo types id array...

	nNumAmmoTypes = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPON_AMMONAME, nNumAmmoTypes);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumAmmoTypes < WMGR_MAX_AMMO_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			AMMO* pAmmo = g_pWeaponMgr->GetAmmo((char*)(LPCSTR)str);
			if (pAmmo)
			{
				aAmmoTypes[nNumAmmoTypes] = pAmmo->nId;
			}
		}

		nNumAmmoTypes++;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPON_AMMONAME, nNumAmmoTypes);
	}

	nDefaultAmmoType = aAmmoTypes[0];
    nAmmoMultiplier  = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_AMMOMULT);


	// Build our mod types id array...

	nNumModTypes = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPON_MODNAME, nNumModTypes);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumModTypes < WMGR_MAX_MOD_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			MOD* pMOD = g_pWeaponMgr->GetMod((char*)(LPCSTR)str);
			if (pMOD)
			{
				aModTypes[nNumModTypes] = pMOD->nId;
			}
		}

		nNumModTypes++;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPON_MODNAME, nNumModTypes);
	}


	// Build our pv fx id array...

	nNumPVFXTypes = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPON_PVFXNAME, nNumPVFXTypes);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumPVFXTypes < WMGR_MAX_PVFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			PVFX* pPVFX = g_pFXButeMgr->GetPVFX((char*)(LPCSTR)str);
			if (pPVFX)
			{
				aPVFXTypes[nNumPVFXTypes] = pPVFX->nId;
			}
		}

		nNumPVFXTypes++;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPON_PVFXNAME, nNumPVFXTypes);
	}


	nMinPerturb			= buteMgr.GetInt(s_aTagName, WMGR_WEAPON_MINPERTURB);
	nMaxPerturb			= buteMgr.GetInt(s_aTagName, WMGR_WEAPON_MAXPERTURB);
	nRange				= buteMgr.GetInt(s_aTagName, WMGR_WEAPON_RANGE);
	nVectorsPerRound	= buteMgr.GetInt(s_aTagName, WMGR_WEAPON_VECTORSPERROUND);

	nAIMinBurstShots	= buteMgr.GetInt(s_aTagName, WMGR_WEAPON_AIMINBURSTSHOTS);
	nAIMaxBurstShots	= buteMgr.GetInt(s_aTagName, WMGR_WEAPON_AIMAXBURSTSHOTS);
    fAIMinBurstInterval = (LTFLOAT)buteMgr.GetDouble(s_aTagName, WMGR_WEAPON_AIMINBURSTINTERVAL);
    fAIMaxBurstInterval = (LTFLOAT)buteMgr.GetDouble(s_aTagName, WMGR_WEAPON_AIMAXBURSTINTERVAL);

    fFireRecoilPitch    = (LTFLOAT)buteMgr.GetDouble(s_aTagName, WMGR_WEAPON_FIRERECOILPITCH);
    fFireRecoilDecay    = (LTFLOAT)buteMgr.GetDouble(s_aTagName, WMGR_WEAPON_FIRERECOILDECAY);

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_HHMUZZLEFXNAME);
	if (!str.IsEmpty())
	{
		pHHMuzzleFX = g_pFXButeMgr->GetMuzzleFX((char*)(LPCSTR)str);
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPON_PVMUZZLEFXNAME);
	if (!str.IsEmpty())
	{
		pPVMuzzleFX = g_pFXButeMgr->GetMuzzleFX((char*)(LPCSTR)str);
	}

	int nDelay = buteMgr.GetInt(s_aTagName, WMGR_WEAPON_FIREDELAY);
	m_nFireDelay = (nDelay > 0) ? nDelay : 0;
	
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON::Cache
//
//	PURPOSE:	Cache all the resources associated with the weapon
//
// ----------------------------------------------------------------------- //

void WEAPON::Cache(CWeaponMgr* pWeaponMgr)
{
#ifndef _CLIENTBUILD

	if (!pWeaponMgr) return;

	if (szPVModel[0])
	{
        g_pLTServer->CacheFile(FT_MODEL, szPVModel);
	}

	if (szPVSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szPVSkin);
	}

	if (szHHModel[0])
	{
        g_pLTServer->CacheFile(FT_MODEL, szHHModel);
	}

	if (szHHSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szHHSkin);
	}

	// Cache sounds...

	if (szFireSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szFireSound);
	}

	if (szDryFireSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szDryFireSound);
	}

    int i;
    for (i=0; i < WMGR_MAX_RELOAD_SNDS; i++)
	{
		if (szReloadSounds[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szReloadSounds[i]);
		}
	}

	if (szSelectSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szSelectSound);
	}

	if (szDeselectSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szDeselectSound);
	}

	// Load all the ammo related resources...

	for (i=0; i < nNumAmmoTypes; i++)
	{
		AMMO* pAmmo = pWeaponMgr->GetAmmo(aAmmoTypes[i]);
		if (pAmmo)
		{
			pAmmo->Cache(pWeaponMgr);
		}
	}

	// Load all the mod related resources...

	for (i=0; i < nNumModTypes; i++)
	{
		MOD* pMod = pWeaponMgr->GetMod(aModTypes[i]);
		if (pMod)
		{
			pMod->Cache(pWeaponMgr);
		}
	}

	// Load all the pv fx related resources...

	for (i=0; i < nNumPVFXTypes; i++)
	{
		PVFX* pPVFX = g_pFXButeMgr->GetPVFX(aPVFXTypes[i]);
		if (pPVFX)
		{
			pPVFX->Cache(g_pFXButeMgr);
		}
	}

	// Load the player-view muzzle fx...

	if (pPVMuzzleFX)
	{
		pPVMuzzleFX->Cache(g_pFXButeMgr);
	}

	// Load the hand-held muzzle fx...

	if (pHHMuzzleFX)
	{
		pHHMuzzleFX->Cache(g_pFXButeMgr);
	}

#endif
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON::Save()
//
//	PURPOSE:	Save any necessary data to the bute file...
//
// ----------------------------------------------------------------------- //

void WEAPON::Save(CButeMgr & buteMgr)
{
	sprintf(s_aTagName, "%s%d", WMGR_WEAPON_TAG, nId);

	CAVector vAVec(vPos.x, vPos.y, vPos.z);
	buteMgr.SetVector(s_aTagName, WMGR_WEAPON_POS, vAVec);

	vAVec.Set(vMuzzlePos.x, vMuzzlePos.y, vMuzzlePos.z);
	buteMgr.SetVector(s_aTagName, WMGR_WEAPON_MUZZLEPOS, vAVec);

	buteMgr.SetDouble(s_aTagName, WMGR_WEAPON_HHBREACH, (double)fHHBreachOffset);
}


/////////////////////////////////////////////////////////////////////////////
//
//	W E A P O N  A N I S  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPONANIS::WEAPONANIS
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

WEAPONANIS::WEAPONANIS()
{
	nId			= WMGR_INVALID_ID;
	szName[0]	= '\0';

	szSelectAni[0]		= '\0';
	szDeselectAni[0]	= '\0';
	szReloadAni[0]		= '\0';

	nNumIdleAnis	= 0;
    int i;
    for (i=0; i < WMGR_MAX_WEAPONANI_IDLE; i++)
	{
		szIdleAnis[i][0] = '\0';
	}

	nNumFireAnis	= 0;
	for (i=0; i < WMGR_MAX_WEAPONANI_FIRE; i++)
	{
		szFireAnis[i][0] = '\0';
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPONANIS::Init
//
//	PURPOSE:	Build the ammo struct
//
// ----------------------------------------------------------------------- //

LTBOOL WEAPONANIS::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, WMGR_WEAPONANI_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPONANI_SELECT);
	if (!str.IsEmpty())
	{
		strncpy(szSelectAni, (char*)(LPCSTR)str, ARRAY_LEN(szSelectAni));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPONANI_DESELECT);
	if (!str.IsEmpty())
	{
		strncpy(szDeselectAni, (char*)(LPCSTR)str, ARRAY_LEN(szDeselectAni));
	}

	str = buteMgr.GetString(aTagName, WMGR_WEAPONANI_RELOAD);
	if (!str.IsEmpty())
	{
		strncpy(szReloadAni, (char*)(LPCSTR)str, ARRAY_LEN(szReloadAni));
	}

	nNumIdleAnis = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPONANI_IDLE, nNumIdleAnis);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumIdleAnis < WMGR_MAX_WEAPONANI_IDLE)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			strncpy(szIdleAnis[nNumIdleAnis], (char*)(LPCSTR)str, ARRAY_LEN(szIdleAnis[nNumIdleAnis]));
		}

		nNumIdleAnis++;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPONANI_IDLE, nNumIdleAnis);
	}


	nNumFireAnis = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPONANI_FIRE, nNumFireAnis);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumFireAnis < WMGR_MAX_WEAPONANI_FIRE)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			strncpy(szFireAnis[nNumFireAnis], (char*)(LPCSTR)str, ARRAY_LEN(szFireAnis[nNumIdleAnis]));
		}

		nNumFireAnis++;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPONANI_FIRE, nNumFireAnis);
	}


    return LTTRUE;
}








/////////////////////////////////////////////////////////////////////////////
//
//	A M M O  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO::AMMO
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AMMO::AMMO()
{
	nId			= WMGR_INVALID_ID;

	nNameId		= 0;
	nDescId		= 0;
	eType		= UNKNOWN_AMMO_TYPE;
	fPriority	= -1.0f;

	szIcon[0]	= '\0';
	szSmallIcon[0]	= '\0';
	szName[0]	= '\0';

	nMaxAmount			= 0;
	nSpawnedAmount		= 0;
	nSelectionAmount	= 0;

	nInstDamage		= 0;
	eInstDamageType	= DT_INVALID;

	nAreaDamage			= 0;
	nAreaDamageRadius	= 0;
	eAreaDamageType		= DT_INVALID;
	fFireRecoilMult		= 1.0f;

	fProgDamage			= 0.0f;
	fProgDamageDuration	= 0.0f;
	fProgDamageRadius   = 0.0f;
	fProgDamageLifetime = 0.0f;
	eProgDamageType		= DT_INVALID;

	pProjectileFX	= NULL;
	pImpactFX		= NULL;
	pUWImpactFX		= NULL;
	pFireFX			= NULL;
	pAniOverrides	= NULL;
	pTracerFX		= NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO::Init
//
//	PURPOSE:	Build the ammo struct
//
// ----------------------------------------------------------------------- //

LTBOOL AMMO::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nNameId				= buteMgr.GetInt(aTagName, WMGR_AMMO_NAMEID);
	nDescId				= buteMgr.GetInt(aTagName, WMGR_AMMO_DESCID);
	eType				= (AmmoType) buteMgr.GetInt(aTagName, WMGR_AMMO_TYPE);

    fPriority			= (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_AMMO_PRIORITY);

	nMaxAmount			= buteMgr.GetInt(aTagName, WMGR_AMMO_MAXAMOUNT);
	nSpawnedAmount		= buteMgr.GetInt(aTagName, WMGR_AMMO_SPAWNEDAMOUNT);
	nSelectionAmount	= buteMgr.GetInt(aTagName, WMGR_AMMO_SELECTIONAMOUNT);

	nInstDamage			= buteMgr.GetInt(aTagName, WMGR_AMMO_INSTDAMAGE);
	eInstDamageType		= (DamageType) buteMgr.GetInt(aTagName, WMGR_AMMO_INSTDAMAGETYPE);

	nAreaDamage			= buteMgr.GetInt(aTagName, WMGR_AMMO_AREADAMAGE);
	eAreaDamageType		= (DamageType) buteMgr.GetInt(aTagName, WMGR_AMMO_AREADAMAGETYPE);
	nAreaDamageRadius	= buteMgr.GetInt(aTagName, WMGR_AMMO_AREADAMAGERADIUS);
    fFireRecoilMult     = (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_AMMO_FIRERECOILMULT);

    fProgDamage         = (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_AMMO_PROGDAMAGE);
	eProgDamageType		= (DamageType) buteMgr.GetInt(aTagName, WMGR_AMMO_PROGDAMAGETYPE);
    fProgDamageDuration = (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_AMMO_PROGDAMAGEDUR);
    fProgDamageRadius   = (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_AMMO_PROGDAMAGERADIUS);
    fProgDamageLifetime = (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_AMMO_PROGDAMAGELIFE);

	CString str = buteMgr.GetString(aTagName, WMGR_AMMO_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, WMGR_AMMO_ICON);
	if (!str.IsEmpty())
	{
		strncpy(szIcon, (char*)(LPCSTR)str, ARRAY_LEN(szIcon));
	}

	str = buteMgr.GetString(aTagName, WMGR_AMMO_SM_ICON);
	if (!str.IsEmpty())
	{
		strncpy(szSmallIcon, (char*)(LPCSTR)str, ARRAY_LEN(szSmallIcon));
	}

	int nId = 0;
	if (buteMgr.Exist(aTagName, WMGR_AMMO_IMPACTFX))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_IMPACTFX);
		if (!str.IsEmpty())
		{
			pImpactFX = g_pFXButeMgr->GetImpactFX((char*)(LPCSTR)str);
		}
	}

	if (buteMgr.Exist(aTagName, WMGR_AMMO_UWIMPACTFX))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_UWIMPACTFX);
		if (!str.IsEmpty())
		{
			pUWImpactFX = g_pFXButeMgr->GetImpactFX((char*)(LPCSTR)str);
		}
	}

	if (buteMgr.Exist(aTagName, WMGR_AMMO_PROJECTILEFX))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_PROJECTILEFX);
		if (!str.IsEmpty())
		{
			pProjectileFX = g_pFXButeMgr->GetProjectileFX((char*)(LPCSTR)str);
		}
	}

	if (buteMgr.Exist(aTagName, WMGR_AMMO_FIREFX))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_FIREFX);
		if (!str.IsEmpty())
		{
			pFireFX = g_pFXButeMgr->GetFireFX((char*)(LPCSTR)str);
		}
	}

	if (buteMgr.Exist(aTagName, WMGR_AMMO_ANIOVERRIDE))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_ANIOVERRIDE);
		if (!str.IsEmpty())
		{
			pAniOverrides = g_pWeaponMgr->GetWeaponAnis((char*)(LPCSTR)str);
		}
	}

	if (buteMgr.Exist(aTagName, WMGR_AMMO_TRACERFX))
	{
		str = buteMgr.GetString(aTagName, WMGR_AMMO_TRACERFX);
		if (!str.IsEmpty())
		{
			pTracerFX = g_pFXButeMgr->GetTracerFX((char*)(LPCSTR)str);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO::Cache
//
//	PURPOSE:	Cache all the resources associated with the ammo
//
// ----------------------------------------------------------------------- //

void AMMO::Cache(CWeaponMgr* pWeaponMgr)
{
#ifndef _CLIENTBUILD

	if (!pWeaponMgr) return;

	if (pImpactFX)
	{
		pImpactFX->Cache(g_pFXButeMgr);
	}

	if (pUWImpactFX)
	{
		pUWImpactFX->Cache(g_pFXButeMgr);
	}

	if (pProjectileFX)
	{
		pProjectileFX->Cache(g_pFXButeMgr);
	}

	if (pFireFX)
	{
		pFireFX->Cache(g_pFXButeMgr);
	}

#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO::GetMaxAmount()
//
//	PURPOSE:	Calculate max amount of ammo of this type allowed for
//				the character
//
// ----------------------------------------------------------------------- //

int AMMO::GetMaxAmount(HOBJECT hCharacter)
{
	int nMaxAmmo = nMaxAmount;
#ifdef _CLIENTBUILD
	if (!g_pGameClientShell->IsMultiplayerGame())
	{
		CPlayerSummaryMgr *pPSummary = g_pGameClientShell->GetPlayerSummary();
        LTFLOAT fMult = pPSummary->m_PlayerRank.fAmmoMultiplier;
        nMaxAmmo =  (int) ( fMult * (LTFLOAT)nMaxAmmo );
	}
#else
	if (IsPlayer(hCharacter) && g_pGameServerShell->GetGameType() == SINGLE)
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hCharacter);
        LTFLOAT fMult = pPlayer->GetPlayerSummaryMgr()->m_PlayerRank.fAmmoMultiplier;
        nMaxAmmo =  (int) ( fMult * (LTFLOAT)nMaxAmmo );
	};
#endif
	return nMaxAmmo;
}


/////////////////////////////////////////////////////////////////////////////
//
//	M O D Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MOD::MOD
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

MOD::MOD()
{
	nId		= WMGR_INVALID_ID;

	eType	= UNKNOWN_MOD_TYPE;

	szSocket[0]				= '\0';
	szName[0]				= '\0';
	szIcon[0]				= '\0';
	szSmallIcon[0]			= '\0';
	szZoomInSound[0]		= '\0';
	szZoomOutSound[0]		= '\0';
	szAttachModel[0]		= '\0';
	szAttachSkin[0]			= '\0';
	szInterfaceModel[0]		= '\0';
	szInterfaceSkin[0]		= '\0';
	szPowerupModel[0]		= '\0';
	szPowerupSkin[0]		= '\0';
	szPickUpSound[0]		= '\0';
	szRespawnSound[0]		= '\0';

    fInterfaceScale			=	1.0f;
    fPowerupScale			=	1.0f;
    vInterfaceOffset.Init();
	vAttachScale.Init(1, 1, 1);

	nNameId				= 0;
	nDescriptionId		= 0;
	nZoomLevel			= 0;
	nPriority			= 0;

    bNightVision        = LTFALSE;

    bIntegrated         = LTFALSE;

	fScreenTintTime	= 0.0f;
 	vScreenTintColor.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MOD::Init
//
//	PURPOSE:	Build the mod struct
//
// ----------------------------------------------------------------------- //

LTBOOL MOD::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nNameId				= buteMgr.GetInt(aTagName, WMGR_MOD_NAMEID);
	nDescriptionId		= buteMgr.GetInt(aTagName, WMGR_MOD_DESCRIPTIONID);
	eType				= (ModType) buteMgr.GetInt(aTagName, WMGR_MOD_TYPE);
	nZoomLevel			= buteMgr.GetInt(aTagName, WMGR_MOD_ZOOMLEVEL);
	nPriority			= buteMgr.GetInt(aTagName, WMGR_MOD_PRIORITY);
    bNightVision        = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_MOD_NIGHTVISION);
    bIntegrated         = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_MOD_INTEGRATED);
	fInterfaceScale		= (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_MOD_INTSCALE);
	vInterfaceOffset	= buteMgr.GetVector(aTagName, WMGR_MOD_INTPOS);
	fScreenTintTime		= (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_MOD_TINT_TIME);
	vScreenTintColor	= buteMgr.GetVector(aTagName, WMGR_MOD_TINT_COLOR);
	fPowerupScale		= (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_MOD_POWERUPSCALE);

	CString str = buteMgr.GetString(aTagName, WMGR_MOD_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_SOCKET);
	if (!str.IsEmpty())
	{
		strncpy(szSocket, (char*)(LPCSTR)str, ARRAY_LEN(szSocket));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_ICON);
	if (!str.IsEmpty())
	{
		strncpy(szIcon, (char*)(LPCSTR)str, ARRAY_LEN(szIcon));

	}
	str = buteMgr.GetString(aTagName, WMGR_MOD_SM_ICON);
	if (!str.IsEmpty())
	{
		strncpy(szSmallIcon, (char*)(LPCSTR)str, ARRAY_LEN(szSmallIcon));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_ATTACHMODEL);
	if (!str.IsEmpty())
	{
		strncpy(szAttachModel, (char*)(LPCSTR)str, ARRAY_LEN(szAttachModel));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_ATTACHSKIN);
	if (!str.IsEmpty())
	{
		strncpy(szAttachSkin, (char*)(LPCSTR)str, ARRAY_LEN(szAttachSkin));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_ZOOMINSND);
	if (!str.IsEmpty())
	{
		strncpy(szZoomInSound, (char*)(LPCSTR)str, ARRAY_LEN(szZoomInSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_ZOOMOUTSND);
	if (!str.IsEmpty())
	{
		strncpy(szZoomOutSound, (char*)(LPCSTR)str, ARRAY_LEN(szZoomOutSound));
	}

	vAttachScale = buteMgr.GetVector(aTagName, WMGR_MOD_ATTACHSCALE);

	str = buteMgr.GetString(aTagName, WMGR_MOD_INTMODEL);
	if (!str.IsEmpty())
	{
		strncpy(szInterfaceModel, (char*)(LPCSTR)str, ARRAY_LEN(szInterfaceModel));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_INTSKIN);
	if (!str.IsEmpty())
	{
		strncpy(szInterfaceSkin, (char*)(LPCSTR)str, ARRAY_LEN(szInterfaceSkin));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_POWERUPMODEL);
	if (!str.IsEmpty())
	{
		strncpy(szPowerupModel, (char*)(LPCSTR)str, ARRAY_LEN(szPowerupModel));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_POWERUPSKIN);
	if (!str.IsEmpty())
	{
		strncpy(szPowerupSkin, (char*)(LPCSTR)str, ARRAY_LEN(szPowerupSkin));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_PICKUPSND);
	if (!str.IsEmpty())
	{
		strncpy(szPickUpSound, (char*)(LPCSTR)str, ARRAY_LEN(szPickUpSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_MOD_RESPAWNSND);
	if (!str.IsEmpty())
	{
		strncpy(szRespawnSound, (char*)(LPCSTR)str, ARRAY_LEN(szRespawnSound));
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MOD::Cache
//
//	PURPOSE:	Cache all the resources associated with the mod
//
// ----------------------------------------------------------------------- //

void MOD::Cache(CWeaponMgr* pWeaponMgr)
{
#ifndef _CLIENTBUILD
	if (!pWeaponMgr) return;

	if (szZoomInSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szZoomInSound);
	}

	if (szZoomOutSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szZoomOutSound);
	}

	if (szAttachModel[0])
	{
        g_pLTServer->CacheFile(FT_MODEL, szAttachModel);
	}

	if (szAttachSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szAttachSkin);
	}

	if (szPickUpSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szPickUpSound);
	}

	if (szRespawnSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szRespawnSound);
	}
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MOD::GetWeaponId()
//
//	PURPOSE:	What is the weapon that uses this mod
//
// ----------------------------------------------------------------------- //

int MOD::GetWeaponId()
{
    WEAPON* pWeapon = LTNULL;

	for (int nWId = 0; nWId < g_pWeaponMgr->GetNumWeapons(); nWId++)
	{
		pWeapon = g_pWeaponMgr->GetWeapon(nWId);

		if (pWeapon)
		{
			for (int i=0; i < pWeapon->nNumModTypes; i++)
			{
				if (pWeapon->aModTypes[i] == nId)
				{
					return pWeapon->nId;
				}
			}
		}
	}

	return WMGR_INVALID_ID;
}


/////////////////////////////////////////////////////////////////////////////
//
//	G E A R Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GEAR::GEAR
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GEAR::GEAR()
{
	nId		= WMGR_INVALID_ID;

	nNameId			= 0;
	nDescriptionId	= 0;

	szName[0]		= '\0';
	szIcon[0]		= '\0';
	szSmallIcon[0]	= '\0';
	szModel[0]		= '\0';
	szSkin[0]		= '\0';

	szInterfaceModel[0]		= '\0';
	szInterfaceSkin[0]		= '\0';
	szPickUpSound[0]		= '\0';
	szRespawnSound[0]		= '\0';

    fInterfaceScale			=	1.0f;
    vInterfaceOffset.Init();

	eProtectionType	= DT_INVALID;
	fProtection		= 0.0f;
	fArmor			= 0.0f;
	fStealth		= 0.0f;
	fScreenTintTime	= 0.0f;
    bSelectable     = LTFALSE;
    bExclusive      = LTFALSE;

	vScreenTintColor.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GEAR::Init
//
//	PURPOSE:	Build the gear struct
//
// ----------------------------------------------------------------------- //

LTBOOL GEAR::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nNameId			= buteMgr.GetInt(aTagName, WMGR_GEAR_NAMEID);
	nDescriptionId	= buteMgr.GetInt(aTagName, WMGR_GEAR_DESCRIPTIONID);

	eProtectionType	= (DamageType) buteMgr.GetInt(aTagName, WMGR_GEAR_PROTECTTYPE);
    bSelectable     = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_GEAR_SELECTABLE);
    bExclusive      = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_GEAR_EXCLUSIVE);

    fProtection     = (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_GEAR_PROTECTION);
    fArmor          = (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_GEAR_ARMOR);
    fStealth        = (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_GEAR_STEALTH);
    fScreenTintTime = (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_GEAR_TINT_TIME);

	fProtection		= (fProtection < 0.0f ? 0.0f : (fProtection > 1.0f ? 1.0f : fProtection));
	fStealth		= (fStealth < 0.0f ? 0.0f : (fStealth > 1.0f ? 1.0f : fStealth));

	vScreenTintColor = buteMgr.GetVector(aTagName, WMGR_GEAR_TINT_COLOR);

	CString str = buteMgr.GetString(aTagName, WMGR_GEAR_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, WMGR_GEAR_ICON);
	if (!str.IsEmpty())
	{
		strncpy(szIcon, (char*)(LPCSTR)str, ARRAY_LEN(szIcon));
	}

	str = buteMgr.GetString(aTagName, WMGR_GEAR_SM_ICON);
	if (!str.IsEmpty())
	{
		strncpy(szSmallIcon, (char*)(LPCSTR)str, ARRAY_LEN(szSmallIcon));
	}

	str = buteMgr.GetString(aTagName, WMGR_GEAR_MODEL);
	if (!str.IsEmpty())
	{
		strncpy(szModel, (char*)(LPCSTR)str, ARRAY_LEN(szModel));
	}

	str = buteMgr.GetString(aTagName, WMGR_GEAR_SKIN);
	if (!str.IsEmpty())
	{
		strncpy(szSkin, (char*)(LPCSTR)str, ARRAY_LEN(szSkin));
	}

	str = buteMgr.GetString(aTagName, WMGR_GEAR_INTMODEL);
	if (!str.IsEmpty())
	{
		strncpy(szInterfaceModel, (char*)(LPCSTR)str, ARRAY_LEN(szInterfaceModel));
	}

	str = buteMgr.GetString(aTagName, WMGR_GEAR_INTSKIN);
	if (!str.IsEmpty())
	{
		strncpy(szInterfaceSkin, (char*)(LPCSTR)str, ARRAY_LEN(szInterfaceSkin));
	}

	str = buteMgr.GetString(aTagName, WMGR_GEAR_PICKUPSND);
	if (!str.IsEmpty())
	{
		strncpy(szPickUpSound, (char*)(LPCSTR)str, ARRAY_LEN(szPickUpSound));
	}

	str = buteMgr.GetString(aTagName, WMGR_GEAR_RESPAWNSND);
	if (!str.IsEmpty())
	{
		strncpy(szRespawnSound, (char*)(LPCSTR)str, ARRAY_LEN(szRespawnSound));
	}


	fInterfaceScale		= (LTFLOAT) buteMgr.GetDouble(aTagName, WMGR_GEAR_INTSCALE);
	vInterfaceOffset	= buteMgr.GetVector(aTagName, WMGR_GEAR_INTPOS);


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GEAR::Cache
//
//	PURPOSE:	Cache all the resources associated with the gear
//
// ----------------------------------------------------------------------- //

void GEAR::Cache(CWeaponMgr* pWeaponMgr)
{
#ifndef _CLIENTBUILD
	if (!pWeaponMgr) return;

	if (szModel[0])
	{
        g_pLTServer->CacheFile(FT_MODEL, szModel);
	}

	if (szSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szSkin);
	}

	if (szPickUpSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szPickUpSound);
	}

	if (szRespawnSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szRespawnSound);
	}

#endif
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	C A C H E  Functions...(SERVER-SIDE ONLY)
//
//  These functions are used to cache the resources associated with weapons.
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::CacheAll()
//
//	PURPOSE:	Cache all the weapon related resources...
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::CacheAll()
{
#ifndef _CLIENTBUILD // No caching on the client...

	// Cache all the weapons...

    WEAPON** pCurWeapon  = LTNULL;
	pCurWeapon = m_WeaponList.GetItem(TLIT_FIRST);

	while (pCurWeapon)
	{
		if (*pCurWeapon)
		{
			(*pCurWeapon)->Cache(this);
		}

		pCurWeapon = m_WeaponList.GetItem(TLIT_NEXT);
	}

#endif
}


#ifndef _CLIENTBUILD  // Server-side only

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::ReadWeaponProp
//
//	PURPOSE:	Read in the weapon properties
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponMgr::ReadWeaponProp(char* pPropName, uint8 & nWeaponId, uint8 & nAmmoId)
{
    if (!pPropName || !pPropName[0]) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric(pPropName, &genProp) == LT_OK)
	{
		// Get the weapon name...

		WEAPON* pWeapon = GetWeapon(strtok(genProp.m_String,","));
		if (pWeapon)
		{
			nWeaponId = pWeapon->nId;
		}

		AMMO* pAmmo = GetAmmo(strtok(NULL,""));
		if (pAmmo)
		{
			nAmmoId = pAmmo->nId;
		}
		else
		{
			// Use the default ammo type for the weapon...

			if (pWeapon)
			{
				nAmmoId = pWeapon->nDefaultAmmoType;
			}
		}

        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::ReadWeapon
//
//	PURPOSE:	Read in the weapon properties
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::ReadWeapon(char* szString, uint8 & nWeaponId, uint8 & nAmmoId)
{
	// Get the weapon name...

	WEAPON* pWeapon = GetWeapon(strtok(szString,","));
	if (pWeapon)
	{
		nWeaponId = pWeapon->nId;
	}

	AMMO* pAmmo = GetAmmo(strtok(NULL,""));
	if (pAmmo)
	{
		nAmmoId = pAmmo->nId;
	}
	else
	{
		// Use the default ammo type for the weapon...

		if (pWeapon)
		{
			nAmmoId = pWeapon->nDefaultAmmoType;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//
// CWeaponMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CWeaponMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!sm_bInitted)
	{
		// Make sure fx mgr is initialized...
		m_FXButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings,	pcStrings, cMaxStrings, cMaxStringLength);

/*
		// Create global fx bute mgr if necessary before the weapon
		// bute mgr is created...

		if (!g_pFXButeMgr)
		{
			sprintf(szFile, "%s\\%s", szRezPath, FXBMGR_DEFAULT_FILE);
            sm_FXButeMgr.SetInRezFile(LTFALSE);
			sm_FXButeMgr.Init(szFile);
		}
*/
		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, WEAPON_DEFAULT_FILE);
        sm_ButeMgr.SetInRezFile(LTFALSE);
        sm_ButeMgr.Init(g_pLTServer, szFile);
        sm_bInitted = LTTRUE;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

void CWeaponMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return;

	// Add an entry for each weapon/ammo combination...

    uint32 dwNumWeapons = sm_ButeMgr.GetNumWeapons();

    WEAPON* pWeapon = LTNULL;

    for (uint32 i=0; i < dwNumWeapons; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pWeapon = sm_ButeMgr.GetWeapon(i);
        uint32 dwWeaponNameLen = strlen(pWeapon->szName);

		if (pWeapon && pWeapon->szName[0] &&
			dwWeaponNameLen < cMaxStringLength &&
			(*pcStrings) + 1 < cMaxStrings)
		{
			// Account for the ';' character

			dwWeaponNameLen++;

			// Append the ammo types to the string if there is more
			// than one ammo type...

			if (pWeapon->nNumAmmoTypes > 1)
			{
				for (int j=0; j < pWeapon->nNumAmmoTypes; j++)
				{
					_ASSERT(cMaxStrings > (*pcStrings) + 1);

					AMMO* pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->aAmmoTypes[j]);
					if (pAmmo && (*pcStrings) + 1 < cMaxStrings)
					{
						if (dwWeaponNameLen + strlen(pAmmo->szName) < cMaxStringLength)
						{
							sprintf(aszStrings[(*pcStrings)++],"%s,%s", pWeapon->szName, pAmmo->szName);
						}
						else
						{
							// Just use the weapon name, the default ammo will be used...

							strcpy(aszStrings[(*pcStrings)++], pWeapon->szName);
						}
					}
				}
			}
			else
			{
				// Just use the weapon name, the default ammo will be used...

				strcpy(aszStrings[(*pcStrings)++], pWeapon->szName);
			}
		}
	}
}

#endif // _CLIENTBUILD