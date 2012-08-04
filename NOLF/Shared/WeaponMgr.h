// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponMgr.h
//
// PURPOSE : WeaponMgr definition - Controls attributes of all weapons
//
// CREATED : 12/02/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_MGR_H__
#define __WEAPON_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "DamageTypes.h"
#include "CommandIds.h"
#include "TemplateList.h"
#include "FXButeMgr.h"

class CWeaponMgr;
extern CWeaponMgr* g_pWeaponMgr;

#define WEAPON_MIN_IDLE_TIME		5.0f
#define WEAPON_MAX_IDLE_TIME		15.0f
#define WEAPON_SOUND_RADIUS			2000.0f
#define WEAPON_KEY_FIRE				"FIRE_KEY"
#define WEAPON_KEY_SOUND			"SOUND_KEY"
#define WEAPON_KEY_FX				"FX_KEY"
#define WEAPON_KEY_FIREFX			"FIREFX_KEY"
#define WEAPON_DEFAULT_FILE			"Attributes\\Weapons.txt"

#define WMGR_INVALID_ID				255

#define WMGR_MAX_NAME_LENGTH		32
#define WMGR_MAX_FILE_PATH			64
#define WMGR_MAX_AMMO_TYPES			10
#define WMGR_MAX_MOD_TYPES			6
#define WMGR_MAX_RELOAD_SNDS		3
#define WMGR_MAX_PVFX_TYPES			5
#define WMGR_MAX_WEAPONANI_IDLE		3
#define WMGR_MAX_WEAPONANI_FIRE		3

enum WeaponState
{
	W_IDLE,
	W_BEGIN_FIRING,
	W_FIRING,
	W_FIRED,
	W_END_FIRING,
	W_RELOADING,
	W_FIRING_NOAMMO,
	W_SELECT,
	W_DESELECT
};

enum AmmoType
{
	// NOTE:  These values 0, 1, 3 are important.  The Weapons.txt file
	// refers to these numbers, DO NOT change them!
	VECTOR=0,
	PROJECTILE=1,
	GADGET=3,
	UNKNOWN_AMMO_TYPE=4
};

enum ModType
{
	SILENCER=0,
	LASER,
	SCOPE,
	UNKNOWN_MOD_TYPE
};


struct MOD
{
	MOD();

    LTBOOL  Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CWeaponMgr* pWeaponMgr);

	int		GetWeaponId(); // CWeaponMgr::GetWeaponIdFromModId(int nModId)

	int		nId;

	ModType	eType;

	char	szSocket[WMGR_MAX_NAME_LENGTH];
	char	szName[WMGR_MAX_NAME_LENGTH];
	char	szIcon[WMGR_MAX_FILE_PATH];
	char	szSmallIcon[WMGR_MAX_FILE_PATH];
	char	szZoomInSound[WMGR_MAX_FILE_PATH];
	char	szZoomOutSound[WMGR_MAX_FILE_PATH];
	char	szAttachModel[WMGR_MAX_FILE_PATH];
	char	szAttachSkin[WMGR_MAX_FILE_PATH];
	char	szInterfaceModel[WMGR_MAX_FILE_PATH];
	char	szInterfaceSkin[WMGR_MAX_FILE_PATH];
	char	szPowerupModel[WMGR_MAX_FILE_PATH];
	char	szPowerupSkin[WMGR_MAX_FILE_PATH];
	char	szPickUpSound[WMGR_MAX_FILE_PATH];
	char	szRespawnSound[WMGR_MAX_FILE_PATH];

    LTVector vAttachScale;

	LTVector    vScreenTintColor;
    LTFLOAT     fScreenTintTime;

	int		nNameId;
	int		nDescriptionId;
	int		nZoomLevel;
	int		nPriority;

    LTBOOL   bNightVision;
    LTBOOL   bIntegrated;
    LTFLOAT	 fInterfaceScale;
    LTFLOAT	 fPowerupScale;
    LTVector vInterfaceOffset;
};

typedef CTList<MOD*> ModList;


struct GEAR
{
	GEAR();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CWeaponMgr* pWeaponMgr);

	int			nId;

	int			nNameId;
	int			nDescriptionId;

	char		szName[WMGR_MAX_NAME_LENGTH];
	char		szIcon[WMGR_MAX_FILE_PATH];
	char		szSmallIcon[WMGR_MAX_FILE_PATH];
	char		szModel[WMGR_MAX_FILE_PATH];
	char		szSkin[WMGR_MAX_FILE_PATH];
	char		szInterfaceModel[WMGR_MAX_FILE_PATH];
	char		szInterfaceSkin[WMGR_MAX_FILE_PATH];
	char		szPickUpSound[WMGR_MAX_FILE_PATH];
	char		szRespawnSound[WMGR_MAX_FILE_PATH];

	DamageType	eProtectionType;
    LTFLOAT     fProtection;
    LTFLOAT     fArmor;
    LTFLOAT     fStealth;
    LTBOOL      bSelectable;
    LTBOOL      bExclusive;
    LTVector    vScreenTintColor;
    LTFLOAT     fScreenTintTime;
    LTFLOAT		fInterfaceScale;
    LTVector	vInterfaceOffset;
};

typedef CTList<GEAR*> GearList;

// This struct is used to contain all the animation overrides for a particular
// ammo type...

struct WEAPONANIS
{
	WEAPONANIS();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	int		nId;
	char	szName[WMGR_MAX_NAME_LENGTH];

	char	szSelectAni[WMGR_MAX_NAME_LENGTH];
	char	szDeselectAni[WMGR_MAX_NAME_LENGTH];
	char	szReloadAni[WMGR_MAX_NAME_LENGTH];

	int		nNumIdleAnis;
	char	szIdleAnis[WMGR_MAX_WEAPONANI_IDLE][WMGR_MAX_NAME_LENGTH];

	int		nNumFireAnis;
	char	szFireAnis[WMGR_MAX_WEAPONANI_FIRE][WMGR_MAX_NAME_LENGTH];
};

typedef CTList<WEAPONANIS*> WeaponAnisList;

struct AMMO
{
	AMMO();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CWeaponMgr* pWeaponMgr);

	int			nId;

	int			nNameId;
	int			nDescId;
	AmmoType	eType;

	LTFLOAT		fPriority;

	char		szIcon[WMGR_MAX_FILE_PATH];
	char		szSmallIcon[WMGR_MAX_FILE_PATH];
	char		szName[WMGR_MAX_NAME_LENGTH];

	int			GetMaxAmount(HOBJECT hCharacter);
	int			nSpawnedAmount;
	int			nSelectionAmount;

	int			nInstDamage;
	DamageType	eInstDamageType;

	int			nAreaDamage;
	int			nAreaDamageRadius;
	DamageType	eAreaDamageType;

	float		fFireRecoilMult;

    LTFLOAT      fProgDamage;
    LTFLOAT      fProgDamageDuration;
    LTFLOAT      fProgDamageRadius;
    LTFLOAT      fProgDamageLifetime;
	DamageType	eProgDamageType;

	PROJECTILEFX*	pProjectileFX;	// Points at CFXButeMgr::m_ProjectileFXList element
	IMPACTFX*		pImpactFX;		// Points at CFXButeMgr::m_ImpactFXList element
	IMPACTFX*		pUWImpactFX;	// Points at CFXButeMgr::m_ImpactFXList element
	FIREFX*			pFireFX;		// Points at CFXButeMgr::m_FireFXList element
	WEAPONANIS*		pAniOverrides;	// Points at m_WeaponAnisList element
	TRACERFX*		pTracerFX;		// Points at CFXButeMgr::m_TracerFXList element

private:
	int			nMaxAmount;
};

typedef CTList<AMMO*> AmmoList;


struct WEAPON
{
	WEAPON();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CWeaponMgr* pWeaponMgr);

    LTBOOL   IsAGadget() {return bGadget;}

	void	Save(CButeMgr & buteMgr);

	int		nId;

	int		nNameId;
	int		nDescriptionId;
    LTBOOL   bGadget;
	int		nAniType;

	char	szName[WMGR_MAX_NAME_LENGTH];
	char	szIcon[WMGR_MAX_FILE_PATH];
	char	szSmallIcon[WMGR_MAX_FILE_PATH];
	char	szPhoto[WMGR_MAX_FILE_PATH];

    LTVector vPos;
    LTVector vMuzzlePos;

	char	szPVModel[WMGR_MAX_FILE_PATH];
	char	szPVSkin[WMGR_MAX_FILE_PATH];
	char	szHHModel[WMGR_MAX_FILE_PATH];
	char	szHHSkin[WMGR_MAX_FILE_PATH];
	char	szInterfaceModel[WMGR_MAX_FILE_PATH];
	char	szInterfaceSkin[WMGR_MAX_FILE_PATH];

    LTVector vHHScale;
    LTFLOAT	 fInterfaceScale;
    LTVector vInterfaceOffset;
    LTFLOAT  fHHBreachOffset;

	char	szSilencedFireSound[WMGR_MAX_FILE_PATH];
	char	szAltFireSound[WMGR_MAX_FILE_PATH];
	char	szFireSound[WMGR_MAX_FILE_PATH];
	char	szDryFireSound[WMGR_MAX_FILE_PATH];
	char	szReloadSounds[WMGR_MAX_RELOAD_SNDS][WMGR_MAX_FILE_PATH];
	char	szSelectSound[WMGR_MAX_FILE_PATH];
	char	szDeselectSound[WMGR_MAX_FILE_PATH];

	int		nFireSoundRadius;
	int		nAIFireSoundRadius;

    LTBOOL  bEnvironmentMap;
    LTBOOL  bInfiniteAmmo;
    LTBOOL  bLooksDangerous;
    LTBOOL  bCanBeDefault;
    LTBOOL  bCanBeMapped;
	LTBOOL	bHideWhenEmpty;

	int		nShotsPerClip;

	int		nNumAmmoTypes;
	int		aAmmoTypes[WMGR_MAX_AMMO_TYPES];

	int		nDefaultAmmoType;
	int		nAmmoMultiplier;

	int		nNumModTypes;
	int		aModTypes[WMGR_MAX_MOD_TYPES];

	int		nNumPVFXTypes;
	int		aPVFXTypes[WMGR_MAX_PVFX_TYPES];

	int		nMinPerturb;
	int		nMaxPerturb;

	int		nRange;
    LTVector vRecoil;
	int		nVectorsPerRound;

	float	fAIMinBurstInterval;
	float	fAIMaxBurstInterval;
	int		nAIMinBurstShots;
	int		nAIMaxBurstShots;

	float	fFireRecoilPitch;
	float	fFireRecoilDecay;

	CMuzzleFX*	pPVMuzzleFX;	// Player-view muzzle fx
	CMuzzleFX*	pHHMuzzleFX;	// Hand-held muzzle fx

	uint32	m_nFireDelay;		// Minimum time (ms) between firing
};

typedef CTList<WEAPON*> WeaponList;


struct WeaponPath
{
	WeaponPath()
	{
		vPath.Init();
		vR.Init();
		vU.Init();
		nWeaponId = WMGR_INVALID_ID;
		fPerturbU = 1.0f;	// 0.0 - 1.0
		fPerturbR = 1.0f;	// 0.0 - 1.0
	}

    LTVector vPath;
    LTVector vU;
    LTVector vR;
    uint8   nWeaponId;
    LTFLOAT  fPerturbU;
    LTFLOAT  fPerturbR;
};

class CWeaponMgr : public CGameButeMgr
{
	public :

		CWeaponMgr();
		~CWeaponMgr();

#ifndef _CLIENTBUILD
        LTBOOL      ReadWeaponProp(char* pPropName, uint8 & m_nWeaponId, uint8 & m_nAmmoId);
        void        ReadWeapon(char* szString, uint8 & nWeaponId, uint8 & nAmmoId);
#endif // _CLIENTBUILD

        LTBOOL      Init(ILTCSBase *pInterface, const char* szAttributeFile=WEAPON_DEFAULT_FILE);
		void		Term();

		void		CacheAll();
		void		CacheWeapon(int nWeaponId);

		void		CalculateWeaponPath(WeaponPath & wp);

        LTBOOL      WriteFile(ILTCSBase *pInterface);
        void        Reload(ILTCSBase *pInterface);

		int			GetNumWeapons()		const { return m_WeaponList.GetLength(); }
		int			GetNumAmmoTypes()	const { return m_AmmoList.GetLength();; }
		int			GetNumModTypes()	const { return m_ModList.GetLength();; }
		int			GetNumGearTypes()	const { return m_GearList.GetLength();; }

        LTBOOL      IsValidWeapon(int nWeaponId);
        LTBOOL      IsValidAmmoType(int nAmmoId);
        LTBOOL      IsValidModType(int nModId);
        LTBOOL      IsValidGearType(int nGearId);

		LTBOOL		IsPlayerWeapon(int nWeaponId);

		int			GetWeaponId(int nCommandId);
		int			GetCommandId(int nWeaponId);
		int			GetFirstWeaponCommandId();
		int			GetLastWeaponCommandId();

		WEAPON*		GetWeapon(int nWeaponId);
		WEAPON*		GetWeapon(char* pWeaponName);

		AMMO*		GetAmmo(int nAmmoId);
		AMMO*		GetAmmo(char* pAmmoName);

		MOD*		GetMod(int nModId);
		MOD*		GetMod(char* pModName);

		GEAR*		GetGear(int nGearId);
		GEAR*		GetGear(char* pGearName);

		WEAPONANIS*	GetWeaponAnis(char* pAnisName);

		// Left in for backwards compatibility...

        CScaleFX*    GetScaleFX(int nScaleFXId)      { return g_pFXButeMgr->GetScaleFX(nScaleFXId); }
        PEXPLFX*     GetPExplFX(int nPExplFXId)      { return g_pFXButeMgr->GetPExplFX(nPExplFXId); }
        DLIGHTFX*    GetDLightFX(int nDLightFXId)    { return g_pFXButeMgr->GetDLightFX(nDLightFXId); }

        IMPACTFX*        GetImpactFX(int nImpactFXId)    { return g_pFXButeMgr->GetImpactFX(nImpactFXId); }
        PROJECTILEFX*    GetProjectileFX(int nProjFXId)  { return g_pFXButeMgr->GetProjectileFX(nProjFXId); }
        FIREFX*          GetFireFX(int nFireFXId)        { return g_pFXButeMgr->GetFireFX(nFireFXId); }
		
		uint32		GetFileCRC() { return m_nFileCRC; }

	protected :

		WeaponList			m_WeaponList;		// All weapon types
		AmmoList			m_AmmoList;			// All ammo types
		ModList				m_ModList;			// All mod types
		GearList			m_GearList;			// All gear types
		WeaponAnisList		m_WeaponAnisList;	// Weapon Ani overrides

		int*				m_pWeaponOrder;			// Order of weapon selection
		int					m_nFirstPlayerWeapon;	// First weapon player can use
		int					m_nLastPlayerWeapon;	// Last weapon player can use

		uint32				m_nFileCRC;
};


// Map a commandid to a weapon id...

inline int CWeaponMgr::GetWeaponId(int nCommandId)
{
	int nId = nCommandId - COMMAND_ID_WEAPON_BASE;
	if (!m_pWeaponOrder || nId < 0 || nId >= m_WeaponList.GetLength())
	{
		return WMGR_INVALID_ID;
	}

	return m_pWeaponOrder[nId];
}

inline int CWeaponMgr::GetFirstWeaponCommandId()
{
	//return COMMAND_ID_WEAPON_BASE;
	return COMMAND_ID_WEAPON_BASE + m_nFirstPlayerWeapon;
}

inline int CWeaponMgr::GetLastWeaponCommandId()
{
	//return COMMAND_ID_WEAPON_BASE + m_WeaponList.GetLength() - 1;
	return COMMAND_ID_WEAPON_BASE + m_nFirstPlayerWeapon + m_nLastPlayerWeapon;
}

inline LTBOOL CWeaponMgr::IsPlayerWeapon(int nWeaponId)
{
	int nCommandId = GetCommandId(nWeaponId);
	return (nCommandId >= GetFirstWeaponCommandId() && nCommandId <= GetLastWeaponCommandId());
}


// Map weapon id to command id...

inline int CWeaponMgr::GetCommandId(int nWeaponId)
{
	int nListLength = m_WeaponList.GetLength();

	if (!m_pWeaponOrder || nWeaponId < 0 || nWeaponId >= nListLength)
	{
		return WMGR_INVALID_ID;
	}

	for (int i=0; i < nListLength; i++)
	{
		if (m_pWeaponOrder[i] == nWeaponId)
		{
			return COMMAND_ID_WEAPON_BASE + i;
		}
	}

	return WMGR_INVALID_ID;
}



inline LTBOOL CWeaponMgr::IsValidWeapon(int nWeaponId)
{
    return (LTBOOL) (GetWeapon(nWeaponId) != LTNULL);
}

inline LTBOOL CWeaponMgr::IsValidAmmoType(int nAmmoId)
{
    return (LTBOOL) (GetAmmo(nAmmoId) != LTNULL);
}

inline LTBOOL CWeaponMgr::IsValidModType(int nModId)
{
    return (LTBOOL) (GetMod(nModId) != LTNULL);
}

inline LTBOOL CWeaponMgr::IsValidGearType(int nGearId)
{
    return (LTBOOL) (GetGear(nGearId) != LTNULL);
}

inline LTBOOL FiredWeapon(WeaponState eState)
{
	return (eState == W_FIRED);
}


////////////////////////////////////////////////////////////////////////////
//
// CWeaponMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CWeaponMgrPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

        void PopulateStringList(char** aszStrings, uint32* pcStrings,
            const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		CFXButeMgrPlugin	m_FXButeMgrPlugin;

        static LTBOOL            sm_bInitted;
		static CWeaponMgr		sm_ButeMgr;
};

#endif // _CLIENTBUILD


#endif // __WEAPON_MGR_H__