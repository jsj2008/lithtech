// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponMgr.h
//
// PURPOSE : WeaponMgr definition - Controls attributes of all weapons
//
// CREATED : 12/02/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_MGR_H__
#define __WEAPON_MGR_H__

#include "ltbasetypes.h"
#include "DamageTypes.h"
#include "CommandIds.h"
#include "TemplateList.h"
#include "GameButeMgr.h"
#include "ButeListReader.h"

class CWeaponMgr;
class CFXButeMgrPlugin;

struct PROJECTILEFX;
struct IMPACTFX;
struct IMPACTFX;
struct IMPACTFX;
struct IMPACTFX;
struct FIREFX;
struct WEAPONANIS;
struct TRACERFX;
struct CMuzzleFX;
struct CScaleFX;
struct PEXPLFX;
struct DLIGHTFX;
struct IMPACTFX;
struct PROJECTILEFX;
struct FIREFX;

extern CWeaponMgr* g_pWeaponMgr;

#define WEAPON_MIN_IDLE_TIME				5.0f
#define WEAPON_MAX_IDLE_TIME				15.0f
#define WEAPON_SOUND_RADIUS					2000.0f
#define WEAPON_KEY_FIRE						"FIRE_KEY"
#define WEAPON_KEY_SOUND					"SOUND_KEY"
#define WEAPON_KEY_BUTE_SOUND				"BUTE_SOUND_KEY"
#define WEAPON_KEY_LOOPSOUND				"LOOP_SOUND_KEY"
#define WEAPON_KEY_FX						"FX"
#define WEAPON_KEY_FIREFX					"FIREFX_KEY"
#define WEAPON_KEY_FLASHLIGHT				"FLASHLIGHT"
#define WEAPON_KEY_DEFLECT					"DEFLECT"
#define WEAPON_KEY_HIDE_MODEL_PIECE			"HIDE_PIECE_KEY"  // usage: HIDE_PIECE_KEY <piece name>
#define WEAPON_KEY_SHOW_MODEL_PIECE			"SHOW_PIECE_KEY"  // usage: SHOW_PIECE_KEY <piece name>
#define WEAPON_KEY_SHELLCASING				"SHELL_CASING"
#define WEAPON_KEY_HIDE_PVATTACHFX			"HIDE_PVATTACHFX"	// usage: HIDE_PVFX <index of a PVAttachFXName#>
#define WEAPON_KEY_SHOW_PVATTACHFX			"SHOW_PVATTACHFX"	// usage: SHOW_PVFX <index of a PVAttachFXName#>
#define WEAPON_KEY_HIDE_PVATTACHMENT		"HIDE_PVATTACHMENT" // usage: HIDE_PVATTACHMENT <index of a PlayerViewAttachment>
#define WEAPON_KEY_SHOW_PVATTACHMENT		"SHOW_PVATTACHMENT" // usage: SHOW_PVATTACHMENT <index of a PlayerViewAttachment>


#define WEAPON_DEFAULT_FILE					"Attributes\\Weapons.txt"
#define WEAPON_DEFAULT_MULTI_FILE			"Attributes\\mp_weapons.txt"

#define WMGR_INVALID_ID						255

#define WMGR_MAX_NAME_LENGTH				32
#define WMGR_MAX_FILE_PATH					64
#define WMGR_MAX_AMMO_IDS					10
#define WMGR_MAX_MOD_IDS					6
#define WMGR_MAX_RELOAD_SNDS				3
#define WMGR_MAX_MISC_SNDS					5
#define WMGR_MAX_PVFX_TYPES					5
#define WMGR_MAX_PVCLIENTFX_ATTACHMENTS		10
#define WMGR_MAX_WEAPONANI_IDLE				3
#define WMGR_MAX_WEAPONANI_FIRE				3


// Helper function for allocating memory for a string property and returning a pointer to the new string
inline char* GetString( CButeMgr & buteMgr, const char* szTagName, const char* szAttName, uint32 dwMaxLen, char* defVal = "" )
{
	static char	szTemp[WMGR_MAX_FILE_PATH] = { '\0' };
	char		*szResult = LTNULL;	

	// clear old string
	szTemp[ 0 ] = '\0';

	// look for the new string
	buteMgr.GetString( szTagName, szAttName, defVal, szTemp, ARRAY_LEN(szTemp) );
	ASSERT( strlen(szTemp) < dwMaxLen );

	// allocate memory for the string and copy the string into it
	// NOTE: the caller of this function is responsible
	// for this memory
	szResult = debug_newa(char, strlen(szTemp) + 1);
	SAFE_STRCPY( szResult, szTemp );

	return szResult;
}


// Helper function for allocating memory for an array of string properties and returning a pointer to the new strings
inline char** GetStringArray( CButeMgr & buteMgr, const char* szTagName, const char* szAttName, uint32 dwMaxStrLen, int nMaxArrayLen, int &nArrayLen )
{
	static char szTemp[WMGR_MAX_FILE_PATH] = {0};
	char		**szResult = LTNULL;

	nArrayLen = 0;
	sprintf(szTemp, "%s%d", szAttName, nArrayLen);

	while( buteMgr.Exist( szTagName, szTemp ) && nArrayLen < nMaxArrayLen )
	{
		nArrayLen++;
		sprintf( szTemp, "%s%d", szAttName, nArrayLen );
	}
	
	if( nArrayLen )
	{
		szResult = debug_newa( char*, nArrayLen );

		for( int i = 0; i < nArrayLen; i++ )
		{
			sprintf( szTemp, "%s%d", szAttName, i );
			szResult[i] = GetString( buteMgr, szTagName, szTemp, dwMaxStrLen );
		}
	}

	return szResult;
}


enum WeaponState
{
	W_INACTIVE,
	W_IDLE,
	W_BEGIN_FIRING,
	W_FIRING,
	W_FIRED,
	W_END_FIRING,
	W_SWAT_DEFENSE,
	W_HOLD_DEFENSE,
	W_ARM_DEFENSE,
	W_RELOADING,
	W_FIRING_NOAMMO,
	W_SELECT,
	W_DESELECT,
	W_AUTO_SWITCH
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
	SCOPE,
	UNKNOWN_MOD_TYPE
};


struct MOD
{
	MOD();
	~MOD();

	LTBOOL  Init(CButeMgr & buteMgr, char* aTagName);
	LTBOOL	Override(CButeMgr &buteMgr, char *aTagName);

	int		GetWeaponId() const; // CWeaponMgr::GetWeaponIdFromModId(int nModId)

	LTBOOL  bInited;

	int		nId;

	ModType	eType;

	char	*szSocket;
	char	*szName;
	char	*szIcon; 
	char	*szZoomInSound;
	char	*szZoomOutSound;
	char	*szAttachModel;
	char	*szPowerupModel;
	char	*szPickUpSound;
	char	*szRespawnSound;
	char	*szPowerupFX;
	char	*szRespawnWaitFX;
	
	CButeListReader blrAttachSkins;
	CButeListReader blrPowerupSkins;

	CButeListReader blrAttachRenderStyles;
	CButeListReader blrPowerupRenderStyles;
	
	CButeListReader	blrRespawnWaitSkins;
	CButeListReader	blrRespawnWaitRenderStyles;

	LTVector vAttachScale;

	LTVector    vScreenTintColor;
	LTFLOAT     fScreenTintTime;

	int		nNameId;
	int		nDescriptionId;
	int		nZoomLevel;
	int		nPriority;
	int		nAISilencedFireSndRadius;

	LTBOOL   bIntegrated;
	LTFLOAT	 fPowerupScale;

	bool	bRespawnWaitVisible;
	bool	bRespawnWaitTranslucent;

private:

	void	InitMembers( CButeMgr &buteMgr, char *aTagName );
};

typedef CTList<MOD*> ModList;


struct GEAR
{
	GEAR();
	~GEAR();

	LTBOOL		Init(CButeMgr & buteMgr, char* aTagName);
	LTBOOL		Override(CButeMgr &buteMgr, char *aTagName);

	LTBOOL      bInited;

	int			nId;

	int			nNameId;
	int			nDescriptionId;

	char		*szName;
	char		*szIcon;
	char		*szModel;
	char		*szPickUpSound;
	char		*szRespawnSound;
	char		*szPowerupFX;
	char		*szRespawnWaitFX;

	CButeListReader blrSkins;
	CButeListReader blrRenderStyles;

	CButeListReader	blrRespawnWaitSkins;
	CButeListReader	blrRespawnWaitRenderStyles;

	DamageType	eProtectionType;
    LTFLOAT     fProtection;
    LTFLOAT     fArmor;
	LTFLOAT		fHealth;
    LTFLOAT     fStealth;
    LTBOOL      bSelectable;
    LTBOOL      bExclusive;
    LTVector    vScreenTintColor;
    LTFLOAT     fScreenTintTime;

	bool	bRespawnWaitVisible;
	bool	bRespawnWaitTranslucent;

	// Indicates server option has set restriction.
	bool	bServerRestricted;
	bool	bCanServerRestrict;

private:

	void	InitMembers( CButeMgr &buteMgr, char *aTagName );
};

typedef CTList<GEAR*> GearList;

// This struct is used to contain all the animation overrides for a particular
// ammo type...

struct WEAPONANIS
{
	WEAPONANIS();
	~WEAPONANIS();

	LTBOOL	Init(CButeMgr & buteMgr, char* aTagName);
	LTBOOL	 Override(CButeMgr &buteMgr, char *aTagName);

	LTBOOL	bInited;

	int		nId;
	char	*szName;

	char	*szSelectAni;
	char	*szDeselectAni;
	char	*szReloadAni;

	int		nNumIdleAnis;
	char	*szIdleAnis[WMGR_MAX_WEAPONANI_IDLE];

	int		nNumFireAnis;
	char	*szFireAnis[WMGR_MAX_WEAPONANI_FIRE];

private:

	void	InitMembers( CButeMgr &buteMgr, char *aTagName );
};

typedef CTList<WEAPONANIS*> WeaponAnisList;

struct AMMO
{
	AMMO();
	~AMMO();

	LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	LTBOOL	 Override(CButeMgr &buteMgr, char *aTagName);

	std::string	GetNormalIcon() const;
	std::string	GetDisabledIcon() const;
	std::string	GetUnselectedIcon() const;


	LTBOOL          bInited;

	int             nId;

//	int             nNameId;
	int             nDescId;
	AmmoType        eType;

	LTFLOAT         fPriority;

	char*			szName;

	char*			szLongName;
	char*			szShortName;

	int             GetMaxAmount(HOBJECT hCharacter) const;
	int             nSpawnedAmount;
	int             nSelectionAmount;

	int             nInstDamage;
	DamageType      eInstDamageType;

	int             nAreaDamage;
	int             nAreaDamageRadius;
	DamageType      eAreaDamageType;

	float           fFireRecoilMult;

	LTFLOAT         fProgDamage;
	LTFLOAT         fProgDamageDuration;
	LTFLOAT         fProgDamageRadius;
	LTFLOAT         fProgDamageLifetime;
	DamageType      eProgDamageType;

	bool			bCanBeDeflected;
	std::string		sDeflectSurfaceType;

	bool			bCanAdjustInstDamage;

	PROJECTILEFX   *pProjectileFX;			// Points at CFXButeMgr::m_ProjectileFXList element
	IMPACTFX       *pImpactFX;				// Points at CFXButeMgr::m_ImpactFXList element
	IMPACTFX       *pUWImpactFX;			// Points at CFXButeMgr::m_ImpactFXList element
	IMPACTFX       *pRicochetFX;			// Points at CFXButeMgr::m_ImpactFXList element
	IMPACTFX       *pBlockedFX;				// Points at CFXButeMgr::m_ImpactFXList element
	IMPACTFX       *pMoveableImpactOverrideFX; // Points at CFXButeMgr::m_MoveableImpactOverrideFXName element
	FIREFX         *pFireFX;				// Points at CFXButeMgr::m_FireFXList element
	WEAPONANIS     *pAniOverrides;			// Points at m_pWeaponAnisList element
	TRACERFX       *pTracerFX;				// Points at CFXButeMgr::m_TracerFXList element

	// Indicates server option has set restriction.
	bool	bServerRestricted;
	bool	bCanServerRestrict;

private:
	int             nNameId;
	int             nMaxAmount;
	char           *szIcon;

	void	InitMembers( CButeMgr &buteMgr, char *aTagName );
};

typedef CTList<AMMO*> AmmoList;


struct WEAPON
{
	WEAPON();
	~WEAPON();

	LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	LTBOOL	 Override(CButeMgr &buteMgr, char *aTagName);

	std::string	GetNormalIcon() const;
	std::string	GetDisabledIcon() const;
	std::string	GetUnselectedIcon() const;
	std::string GetSilhouetteIcon() const;

	void	Save(CButeMgr & buteMgr) const;

	LTBOOL  bInited;

	int		nId;

	int		nDescriptionId;
	int		nClientWeaponType;
	int		nIsAmmoNoPickupId;
	int		nAniType;

	char*	szName;

	char*	szLongName;
	char*	szShortName;

	LTVector vPos;
  	LTVector vMuzzlePos;
	LTVector vBreachOffset;

	char	*szPVModel;
	char	*szHHModel;

	CButeListReader blrPVSkins;
	CButeListReader blrHHSkins;
	CButeListReader blrPVRenderStyles;
	CButeListReader blrHHRenderStyles;

	CButeListReader	blrHiddenPieceNames;	// A list of model pieces that will be hidden when the weapon fires

	LTVector vHHScale;

	char	*szSilencedFireSound;
	char	*szAltFireSound;
	char	*szFireSound;
	char	*szDryFireSound;
	char	*szReloadSounds[WMGR_MAX_RELOAD_SNDS];
	char	*szSelectSound;
	char	*szDeselectSound;
	char	*szPrimitiveType;
	char	*szSubroutineNeeded;
	char	*szMiscSounds[WMGR_MAX_MISC_SNDS];

	int		nFireSoundRadius;
	int		nAIFireSoundRadius;
	int		nWeaponSoundRadius;		// All non-fire sounds (Reload, Select, Deselect... etc.)

	LTBOOL  bEnvironmentMap;
	LTBOOL  bInfiniteAmmo;
	LTBOOL  bLooksDangerous;
	LTBOOL	bHideWhenEmpty;
	LTBOOL	bIsAmmo;
	LTBOOL	bUseUWMuzzleFX;

	int		nShotsPerClip;

	int		nNumAmmoIds;
	int		*aAmmoIds;

	int		nDefaultAmmoId;

	int		nNumModIds;
	int		*aModIds;

	int		nNumPVFXTypes;
	int		*aPVFXTypes;

	int		nMinPerturb;
	int		nMaxPerturb;

	int		nRange;
	LTVector vRecoil;
	int		nVectorsPerRound;

	int		nAIWeaponType;
	float	fAIMinBurstInterval;
	float	fAIMaxBurstInterval;
	int		nAIMinBurstShots;
	int		nAIMaxBurstShots;
	LTBOOL	bAIAnimatesReloads;

	float	fFireRecoilPitch;
	float	fFireRecoilDecay;

	CMuzzleFX	*pPVMuzzleFX;	// Player-view muzzle fx

	char	*szPVMuzzleFxName;
	char	*szHHMuzzleFxName;

	int		nNumPVAttachClientFX;		// FXEd player view attachments
	char	*szPVAttachClientFX[ WMGR_MAX_PVCLIENTFX_ATTACHMENTS ];

	uint32	m_nFireDelay;		// Minimum time (ms) between firing

	char	*szHolsterAttachment;
	
	float	fFireAnimRateScale;
	float	fReloadAnimRateScale;

	char	*szPowerupFX;
	char	*szRespawnWaitFX;

	CButeListReader	blrRespawnWaitSkins;
	CButeListReader	blrRespawnWaitRenderStyles;

	bool	bRespawnWaitVisible;
	bool	bRespawnWaitTranslucent;

	// Indicates server option has set restriction.
	bool	bServerRestricted;
	bool	bCanServerRestrict;

private:

	int		nNameId;
	void	InitMembers( CButeMgr &ButeMgr, char *aTagName );
	char	*szIcon; 

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
	private :

		// Not allowed to create directly.  Use Instance().
		CWeaponMgr();

		// Copy ctor and assignment operator not implemented and should never be used.
		CWeaponMgr( CWeaponMgr const& other );
		CWeaponMgr& operator=( CWeaponMgr const& other );

	public :

		// This destructor should be private, but if it is, the compiler complains
		// that the Instance function does not have access to it.  Instance should
		// have access since it's a member function.  Compiler bug?
		~CWeaponMgr();

		// Call this to get the singleton instance of the weapon mgr.
		static CWeaponMgr& Instance( );

#ifndef _CLIENTBUILD
        LTBOOL      ReadWeaponProp(char* pPropName, uint8 & m_nWeaponId, uint8 & m_nAmmoId);
        void        ReadWeapon(char* szString, uint8 & nWeaponId, uint8 & nAmmoId);
#endif // _CLIENTBUILD

		LTBOOL      Init(const char* szAttributeFile = WEAPON_DEFAULT_FILE);
		LTBOOL		LoadOverrideButes( const char *szAttributeFile = WEAPON_DEFAULT_MULTI_FILE);
		void		Term();

		void		CalculateWeaponPath(WeaponPath & wp);

		LTBOOL      WriteFile();
		void        Reload();

		int			GetNumWeapons()			const { return m_nNumWeapons; }
		uint8		GetNumWeaponClasses()	const { return m_nNumClasses; }
		int			GetNumAmmoIds()			const { return m_nNumAmmos; }
		int			GetNumModIds()			const { return m_nNumMods; }
		int			GetNumGearIds()			const { return m_nNumGear; }

		LTBOOL      IsValidWeaponId(int nWeaponId) const;
		LTBOOL      IsValidAmmoId(int nAmmoId) const;
		LTBOOL      IsValidModId(int nModId) const ;
		LTBOOL      IsValidGearId(int nGearId) const;

		LTBOOL		IsPlayerWeapon(int nWeaponId) const;

		int			GetWeaponId(int nCommandId) const;
		int			GetCommandId(int nWeaponId) const;
		uint8		GetWeaponClass(int nWeaponId) const;
		int			GetFirstWeaponCommandId() const;
		int			GetLastWeaponCommandId() const;

		uint8		GetWeaponPriorities(uint8* pPriorityArray, int nArrayLen) const;

		WEAPON const *GetWeapon(int nWeaponId) const;
		WEAPON const *GetWeapon(const char* pWeaponName) const;

		AMMO const   *GetAmmo(int nAmmoId) const;
		AMMO const   *GetAmmo(char const* pAmmoName) const;

		MOD const    *GetMod(int nModId) const;
		MOD const    *GetMod(char const* pModName) const;

		GEAR const   *GetGear(int nGearId) const;
		GEAR const   *GetGear(char const* pGearName) const;

		WEAPONANIS*	GetWeaponAnis(char* pAnisName);

		// Find the first weapon that uses this type of ammo...

		WEAPON const *GetCorrespondingWeapon(AMMO const *pAmmo) const;

		// Left in for backwards compatibility...

		CScaleFX*    GetScaleFX(int nScaleFXId);
		PEXPLFX*     GetPExplFX(int nPExplFXId);
		DLIGHTFX*    GetDLightFX(int nDLightFXId);

		IMPACTFX*        GetImpactFX(int nImpactFXId);
		PROJECTILEFX*    GetProjectileFX(int nProjFXId);
		FIREFX*          GetFireFX(int nFireFXId);

		uint32		GetFileCRC() { return m_nFileCRC; }

		// Checks if weaponida is better than weaponidb.
		bool		IsBetterWeapon( uint8 nWeaponIdA, uint8 nWeaponIdB ) const;

		const char* GetMPDefaultWeapons() const;

	protected :

		WEAPON**			m_pWeaponList;		// All weapon data
		int32				m_nNumWeapons;
	
		AMMO**				m_pAmmoList;		// All ammo data
		int32				m_nNumAmmos;

		MOD**				m_pModList;			// All mod data
		int32				m_nNumMods;

		GEAR**				m_pGearList;		// All gear data
		int32				m_nNumGear;
	
		WEAPONANIS**		m_pWeaponAnisList;	// Weapon Ani overrides
		int32				m_nNumWeaponAnis;

		int*				m_pWeaponOrder;				// Order of weapon selection
		int					m_nFirstPlayerWeapon;		// First weapon player can use
		int					m_nLastPlayerWeapon;		// Last weapon player can use

		uint8*				m_pClasses;				// indices into weapon order
		uint8				m_nNumClasses;

		uint8*				m_pWeaponPriorities;		// Used with auto-weapon switching
		uint8				m_nNumWeaponPriorities;

		void				ClearLists();	// Clear all the above lists

		uint32				m_nFileCRC;

		std::string		m_sMPDefaults;
};


// Map a commandid to a weapon id...

inline int CWeaponMgr::GetWeaponId(int nCommandId) const
{
	int nId = nCommandId - COMMAND_ID_WEAPON_BASE;
	if (!m_pWeaponOrder || nId < 0 || nId >= m_nNumWeapons)
	{
		return WMGR_INVALID_ID;
	}

	return m_pWeaponOrder[nId];
}

inline int CWeaponMgr::GetFirstWeaponCommandId() const
{
	return COMMAND_ID_WEAPON_BASE + m_nFirstPlayerWeapon;
}

inline int CWeaponMgr::GetLastWeaponCommandId() const
{
	return COMMAND_ID_WEAPON_BASE + m_nLastPlayerWeapon;
}

inline LTBOOL CWeaponMgr::IsPlayerWeapon(int nWeaponId)  const
{
	int nCommandId = GetCommandId(nWeaponId);
	return (nCommandId >= GetFirstWeaponCommandId() && nCommandId <= GetLastWeaponCommandId());
}


// Map weapon id to command id...

inline int CWeaponMgr::GetCommandId(int nWeaponId) const
{
	if (!m_pWeaponOrder || !IsValidWeaponId(nWeaponId))
	{
		return WMGR_INVALID_ID;
	}

	for (int i=0; i < m_nNumWeapons; i++)
	{
		if (m_pWeaponOrder[i] == nWeaponId)
		{
			return COMMAND_ID_WEAPON_BASE + i;
		}
	}

	return WMGR_INVALID_ID;
}

// Get a weapons class
inline uint8 CWeaponMgr::GetWeaponClass(int nWeaponId) const
{
	if (!m_pClasses || !m_pWeaponOrder || !IsValidWeaponId(nWeaponId))
	{
		return 0;
	}

	int i=0;
	int nOrder = -1;
	while (i < m_nNumWeapons && nOrder < 0)
	{
		if (m_pWeaponOrder[i] == nWeaponId)
		{
			nOrder = i;
		}
		i++;
	}

	if (nOrder < 0)
		return 0;
	
	uint8 nClass = 0;
	while (nClass < m_nNumClasses)
	{
		if (nOrder <= (int)m_pClasses[nClass])
			return (nClass+1);
		nClass++;
	}

	return 0;
}

inline uint8 CWeaponMgr::GetWeaponPriorities(uint8* pPriorityArray, int nArrayLen) const
{
	if (!m_pWeaponPriorities || m_nNumWeaponPriorities < 1 || !pPriorityArray) return 0;

	int nNumWeapons = 0;
	for (int i=0; i < m_nNumWeaponPriorities; i++)
	{
		if (i < nArrayLen)
		{
			pPriorityArray[i] = m_pWeaponPriorities[i];
			nNumWeapons++;
		}
		else
		{
			break;
		}
	}

	return nNumWeapons;
}

inline LTBOOL CWeaponMgr::IsValidWeaponId(int nWeaponId) const
{
	return (LTBOOL) (0 <= nWeaponId && nWeaponId < m_nNumWeapons);
}

inline LTBOOL CWeaponMgr::IsValidAmmoId(int nAmmoId) const
{
	return (LTBOOL) (0 <= nAmmoId && nAmmoId < m_nNumAmmos);
}

inline LTBOOL CWeaponMgr::IsValidModId(int nModId) const
{
	return (LTBOOL) (0 <= nModId && nModId < m_nNumMods);
}

inline LTBOOL CWeaponMgr::IsValidGearId(int nGearId) const
{
	return (LTBOOL) (0 <= nGearId && nGearId < m_nNumGear);
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
		CWeaponMgrPlugin();
		virtual ~CWeaponMgrPlugin();

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

		CFXButeMgrPlugin*	m_pFXButeMgrPlugin;

		static LTBOOL            sm_bInitted;
};

#endif // _CLIENTBUILD

#endif // __WEAPON_MGR_H__
