// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponMgr.cpp
//
// PURPOSE : WeaponMgr implementation - Controls attributes of all weapons
//
// CREATED : 12/02/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponMgr.h"
#include "CommonUtilities.h"
#include "WeaponFXTypes.h"
#include "winbase.h"
#include "FXButeMgr.h"
#include "CRC32.h"

#ifdef _CLIENTBUILD
// **************** Client only includes
#include "GameClientShell.h"
#include "ClientUtilities.h"
extern CGameClientShell* g_pGameClientShell;
#else
// **************** Server only includes
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "AIStimulusMgr.h"
#endif

#define WMGR_WEAPONORDER_TAG				"WeaponOrder"
#define WMGR_WEAPONORDER_WEAPON				"Weapon"
#define WMGR_WEAPONORDER_FIRSTPLAYER		"FirstPlayerWeapon"
#define WMGR_WEAPONORDER_LASTPLAYER			"LastPlayerWeapon"
#define WMGR_WEAPONORDER_CLASS				"EndOfClass"

#define WMGR_AUTOSWITCH_WEAPON				"AutoSwitchWeapon"

#define WMGR_ALL_CANSERVERRESTRICT			"CanServerRestrict"

#define WMGR_WEAPON_TAG						"Weapon"

#define WMGR_WEAPON_NAME					"Name"
#define WMGR_WEAPON_NAMEID					"NameId"
#define WMGR_WEAPON_DESCRIPTIONID			"DescriptionId"
#define WMGR_WEAPON_ISAMMONOPICKUPID		"IsAmmoNoPickupId"
#define WMGR_WEAPON_CLIENT_WEAPON_TYPE		"ClientWeaponType"
#define WMGR_WEAPON_ANITYPE					"AniType"
#define WMGR_WEAPON_ICON					"Icon"
#define WMGR_WEAPON_POS						"Pos"
#define WMGR_WEAPON_POS2					"Pos2"
#define WMGR_WEAPON_MUZZLEPOS				"MuzzlePos"
#define WMGR_WEAPON_BREACHOFFSET			"BreachOffset"
#define WMGR_WEAPON_MINPERTURB				"MinPerturb"
#define WMGR_WEAPON_MAXPERTURB				"MaxPerturb"
#define WMGR_WEAPON_RANGE					"Range"
#define WMGR_WEAPON_RECOIL					"Recoil"
#define WMGR_WEAPON_VECTORSPERROUND			"VectorsPerRound"
#define WMGR_WEAPON_PVMODEL					"PVModel"
#define WMGR_WEAPON_PVSKIN					"PVSkin"
#define WMGR_WEAPON_PVMUZZLEFXNAME			"PVMuzzleFXName"
#define WMGR_WEAPON_PVATTACHCLIENTFXNAME	"PVAttachFXName"
#define WMGR_WEAPON_PVRENDERSTYLE			"PVRenderStyle"
#define WMGR_WEAPON_HHMODEL					"HHModel"
#define WMGR_WEAPON_HHSKIN					"HHSkin"
#define WMGR_WEAPON_HHSCALE					"HHModelScale"
#define WMGR_WEAPON_HHMUZZLEFXNAME			"HHMuzzleFXName"
#define WMGR_WEAPON_HHRENDERSTYLE			"HHRenderStyle"
#define	WMGR_WEAPON_HIDDENPIECENAME			"HiddenPiece"
#define WMGR_WEAPON_SILENCEDFIRESND			"SilencedFireSnd"
#define WMGR_WEAPON_ALTFIRESND				"AltFireSnd"
#define WMGR_WEAPON_FIRESND					"FireSnd"
#define WMGR_WEAPON_DRYFIRESND				"DryFireSnd"
#define WMGR_WEAPON_RELOADSND				"ReloadSnd"
#define WMGR_WEAPON_MISCSND					"MiscSound"
#define WMGR_WEAPON_SELECTSND				"SelectSnd"
#define WMGR_WEAPON_DESELECTSND				"DeselectSnd"
#define WMGR_WEAPON_FIRESNDRADIUS			"FireSndRadius"
#define WMGR_WEAPON_AIFIRESNDRADIUS			"AIFireSndRadius"
#define	WMGR_WEAPON_WEAPONSNDRADIUS			"WeaponSndRadius"
#define WMGR_WEAPON_ENVMAP					"EnvironmentMap"
#define WMGR_WEAPON_INFINITEAMMO			"InfiniteAmmo"
#define WMGR_WEAPON_LOOKSDANGEROUS			"LooksDangerous"
#define WMGR_WEAPON_HIDEWHENEMPTY			"HideWhenEmpty"
#define WMGR_WEAPON_ISAMMO					"IsAmmo"
#define WMGR_WEAPON_CLASS					"Class"
#define WMGR_WEAPON_SHOTSPERCLIP			"ShotsPerClip"
#define WMGR_WEAPON_AMMONAME				"AmmoName"
#define WMGR_WEAPON_MODNAME					"ModName"
#define WMGR_WEAPON_PVFXNAME				"PVFXName"
#define WMGR_WEAPON_AIWEAPONTYPE			"AIWeaponType"
#define WMGR_WEAPON_AIMINBURSTSHOTS			"AIMinBurstShots"
#define WMGR_WEAPON_AIMAXBURSTSHOTS			"AIMaxBurstShots"
#define WMGR_WEAPON_AIMINBURSTINTERVAL		"AIMinBurstInterval"
#define WMGR_WEAPON_AIMAXBURSTINTERVAL		"AIMaxBurstInterval"
#define WMGR_WEAPON_AIANIMATESRELOADS		"AIAnimatesReloads"
#define WMGR_WEAPON_FIRERECOILPITCH			"FireRecoilPitch"
#define WMGR_WEAPON_FIRERECOILDECAY			"FireRecoilDecay"
#define WMGR_WEAPON_FIREDELAY				"FireDelay"
#define WMGR_WEAPON_HOLSTERATTACHMNET		"HolsterAttachment"
#define WMGR_WEAPON_PRIMITIVE_TYPE			"PrimitiveType"
#define WMGR_WEAPON_SUBROUTINE_NEEDED		"SubroutineNeeded"
#define WMGR_WEAPON_USEUWMUZZLEFX			"UseUWMuzzleFX"
#define WMGR_WEAPON_FIREANIMRATESCALE		"FireAnimRateScale"
#define WMGR_WEAPON_RELOADANIMRATESCALE		"ReloadAnimRateScale"
#define WMGR_WEAPON_POWERUPFX				"PowerupFX"
#define WMGR_WEAPON_RESPAWNWAITFX			"RespawnWaitFX"
#define WMGR_WEAPON_RESPAWNWAITSKIN			"RespawnWaitSkin"
#define WMGR_WEAPON_RESPAWNWAITRENDERSTYLE	"RespawnWaitRenderStyle"
#define WMGR_WEAPON_RESPAWNWAITVISIBLE		"RespawnWaitVisible"
#define WMGR_WEAPON_RESPAWNWAITTRANSLUCENT	"RespawnWaitTranslucent"



#define WMGR_AMMO_TAG				"Ammo"

#define WMGR_AMMO_NAME              "Name"
#define WMGR_AMMO_NAMEID            "NameId"
#define WMGR_AMMO_DESCID            "DescId"
#define WMGR_AMMO_TYPE              "Type"
#define WMGR_AMMO_PRIORITY          "Priority"
#define WMGR_AMMO_ICON              "Icon"
#define WMGR_AMMO_MAXAMOUNT         "MaxAmount"
#define WMGR_AMMO_SPAWNEDAMOUNT     "SpawnedAmount"
#define WMGR_AMMO_SELECTIONAMOUNT   "SelectionAmount"
#define WMGR_AMMO_INSTDAMAGE        "InstDamage"
#define WMGR_AMMO_INSTDAMAGETYPE    "InstDamageType"
#define WMGR_AMMO_AREADAMAGE        "AreaDamage"
#define WMGR_AMMO_AREADAMAGETYPE    "AreaDamageType"
#define WMGR_AMMO_AREADAMAGERADIUS  "AreaDamageRadius"
#define WMGR_AMMO_PROGDAMAGE        "ProgDamage"
#define WMGR_AMMO_PROGDAMAGETYPE    "ProgDamageType"
#define WMGR_AMMO_PROGDAMAGEDUR     "ProgDamageDuration"
#define WMGR_AMMO_PROGDAMAGERADIUS  "ProgDamageRadius"
#define WMGR_AMMO_PROGDAMAGELIFE    "ProgDamageLifetime"
#define WMGR_AMMO_FIRERECOILMULT    "FireRecoilMult"
#define WMGR_AMMO_IMPACTFX          "ImpactFXName"
#define WMGR_AMMO_UWIMPACTFX        "UWImpactFXName"
#define WMGR_AMMO_MOVEABLEIMPACTOVERRIDEFX	"MoveableImpactOverrideFXName"
#define WMGR_AMMO_RICOCHETFX        "RicochetFXName"
#define WMGR_AMMO_BLOCKEDFX         "BlockedFXName"
#define WMGR_AMMO_FIREFX            "FireFXName"
#define WMGR_AMMO_TRACERFX          "TracerFXName"
#define WMGR_AMMO_PROJECTILEFX      "ProjectileFXName"
#define WMGR_AMMO_ANIOVERRIDE       "WeaponAniOverride"
#define WMGR_AMMO_CANBEDEFLECTED    "CanBeDeflected"
#define WMGR_AMMO_DEFLECTSUFRACETYPE	"DeflectSurfaceType"
#define WMGR_AMMO_CANADJUSTINSTDAMAGE	"CanAdjustInstDamage"

#define WMGR_MOD_TAG					"Mod"

#define WMGR_MOD_DESCRIPTIONID			"DescriptionId"
#define WMGR_MOD_NAMEID					"NameId"
#define WMGR_MOD_NAME					"Name"
#define WMGR_MOD_SOCKET					"Socket"
#define WMGR_MOD_ICON					"Icon"
#define WMGR_MOD_TYPE					"Type"
#define WMGR_MOD_ATTACHMODEL			"AttachModel"
#define WMGR_MOD_ATTACHSKIN				"AttachSkin"
#define WMGR_MOD_ATTACHSCALE			"AttachScale"
#define WMGR_MOD_ATTACHRENDERSTYLE		"AttachRenderStyle"
#define WMGR_MOD_ZOOMLEVEL				"ZoomLevel"
#define WMGR_MOD_ZOOMINSND				"ZoomInSound"
#define WMGR_MOD_ZOOMOUTSND				"ZoomOutSound"
#define WMGR_MOD_INTEGRATED				"Integrated"
#define WMGR_MOD_PRIORITY				"Priority"
#define WMGR_MOD_PICKUPSND				"PickUpSound"
#define WMGR_MOD_RESPAWNSND				"RespawnSound"
#define WMGR_MOD_POWERUPMODEL			"PowerupModel"
#define WMGR_MOD_POWERUPSKIN			"PowerupSkin"
#define WMGR_MOD_POWERUPSCALE			"PowerupModelScale"
#define WMGR_MOD_POWERUPRENDERSTYLE		"PowerupRenderStyle"
#define WMGR_MOD_TINT_COLOR				"TintColor"
#define WMGR_MOD_TINT_TIME				"TintTime"
#define WMGR_MOD_SILENCESND_RADIUS		"AISilencedFireSndRadius"
#define WMGR_MOD_POWERUPFX				"PowerupFX"
#define WMGR_MOD_RESPAWNWAITFX			"RespawnWaitFX"
#define WMGR_MOD_RESPAWNWAITSKIN		"RespawnWaitSkin"
#define WMGR_MOD_RESPAWNWAITRENDERSTYLE	"RespawnWaitRenderStyle"
#define WMGR_MOD_RESPAWNWAITVISIBLE		"RespawnWaitVisible"
#define WMGR_MOD_RESPAWNWAITTRANSLUCENT	"RespawnWaitTranslucent"


#define WMGR_GEAR_TAG						"Gear"

#define WMGR_GEAR_NAME						"Name"
#define WMGR_GEAR_NAMEID					"NameId"
#define WMGR_GEAR_DESCRIPTIONID				"DescriptionId"
#define WMGR_GEAR_MODEL						"Model"
#define WMGR_GEAR_SKIN						"Skin"
#define WMGR_GEAR_RENDERSTYLE				"RenderStyle"
#define WMGR_GEAR_ICON						"Icon"
#define WMGR_GEAR_PROTECTTYPE				"ProtectionType"
#define WMGR_GEAR_PROTECTION				"Protection"
#define WMGR_GEAR_ARMOR						"Armor"
#define WMGR_GEAR_HEALTH					"Health"
#define WMGR_GEAR_STEALTH					"Stealth"
#define WMGR_GEAR_SELECTABLE				"Selectable"
#define WMGR_GEAR_EXCLUSIVE					"Exclusive"
#define WMGR_GEAR_TINT_COLOR				"TintColor"
#define WMGR_GEAR_TINT_TIME					"TintTime"
#define WMGR_GEAR_PICKUPSND					"PickUpSound"
#define WMGR_GEAR_RESPAWNSND				"RespawnSound"
#define WMGR_GEAR_POWERUPFX					"PowerupFX"
#define WMGR_GEAR_RESPAWNWAITFX				"RespawnWaitFX"
#define WMGR_GEAR_RESPAWNWAITSKIN			"RespawnWaitSkin"
#define WMGR_GEAR_RESPAWNWAITRENDERSTYLE	"RespawnWaitRenderStyle"
#define WMGR_GEAR_RESPAWNWAITVISIBLE		"RespawnWaitVisible"
#define WMGR_GEAR_RESPAWNWAITTRANSLUCENT	"RespawnWaitTranslucent"

#define WMGR_WEAPONANI_TAG			"WeaponAnis"

#define WMGR_WEAPONANI_NAME			"Name"
#define WMGR_WEAPONANI_SELECT		"Select"
#define WMGR_WEAPONANI_DESELECT		"Deselect"
#define WMGR_WEAPONANI_RELOAD		"Reload"
#define WMGR_WEAPONANI_IDLE			"Idle"
#define WMGR_WEAPONANI_FIRE			"Fire"

#define WMGR_MULTI_TAG			"Multiplayer"
#define WMGR_MULTI_DEFAULTS		"DefaultWeapons"


static char s_aTagName[30];
static char s_aAttName[100];
static char s_FileBuffer[MAX_CS_FILENAME_LEN];
static char s_AttributeFile[MAX_CS_FILENAME_LEN];


CWeaponMgr* g_pWeaponMgr = LTNULL;

#ifndef _CLIENTBUILD

// Plugin statics

LTBOOL CWeaponMgrPlugin::sm_bInitted = LTFALSE;

#endif // _CLIENTBUILD


// Helper for testing for the existence of a string and getting it if it exists.
void GetStringIfExist( CButeMgr &buteMgr, const char *szTagName, const char *szAttName, char **pOut, uint32 dwMaxLen )
{
	char szTemp[256];
	buteMgr.GetString( szTagName, szAttName, "", szTemp, ARRAY_LEN( szTemp ));
	if( buteMgr.Success( ))
	{
		debug_deletea( *pOut );

		// allocate memory for the string and copy the string into it
		// NOTE: the caller of this function is responsible
		// for this memory
		int nLen = strlen(szTemp) + 1;
		*pOut = debug_newa( char, nLen );
		strncpy( *pOut, szTemp, nLen );
		(*pOut)[nLen-1] = 0;
	}
	else
	{
		// Leave pOut if it exsits otherwise allocate a new string...
		
		if( !*pOut )
		{
			*pOut = debug_newa( char, 1 );
			(*pOut)[0] = 0;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::CWeaponMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CWeaponMgr::CWeaponMgr()
{
	m_pWeaponList = LTNULL;
	m_nNumWeapons = 0;

	m_pAmmoList = LTNULL;
	m_nNumAmmos = 0;

	m_pModList = LTNULL;
	m_nNumMods = 0;

	m_pGearList = LTNULL;
	m_nNumGear = 0;

	m_pWeaponAnisList = LTNULL;
	m_nNumWeaponAnis = 0;

	m_pWeaponOrder = LTNULL;

	m_pClasses = LTNULL;
	m_nNumClasses = 0;

	m_pWeaponPriorities = 0;
	m_nNumWeaponPriorities = 0;

	m_nFirstPlayerWeapon = 0;
	m_nLastPlayerWeapon = 0;

	m_nFileCRC = 0;

	m_sMPDefaults = "";
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
//	ROUTINE:	CWeaponMgr::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

CWeaponMgr& CWeaponMgr::Instance( )
{
	// Putting the singleton as a static function variable ensures that this
	// object is only created if it is used.
	static CWeaponMgr sSingleton;
	return sSingleton;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponMgr::Init(const char* szAttributeFile)
{
	if (g_pWeaponMgr || !szAttributeFile) return LTFALSE;
	if (!Parse(szAttributeFile)) return LTFALSE;

	SAFE_STRCPY(s_AttributeFile,szAttributeFile);

	// Set up global pointer...

	g_pWeaponMgr = this;

	// CRC the attribute file
	m_nFileCRC = CRC32::CalcRezFileCRC(szAttributeFile);

	// Clear all our lists before we start...

	ClearLists();


	// Read in the properties for each mod type...

	ModList tempModList;
	tempModList.Init(LTFALSE);

	m_nNumMods = 0;
	sprintf(s_aTagName, "%s%d", WMGR_MOD_TAG, m_nNumMods);

	while (m_buteMgr.Exist(s_aTagName))
	{
		MOD *pMod = debug_new(MOD);

		if (pMod && pMod->Init(m_buteMgr, s_aTagName))
		{
			pMod->nId = m_nNumMods;
			tempModList.AddTail(pMod);
		}
		else
		{
			debug_delete(pMod);
			return LTFALSE;
		}

		m_nNumMods++;
		sprintf(s_aTagName, "%s%d", WMGR_MOD_TAG, m_nNumMods);
	}

	// Build our mod array...

	if (m_nNumMods > 0)
	{
		m_pModList = debug_newa(MOD*, m_nNumMods);
		if (!m_pModList) return LTFALSE;

		MOD **pCurMod = LTNULL;
		pCurMod = tempModList.GetItem(TLIT_FIRST);

		int i=0;
		while (pCurMod)
		{	
			if (*pCurMod)
			{
				m_pModList[i] = (*pCurMod);
				i++;
			}
			pCurMod = tempModList.GetItem(TLIT_NEXT);
		}
	}

	// Read in the properties for each gear type...

	GearList tempGearList;
	tempGearList.Init(LTFALSE);

	m_nNumGear = 0;
	sprintf(s_aTagName, "%s%d", WMGR_GEAR_TAG, m_nNumGear);

	while (m_buteMgr.Exist(s_aTagName))
	{
		GEAR * pGear = debug_new(GEAR);

		if (pGear && pGear->Init(m_buteMgr, s_aTagName))
		{
			pGear->nId = m_nNumGear;
			tempGearList.AddTail(pGear);
		}
		else
		{
			debug_delete(pGear);
			return LTFALSE;
		}

		m_nNumGear++;
		sprintf(s_aTagName, "%s%d", WMGR_GEAR_TAG, m_nNumGear);
	}

	// Build our gear array...

	if (m_nNumGear > 0)
	{
		m_pGearList = debug_newa(GEAR*, m_nNumGear);
		if (!m_pGearList) return LTFALSE;

		GEAR **pCurGear  = LTNULL;
		pCurGear = tempGearList.GetItem(TLIT_FIRST);

		int i=0;
		while (pCurGear)
		{	
			if (*pCurGear)
			{
				m_pGearList[i] = (*pCurGear);
				i++;
			}
			pCurGear = tempGearList.GetItem(TLIT_NEXT);
		}
	}

	// Read in the properties for each weapon anis type...

	WeaponAnisList tempWeaponAniList;
	tempWeaponAniList.Init(LTFALSE);

	m_nNumWeaponAnis = 0;
	sprintf(s_aTagName, "%s%d", WMGR_WEAPONANI_TAG, m_nNumWeaponAnis);

	while (m_buteMgr.Exist(s_aTagName))
	{
		WEAPONANIS* pWeaponAni = debug_new(WEAPONANIS);

		if (pWeaponAni && pWeaponAni->Init(m_buteMgr, s_aTagName))
		{
			pWeaponAni->nId = m_nNumWeaponAnis;
			tempWeaponAniList.AddTail(pWeaponAni);
		}
		else
		{
			debug_delete(pWeaponAni);
			return LTFALSE;
		}

		m_nNumWeaponAnis++;
		sprintf(s_aTagName, "%s%d", WMGR_WEAPONANI_TAG, m_nNumWeaponAnis);
	}

	// Build our weapon ani array...

	if (m_nNumWeaponAnis > 0)
	{
		m_pWeaponAnisList = debug_newa(WEAPONANIS*, m_nNumWeaponAnis);
		if (!m_pWeaponAnisList) return LTFALSE;

		WEAPONANIS **pCurWeaponAni  = LTNULL;
		pCurWeaponAni = tempWeaponAniList.GetItem(TLIT_FIRST);

		int i=0;
		while (pCurWeaponAni)
		{	
			if (*pCurWeaponAni)
			{
				m_pWeaponAnisList[i] = (*pCurWeaponAni);
				i++;
			}
			pCurWeaponAni = tempWeaponAniList.GetItem(TLIT_NEXT);
		}
	}


	// Read in the properties for each ammo type...

	AmmoList tempAmmoList;
	tempAmmoList.Init(LTFALSE);

	m_nNumAmmos = 0;
	sprintf(s_aTagName, "%s%d", WMGR_AMMO_TAG, m_nNumAmmos);

	while (m_buteMgr.Exist(s_aTagName))
	{
		AMMO *pAmmo = debug_new(AMMO);

		if (pAmmo && pAmmo->Init(m_buteMgr, s_aTagName))
		{
			pAmmo->nId = m_nNumAmmos;
			tempAmmoList.AddTail(pAmmo);
		}
		else
		{
			debug_delete(pAmmo);
			return LTFALSE;
		}

		m_nNumAmmos++;
		sprintf(s_aTagName, "%s%d", WMGR_AMMO_TAG, m_nNumAmmos);
	}

	// Build our ammo array...

	if (m_nNumAmmos > 0)
	{
		m_pAmmoList = debug_newa(AMMO*, m_nNumAmmos);
		if (!m_pAmmoList) return LTFALSE;

		AMMO **pCurAmmo  = LTNULL;
		pCurAmmo = tempAmmoList.GetItem(TLIT_FIRST);

		int i=0;
		while (pCurAmmo)
		{	
			if (*pCurAmmo)
			{
				m_pAmmoList[i] = (*pCurAmmo);
				i++;
			}
			pCurAmmo = tempAmmoList.GetItem(TLIT_NEXT);
		}
	}
	else
	{
		return LTFALSE;
	}


	// Read in the properties for each weapon into the temp list...

	WeaponList tempWeaponList;
	tempWeaponList.Init(LTFALSE);

	m_nNumWeapons = 0;
	sprintf(s_aTagName, "%s%d", WMGR_WEAPON_TAG, m_nNumWeapons);

	while (m_buteMgr.Exist(s_aTagName))
	{
		WEAPON *pWeapon = debug_new(WEAPON);

		if (pWeapon && pWeapon->Init(m_buteMgr, s_aTagName))
		{
			pWeapon->nId = m_nNumWeapons;
			tempWeaponList.AddTail(pWeapon);
		}
		else
		{
			debug_delete(pWeapon);
			return LTFALSE;
		}

		m_nNumWeapons++;
		sprintf(s_aTagName, "%s%d", WMGR_WEAPON_TAG, m_nNumWeapons);
	}

	// Build our weapons array...

	if (m_nNumWeapons > 0)
	{
		m_pWeaponList = debug_newa(WEAPON*, m_nNumWeapons);
		if (!m_pWeaponList) return LTFALSE;

		WEAPON **pCurWeapon  = LTNULL;
		pCurWeapon = tempWeaponList.GetItem(TLIT_FIRST);

		int i=0;
		while (pCurWeapon)
		{	
			if (*pCurWeapon)
			{
				m_pWeaponList[i] = (*pCurWeapon);
				i++;
			}

			pCurWeapon = tempWeaponList.GetItem(TLIT_NEXT);
		}
	}
	else
	{
		return LTFALSE;
	}


	// Read in the order of the weapons...

	m_pWeaponOrder = debug_newa(int, m_nNumWeapons);
	if (!m_pWeaponOrder) return LTFALSE;

	int nNum = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPONORDER_WEAPON, nNum);

	char szWeaponName[128] = "";
	WEAPON const *pWeapon = LTNULL;

	while (nNum < m_nNumWeapons)
	{
		m_buteMgr.GetString(WMGR_WEAPONORDER_TAG, s_aAttName,"", szWeaponName,sizeof(szWeaponName));
		if( !m_buteMgr.Success( ))
			break;

		if (szWeaponName[0])
		{
			pWeapon = GetWeapon(szWeaponName);
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


	// Read in the weapon classes...

	m_nNumClasses = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPONORDER_CLASS, (m_nNumClasses+1));
	while (m_buteMgr.Exist(WMGR_WEAPONORDER_TAG, s_aAttName))
	{
		m_nNumClasses++;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPONORDER_CLASS, (m_nNumClasses+1));
	}

	
	if (m_nNumClasses)
	{
		m_pClasses = debug_newa(uint8, m_nNumClasses);
		if (!m_pClasses) return LTFALSE;

		for (int nClass = 0; nClass < m_nNumClasses; nClass++)
		{
			sprintf(s_aAttName, "%s%d", WMGR_WEAPONORDER_CLASS, (nClass+1));
			m_pClasses[nClass] = (uint8)m_buteMgr.GetInt(WMGR_WEAPONORDER_TAG, s_aAttName);
		}
	}
	else
	{
		m_nNumClasses = 1;
		m_pClasses = debug_newa(uint8, 1);
		if (!m_pClasses) return LTFALSE;
		m_pClasses[0] = m_nLastPlayerWeapon;
	}


	// Read in the auto-switch weapon priorities...

	m_nNumWeaponPriorities = 0;
	sprintf(s_aAttName, "%s%d", WMGR_AUTOSWITCH_WEAPON, m_nNumWeaponPriorities);
	while (m_buteMgr.Exist(WMGR_WEAPONORDER_TAG, s_aAttName))
	{
		m_nNumWeaponPriorities++;
		sprintf(s_aAttName, "%s%d", WMGR_AUTOSWITCH_WEAPON, m_nNumWeaponPriorities);
	}
	
	if (m_nNumWeaponPriorities)
	{
		m_pWeaponPriorities = debug_newa(uint8, m_nNumWeaponPriorities);
		if (!m_pWeaponPriorities) return LTFALSE;

		for (int i = 0; i < m_nNumWeaponPriorities; i++)
		{
			m_pWeaponPriorities[i] = WMGR_INVALID_ID;

			sprintf(s_aAttName, "%s%d", WMGR_AUTOSWITCH_WEAPON, i);

			m_buteMgr.GetString(WMGR_WEAPONORDER_TAG, s_aAttName,"", szWeaponName, sizeof(szWeaponName));
			if ( m_buteMgr.Success() )
			{
				if (szWeaponName[0])
				{
					pWeapon = GetWeapon(szWeaponName);
					if (pWeapon)
					{
						m_pWeaponPriorities[i] = pWeapon->nId;
					}
				}
			}
		}
	}

	char szTmp[128];
	m_buteMgr.GetString(WMGR_MULTI_TAG, WMGR_MULTI_DEFAULTS,"", szTmp, sizeof(szTmp));
	m_sMPDefaults = szTmp;


	// Free up the bute mgr's memory...

	m_buteMgr.Term();

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeaponMgr::LoadOverrideButes
//
//  PURPOSE:	Look for an overriden set of butes based of the default set
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponMgr::LoadOverrideButes( const char *szAttributeFile /*=WEAPON_DEFAULT_MULTI_FILE*/ )
{
	// Loop through the current set of butes and then look for butes with a tag [name]
	// and change the values for the current butes.

	if( !g_pWeaponMgr || !szAttributeFile ) return LTFALSE;
	if( !Parse( szAttributeFile )) return LTFALSE;

	// CRC the attribute file

	uint32 nDefaultileCRC = CRC32::CalcRezFileCRC( WEAPON_DEFAULT_FILE );
	m_nFileCRC = nDefaultileCRC & CRC32::CalcRezFileCRC( szAttributeFile );

	char szName[WMGR_MAX_NAME_LENGTH] = {0};


	// Read in the properties for each mod...
	
	MOD *pMod = LTNULL;
	int nNum = 0;

	while( true )
	{
		sprintf( s_aTagName, "%s%d", WMGR_MOD_TAG, nNum );
		m_buteMgr.GetString( s_aTagName, WMGR_MOD_NAME, "", szName, sizeof( szName ));
		if( !m_buteMgr.Success( ))
			break;

		if( szName[0] )
		{
			pMod = const_cast<MOD*>(GetMod( szName ));
			if( pMod )
				pMod->Override( m_buteMgr, s_aTagName );
		}

		++nNum;
	}


	// Read in the properties for each gear...

	GEAR *pGear = LTNULL;
	nNum = 0;

	while( true )
	{
		sprintf( s_aTagName, "%s%d", WMGR_GEAR_TAG, nNum );
		m_buteMgr.GetString( s_aTagName, WMGR_GEAR_NAME, "", szName, sizeof( szName ));
		if( !m_buteMgr.Success( ))
			break;

		if( szName[0] )
		{
			pGear = const_cast<GEAR*>(GetGear( szName ));
			if( pGear )
				pGear->Override( m_buteMgr, s_aTagName );
		}

		++nNum;
	}


	// Read in the properties for each weaponanis...
	
	WEAPONANIS *pWeaponAnis = LTNULL;
	nNum = 0;

	while( true )
	{
		sprintf( s_aTagName, "%s%d", WMGR_WEAPONANI_TAG, nNum );
		m_buteMgr.GetString( s_aTagName, WMGR_WEAPONANI_NAME, "", szName, sizeof( szName ));
		if( !m_buteMgr.Success( ))
			break;

		if( szName[0] )
		{
			pWeaponAnis = const_cast<WEAPONANIS*>(GetWeaponAnis( szName ));
			if( pWeaponAnis )
				pWeaponAnis->Override( m_buteMgr, s_aTagName );
		}

		++nNum;
	}


	// Read in the properties for each ammo...

	AMMO *pAmmo = LTNULL;
	nNum = 0;
	
	while( true )
	{
		sprintf( s_aTagName, "%s%d", WMGR_AMMO_TAG, nNum );
		m_buteMgr.GetString( s_aTagName, WMGR_AMMO_NAME, "", szName, sizeof( szName ));
		if( !m_buteMgr.Success( ))
			break;

		if( szName[0] )
		{
			pAmmo = const_cast<AMMO*>(GetAmmo( szName ));
			if( pAmmo )
				pAmmo->Override( m_buteMgr, s_aTagName );
		}
		
		++nNum;
	}

	
	// Read in the properties for each weapon...

	WEAPON	*pWeapon = LTNULL;
	nNum = 0;

	while( true )
	{
		sprintf(s_aTagName, "%s%d", WMGR_WEAPON_TAG, nNum);
		m_buteMgr.GetString( s_aTagName, WMGR_WEAPON_NAME, "", szName, sizeof( szName ));
		if( !m_buteMgr.Success( ))
			break;

		if( szName[0] )
		{
			pWeapon = const_cast<WEAPON*>(GetWeapon( szName ));
			if( pWeapon )
				pWeapon->Override( m_buteMgr, s_aTagName );
		}

		++nNum;
		sprintf(s_aTagName, "%s%d", WMGR_WEAPON_TAG, nNum);
	}

	// Free up the bute mgr's memory...

	m_buteMgr.Term();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::ClearLists()
//
//	PURPOSE:	Clean up the lists
//
// ----------------------------------------------------------------------- //

void CWeaponMgr::ClearLists()
{
	if (m_pWeaponList)
	{
		for (int i=0; i < m_nNumWeapons; i++)
		{
			debug_delete(m_pWeaponList[i]);
		}

		debug_deletea(m_pWeaponList);
		m_pWeaponList = LTNULL;
		m_nNumWeapons = 0;
	}

	if (m_pWeaponOrder)
	{
		debug_deletea(m_pWeaponOrder);
		m_pWeaponOrder = LTNULL;
	}

	if (m_pAmmoList)
	{
		for (int i=0; i < m_nNumAmmos; i++)
		{
			debug_delete(m_pAmmoList[i]);
		}

		debug_deletea(m_pAmmoList);
		m_pAmmoList = LTNULL;
		m_nNumAmmos = 0;
	}

	if (m_pModList)
	{
		for (int i=0; i < m_nNumMods; i++)
		{
			debug_delete(m_pModList[i]);
		}

		debug_deletea(m_pModList);
		m_pModList = LTNULL;
	}

	if (m_pGearList)
	{
		for (int i=0; i < m_nNumGear; i++)
		{
			debug_delete(m_pGearList[i]);
		}

		debug_deletea(m_pGearList);
		m_pGearList = LTNULL;
		m_nNumGear = 0;
	}

	if (m_pWeaponAnisList)
	{
		for (int i=0; i < m_nNumWeaponAnis; i++)
		{
			debug_delete(m_pWeaponAnisList[i]);
		}

		debug_deletea(m_pWeaponAnisList);
		m_pWeaponAnisList = LTNULL;
		m_nNumWeaponAnis = 0;
	}

	if (m_pClasses)
	{
		debug_deletea(m_pClasses);
		m_pClasses = LTNULL;
		m_nNumClasses = 0;
	}

	if (m_pWeaponPriorities)
	{
		debug_deletea(m_pWeaponPriorities);
		m_pWeaponPriorities = LTNULL;
		m_nNumWeaponPriorities = 0;
	}
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

	ClearLists();

	CGameButeMgr::Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::WriteFile()
//
//	PURPOSE:	Write necessary data back out to bute file
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponMgr::WriteFile()
{
	// Re-init our bute mgr...

	m_buteMgr.Init(GBM_DisplayError);
	Parse(WEAPON_DEFAULT_FILE);


	// Save the necessary weapon data...

	for (int i=0; i < m_nNumWeapons; i++)
	{
		if (m_pWeaponList[i])
		{
			m_pWeaponList[i]->Save(m_buteMgr);
		}
	}


	// Save the file...

	LTBOOL bRet = m_buteMgr.Save(); // "Game\\Attributes\\Weapons.txt");
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

void CWeaponMgr::Reload()
{
	Term();
	Init();
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
	WEAPON const *pWeapon = GetWeapon(wp.nWeaponId);
	if (!pWeapon) return;

	// Make sure the path has been normalized...

	wp.vPath.Normalize();

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

		wp.vPath.Normalize();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetWeapon
//
//	PURPOSE:	Get the specified weapon struct
//
// ----------------------------------------------------------------------- //

WEAPON const *CWeaponMgr::GetWeapon(int nWeaponId) const
{
	if (IsValidWeaponId(nWeaponId)) return m_pWeaponList[nWeaponId];

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetCorrespondingWeapon
//
//	PURPOSE:	Find the first weapon that uses this type of ammo...
//
// ----------------------------------------------------------------------- //

WEAPON const *CWeaponMgr::GetCorrespondingWeapon(AMMO const *pAmmo) const
{
	if (!pAmmo || !m_pWeaponList) return LTNULL;

	for (int i=0; i < m_nNumWeapons; i++)
	{
		WEAPON* pWeapon = m_pWeaponList[i];

		// See if any of this weapon's ammo types matches...
		if (pWeapon && pWeapon->aAmmoIds)
		{
			for (int j=0; j < pWeapon->nNumAmmoIds; j++)
			{
				if (pAmmo->nId == pWeapon->aAmmoIds[j])
				{
					return pWeapon;
				}
			}
		}
		else
		{
			return LTNULL;
		}
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

WEAPON const *CWeaponMgr::GetWeapon(const char* pWeaponName) const
{
	if (!pWeaponName) return LTNULL;

	for (int i=0; i < m_nNumWeapons; i++)
	{
		if (m_pWeaponList[i] && m_pWeaponList[i]->szName[0] && (_stricmp(m_pWeaponList[i]->szName, pWeaponName) == 0))
		{
			return m_pWeaponList[i];
		}
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

AMMO const *CWeaponMgr::GetAmmo(int nAmmoId) const
{
	if (IsValidAmmoId(nAmmoId)) return m_pAmmoList[nAmmoId];

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetAmmo
//
//	PURPOSE:	Get the specified ammo struct
//
// ----------------------------------------------------------------------- //

AMMO const *CWeaponMgr::GetAmmo(char const* pAmmoName) const
{
	if (!pAmmoName) return LTNULL;

	for (int i=0; i < m_nNumAmmos; i++)
	{
		if (m_pAmmoList[i] && m_pAmmoList[i]->szName[0] && (_stricmp(m_pAmmoList[i]->szName, pAmmoName) == 0))
		{
			return m_pAmmoList[i];
		}
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

MOD const *CWeaponMgr::GetMod(int nModId) const
{
	if (IsValidModId(nModId)) return m_pModList[nModId];

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetMod
//
//	PURPOSE:	Get the specified mod struct
//
// ----------------------------------------------------------------------- //

MOD const *CWeaponMgr::GetMod(char const* pModName) const
{
	if (!pModName) return LTNULL;

	for (int i=0; i < m_nNumMods; i++)
	{
		if (m_pModList[i] && m_pModList[i]->szName[0] && (_stricmp(m_pModList[i]->szName, pModName) == 0))
		{
			return m_pModList[i];
		}
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

GEAR const * CWeaponMgr::GetGear(int nGearId) const
{
	if (IsValidGearId(nGearId)) return m_pGearList[nGearId];

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::GetGear
//
//	PURPOSE:	Get the specified gear struct
//
// ----------------------------------------------------------------------- //

GEAR const* CWeaponMgr::GetGear(char const* pGearName) const
{
	if (!pGearName) return LTNULL;

	for (int i=0; i < m_nNumGear; i++)
	{
		if (m_pGearList[i] && m_pGearList[i]->szName[0] && (_stricmp(m_pGearList[i]->szName, pGearName) == 0))
		{
			return m_pGearList[i];
		}
	}

	return LTNULL;
}

CScaleFX*		CWeaponMgr::GetScaleFX(int nScaleFXId)      { return g_pFXButeMgr->GetScaleFX(nScaleFXId); }
PEXPLFX*		CWeaponMgr::GetPExplFX(int nPExplFXId)      { return g_pFXButeMgr->GetPExplFX(nPExplFXId); }
DLIGHTFX*		CWeaponMgr::GetDLightFX(int nDLightFXId)    { return g_pFXButeMgr->GetDLightFX(nDLightFXId); }
IMPACTFX*		CWeaponMgr::GetImpactFX(int nImpactFXId)    { return g_pFXButeMgr->GetImpactFX(nImpactFXId); }
PROJECTILEFX*	CWeaponMgr::GetProjectileFX(int nProjFXId)  { return g_pFXButeMgr->GetProjectileFX(nProjFXId); }
FIREFX*			CWeaponMgr::GetFireFX(int nFireFXId)        { return g_pFXButeMgr->GetFireFX(nFireFXId); }

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

	for (int i=0; i < m_nNumWeaponAnis; i++)
	{
		if (m_pWeaponAnisList[i] && m_pWeaponAnisList[i]->szName[0] && (_stricmp(m_pWeaponAnisList[i]->szName, pAnisName) == 0))
		{
			return m_pWeaponAnisList[i];
		}
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponMgr::IsBetterWeapon
//
//	PURPOSE:	Checks if weaponida is better than weaponidb.
//
// ----------------------------------------------------------------------- //

bool CWeaponMgr::IsBetterWeapon( uint8 nWeaponIdA, uint8 nWeaponIdB ) const
{
	// Check if they're the same weapon.
	if( nWeaponIdA == nWeaponIdB )
		return false;

	// If we find A before B, then A is better.
	for( int i = 0; i < m_nNumWeaponPriorities; i++ )
	{
		if( m_pWeaponPriorities[i] == nWeaponIdA )
			return true;

		if( m_pWeaponPriorities[i] == nWeaponIdB )
			return false;
	}

	return false;
}

const char* CWeaponMgr::GetMPDefaultWeapons() const
{
	return m_sMPDefaults.c_str();
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
	int i;

	bInited         = LTFALSE;

	nId				= WMGR_INVALID_ID;

	nNameId				= 0;
	nDescriptionId		= 0;
	nIsAmmoNoPickupId	= 0;
	nAniType			= 0;

	szName				= LTNULL;
	szLongName			= LTNULL;
	szShortName			= LTNULL;
	szIcon				= LTNULL;
	szPVModel			= LTNULL;
	szHHModel			= LTNULL;
	szSilencedFireSound	= LTNULL;
	szAltFireSound		= LTNULL;
	szFireSound			= LTNULL;
	szDryFireSound		= LTNULL;
	szSelectSound		= LTNULL;
	szDeselectSound		= LTNULL;
	szPVMuzzleFxName	= LTNULL;
	szHHMuzzleFxName	= LTNULL;
	szHolsterAttachment	= LTNULL;
	szPrimitiveType		= LTNULL;
	szSubroutineNeeded	= LTNULL;
	szPowerupFX			= LTNULL;
	szRespawnWaitFX		= LTNULL;

	for( i = 0; i < WMGR_MAX_RELOAD_SNDS; ++i )
	{
		szReloadSounds[i] = LTNULL;
	}

	for( i = 0; i < WMGR_MAX_MISC_SNDS; ++i )
	{
		szMiscSounds[i] = LTNULL;
	}

  	vPos.Init();
	vMuzzlePos.Init();
	vBreachOffset.Init();
	vHHScale.Init();
    
	nFireSoundRadius	= 0;
	nAIFireSoundRadius	= 0;
	nWeaponSoundRadius	= (int)WEAPON_SOUND_RADIUS;

    bEnvironmentMap     = LTFALSE;
    bInfiniteAmmo       = LTFALSE;
    bLooksDangerous     = LTFALSE;
	bHideWhenEmpty		= LTFALSE;
	bIsAmmo				= LTFALSE;
	bUseUWMuzzleFX		= LTFALSE;

	nShotsPerClip		= 0;

	nNumAmmoIds		= 0;
	aAmmoIds			= LTNULL;
	

	nDefaultAmmoId		= 0;

	nNumModIds		= 0;
	aModIds			= LTNULL;
	

	nNumPVFXTypes		= 0;
	aPVFXTypes			= LTNULL;
	

	vRecoil.Init();

	nMinPerturb			= 0;
	nMaxPerturb			= 0;

	nRange				= 0;
	nVectorsPerRound	= 0;

	nAIWeaponType		= 0;
	nAIMinBurstShots	= 0;
	nAIMaxBurstShots	= 0;
	fAIMinBurstInterval	= 1.0f;
	fAIMaxBurstInterval	= 1.0f;
	bAIAnimatesReloads	= LTTRUE;

	fFireRecoilPitch	= 0.0f;
	fFireRecoilDecay	= 0.0f;

    pPVMuzzleFX         = LTNULL;

	nNumPVAttachClientFX = 0;
	for( i = 0; i < WMGR_MAX_PVCLIENTFX_ATTACHMENTS; ++i )
	{
		szPVAttachClientFX[ i ] = LTNULL;
	}

	fFireAnimRateScale		= 1.0f;
	fReloadAnimRateScale	= 1.0f;

	bRespawnWaitVisible		= false;
	bRespawnWaitTranslucent = false;

	bServerRestricted = false;
	bCanServerRestrict = true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WEAPON::~WEAPON
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

WEAPON::~WEAPON()
{
	int i;

	bInited = LTFALSE;

	debug_deletea( szName );
	debug_deletea( szLongName );
	debug_deletea( szShortName );
	debug_deletea( szIcon );
	debug_deletea( szPVModel );
	debug_deletea( szHHModel );
	debug_deletea( szSilencedFireSound );
	debug_deletea( szAltFireSound );
	debug_deletea( szFireSound );
	debug_deletea( szDryFireSound );
	debug_deletea( szSelectSound );
	debug_deletea( szDeselectSound );
	debug_deletea( szPVMuzzleFxName );
	debug_deletea( szHHMuzzleFxName );
	debug_deletea( szHolsterAttachment );
	debug_deletea( szPrimitiveType );
	debug_deletea( szSubroutineNeeded );
	debug_deletea( szPowerupFX );
	debug_deletea( szRespawnWaitFX );

	for( i = 0; i < WMGR_MAX_RELOAD_SNDS; ++i )
	{
		debug_deletea( szReloadSounds[i] );
	}
	
	for( i = 0; i < WMGR_MAX_MISC_SNDS; ++i )
	{
		debug_deletea( szMiscSounds[i] );
	}
	
	debug_deletea( aAmmoIds );
	debug_deletea( aModIds );
	debug_deletea( aPVFXTypes );

	for( i = 0; i < WMGR_MAX_PVCLIENTFX_ATTACHMENTS; ++i )
	{
		debug_deletea( szPVAttachClientFX[i] );
	}
	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WEAPON::InitMembers
//
//  PURPOSE:	Actually set the member vars...
//				This allows us to ReInit members with multiple bute files
//
// ----------------------------------------------------------------------- //

void WEAPON::InitMembers( CButeMgr &buteMgr, char *aTagName )
{
	ASSERT( aTagName != NULL );

	// Use the members as the default value incase the attribute doesn't exist...

	nNameId					= buteMgr.GetInt( aTagName, WMGR_WEAPON_NAMEID, nNameId );
	nDescriptionId          = buteMgr.GetInt( aTagName, WMGR_WEAPON_DESCRIPTIONID, nDescriptionId );
	nIsAmmoNoPickupId		= buteMgr.GetInt( aTagName, WMGR_WEAPON_ISAMMONOPICKUPID, nIsAmmoNoPickupId );
	nClientWeaponType       = buteMgr.GetInt( aTagName, WMGR_WEAPON_CLIENT_WEAPON_TYPE, nClientWeaponType );
	nAniType                = buteMgr.GetInt( aTagName, WMGR_WEAPON_ANITYPE, nAniType );
	vPos                    = buteMgr.GetVector( aTagName, WMGR_WEAPON_POS, CAVector( VEC_EXPAND(vPos) ));
	vMuzzlePos              = buteMgr.GetVector( aTagName, WMGR_WEAPON_MUZZLEPOS, CAVector( VEC_EXPAND(vMuzzlePos) ));
	vBreachOffset           = buteMgr.GetVector( aTagName, WMGR_WEAPON_BREACHOFFSET, CAVector( VEC_EXPAND(vBreachOffset) ));
	vHHScale                = buteMgr.GetVector( aTagName, WMGR_WEAPON_HHSCALE, CAVector( VEC_EXPAND(vHHScale) ));
	vRecoil                 = buteMgr.GetVector( aTagName, WMGR_WEAPON_RECOIL, CAVector( VEC_EXPAND(vRecoil) ));

	// The name is only set in the Init so we cannot override it!
	// DO NOT list name here.

	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_ICON, &szIcon, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_PVMODEL, &szPVModel, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_HHMODEL, &szHHModel, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_ALTFIRESND, &szAltFireSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_SILENCEDFIRESND, &szSilencedFireSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_FIRESND, &szFireSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_DRYFIRESND, &szDryFireSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_SELECTSND, &szSelectSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_DESELECTSND, &szDeselectSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_PVMUZZLEFXNAME, &szPVMuzzleFxName, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_HHMUZZLEFXNAME, &szHHMuzzleFxName, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_HOLSTERATTACHMNET, &szHolsterAttachment, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_PRIMITIVE_TYPE, &szPrimitiveType, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_SUBROUTINE_NEEDED, &szSubroutineNeeded, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_POWERUPFX, &szPowerupFX, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPON_RESPAWNWAITFX, &szRespawnWaitFX, WMGR_MAX_NAME_LENGTH );
	
	int i;
	for( i = 0; i < WMGR_MAX_RELOAD_SNDS; ++i )
	{
		sprintf(s_aAttName, "%s%d", WMGR_WEAPON_RELOADSND, i + 1);
		GetStringIfExist( buteMgr, aTagName, s_aAttName, &szReloadSounds[i], WMGR_MAX_FILE_PATH );
	}

	for( i = 0; i < WMGR_MAX_MISC_SNDS; ++i )
	{
		sprintf( s_aAttName, "%s%d", WMGR_WEAPON_MISCSND, i + 1 );
		GetStringIfExist( buteMgr, aTagName, s_aAttName, &szMiscSounds[i], WMGR_MAX_FILE_PATH );
	}

	// The bute list reader will delete the list if one already exists
	// This means if one of the properties is going to be overwriten, all of them must be.
	
	blrPVSkins.Read(&buteMgr, aTagName, WMGR_WEAPON_PVSKIN, WMGR_MAX_FILE_PATH);
	blrPVRenderStyles.Read(&buteMgr, aTagName, WMGR_WEAPON_PVRENDERSTYLE, WMGR_MAX_FILE_PATH);

	blrHHSkins.Read(&buteMgr, aTagName, WMGR_WEAPON_HHSKIN, WMGR_MAX_FILE_PATH);
	blrHHRenderStyles.Read(&buteMgr, aTagName, WMGR_WEAPON_HHRENDERSTYLE, WMGR_MAX_FILE_PATH);

	blrHiddenPieceNames.Read(&buteMgr, aTagName, WMGR_WEAPON_HIDDENPIECENAME, WMGR_MAX_NAME_LENGTH);

	blrRespawnWaitSkins.Read( &buteMgr, aTagName, WMGR_WEAPON_RESPAWNWAITSKIN, WMGR_MAX_FILE_PATH );
	blrRespawnWaitRenderStyles.Read( &buteMgr, aTagName, WMGR_WEAPON_RESPAWNWAITRENDERSTYLE, WMGR_MAX_FILE_PATH );
	

	nFireSoundRadius	= buteMgr.GetInt(aTagName, WMGR_WEAPON_FIRESNDRADIUS, nFireSoundRadius);
    nAIFireSoundRadius  = buteMgr.GetInt(aTagName, WMGR_WEAPON_AIFIRESNDRADIUS, nAIFireSoundRadius);
	nWeaponSoundRadius	= buteMgr.GetInt( aTagName, WMGR_WEAPON_WEAPONSNDRADIUS, nWeaponSoundRadius );
    bEnvironmentMap     = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_ENVMAP, bEnvironmentMap);
    bInfiniteAmmo       = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_INFINITEAMMO, bInfiniteAmmo);
    bLooksDangerous     = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_LOOKSDANGEROUS, bLooksDangerous);
    bHideWhenEmpty      = (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_HIDEWHENEMPTY, bHideWhenEmpty);
    bIsAmmo				= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_ISAMMO, bIsAmmo);
    nShotsPerClip       = buteMgr.GetInt(aTagName, WMGR_WEAPON_SHOTSPERCLIP, nShotsPerClip);
    bUseUWMuzzleFX		= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_USEUWMUZZLEFX, bUseUWMuzzleFX);


	// Build our ammo types id array...

	int nAmmoIdCount = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPON_AMMONAME, nAmmoIdCount);

	while (buteMgr.Exist(aTagName, s_aAttName) && nAmmoIdCount < WMGR_MAX_AMMO_IDS)
	{
		++nAmmoIdCount;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPON_AMMONAME, nAmmoIdCount);
	}

	if( nAmmoIdCount )
	{
		if( nAmmoIdCount != nNumAmmoIds )
		{
			// Reset the number of ammo Ids we have...

			nNumAmmoIds = nAmmoIdCount;
			debug_deletea( aAmmoIds );
			aAmmoIds = debug_newa( int, nNumAmmoIds );
		}
		

		char szStr[WMGR_MAX_NAME_LENGTH] = {0};
		int n = 0;
		for( int i = 0; i < nNumAmmoIds; ++i )
		{
			aAmmoIds[i] = WMGR_INVALID_ID;
			sprintf( s_aAttName, "%s%d", WMGR_WEAPON_AMMONAME, i );

			buteMgr.GetString(aTagName, s_aAttName, szStr, sizeof(szStr));
			if (szStr[0])
			{
				AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(szStr);
				ASSERT(pAmmo);
				if (pAmmo)
				{
					aAmmoIds[n] = pAmmo->nId;
					++n;
				}
				else
				{
#ifdef _CLIENTBUILD
					g_pLTClient->CPrint("Error in %s: cannot load ammo %s",s_AttributeFile,szStr);
#endif
				}
			}
		}

		if (n < nNumAmmoIds)
		{
			int		*tmpAmmoIds = debug_newa( int, n );
			for (int i = 0; i < n; ++i)
			{
				tmpAmmoIds[i] = aAmmoIds[i];
			}

			debug_deletea( aAmmoIds );
			aAmmoIds = tmpAmmoIds;
			nNumAmmoIds = n;

		}

		nDefaultAmmoId = aAmmoIds[0];
	}
	

	// Build our mod types id array...

	int nModIdCount = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPON_MODNAME, nModIdCount);

	while (buteMgr.Exist(aTagName, s_aAttName) && nModIdCount < WMGR_MAX_MOD_IDS)
	{
		++nModIdCount;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPON_MODNAME, nModIdCount);
	}

	if( nModIdCount )
	{
		if( nModIdCount != nNumModIds )
		{
			// Reset the number of mod Ids we have...

			nNumModIds = nModIdCount;
			debug_deletea( aModIds );
			aModIds = debug_newa( int, nNumModIds );
		}
		

		char szStr[WMGR_MAX_NAME_LENGTH] = {0};
		int n = 0;
		for( int i = 0; i < nNumModIds; ++i )
		{
			aModIds[i] = WMGR_INVALID_ID;
			sprintf( s_aAttName, "%s%d", WMGR_WEAPON_MODNAME, i );

			buteMgr.GetString(aTagName, s_aAttName, szStr, sizeof(szStr));
			if (szStr[0])
			{
				MOD const *pMOD = g_pWeaponMgr->GetMod(szStr);
				ASSERT(pMOD);
				if (pMOD)
				{
					aModIds[n] = pMOD->nId;
					++n;
				}
				else
				{
#ifdef _CLIENTBUILD
					g_pLTClient->CPrint("Error in %s: cannot load mod %s",s_AttributeFile,szStr);
#endif
				}
			}
		}

		if (n < nNumModIds)
		{
			int		*tmpModIds = debug_newa( int, n );
			for (int i = 0; i < n; ++i)
			{
				tmpModIds[i] = aModIds[i];
			}

			debug_deletea( aModIds );
			aModIds = tmpModIds;
			nNumModIds = n;

		}
	}


	// Build our pv fx id array...

	int nPVFXTypeCount = 0;
	sprintf(s_aAttName, "%s%d", WMGR_WEAPON_PVFXNAME, nPVFXTypeCount);

	while (buteMgr.Exist(aTagName, s_aAttName) && nPVFXTypeCount < WMGR_MAX_PVFX_TYPES)
	{
		++nPVFXTypeCount;
		sprintf(s_aAttName, "%s%d", WMGR_WEAPON_PVFXNAME, nPVFXTypeCount);
	}

	if( nPVFXTypeCount )
	{
		if( nPVFXTypeCount != nNumPVFXTypes )
		{
			// Reset the number of Player view fx we have...
			
			nNumPVFXTypes = nPVFXTypeCount;
			debug_deletea( aPVFXTypes );
			aPVFXTypes = debug_newa( int, nNumPVFXTypes );
		}
		

		char szStr[WMGR_MAX_NAME_LENGTH] = {0};
		for( int i = 0; i < nNumPVFXTypes; ++i )
		{
			aPVFXTypes[i] = WMGR_INVALID_ID;
			sprintf( s_aAttName, "%s%d", WMGR_WEAPON_PVFXNAME, i );

			buteMgr.GetString(aTagName, s_aAttName, szStr, sizeof(szStr));
			if (szStr[0])
			{
				PVFX* pPVFX = g_pFXButeMgr->GetPVFX(szStr);
				if (pPVFX)
				{
					aPVFXTypes[i] = pPVFX->nId;
				}
			}
		}
	}

	nMinPerturb         = buteMgr.GetInt(aTagName, WMGR_WEAPON_MINPERTURB, nMinPerturb );
	nMaxPerturb         = buteMgr.GetInt(aTagName, WMGR_WEAPON_MAXPERTURB, nMaxPerturb );
	nRange              = buteMgr.GetInt(aTagName, WMGR_WEAPON_RANGE, nRange );
	nVectorsPerRound    = buteMgr.GetInt(aTagName, WMGR_WEAPON_VECTORSPERROUND, nVectorsPerRound );

	nAIWeaponType		= buteMgr.GetInt(aTagName, WMGR_WEAPON_AIWEAPONTYPE, nAIWeaponType );
	nAIMinBurstShots    = buteMgr.GetInt(aTagName, WMGR_WEAPON_AIMINBURSTSHOTS, nAIMinBurstShots );
	nAIMaxBurstShots    = buteMgr.GetInt(aTagName, WMGR_WEAPON_AIMAXBURSTSHOTS, nAIMaxBurstShots );
	fAIMinBurstInterval = (LTFLOAT)buteMgr.GetDouble(aTagName, WMGR_WEAPON_AIMINBURSTINTERVAL, fAIMinBurstInterval );
	fAIMaxBurstInterval = (LTFLOAT)buteMgr.GetDouble(aTagName, WMGR_WEAPON_AIMAXBURSTINTERVAL, fAIMaxBurstInterval );
    bAIAnimatesReloads	= (LTBOOL) buteMgr.GetInt(aTagName, WMGR_WEAPON_AIANIMATESRELOADS, bAIAnimatesReloads);

	fFireRecoilPitch    = (LTFLOAT)buteMgr.GetDouble(aTagName, WMGR_WEAPON_FIRERECOILPITCH, fFireRecoilPitch );
	fFireRecoilDecay    = (LTFLOAT)buteMgr.GetDouble(aTagName, WMGR_WEAPON_FIRERECOILDECAY, fFireRecoilDecay );

	// Build our pv attach fx array (these reference FX from FXEd)...
	for (int i = 0; i < WMGR_MAX_PVCLIENTFX_ATTACHMENTS; ++i )
	{
		sprintf( s_aAttName, "%s%d", WMGR_WEAPON_PVATTACHCLIENTFXNAME, i );
		GetStringIfExist( buteMgr, aTagName, s_aAttName, &szPVAttachClientFX[i], WMGR_MAX_NAME_LENGTH );
	}

	int nDelay = buteMgr.GetInt(aTagName, WMGR_WEAPON_FIREDELAY, m_nFireDelay );
	m_nFireDelay = (nDelay > 0) ? nDelay : 0;

	fFireAnimRateScale		= (float)buteMgr.GetDouble( aTagName, WMGR_WEAPON_FIREANIMRATESCALE, fFireAnimRateScale );
	fReloadAnimRateScale	= (float)buteMgr.GetDouble( aTagName, WMGR_WEAPON_RELOADANIMRATESCALE, fReloadAnimRateScale );

	bRespawnWaitVisible		= !!(buteMgr.GetInt( aTagName, WMGR_WEAPON_RESPAWNWAITVISIBLE, bRespawnWaitVisible ));
	bRespawnWaitTranslucent	= !!(buteMgr.GetInt( aTagName, WMGR_WEAPON_RESPAWNWAITTRANSLUCENT, bRespawnWaitTranslucent ));
	
	bCanServerRestrict = !!( buteMgr.GetInt( aTagName, WMGR_ALL_CANSERVERRESTRICT, bCanServerRestrict ));
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
	ASSERT( LTFALSE == bInited );
	if( bInited ) return LTFALSE;

	bInited = LTTRUE;

	// We only want to get the name of this weapon since it cannot be overriden

	szName	= GetString( buteMgr, aTagName, WMGR_WEAPON_NAME, WMGR_MAX_NAME_LENGTH );

	// Initialize all other values...

	InitMembers( buteMgr, aTagName );


#ifdef _CLIENTBUILD
	char szTmp[128] = "";
	LoadString(nNameId,szTmp,sizeof(szTmp));

	char* pTok = strtok(szTmp,"@");

	if (pTok)
	{
		int l = strlen(pTok)+1;
		szLongName = debug_newa( char, l);
		LTStrCpy(szLongName,pTok,l);
	}
	else
	{
		szLongName = debug_newa( char, 1);
		szLongName[0] = NULL;
	}

	pTok = strtok(NULL,"@");

	if (pTok)
	{
		int l = strlen(pTok)+1;
		szShortName = debug_newa( char, l);
		LTStrCpy(szShortName,pTok,l);
	}
	else
	{
		int l = strlen(szLongName)+1;
		szShortName = debug_newa( char, l);
		LTStrCpy(szShortName,szLongName,l);
	}
#else
	int l = strlen(szName)+1;
	szLongName = debug_newa( char, l);
	LTStrCpy(szLongName,szName,l);
	szShortName = debug_newa( char, l);
	LTStrCpy(szShortName,szName,l);

#endif

	

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WEAPON::Override
//
//  PURPOSE:	Allow the weapon to override it's data
//
// ----------------------------------------------------------------------- //

LTBOOL WEAPON::Override( CButeMgr &buteMgr, char *aTagName )
{
	ASSERT( bInited == LTTRUE );
	if( !aTagName || !bInited ) return LTFALSE;
	
	// ReInit any members we wish to override, If a member is not being overriden it stays at is initial value
	InitMembers( buteMgr, aTagName );
	
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON::Save()
//
//	PURPOSE:	Save any necessary data to the bute file...
//
// ----------------------------------------------------------------------- //

void WEAPON::Save(CButeMgr & buteMgr) const
{
	sprintf(s_aTagName, "%s%d", WMGR_WEAPON_TAG, nId);

	CAVector vAVec(vPos.x, vPos.y, vPos.z);
	buteMgr.SetVector(s_aTagName, WMGR_WEAPON_POS, vAVec);

	vAVec.Set(vMuzzlePos.x, vMuzzlePos.y, vMuzzlePos.z);
	buteMgr.SetVector(s_aTagName, WMGR_WEAPON_MUZZLEPOS, vAVec);

	vAVec.Set(vBreachOffset.x, vBreachOffset.y, vBreachOffset.z);
	buteMgr.SetVector(s_aTagName, WMGR_WEAPON_BREACHOFFSET, vAVec);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WEAPON::Get???Icon
//
//	PURPOSE:	Build the weapon icon name
//
// ----------------------------------------------------------------------- //

std::string	WEAPON::GetNormalIcon() const
{
	std::string str(szIcon);
	str += ".dtx";
	return str;
}
std::string	WEAPON::GetDisabledIcon() const
{
	std::string str(szIcon);
	str += "D.dtx";
	return str;
}
std::string	WEAPON::GetUnselectedIcon() const
{
	std::string str(szIcon);
	str += "U.dtx";
	return str;
}
std::string	WEAPON::GetSilhouetteIcon() const
{
	std::string str(szIcon);
	str += "X.dtx";
	return str;
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
    bInited     = LTFALSE;

	nId			= WMGR_INVALID_ID;

	szName			= LTNULL;
	szSelectAni		= LTNULL;
	szDeselectAni	= LTNULL;
	szReloadAni		= LTNULL;

	for( int i = 0; i < WMGR_MAX_WEAPONANI_IDLE; i++ )
	{
		szIdleAnis[i] = LTNULL;
	}
	
	for(int i = 0; i < WMGR_MAX_WEAPONANI_FIRE; i++ )
	{
		szFireAnis[i] = LTNULL;
	}

	nNumIdleAnis	= 0;
  	nNumFireAnis	= 0;

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WEAPONANIS::~WEAPONANIS
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

WEAPONANIS::~WEAPONANIS()
{
    bInited = LTFALSE;

	debug_deletea( szName );
	debug_deletea( szSelectAni );
	debug_deletea( szDeselectAni );
	debug_deletea( szReloadAni );

	for( int i = 0; i < WMGR_MAX_WEAPONANI_IDLE; i++ )
	{
		debug_deletea( szIdleAnis[i] );
	}

	for(int i = 0; i < WMGR_MAX_WEAPONANI_FIRE; i++ )
	{
		debug_deletea( szFireAnis[i] );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WEAPONANIS::InitMembers
//
//  PURPOSE:	Actually set the member vars...
//				This allows us to ReInit members with multiple bute files
//
// ----------------------------------------------------------------------- //

void WEAPONANIS::InitMembers( CButeMgr &buteMgr, char *aTagName )
{
	ASSERT( aTagName != NULL );

	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPONANI_SELECT, &szSelectAni, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPONANI_DESELECT, &szDeselectAni, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_WEAPONANI_RELOAD, &szReloadAni, WMGR_MAX_NAME_LENGTH );

	nNumIdleAnis = 0;
	for( int i = 0; i < WMGR_MAX_WEAPONANI_IDLE; ++i )
	{
		sprintf(s_aAttName, "%s%d", WMGR_WEAPONANI_IDLE, i + 1);
		GetStringIfExist( buteMgr, aTagName, s_aAttName, &szIdleAnis[i], WMGR_MAX_NAME_LENGTH );

		if( szIdleAnis[i][0] )
			++nNumIdleAnis;
	}

	nNumFireAnis = 0;
	for(int i = 0; i < WMGR_MAX_WEAPONANI_FIRE; ++i )
	{
		sprintf(s_aAttName, "%s%d", WMGR_WEAPONANI_FIRE, i + 1);
		GetStringIfExist( buteMgr, aTagName, s_aAttName, &szFireAnis[i], WMGR_MAX_NAME_LENGTH );

		if( szFireAnis[i][0] )
			++nNumFireAnis;
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
    ASSERT( LTFALSE == bInited );
	if( bInited ) return LTFALSE;

    bInited = LTTRUE;

	// We only want to get the name of this weapon since it cannot be overriden
	
	szName			= GetString( buteMgr, aTagName, WMGR_WEAPONANI_NAME, WMGR_MAX_NAME_LENGTH );

	// Initialize all other values...

	InitMembers( buteMgr, aTagName );

    return LTTRUE;
}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WEAPONANIS::Override
//
//  PURPOSE:	Allow the Weaponanis to override it's data
//
// ----------------------------------------------------------------------- //

LTBOOL WEAPONANIS::Override( CButeMgr &buteMgr, char *aTagName )
{
	ASSERT( bInited == LTTRUE );
	if( !aTagName || !bInited ) return LTFALSE;
	
	// ReInit any members we wish to override, If a member is not being overriden it stays at is initial value
	InitMembers( buteMgr, aTagName );
	
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
	bInited     = LTFALSE;

	nId         = WMGR_INVALID_ID;

	nNameId     = 0;
	nDescId     = 0;
	eType       = UNKNOWN_AMMO_TYPE;
	fPriority   = -1.0f;

	szIcon  = LTNULL;
	szName  = LTNULL;
	szLongName			= LTNULL;
	szShortName			= LTNULL;

	nMaxAmount          = 0;
	nSpawnedAmount      = 0;
	nSelectionAmount    = 0;

	nInstDamage     = 0;
	eInstDamageType = DT_INVALID;

	nAreaDamage         = 0;
	nAreaDamageRadius   = 0;
	eAreaDamageType     = DT_INVALID;
	fFireRecoilMult     = 1.0f;

	fProgDamage         = 0.0f;
	fProgDamageDuration = 0.0f;
	fProgDamageRadius   = 0.0f;
	fProgDamageLifetime = 0.0f;
	eProgDamageType     = DT_INVALID;

	bCanBeDeflected = false;
	bCanAdjustInstDamage = true;

	pProjectileFX			= NULL;
	pImpactFX				= NULL;
	pUWImpactFX				= NULL;
	pRicochetFX				= NULL;
	pBlockedFX				= NULL;
	pFireFX					= NULL;
	pAniOverrides			= NULL;
	pTracerFX				= NULL;
	pMoveableImpactOverrideFX = NULL;

	bServerRestricted = false;
	bCanServerRestrict = true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AMMO::~AMMO
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

AMMO::~AMMO()
{	
	bInited = LTFALSE;

	debug_deletea( szName );
	debug_deletea( szIcon );
	debug_deletea( szLongName );
	debug_deletea( szShortName );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AMMO::InitMembers
//
//  PURPOSE:	Actually set the member vars...
//
// ----------------------------------------------------------------------- //

void AMMO::InitMembers( CButeMgr &buteMgr, char *aTagName )
{
	ASSERT( aTagName != NULL );

	// Use the members as the default value incase the attribute doesn't exist...

	char const* pszValue = "";

	nNameId               = buteMgr.GetInt( aTagName, WMGR_AMMO_NAMEID, nNameId );
	nDescId               = buteMgr.GetInt( aTagName, WMGR_AMMO_DESCID, nDescId );
	eType                 = (AmmoType) buteMgr.GetInt( aTagName, WMGR_AMMO_TYPE, eType );
	fPriority             = (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_AMMO_PRIORITY, fPriority );
	nMaxAmount            = buteMgr.GetInt( aTagName, WMGR_AMMO_MAXAMOUNT, nMaxAmount );
	nSpawnedAmount        = buteMgr.GetInt( aTagName, WMGR_AMMO_SPAWNEDAMOUNT, nSpawnedAmount );
	nSelectionAmount      = buteMgr.GetInt( aTagName, WMGR_AMMO_SELECTIONAMOUNT, nSelectionAmount );
	nInstDamage           = buteMgr.GetInt( aTagName, WMGR_AMMO_INSTDAMAGE, nInstDamage );
	pszValue = buteMgr.GetString( aTagName, WMGR_AMMO_INSTDAMAGETYPE, "" );
	if( pszValue[0] )
		eInstDamageType       = StringToDamageType( pszValue );
	nAreaDamage           = buteMgr.GetInt( aTagName, WMGR_AMMO_AREADAMAGE, nAreaDamage );
	pszValue = buteMgr.GetString( aTagName, WMGR_AMMO_AREADAMAGETYPE, "" );
	if( pszValue[0] )
		eAreaDamageType       = StringToDamageType( pszValue );
	nAreaDamageRadius     = buteMgr.GetInt( aTagName, WMGR_AMMO_AREADAMAGERADIUS, nAreaDamageRadius );
	fFireRecoilMult       = (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_AMMO_FIRERECOILMULT, fFireRecoilMult );
	fProgDamage           = (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_AMMO_PROGDAMAGE, fProgDamage );
	pszValue = buteMgr.GetString( aTagName, WMGR_AMMO_PROGDAMAGETYPE, "" );
	if( pszValue[0] )
		eProgDamageType       = StringToDamageType( pszValue );
	fProgDamageDuration   = (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_AMMO_PROGDAMAGEDUR, fProgDamageDuration );
	fProgDamageRadius     = (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_AMMO_PROGDAMAGERADIUS, fProgDamageRadius );
	fProgDamageLifetime   = (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_AMMO_PROGDAMAGELIFE, fProgDamageLifetime );
	bCanBeDeflected       = !!buteMgr.GetInt( aTagName, WMGR_AMMO_CANBEDEFLECTED, bCanBeDeflected );
	bCanAdjustInstDamage  = !!buteMgr.GetInt( aTagName, WMGR_AMMO_CANADJUSTINSTDAMAGE, bCanAdjustInstDamage );


	GetStringIfExist( buteMgr, aTagName, WMGR_AMMO_ICON, &szIcon, WMGR_MAX_FILE_PATH );


	int nId = 0;
	char szStr[128] = "";
	buteMgr.GetString(aTagName, WMGR_AMMO_IMPACTFX, "", szStr, sizeof(szStr));
	if( buteMgr.Success( ))
	{
		if (szStr[0])
		{
			pImpactFX = g_pFXButeMgr->GetImpactFX(szStr);
		}
	}

	buteMgr.GetString(aTagName, WMGR_AMMO_UWIMPACTFX, "", szStr, sizeof(szStr));
	if( buteMgr.Success( ))
	{
		if (szStr[0])
		{
			pUWImpactFX = g_pFXButeMgr->GetImpactFX(szStr);
		}
	}

	buteMgr.GetString(aTagName, WMGR_AMMO_MOVEABLEIMPACTOVERRIDEFX, "", szStr, sizeof(szStr));
	if( buteMgr.Success( ))
	{
		if (szStr[0])
		{
			pMoveableImpactOverrideFX = g_pFXButeMgr->GetImpactFX(szStr);
		}
	}

	buteMgr.GetString(aTagName, WMGR_AMMO_RICOCHETFX, "", szStr, sizeof(szStr));
	if( buteMgr.Success( ))
	{
		if (szStr[0])
		{
			pRicochetFX = g_pFXButeMgr->GetImpactFX(szStr);
		}
	}


	buteMgr.GetString(aTagName, WMGR_AMMO_BLOCKEDFX, "", szStr, sizeof(szStr));
	if( buteMgr.Success( ))
	{
		if (szStr[0])
		{
			pBlockedFX = g_pFXButeMgr->GetImpactFX(szStr);
		}
	}

	buteMgr.GetString(aTagName, WMGR_AMMO_DEFLECTSUFRACETYPE, "", szStr, sizeof(szStr));
	if( buteMgr.Success( ))
	{
		if (szStr[0])
		{
			sDeflectSurfaceType = szStr;
		}
	}

	buteMgr.GetString(aTagName, WMGR_AMMO_PROJECTILEFX, "", szStr, sizeof(szStr));
	if( buteMgr.Success( ))
	{
		if (szStr[0])
		{
			pProjectileFX = g_pFXButeMgr->GetProjectileFX(szStr);
		}
	}

	buteMgr.GetString(aTagName, WMGR_AMMO_FIREFX, "", szStr, sizeof(szStr));
	if( buteMgr.Success( ))
	{
		if (szStr[0])
		{
			pFireFX = g_pFXButeMgr->GetFireFX(szStr);
		}
	}

	buteMgr.GetString(aTagName, WMGR_AMMO_ANIOVERRIDE, "", szStr,sizeof(szStr));
	if( buteMgr.Success( ))
	{
		if (szStr[0])
		{
			pAniOverrides = g_pWeaponMgr->GetWeaponAnis(szStr);
		}
	}

	buteMgr.GetString(aTagName, WMGR_AMMO_TRACERFX, "", szStr, sizeof(szStr));
	if( buteMgr.Success( ))
	{
		if (szStr[0])
		{
			pTracerFX = g_pFXButeMgr->GetTracerFX(szStr);
		}
	}

	bCanServerRestrict = !!( buteMgr.GetInt( aTagName, WMGR_ALL_CANSERVERRESTRICT, bCanServerRestrict ));
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
	ASSERT( LTFALSE == bInited );
	if( bInited ) return LTFALSE;

	bInited = LTTRUE;

	// We only want to get the name of this ammo since it cannot be overriden

	szName	= GetString( buteMgr, aTagName, WMGR_AMMO_NAME, WMGR_MAX_NAME_LENGTH );

	// Initialize all other values...

	InitMembers( buteMgr, aTagName );

#ifdef _CLIENTBUILD
	char szTmp[128] = "";
	LoadString(nNameId,szTmp,sizeof(szTmp));

	char* pTok = strtok(szTmp,"@");

	if (pTok)
	{
		int l = strlen(pTok)+1;
		szLongName = debug_newa( char, l);
		LTStrCpy(szLongName,pTok,l);
	}
	else
	{
		szLongName = debug_newa( char, 1);
		szLongName[0] = NULL;
	}

	pTok = strtok(NULL,"@");

	if (pTok)
	{
		int l = strlen(pTok)+1;
		szShortName = debug_newa( char, l);
		LTStrCpy(szShortName,pTok,l);
	}
	else
	{
		int l = strlen(szLongName)+1;
		szShortName = debug_newa( char, l);
		LTStrCpy(szShortName,szLongName,l);
	}
#else
	int l = strlen(szName)+1;
	szLongName = debug_newa( char, l);
	LTStrCpy(szLongName,szName,l);
	szShortName = debug_newa( char, l);
	LTStrCpy(szShortName,szName,l);

#endif

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AMMO::Override
//
//  PURPOSE:	Override the current data with props from the bute, if listed
//
// ----------------------------------------------------------------------- //

LTBOOL AMMO::Override( CButeMgr &buteMgr, char *aTagName )
{
	ASSERT( bInited == LTTRUE );
	if( !aTagName || !bInited ) return LTFALSE;

	// ReInit any members we wish to override, If a member is not being overriden it stays at is initial value
	InitMembers( buteMgr, aTagName );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO::GetMaxAmount()
//
//	PURPOSE:	Calculate max amount of ammo of this type allowed for
//				the character
//
// ----------------------------------------------------------------------- //

int AMMO::GetMaxAmount(HOBJECT hCharacter) const
{
	int nMaxAmmo = nMaxAmount;

#ifdef _CLIENTBUILD

	LTFLOAT fMult = g_pPlayerStats->GetSkillModifier(SKL_CARRY,CarryModifiers::eMaxAmmo);
    nMaxAmmo =  (int) ( fMult * (LTFLOAT)nMaxAmmo );

#else

	if (IsPlayer(hCharacter))
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hCharacter);
        LTFLOAT fMult = pPlayer->GetPlayerSkills()->GetSkillModifier(SKL_CARRY,CarryModifiers::eMaxAmmo);
        nMaxAmmo =  (int) ( fMult * (LTFLOAT)nMaxAmmo );
	};

#endif

	return nMaxAmmo;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AMMO::Get???Icon
//
//	PURPOSE:	Build the ammo icon name
//
// ----------------------------------------------------------------------- //

std::string	AMMO::GetNormalIcon() const
{
	std::string str(szIcon);
	str += ".dtx";
	return str;
}
std::string	AMMO::GetDisabledIcon() const
{
	std::string str(szIcon);
	str += "D.dtx";
	return str;
}
std::string	AMMO::GetUnselectedIcon() const
{
	std::string str(szIcon);
	str += "U.dtx";
	return str;
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
    bInited = LTFALSE;

	nId		= WMGR_INVALID_ID;

	eType	= UNKNOWN_MOD_TYPE;

	szSocket			= LTNULL;
	szName				= LTNULL;
	szIcon				= LTNULL;
	szZoomInSound		= LTNULL;
	szZoomOutSound		= LTNULL;
	szAttachModel		= LTNULL;
	szPowerupModel		= LTNULL;
	szPickUpSound		= LTNULL;
	szRespawnSound		= LTNULL;
	szPowerupFX			= LTNULL;
	szRespawnWaitFX		= LTNULL;

    fPowerupScale		=	1.0f;
	vAttachScale.Init(1, 1, 1);

	nNameId				= 0;
	nDescriptionId		= 0;
	nZoomLevel			= 0;
	nPriority			= 0;
	nAISilencedFireSndRadius = 256;

    bIntegrated         = LTFALSE;

	fScreenTintTime	= 0.0f;
 	vScreenTintColor.Init();

	bRespawnWaitVisible		= false;
	bRespawnWaitTranslucent	= false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	MOD::~MOD
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

MOD::~MOD()
{
    bInited = LTFALSE;

	debug_deletea( szSocket );
	debug_deletea( szName );
	debug_deletea( szIcon );
	debug_deletea( szZoomInSound );
	debug_deletea( szZoomOutSound );
	debug_deletea( szAttachModel );
	debug_deletea( szPowerupModel );
	debug_deletea( szPickUpSound );
	debug_deletea( szRespawnSound );
	debug_deletea( szPowerupFX );
	debug_deletea( szRespawnWaitFX );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	MOD::InitMembers
//
//  PURPOSE:	Actually set the member vars...
//				This allows us to ReInit members with multiple bute files
//
// ----------------------------------------------------------------------- //

void MOD::InitMembers( CButeMgr &buteMgr, char *aTagName )
{
	ASSERT( aTagName != NULL );

	// Use the members as the default value incase the attribute doesn't exist...

	nNameId				= buteMgr.GetInt( aTagName, WMGR_MOD_NAMEID, nNameId );
	nDescriptionId		= buteMgr.GetInt( aTagName, WMGR_MOD_DESCRIPTIONID, nDescriptionId );
	eType				= (ModType) buteMgr.GetInt( aTagName, WMGR_MOD_TYPE, eType );
	nZoomLevel			= buteMgr.GetInt( aTagName, WMGR_MOD_ZOOMLEVEL, nZoomLevel );
	nPriority			= buteMgr.GetInt( aTagName, WMGR_MOD_PRIORITY, nPriority );
    bIntegrated         = (LTBOOL) buteMgr.GetInt( aTagName, WMGR_MOD_INTEGRATED, bIntegrated );
	fScreenTintTime		= (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_MOD_TINT_TIME, fScreenTintTime );
	vScreenTintColor	= buteMgr.GetVector( aTagName, WMGR_MOD_TINT_COLOR, CAVector( VEC_EXPAND(vScreenTintColor) ));
	fPowerupScale		= (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_MOD_POWERUPSCALE, fPowerupScale );
	nAISilencedFireSndRadius = buteMgr.GetInt( aTagName, WMGR_MOD_SILENCESND_RADIUS, nAISilencedFireSndRadius );

	// The name is only set in the Init so we cannot override it!
	// DO NOT list name here.
	
	GetStringIfExist( buteMgr, aTagName, WMGR_MOD_SOCKET, &szSocket, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_MOD_ICON, &szIcon, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_MOD_ATTACHMODEL, &szAttachModel, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_MOD_ZOOMINSND, &szZoomInSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_MOD_ZOOMOUTSND, &szZoomOutSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_MOD_POWERUPMODEL, &szPowerupModel, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_MOD_PICKUPSND, &szPickUpSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_MOD_RESPAWNSND, &szRespawnSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_MOD_POWERUPFX, &szPowerupFX, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_MOD_RESPAWNWAITFX, &szRespawnWaitFX, WMGR_MAX_NAME_LENGTH );
	
		
	// The bute list reader will delete the list if one already exists
	// This means if one of the properties is going to be overwriten, all of them must be.

	blrAttachSkins.Read(&buteMgr, aTagName, WMGR_MOD_ATTACHSKIN, WMGR_MAX_FILE_PATH);
	blrAttachRenderStyles.Read(&buteMgr, aTagName, WMGR_MOD_ATTACHRENDERSTYLE, WMGR_MAX_FILE_PATH);

	vAttachScale = buteMgr.GetVector(aTagName, WMGR_MOD_ATTACHSCALE);

	blrPowerupSkins.Read(&buteMgr, aTagName, WMGR_MOD_POWERUPSKIN, WMGR_MAX_FILE_PATH);
	blrPowerupRenderStyles.Read(&buteMgr, aTagName, WMGR_MOD_POWERUPRENDERSTYLE, WMGR_MAX_FILE_PATH);

	blrRespawnWaitSkins.Read( &buteMgr, aTagName, WMGR_MOD_RESPAWNWAITSKIN, WMGR_MAX_FILE_PATH );
	blrRespawnWaitRenderStyles.Read( &buteMgr, aTagName, WMGR_MOD_RESPAWNWAITRENDERSTYLE, WMGR_MAX_FILE_PATH );

	bRespawnWaitVisible		= !!(buteMgr.GetInt( aTagName, WMGR_MOD_RESPAWNWAITVISIBLE, bRespawnWaitVisible ));
	bRespawnWaitTranslucent	= !!(buteMgr.GetInt( aTagName, WMGR_MOD_RESPAWNWAITTRANSLUCENT, bRespawnWaitTranslucent ));
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
    ASSERT( LTFALSE == bInited );
	if( bInited ) return LTFALSE;

    bInited = LTTRUE;

	// We only want to get the name of this weapon since it cannot be overriden

	szName = GetString( buteMgr, aTagName, WMGR_MOD_NAME, WMGR_MAX_NAME_LENGTH );

	// Initialize all other values...

	InitMembers( buteMgr, aTagName );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	MOD::Override
//
//  PURPOSE:	Allow the Mod to override it's data
//
// ----------------------------------------------------------------------- //

LTBOOL MOD::Override( CButeMgr &buteMgr, char *aTagName )
{
	ASSERT( bInited == LTTRUE );
	if( !aTagName || !bInited ) return LTFALSE;

	// ReInit any members we wish to override, If a member is not being overriden it stays at is initial value
	InitMembers( buteMgr, aTagName );

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MOD::GetWeaponId()
//
//	PURPOSE:	What is the weapon that uses this mod
//
// ----------------------------------------------------------------------- //

int MOD::GetWeaponId() const
{
    WEAPON const *pWeapon = LTNULL;

	for (int nWId = 0; nWId < g_pWeaponMgr->GetNumWeapons(); nWId++)
	{
		pWeapon = g_pWeaponMgr->GetWeapon(nWId);

		if (pWeapon)
		{
			for (int i=0; i < pWeapon->nNumModIds; i++)
			{
				if (pWeapon->aModIds[i] == nId)
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
    bInited = LTFALSE;

	nId		= WMGR_INVALID_ID;

	nNameId			= 0;
	nDescriptionId	= 0;

	szName				= LTNULL;
	szIcon				= LTNULL;
	szModel				= LTNULL;
	szPickUpSound		= LTNULL;
	szRespawnSound		= LTNULL;
	szPowerupFX			= LTNULL;
	szRespawnWaitFX		= LTNULL;

	eProtectionType	= DT_INVALID;
	fProtection		= 0.0f;
	fArmor			= 0.0f;
	fHealth			= 0.0f;
	fStealth		= 0.0f;
	fScreenTintTime	= 0.0f;
    bSelectable     = LTFALSE;
    bExclusive      = LTFALSE;

	vScreenTintColor.Init();

	bRespawnWaitVisible		= false;
	bRespawnWaitTranslucent	= false;

	bServerRestricted = false;
	bCanServerRestrict = true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GEAR::~GEAR
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

GEAR::~GEAR()
{
    bInited = LTFALSE;

	debug_deletea( szName );
	debug_deletea( szIcon );
	debug_deletea( szModel );
	debug_deletea( szPickUpSound );
	debug_deletea( szRespawnSound );
	debug_deletea( szPowerupFX );
	debug_deletea( szRespawnWaitFX );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GEAR::InitMembers
//
//  PURPOSE:	Actually set the member vars...
//				This allows us to ReInit members with multiple bute files
//
// ----------------------------------------------------------------------- //

void GEAR::InitMembers( CButeMgr &buteMgr, char *aTagName )
{
	ASSERT( aTagName != LTNULL );

	// Use the members as the default value incase the attribute doesn't exist...
	char const* pszValue = "";

	nNameId				= buteMgr.GetInt( aTagName, WMGR_GEAR_NAMEID, nNameId );
	nDescriptionId		= buteMgr.GetInt( aTagName, WMGR_GEAR_DESCRIPTIONID, nDescriptionId );
	pszValue = buteMgr.GetString( aTagName, WMGR_GEAR_PROTECTTYPE, "" );
	if( pszValue[0] )
		eProtectionType       = StringToDamageType( pszValue );
    bSelectable			= (LTBOOL) buteMgr.GetInt( aTagName, WMGR_GEAR_SELECTABLE, bSelectable );
    bExclusive			= (LTBOOL) buteMgr.GetInt( aTagName, WMGR_GEAR_EXCLUSIVE, bExclusive );
    fProtection			= (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_GEAR_PROTECTION, fProtection );
    fArmor				= (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_GEAR_ARMOR, fArmor );
    fHealth				= (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_GEAR_HEALTH, fHealth );
    fStealth			= (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_GEAR_STEALTH, fStealth );
    fScreenTintTime		= (LTFLOAT) buteMgr.GetDouble( aTagName, WMGR_GEAR_TINT_TIME, fScreenTintTime );

	fProtection			= (fProtection < 0.0f ? 0.0f : (fProtection > 1.0f ? 1.0f : fProtection));
	fStealth			= (fStealth < 0.0f ? 0.0f : (fStealth > 1.0f ? 1.0f : fStealth));

	vScreenTintColor	= buteMgr.GetVector( aTagName, WMGR_GEAR_TINT_COLOR, CAVector( VEC_EXPAND( vScreenTintColor ) ));


	// The name is only set in the Init so we cannot override it!
	// DO NOT list name here.

	GetStringIfExist( buteMgr, aTagName, WMGR_GEAR_ICON, &szIcon, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_GEAR_MODEL, &szModel, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_GEAR_PICKUPSND, &szPickUpSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_GEAR_RESPAWNSND, &szRespawnSound, WMGR_MAX_FILE_PATH );
	GetStringIfExist( buteMgr, aTagName, WMGR_GEAR_POWERUPFX, &szPowerupFX, WMGR_MAX_NAME_LENGTH );
	GetStringIfExist( buteMgr, aTagName, WMGR_GEAR_RESPAWNWAITFX, &szRespawnWaitFX, WMGR_MAX_NAME_LENGTH );


	// The bute list reader will delete the list if one already exists
	// This means if one of the properties is going to be overwriten, all of them must be.

	blrSkins.Read(&buteMgr, aTagName, WMGR_GEAR_SKIN, WMGR_MAX_FILE_PATH);
	blrRenderStyles.Read(&buteMgr, aTagName, WMGR_GEAR_RENDERSTYLE, WMGR_MAX_FILE_PATH);

	blrRespawnWaitSkins.Read( &buteMgr, aTagName, WMGR_GEAR_RESPAWNWAITSKIN, WMGR_MAX_FILE_PATH );
	blrRespawnWaitRenderStyles.Read( &buteMgr, aTagName, WMGR_GEAR_RESPAWNWAITRENDERSTYLE, WMGR_MAX_FILE_PATH );

	bRespawnWaitVisible		=!!(buteMgr.GetInt( aTagName, WMGR_MOD_RESPAWNWAITVISIBLE, bRespawnWaitVisible ));
	bRespawnWaitTranslucent	= !!(buteMgr.GetInt( aTagName, WMGR_GEAR_RESPAWNWAITTRANSLUCENT, bRespawnWaitTranslucent ));

	bCanServerRestrict = !!( buteMgr.GetInt( aTagName, WMGR_ALL_CANSERVERRESTRICT, bCanServerRestrict ));
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
    ASSERT( LTFALSE == bInited );
	if( bInited	) return LTFALSE;

    bInited = LTTRUE;

	// We only want to get the name of this weapon since it cannot be overriden

	szName = GetString( buteMgr, aTagName, WMGR_GEAR_NAME, WMGR_MAX_NAME_LENGTH );

	// Initialize all other values...

	InitMembers( buteMgr, aTagName );

	return LTTRUE;
}

	
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GEAR::Override
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //
	
LTBOOL GEAR::Override( CButeMgr &buteMgr, char *aTagName )
{
	ASSERT( bInited == LTTRUE );
	if( !aTagName || !bInited ) return LTFALSE;

	// ReInit any members we wish to override, If a member is not being overriden it stays at is initial value
	InitMembers( buteMgr, aTagName );

    return LTTRUE;
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

		WEAPON const *pWeapon = GetWeapon(strtok(genProp.m_String,","));
		if (pWeapon)
		{
			nWeaponId = pWeapon->nId;
		}

		AMMO const *pAmmo = GetAmmo(strtok(NULL,""));
		if (pAmmo)
		{
			nAmmoId = pAmmo->nId;
		}
		else
		{
			// Use the default ammo type for the weapon...

			if (pWeapon)
			{
				nAmmoId = pWeapon->nDefaultAmmoId;
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

	WEAPON const *pWeapon = GetWeapon(strtok(szString,","));
	if (pWeapon)
	{
		nWeaponId = pWeapon->nId;
	}

	AMMO const *pAmmo = GetAmmo(strtok(NULL,""));
	if (pAmmo)
	{
		nAmmoId = pAmmo->nId;
	}
	else
	{
		// Use the default ammo type for the weapon...

		if (pWeapon)
		{
			nAmmoId = pWeapon->nDefaultAmmoId;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//
// CWeaponMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
////////////////////////////////////////////////////////////////////////////

CWeaponMgrPlugin::CWeaponMgrPlugin()
{
	m_pFXButeMgrPlugin = debug_new(CFXButeMgrPlugin);
}

/*virtual*/ CWeaponMgrPlugin::~CWeaponMgrPlugin()
{
	if ( m_pFXButeMgrPlugin )
	{
		debug_delete(m_pFXButeMgrPlugin);
		m_pFXButeMgrPlugin = NULL;
	}
}


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
		m_pFXButeMgrPlugin->PreHook_EditStringList(szRezPath, szPropName,
			aszStrings,	pcStrings, cMaxStrings, cMaxStringLength);

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, WEAPON_DEFAULT_FILE);
		CWeaponMgr& weaponMgr = CWeaponMgr::Instance( );
        weaponMgr.SetInRezFile(LTFALSE);
        weaponMgr.Init(szFile);
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
	if (!aszStrings || !pcStrings || !g_pWeaponMgr)
	{
		ASSERT( !"CWeaponMgrPlugin::PopulateStringList: Invalid input parameters" );
		return;
	}

	// Add an entry for each weapon/ammo combination...

    uint32 dwNumWeapons = g_pWeaponMgr->GetNumWeapons();

    WEAPON const *pWeapon = LTNULL;

    for (uint32 i=0; i < dwNumWeapons; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pWeapon = g_pWeaponMgr->GetWeapon(i);
        uint32 dwWeaponNameLen = strlen(pWeapon->szName);

		if (pWeapon && pWeapon->szName[0] &&
			dwWeaponNameLen < cMaxStringLength &&
			(*pcStrings) + 1 < cMaxStrings)
		{
			// Account for the ';' character

			dwWeaponNameLen++;

			// Append the ammo types to the string if there is more
			// than one ammo type...

			if (pWeapon->nNumAmmoIds > 1)
			{
				for (int j=0; j < pWeapon->nNumAmmoIds; j++)
				{
					_ASSERT(cMaxStrings > (*pcStrings) + 1);

					AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->aAmmoIds[j]);
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
