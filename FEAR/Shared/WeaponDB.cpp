// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponDB.cpp
//
// PURPOSE : Weapon database implementation
//
// CREATED : 06/16/03
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "WeaponDB.h"
	#include "FXDB.h"
	#include "crc32utils.h"
	#include "iltfilemgr.h"

//
// Defines...
//
	
const char* const WDB_WeaponData_Default	= "Default";
const char* const WDB_WeaponData_Multi		= "MultiPlayer";
const char* const WDB_WeaponData_AI			= "AI";
const char* const WDB_AmmoData_Default		= "Player";
const char* const WDB_AmmoData_Multi		= "MultiPlayer";
const char* const WDB_AmmoData_AI			= "AI";

//
// Globals...
//
CWeaponDB* g_pWeaponDB = NULL;

//given a record name and a parent category, this will attempt to find the appropriate record handle, and
//if it cannot find it using the normal name, will examine it to see if it has a '/' in the name and use
//only the portion beyond the slash
static HRECORD FindRecordInCategory(HCATEGORY hCategory, const char* pszRecord)
{
	if( !pszRecord )
		return NULL;

	HRECORD hRecord = g_pLTDatabase->GetRecord( hCategory, pszRecord );
	if( !hRecord )
	{
		std::string sRecord = pszRecord;
		uint32 nPos = sRecord.find_last_of( '/' );
		if( nPos != std::string::npos )
		{
			hRecord = g_pLTDatabase->GetRecord( hCategory, pszRecord + (nPos + 1) );
		}
	}

	return hRecord;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::CWeaponDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CWeaponDB::CWeaponDB()
:	CGameDatabaseMgr	( ),
	m_hCatWeapons		( NULL ),
	m_hCatAmmo			( NULL ),
	m_hCatMeleeCollider	( NULL ),
	m_hCatMeleeDamage	( NULL ),
	m_hCatGear			( NULL ),
	m_hCatMods			( NULL ),
	m_hCatGlobal		( NULL ),
	m_hCatTurrets		( NULL ),
	m_hRecGlobal		( NULL ),
	m_nNumWeapons		( 0 ),
	m_nNumAmmo			( 0 ),
	m_nNumGear			( 0 ),
	m_nNumMods			( 0 ),
	m_nNumTurrets		( 0 ),
	m_hUnarmedRecord	( NULL ),
	m_hDetonatorRecord	( NULL )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::~CWeaponDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CWeaponDB::~CWeaponDB()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetWeaponRecord()
//
//	PURPOSE:	Get a handle to a specified weapon record...
//
// ----------------------------------------------------------------------- //

HWEAPON CWeaponDB::GetWeaponRecord( const char *pszWeapon ) const
{
	return FindRecordInCategory(m_hCatWeapons, pszWeapon);
}

HWEAPON CWeaponDB::GetWeaponRecord( uint8 nIndex ) const
{ 
	return g_pLTDatabase->GetRecordByIndex( m_hCatWeapons, nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetWeaponData()
//
//	PURPOSE:	Get a handle to a weapon data record from the weapon handle
//
// ----------------------------------------------------------------------- //
HWEAPONDATA CWeaponDB::GetWeaponData(HWEAPON hWeapon, bool bUseAIStats)
{
	if (!hWeapon) return NULL;

	if (bUseAIStats)
	{
		HWEAPONDATA hOver = GetRecordLink(hWeapon, WDB_WeaponData_AI );
		if (hOver)
			return hOver;
	}

#ifdef _CLIENTBUILD
	if (IsMultiplayerGameClient())
#else
	if (IsMultiplayerGameServer())
#endif
	{
		HWEAPONDATA hOver = GetRecordLink(hWeapon, WDB_WeaponData_Multi );
		if (hOver)
			return hOver;
	}

	return GetRecordLink(hWeapon, WDB_WeaponData_Default );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetAmmoRecord()
//
//	PURPOSE:	Get a handle to a specified ammo record...
//
// ----------------------------------------------------------------------- //

HAMMO CWeaponDB::GetAmmoRecord( const char *pszAmmo ) const
{
	return FindRecordInCategory(m_hCatAmmo, pszAmmo);
}
HAMMO CWeaponDB::GetAmmoRecord( uint8 nIndex ) const
{
	return g_pLTDatabase->GetRecordByIndex( m_hCatAmmo, nIndex );
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetAmmoData()
//
//	PURPOSE:	Get a handle to a Ammo data record from the Ammo handle
//
// ----------------------------------------------------------------------- //
HAMMODATA CWeaponDB::GetAmmoData(HAMMO hAmmo,bool bUseAIStats)
{
	if (!hAmmo) return NULL;

	if (bUseAIStats)
	{
		HAMMODATA hOver = GetRecordLink(hAmmo, WDB_AmmoData_AI );
		if (hOver)
			return hOver;
	}

#ifdef _CLIENTBUILD
	if (IsMultiplayerGameClient())
#else
	if (IsMultiplayerGameServer())
#endif
	{
		HAMMODATA hOver = GetRecordLink(hAmmo, WDB_AmmoData_Multi );
		if (hOver)
			return hOver;
	}

	return GetRecordLink(hAmmo, WDB_AmmoData_Default );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetGearRecord()
//
//	PURPOSE:	Get a handle to a specified gear record...
//
// ----------------------------------------------------------------------- //

HGEAR CWeaponDB::GetGearRecord( const char *pszGear ) const
{
	return FindRecordInCategory(m_hCatGear, pszGear);
}
HGEAR CWeaponDB::GetGearRecord( uint8 nIndex ) const
{
	return g_pLTDatabase->GetRecordByIndex( m_hCatGear, nIndex );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetModRecord()
//
//	PURPOSE:	Get a handle to a specified mod record...
//
// ----------------------------------------------------------------------- //

HMOD CWeaponDB::GetModRecord( const char *pszMod ) const
{
	return FindRecordInCategory(m_hCatMods, pszMod);
}
HMOD CWeaponDB::GetModRecord( uint8 nIndex ) const
{
	return g_pLTDatabase->GetRecordByIndex( m_hCatMods, nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetMeleeColliderRecord()
//
//	PURPOSE:	Get a handle to a specified melee collider record...
//
// ----------------------------------------------------------------------- //

HRECORD	CWeaponDB::GetMeleeColliderRecord( const char *pszMeleeCollider ) const
{
	return FindRecordInCategory(m_hCatMeleeCollider, pszMeleeCollider);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetMeleeDamageRecord()
//
//	PURPOSE:	Get a handle to a specified melee damage record...
//
// ----------------------------------------------------------------------- //

HRECORD	CWeaponDB::GetMeleeDamageRecord( const char *pszMeleeDamage ) const
{
	return FindRecordInCategory(m_hCatMeleeDamage, pszMeleeDamage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetActionIcon()
//
//	PURPOSE:	Get a the name of the fx.activate record to show the user when
//				they are properly positioned to perform some action
//				(like collecting evidence).
//
// ----------------------------------------------------------------------- //

const char* CWeaponDB::GetActionIcon( HRECORD hWeapon, int nIndex )
{
	HWEAPONDATA hWpnData = GetWeaponData(hWeapon, !USE_AI_DATA);
	if (!hWpnData)
		return "";

	return g_pLTDatabase->GetRecordName(GetRecordLink(hWpnData, WDB_WEAPON_rActionIcon, nIndex));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetTurretRecord()
//
//	PURPOSE:	Get a handle to a specified turret record by name...
//
// ----------------------------------------------------------------------- //

HTURRET CWeaponDB::GetTurretRecord( const char *pszTurret ) const
{
	return FindRecordInCategory( m_hCatTurrets, pszTurret );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetTurretRecord()
//
//	PURPOSE:	Get a handle to a specified turret record by index...
//
// ----------------------------------------------------------------------- //

HTURRET CWeaponDB::GetTurretRecord( uint8 nIndex ) const
{
	return g_pLTDatabase->GetRecordByIndex( m_hCatTurrets, nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetNumLoadouts
//
//	PURPOSE:	Get the number of loadouts
//
// ----------------------------------------------------------------------- //

uint8 CWeaponDB::GetNumLoadouts() const
{
	HATTRIBUTE hLO = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rLoadoutOrder );
	if (!hLO)
		return 0;

	uint32 nNumLO = g_pLTDatabase->GetNumValues(hLO);
	ASSERT(nNumLO == (uint8)nNumLO);
	return (uint8)nNumLO;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetLoadout()
//
//	PURPOSE:	Get a handle to a specified loadout record...
//
// ----------------------------------------------------------------------- //

HRECORD CWeaponDB::GetLoadout( uint8 nIndex ) const
{
	HATTRIBUTE hLO = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rLoadoutOrder );
	if (!hLO)
		return NULL;

	return g_pLTDatabase->GetRecordLink(hLO,nIndex,NULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetFallbackLoadout()
//
//	PURPOSE:	Get a handle to the fallback loadout record...
//
// ----------------------------------------------------------------------- //

HRECORD CWeaponDB::GetFallbackLoadout() const
{
	HATTRIBUTE hLO = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rFallbackLoadout );
	if (!hLO)
		return NULL;

	return g_pLTDatabase->GetRecordLink(hLO,0,NULL);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::Init()
//
//	PURPOSE:	Initialize the weapon database...
//
// ----------------------------------------------------------------------- //

bool CWeaponDB::Init( const char *szDatabaseFile /* = DB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global weapon database pointer...

	g_pWeaponDB = this;

	// Get handles to all of the categories in the database...

	// Weapons...
	m_hCatWeapons = g_pLTDatabase->GetCategory( m_hDatabase, WDB_WEAPON_CATEGORY );
	if( !m_hCatWeapons )
		return false;

	LTASSERT( g_pLTDatabase->GetNumRecords( m_hCatWeapons ) <= 255, "Too many weapon records defined!" );
	m_nNumWeapons = ( uint8 )LTMIN( g_pLTDatabase->GetNumRecords( m_hCatWeapons ), 255 );


	// Ammo...
	m_hCatAmmo = g_pLTDatabase->GetCategory( m_hDatabase, WDB_AMMO_CATEGORY );
	if( !m_hCatAmmo )
		return false;

	LTASSERT( g_pLTDatabase->GetNumRecords( m_hCatAmmo ) <= 255, "Too many ammo records defined!" );
	m_nNumAmmo = g_pLTDatabase->GetNumRecords( m_hCatAmmo );

	
	// MeleeCollider...
	m_hCatMeleeCollider = g_pLTDatabase->GetCategory( m_hDatabase, WDB_MELEECOLLIDER_CATEGORY );
	if( !m_hCatMeleeCollider )
		return false;

	// MeleeDamage...
	m_hCatMeleeDamage = g_pLTDatabase->GetCategory( m_hDatabase, WDB_MELEEDAMAGE_CATEGORY );
	if( !m_hCatMeleeDamage )
		return false;

	// Gear...
	m_hCatGear = g_pLTDatabase->GetCategory( m_hDatabase, WDB_GEAR_CATEGORY );
	if( !m_hCatGear )
		return false;
	
	LTASSERT( g_pLTDatabase->GetNumRecords( m_hCatGear ) <= 255, "Too many gear records defined!" );
	m_nNumGear = ( uint8 )LTMIN( g_pLTDatabase->GetNumRecords( m_hCatGear ), 255 );

	// Pre-process the gear's damagetype protection. This prevents having to do 
	// string lookups every time something gets damaged.
	m_vecGearDamageTypeProtection.clear( );
	for( uint32 nGear = 0; nGear < m_nNumGear; nGear++ )
	{
		GearDamageTypeProtection gearDamageTypeProtection;
		gearDamageTypeProtection.m_hGear = g_pLTDatabase->GetRecordByIndex( m_hCatGear, nGear );
		char const* pszProtectionType = GetString(gearDamageTypeProtection.m_hGear,WDB_GEAR_sProtectionType);
		if( !pszProtectionType || !pszProtectionType[0] )
			continue;
		gearDamageTypeProtection.m_eDamageType = StringToDamageType( pszProtectionType );
		if( gearDamageTypeProtection.m_eDamageType == DT_INVALID )
			continue;
		m_vecGearDamageTypeProtection.push_back( gearDamageTypeProtection );
	}

	// Mods...
	m_hCatMods = g_pLTDatabase->GetCategory( m_hDatabase, WDB_MOD_CATEGORY );
	if( !m_hCatMods )
		return false;

	LTASSERT( g_pLTDatabase->GetNumRecords( m_hCatMods ) <= 255, "Too many mod records defined!" );
	m_nNumMods = ( uint8 )LTMIN( g_pLTDatabase->GetNumRecords( m_hCatMods ), 255 );

	
	// Loadouts...
	m_hCatLoadouts = g_pLTDatabase->GetCategory( m_hDatabase, WDB_LOADOUT_CATEGORY );
	if( !m_hCatLoadouts )
		return false;


	// Turrets...
	m_hCatTurrets = g_pLTDatabase->GetCategory( m_hDatabase, WDB_TURRET_CATEGORY );
	if( !m_hCatTurrets )
		return false;

	LTASSERT( g_pLTDatabase->GetNumRecords( m_hCatTurrets ) <= 255, "Too many turret records defined!" );
	m_nNumTurrets = (uint8)LTMIN( g_pLTDatabase->GetNumRecords( m_hCatTurrets ), 255 );

	// Read in the weapon priority and class lists...

	m_hCatGlobal = g_pLTDatabase->GetCategory( m_hDatabase, WDB_GLOBAL_CATEGORY );
	if( !m_hCatGlobal )
		return false;

	m_hRecGlobal = g_pLTDatabase->GetRecord( m_hCatGlobal, WDB_GLOBAL_RECORD );
	if( !m_hRecGlobal )
		return false;

	m_hUnarmedRecord = GetRecordLink(m_hRecGlobal, WDB_GLOBAL_rUnarmed );
	m_hDetonatorRecord = GetRecordLink(m_hRecGlobal, WDB_GLOBAL_rDetonator );


	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::ReadWeapon
//
//	PURPOSE:	Find the weapon and ammo records based on a string in the
//				in the from of "WeaponName, AmmoName"...
//
// ----------------------------------------------------------------------- //

void CWeaponDB::ReadWeapon( const char* szWeaponString, HWEAPON &hWeapon, HAMMO &hAmmo, bool bUseAIData )
{
	// Get the weapon name...

	char szString[256];
	LTStrCpy( szString, szWeaponString, ARRAY_LEN( szString ));

	hWeapon = GetWeaponRecord( strtok( szString, "," ));
	if (!hWeapon)
	{
		hAmmo = NULL;
		DebugCPrint(0,"CWeaponDB::ReadWeapon() - Unknown weapon %s",szString);
		return;
	}

	
	hAmmo = GetAmmoRecord( strtok( NULL, "" ));

	if( !hAmmo )
	{
		hAmmo = GetRecordLink( GetWeaponData(hWeapon, bUseAIData), WDB_WEAPON_rAmmoName ) ;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetNumPlayerWeapons
//
//	PURPOSE:	Get the number of weapons the player can use
//
// ----------------------------------------------------------------------- //

uint8 CWeaponDB::GetNumPlayerWeapons() const
{
	HATTRIBUTE hPlayerWeaponAtt = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rPlayerWeapons );
	if (!hPlayerWeaponAtt)
		return 0;

	uint32 nNumPlayerWeapons = g_pLTDatabase->GetNumValues(hPlayerWeaponAtt);
	ASSERT(nNumPlayerWeapons == (uint8)nNumPlayerWeapons);
	return (uint8)nNumPlayerWeapons;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetPlayerWeapon
//
//	PURPOSE:	Retrieve a player weapon from an index
//
// ----------------------------------------------------------------------- //

HWEAPON	CWeaponDB::GetPlayerWeapon( uint8 nIndex ) const
{
	HATTRIBUTE hPlayerWeaponAtt = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rPlayerWeapons );
	if (!hPlayerWeaponAtt)
		return NULL;

	return g_pLTDatabase->GetRecordLink(hPlayerWeaponAtt,nIndex,NULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::IsPlayerWeapon
//
//	PURPOSE:	Checks if the weapon is one the player can use...
//
// ----------------------------------------------------------------------- //

bool CWeaponDB::IsPlayerWeapon( HWEAPON hWeapon )
{
	return (GetPlayerWeaponIndex(hWeapon) != WDB_INVALID_WEAPON_INDEX);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetPlayerWeaponIndex
//
//	PURPOSE:	Gets the player weapon index of the weapon...
//
// ----------------------------------------------------------------------- //

uint8 CWeaponDB::GetPlayerWeaponIndex( HWEAPON hWeapon )
{
	if( !hWeapon )
		return WDB_INVALID_WEAPON_INDEX;

	HATTRIBUTE hPlayerWeaponAtt = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rPlayerWeapons );
	if (!hPlayerWeaponAtt)
		return WDB_INVALID_WEAPON_INDEX;

	uint8 nNumPlayerWeapons = GetNumPlayerWeapons();
	for( uint8 n = 0; n < nNumPlayerWeapons; ++n )
	{
		HWEAPON hTest = g_pLTDatabase->GetRecordLink(hPlayerWeaponAtt,n,NULL);
		if( hTest == hWeapon )
		{
			return n;
		}
	}

	return WDB_INVALID_WEAPON_INDEX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetNumPlayerGrenades
//
//	PURPOSE:	Get the number of grenades the player can use
//
// ----------------------------------------------------------------------- //

uint8 CWeaponDB::GetNumPlayerGrenades() const
{
	HATTRIBUTE hPlayerGrenadeAtt = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rPlayerGrenades );
	if (!hPlayerGrenadeAtt)
		return 0;

	uint32 nNumPlayerGrenades = g_pLTDatabase->GetNumValues(hPlayerGrenadeAtt);
	ASSERT(nNumPlayerGrenades == (uint8)nNumPlayerGrenades);
	return (uint8)nNumPlayerGrenades;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetPlayerGrenade
//
//	PURPOSE:	Retrieve a player grenade from an index
//
// ----------------------------------------------------------------------- //

HWEAPON	CWeaponDB::GetPlayerGrenade( uint8 nIndex ) const
{
	HATTRIBUTE hPlayerGrenadeAtt = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rPlayerGrenades );
	if (!hPlayerGrenadeAtt)
		return NULL;

	return g_pLTDatabase->GetRecordLink(hPlayerGrenadeAtt,nIndex,NULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::IsPlayerGrenade
//
//	PURPOSE:	Checks if the weapon is a grenade the player can use...
//
// ----------------------------------------------------------------------- //

bool CWeaponDB::IsPlayerGrenade( HWEAPON hWeapon )
{
	return (GetPlayerGrenadeIndex(hWeapon) != WDB_INVALID_WEAPON_INDEX);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetPlayerGrenadeIndex
//
//	PURPOSE:	Gets the index of the weapon in the array of player grenades...
//
// ----------------------------------------------------------------------- //

uint8 CWeaponDB::GetPlayerGrenadeIndex( HWEAPON hWeapon )
{
	if( !hWeapon )
		return WDB_INVALID_WEAPON_INDEX;

	HATTRIBUTE hPlayerGrenadeAtt = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rPlayerGrenades );
	if (!hPlayerGrenadeAtt)
		return WDB_INVALID_WEAPON_INDEX;

	uint8 nNumPlayerGrenades = GetNumPlayerGrenades();
	for( uint8 n = 0; n < nNumPlayerGrenades; ++n )
	{
		HWEAPON hTest = g_pLTDatabase->GetRecordLink(hPlayerGrenadeAtt,n,NULL);
		if( hTest == hWeapon )
		{
			return n;
		}
	}

	return WDB_INVALID_WEAPON_INDEX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetNumDefaultWeaponPriorities
//
//	PURPOSE:	Get the number of weapon priorities
//
// ----------------------------------------------------------------------- //

uint8 CWeaponDB::GetNumDefaultWeaponPriorities() const
{
	HATTRIBUTE hPriorityAtt = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rWeaponPriority );
	if (!hPriorityAtt)
		return 0;

	uint32 nPriorities = g_pLTDatabase->GetNumValues(hPriorityAtt);
	ASSERT(nPriorities == (uint8)nPriorities);
	return (uint8)nPriorities;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetWeaponFromDefaultPriority
//
//	PURPOSE:	Retrieve a weapon from it's priority
//
// ----------------------------------------------------------------------- //

HWEAPON	CWeaponDB::GetWeaponFromDefaultPriority( uint8 nIndex ) const
{
	HATTRIBUTE hPriorityAtt = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rWeaponPriority );
	if (!hPriorityAtt)
		return NULL;

	return g_pLTDatabase->GetRecordLink(hPriorityAtt,nIndex,NULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetDefaultWeaponPriority
//
//	PURPOSE:	Gets the priority of the weapon...
//
// ----------------------------------------------------------------------- //

uint8 CWeaponDB::GetDefaultWeaponPriority( HWEAPON hWeapon )
{
	if( !hWeapon )
		return WDB_INVALID_WEAPON_INDEX;

	HATTRIBUTE hPriorityAtt = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rWeaponPriority );
	if (!hPriorityAtt)
		return WDB_INVALID_WEAPON_INDEX;

	uint8 nPriorities = GetNumDefaultWeaponPriorities();
	for( uint32 n = 0; n < nPriorities; ++n )
	{
		HWEAPON hTest = g_pLTDatabase->GetRecordLink(hPriorityAtt,n,NULL);
		if( hTest == hWeapon )
			return n;
	}

	return WDB_INVALID_WEAPON_INDEX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::IsBetterWeapon
//
//	PURPOSE:	Checks if WeaponA is better than WeaponB.
//
// ----------------------------------------------------------------------- //

bool CWeaponDB::IsBetterWeapon( HWEAPON hWeaponA, HWEAPON hWeaponB )
{
	// If equal, A is not better...
	if( hWeaponA == hWeaponB )
		return false;

	uint8 nPriA = GetDefaultWeaponPriority(hWeaponA);
	if (nPriA == WDB_INVALID_WEAPON_INDEX)
		return false;

	uint8 nPriB = GetDefaultWeaponPriority(hWeaponB);
	if (nPriB == WDB_INVALID_WEAPON_INDEX)
		return true;

	return (nPriA > nPriB);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetConditionalPriority
//
//	PURPOSE:	Gets the priority of the conditional group...
//
// ----------------------------------------------------------------------- //

uint32 CWeaponDB::GetConditionalPriority( HRECORD hGroup )
{
	if( !hGroup )
		return (uint32)WDB_LOWEST_CONDITIONAL_PRIORITY;

	HATTRIBUTE hPriorityAtt = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rConditionalPriority );
	if (!hPriorityAtt)
		return (uint32)WDB_LOWEST_CONDITIONAL_PRIORITY;

	uint32 nPriorities = g_pLTDatabase->GetNumValues(hPriorityAtt);
	for( uint32 n = 0; n < nPriorities; ++n )
	{
		HWEAPON hTest = g_pLTDatabase->GetRecordLink(hPriorityAtt,n,NULL);
		if( hTest == hGroup )
			return n;
	}

	char szError[256];
	LTSNPrintF(szError, LTARRAYSIZE(szError), "'%s' does not have a priority!", g_pLTDatabase->GetRecordName(hGroup));
	LTERROR(szError);

	return (uint32)WDB_LOWEST_CONDITIONAL_PRIORITY;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::IsBetterConditionalGroup
//
//	PURPOSE:	Checks if Conditional GroupA is better than GroupB.
//
// ----------------------------------------------------------------------- //

bool CWeaponDB::IsBetterConditionalGroup( HRECORD hGroupA, HRECORD hGroupB )
{
	// If equal, A is not better...
	if( hGroupA == hGroupB )
		return false;

	uint32 nPriA = GetConditionalPriority(hGroupA);
	uint32 nPriB = GetConditionalPriority(hGroupB);

	return (nPriA > nPriB);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetNumDisplayGear
//
//	PURPOSE:	Get the number of gear items displayed in the HUD
//
// ----------------------------------------------------------------------- //

uint8 CWeaponDB::GetNumDisplayGear() const
{
	HATTRIBUTE hGO = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rGearOrder );
	if (!hGO)
		return 0;

	uint32 nNumGear = g_pLTDatabase->GetNumValues(hGO);
	ASSERT(nNumGear == (uint8)nNumGear);
	return (uint8)nNumGear;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetDisplayGear
//
//	PURPOSE:	Retrieve a gear item from it's index
//
// ----------------------------------------------------------------------- //

HWEAPON	CWeaponDB::GetDisplayGear( uint8 nIndex ) const
{
	HATTRIBUTE hGO = g_pLTDatabase->GetAttribute( m_hRecGlobal, WDB_GLOBAL_rGearOrder );
	if (!hGO)
		return NULL;

	return g_pLTDatabase->GetRecordLink(hGO,nIndex,NULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetWeaponCapacity
//
//	PURPOSE:	Get the number of weapons the player can carry
//
// ----------------------------------------------------------------------- //

uint8 CWeaponDB::GetWeaponCapacity() const
{
	uint32 nCap = GetInt32( m_hRecGlobal, WDB_GLOBAL_nMaxWeaponCapacity );
	ASSERT(nCap == (uint8)nCap);
	return (uint8)nCap;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetSemiAutoBufferTime
//
//	PURPOSE:	How long should fire commands be buffered for semi-auto weapons
//
// ----------------------------------------------------------------------- //

float CWeaponDB::GetSemiAutoBufferTime() const
{
	return float(g_pWeaponDB->GetInt32(m_hRecGlobal,WDB_GLOBAL_tSemiAutoBufferTime)) / 1000.0f;;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetWeaponFromAmmo
//
//	PURPOSE:	Find the first weapon that uses this type of ammo...
//
// ----------------------------------------------------------------------- //

HWEAPON CWeaponDB::GetWeaponFromAmmo( HAMMO hAmmo, bool bUseAIData ) 
{
	if( !hAmmo )
		return NULL;

	// Search all weapons for the ammo name...
	for( uint8 nWeapon = 0; nWeapon < m_nNumWeapons; ++nWeapon )
	{
		HWEAPON hWeapon = GetWeaponRecord( nWeapon );

		if( !hWeapon )
			continue;

		if (CanWeaponUseAmmo(hWeapon,hAmmo,bUseAIData))
		{
			return hWeapon;
		}

	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetWeaponFromAmmo
//
//	PURPOSE:	Find the first weapon that uses this type of ammo...
//
// ----------------------------------------------------------------------- //

bool CWeaponDB::CanWeaponUseAmmo(HWEAPON hWeapon, HAMMO hAmmo, bool bUseAIData)
{
	// Search the list of ammo names for the one we want...

	uint32 nNumAmmos = GetNumValues( GetWeaponData(hWeapon, bUseAIData), WDB_WEAPON_rAmmoName );
	for( uint32 nAmmo = 0; nAmmo < nNumAmmos; ++nAmmo )
	{
		if( hAmmo == GetRecordLink( GetWeaponData(hWeapon, bUseAIData), WDB_WEAPON_rAmmoName, nAmmo ))
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetWeaponFromMod
//
//	PURPOSE:	Find the first weapon that uses this type of mod...
//
// ----------------------------------------------------------------------- //

HWEAPON CWeaponDB::GetWeaponFromMod( HMOD hMod, bool bUseAIData )
{
	if( !hMod )
		return NULL;

	// Search all weapons for the mod name...

	for( uint8 nWeapon = 0; nWeapon < m_nNumWeapons; ++nWeapon )
	{
		HWEAPON hWeapon = GetWeaponRecord( nWeapon );

		if( !hWeapon )
			continue;

		// Search the list of mod names for the one we want...

		uint32 nNumMods = GetNumValues( GetWeaponData(hWeapon, bUseAIData), WDB_WEAPON_rModName );
		for( uint32 nMod = 0; nMod < nNumMods; ++nMod )
		{
			if( GetRecordLink( GetWeaponData(hWeapon, bUseAIData), WDB_WEAPON_rModName, nMod ) == hMod )
				return hWeapon;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::Restrict
//
//	PURPOSE:	Restrict the use of this record if it can be restricted...
//
// ----------------------------------------------------------------------- //

void CWeaponDB::Restrict( HRECORD hRecord )
{
	if( !hRecord )
		return;

	// See if it is already restricted and if it is allowed to be restricted...

	if( IsRestricted( hRecord ) || !GetBool( hRecord, WDB_ALL_bCanServerRestrict, false ))
		return;

	m_vecRestricted.push_back( hRecord );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::IsRestricted
//
//	PURPOSE:	See if the record is restricted...
//
// ----------------------------------------------------------------------- //

bool CWeaponDB::IsRestricted( HRECORD hRecord )
{
	if( !hRecord )
		return false;

	HRecordArray::iterator iter;
	for( iter = m_vecRestricted.begin(); iter != m_vecRestricted.end(); ++iter )
	{
		if( (*iter) == hRecord )
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetAmmoSurfaceFXTypes
//
//	PURPOSE:	Build a list of all AMMO::surfaceFXTypes
//
// ----------------------------------------------------------------------- //

uint8 CWeaponDB::GetAmmoSurfaceFXTypes( const char **pszSurfaceFXTypesArray, uint8 nArrayLen ) const
{
	if( !pszSurfaceFXTypesArray )
		return 0;

	uint8 nNumAmmoTypes = g_pWeaponDB->GetNumAmmo();
	uint8 nUniqueSurfaceFXTypes = 0;

	for( uint8 nAmmo = 0; (nAmmo < nNumAmmoTypes && nUniqueSurfaceFXTypes < nArrayLen); ++nAmmo )
	{
		HAMMO hAmmo = g_pWeaponDB->GetAmmoRecord( nAmmo );
		if( hAmmo )
		{
			HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo);
			const char *pszSurfaceFXType = g_pWeaponDB->GetString( hAmmoData, WDB_AMMO_sSurfaceFXType );
			if( pszSurfaceFXType && pszSurfaceFXType[0] )
			{
				// See if we've already found this surface fx type...
				bool bFoundRepeat = false;
				for( int nFoundFX = 0; nFoundFX < nUniqueSurfaceFXTypes; ++nFoundFX )
				{
					if( LTStrIEquals( pszSurfaceFXTypesArray[nFoundFX], pszSurfaceFXType ))
					{
						bFoundRepeat = true;
						break; // found repeat
					}
				}

				if (!bFoundRepeat)
				{
					// Add the surface to the list...
					pszSurfaceFXTypesArray[nUniqueSurfaceFXTypes] = pszSurfaceFXType;
					nUniqueSurfaceFXTypes++;
				}
			}
		}
	}

	return nUniqueSurfaceFXTypes;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetEffectiveVectorRangeDamageFactor
//
//	PURPOSE:	Calculate the damage factor for the specified weapon
//				based on the distance passed in...
//
// ----------------------------------------------------------------------- //

float CWeaponDB::GetEffectiveVectorRangeDamageFactor(HWEAPON hWeapon, float fDist, bool bUseAIData)
{
	HWEAPONDATA hWpnData = GetWeaponData(hWeapon, bUseAIData);
	
	float fEffectiveVectorRange = (float)GetInt32( hWpnData, WDB_WEAPON_nEffectiveVectorRange );
	if( fEffectiveVectorRange > 0.0f && fDist > fEffectiveVectorRange )
	{
		// Calculate damage rolloff using the following form:
		// y = 1 / ( e^(k*x) ), where k is
		// ( -ln(.5f) / half_damage_dist )
		float fOutsideDist = fDist - fEffectiveVectorRange;
		float fVectorHalfDamageDist = ( float )GetInt32( hWpnData, WDB_WEAPON_nVectorHalfDamageDist );
		float fK = ( 0.69314718f / fVectorHalfDamageDist );
		return ( 1.0f / ( float )exp( fK * fOutsideDist ) );
	}

	// Use full damage...
	return (1.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModType() const
//
//	PURPOSE:	Gets the mod type.
//
// ----------------------------------------------------------------------- //
ModType CWeaponDB::GetModType( HMOD hMod ) const
{
	// Initialize our search table.
	struct ModTypeStringToEnum
	{
		CParsedMsg::CToken	m_tokString;
		ModType				m_eEnum;
	};
	static ModTypeStringToEnum table[] = 
	{
		{ "SILENCER", SILENCER },
		{ "SCOPE", SCOPE },
	};
	static uint32 nTableCount = LTARRAYSIZE( table );

	// Find the string location and return the enum.
	CParsedMsg::CToken tokLocation = GetString(( HRECORD )hMod, WDB_MOD_nType );
	for( uint32 i = 0; i < nTableCount; i++ )	
	{
		if( table[i].m_tokString == tokLocation )
			return table[i].m_eEnum;
	}

	return UNKNOWN_MOD_TYPE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDB::GetModelFilesCRC
//
//	PURPOSE:	Calculate the CRC for all weapon models..
//
// ----------------------------------------------------------------------- //

uint32 CWeaponDB::GetModelFilesCRC( )
{
	uint32 nFileCRC = 0;
	for( uint8 nWeapon = 0; nWeapon < m_nNumWeapons; ++nWeapon )
	{
		HWEAPON hWeapon = GetWeaponRecord( nWeapon );

		if( !hWeapon )
			continue;

		const char* pszModel = GetString(GetWeaponData(hWeapon, !USE_AI_DATA),WDB_WEAPON_sPVModel);
		if (!pszModel)
			continue;

		ILTInStream* pFileStream = g_pLTBase->FileMgr()->OpenFile(pszModel);
		nFileCRC += CRC32Utils::CalcArchiveFileCRC( pFileStream, pszModel );
		if (pFileStream)
		{
			pFileStream->Release();
		}
	}

	return nFileCRC;
}

#ifdef _SERVERBUILD // Server-side only

/****************************************************************************
*
* CWeaponDBPlugin is used to help facilitate populating the WorldEdit object
* properties that use WeaponMgr
*
*****************************************************************************/


CWeaponDBPlugin *g_pWeaponDBPlugin = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDBPlugin::CWeaponDBPlugin()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CWeaponDBPlugin::CWeaponDBPlugin()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDBPlugin::~CWeaponDBPlugin()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CWeaponDBPlugin::~CWeaponDBPlugin()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDBPlugin::Init()
//
//	PURPOSE:	Initialize the plugin...
//
// ----------------------------------------------------------------------- //

bool CWeaponDBPlugin::Init()
{
	if( g_pWeaponDBPlugin )
		return false;

	// Set the global access for the plugin...

	g_pWeaponDBPlugin = this;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDBPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CWeaponDBPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponDBPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

void CWeaponDBPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
										  const uint32 cMaxStrings, const uint32 cMaxStringLength, bool bPickupList	)
{
	if( !aszStrings || !pcStrings || !g_pWeaponDB )
	{
		LTERROR( "CWeaponDBPlugin::PopulateStringList: Invalid input parameters" );
		return;
	}

	// Add an entry for each weapon/ammo combination...

	uint8 nNumWeapons = g_pWeaponDB->GetNumWeapons();

	for( uint8 nWeapon = 0; nWeapon < nNumWeapons; ++nWeapon )
	{
		LTASSERT( cMaxStrings > (*pcStrings) + 1, "Too many weapons to fit in the list.  Enlarge list size?" );

		HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord( nWeapon );
		if( !hWeapon )
			continue;

		const char *pszWeaponName = g_pWeaponDB->GetRecordName( hWeapon );
		if( !pszWeaponName )
			continue;

		uint32 nWeaponNameLen = LTStrLen( pszWeaponName );

		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		if	(!g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bCanBePlaced) && bPickupList)
		{
			continue;
		}

		if( (nWeaponNameLen < cMaxStringLength) &&	 ((*pcStrings) + 1 < cMaxStrings) )
		{
			// Append the ammo types to the string if there is more
			// than one ammo type...
			


			uint8 nNumAmmo = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rAmmoName );

			if( nNumAmmo > 1 )
			{
				for( uint8 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
				{
					LTASSERT( cMaxStrings > (*pcStrings) + 1, "Too many weapons to fit in the list.  Enlarge list size?" );

					HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, nAmmo );
					if( hAmmo && (*pcStrings) + 1 < cMaxStrings )
					{
						const char *pszAmmoName = g_pWeaponDB->GetRecordName( hAmmo );
						if( !pszAmmoName )
							continue;

						uint32 nAmmoNameLen = LTStrLen( pszAmmoName );

						if( nWeaponNameLen + 1 + nAmmoNameLen < cMaxStringLength )
						{
							LTSNPrintF( aszStrings[(*pcStrings)++], cMaxStringLength, "%s,%s", pszWeaponName, pszAmmoName );
						}
						else
						{
							// Just use the weapon name, the default ammo will be used...

							LTStrCpy( aszStrings[(*pcStrings)++], pszWeaponName, cMaxStringLength );
						}
					}
				}
			}
			else
			{
				// Just use the weapon name, the default ammo will be used...

				LTStrCpy( aszStrings[(*pcStrings)++], pszWeaponName, cMaxStringLength );
			}
		}
	}
}


#endif // _SERVERBUILD
