// ----------------------------------------------------------------------- //
//
// MODULE  : FXDB.cpp
//
// PURPOSE : Implementation of the effects database
//
// CREATED : 02/09/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "FXDB.h"
#include "WeaponFXTypes.h"

#if defined( _CLIENTBUILD )
#include "..\ClientShellDLL\GameClientShell.h"
#include "..\ClientShellDLL\CMoveMgr.h"
#include "..\ClientShellDLL\PlayerCamera.h"
extern CGameClientShell* g_pGameClientShell;
#endif

//
// Defines...
//

//FX categories
const char* const FXDB_ImpactCat =		"FX/ImpactFX";
const char* const FXDB_TracerCat =		"FX/TracerFX";
const char* const FXDB_FireCat =		"FX/FireFX";
const char* const FXDB_ProjectileCat =	"FX/ProjectileFX";
const char* const FXDB_SpearClassDataCat =	"FX/ProjectileFX/ProjectileClassData/SpearClassData";
const char* const FXDB_ProximityClassDataCat =	"FX/ProjectileFX/ProjectileClassData/ProximityClassData";
const char* const FXDB_RemoteClassDataCat =	"FX/ProjectileFX/ProjectileClassData/RemoteClassData";

const char* const FXDB_bEjectShells =	"EjectShells";
const char* const FXDB_bFireSound =		"FireSound";
const char* const FXDB_bExitMark =		"ExitMark";
	
//
// Globals...
//

CFXDB* g_pFXDB = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXDB::CFXDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CFXDB::CFXDB()
:	CGameDatabaseMgr( ),
	m_hImpactCat(NULL),
	m_hTracerCat(NULL),
	m_hFireCat(NULL),
	m_hProjectileCat(NULL),
	m_hSpearClassDataCat(NULL),
	m_hProximityClassDataCat(NULL),
	m_hRemoteClassDataCat(NULL)
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXDB::~CFXDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CFXDB::~CFXDB()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool CFXDB::Init( const char *szDatabaseFile /* = FDB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global database pointer...
	g_pFXDB = this;

	// Get handles to all of the categories in the database...
	m_hImpactCat = g_pLTDatabase->GetCategory(m_hDatabase,FXDB_ImpactCat);
	m_hTracerCat = g_pLTDatabase->GetCategory(m_hDatabase,FXDB_TracerCat);
	m_hFireCat = g_pLTDatabase->GetCategory(m_hDatabase,FXDB_FireCat);
	m_hProjectileCat = g_pLTDatabase->GetCategory(m_hDatabase,FXDB_ProjectileCat);
	m_hSpearClassDataCat = g_pLTDatabase->GetCategory(m_hDatabase,FXDB_SpearClassDataCat);
	m_hProximityClassDataCat = g_pLTDatabase->GetCategory(m_hDatabase,FXDB_ProximityClassDataCat);
	m_hRemoteClassDataCat = g_pLTDatabase->GetCategory(m_hDatabase,FXDB_RemoteClassDataCat);

	return true;
}


#if defined(_CLIENTBUILD)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXDB::CreatePusherFX()
//
//	PURPOSE:	Create a pusher fx
//
// ----------------------------------------------------------------------- //

void CFXDB::CreatePusherFX(HRECORD hPusherFX, const LTVector &vPos)
{
	if (!hPusherFX) return;

	g_pPlayerMgr->GetMoveMgr()->AddPusher(vPos, GetFloat(hPusherFX,FXDB_fRadius),
												GetFloat(hPusherFX,FXDB_fStartDelay),
												GetFloat(hPusherFX,FXDB_fDuration),
												GetFloat(hPusherFX,FXDB_fStrength)
											);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXDB::CreateImpactFX()
//
//	PURPOSE:	Create the specified impact fx
//
// ----------------------------------------------------------------------- //

void CFXDB::CreateImpactFX(HRECORD	hImpactFX, IFXCS & cs)
{
	// Sanity checks...

	if (!hImpactFX) return;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	int nDetailLevel = pSettings->SpecialFXSetting();

	// Add a pusher if necessary...

	CreatePusherFX(GetRecordLink(hImpactFX,FXDB_rPusher), cs.vPos);
}
#endif


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXDB::GetImpactFX
//
//	PURPOSE:	Get the specified impact fx record
//
// ----------------------------------------------------------------------- //
HRECORD		CFXDB::GetImpactFX(uint32 nImpactFXId)
{
	return g_pLTDatabase->GetRecordByIndex(m_hImpactCat,nImpactFXId);
}

HRECORD		CFXDB::GetImpactFX(const char *pName )
{
	return g_pLTDatabase->GetRecord(m_hImpactCat,pName);
}

#if defined( _SERVERBUILD )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXDB::ReadImpactFXProp
//
//	PURPOSE:	Read in the impact fx properties
//
// ----------------------------------------------------------------------- //
uint32 CFXDB::GetNumImpactFX() const
{
	return g_pLTDatabase->GetNumRecords(m_hImpactCat);
}

bool CFXDB::ReadImpactFXProp( const GenericPropList *pProps, const char *pszPropName, uint32 & nImpactFXId)
{
	if( !pszPropName || !pszPropName[0] )
		return false;

	// Get the impact fx
	const char *pszImpactFX = pProps->GetString( pszPropName, "" );
	HRECORD hImpactFX = GetImpactFX( pszImpactFX );
	if( hImpactFX )
	{
		nImpactFXId = g_pLTDatabase->GetRecordIndex(hImpactFX);
		return true;
	}

	return false;
}
#endif //defined( _SERVERBUILD )




HRECORD CFXDB::GetTracerFX(const char* pName )
{
	return g_pLTDatabase->GetRecord(m_hTracerCat,pName);
}

HRECORD CFXDB::GetFireFX(const char* pName )
{
	return g_pLTDatabase->GetRecord(m_hFireCat,pName);
}

uint16 CFXDB::GetFireFlags(HRECORD hFire)
{
	
	if (!hFire) return 0;

	uint16 nFlags = 0;
	if (GetBool(hFire,FXDB_bEjectShells))
	{
		nFlags |= WFX_SHELL;
	}

	if (GetBool(hFire,FXDB_bFireSound))
	{
		nFlags |= WFX_FIRESOUND;
	}

	if (GetBool(hFire,FXDB_bExitMark))
	{
		nFlags |= WFX_EXITMARK;
	}

	return nFlags;

}


HRECORD CFXDB::GetProjectileFX(const char* pName )
{
	return g_pLTDatabase->GetRecord(m_hProjectileCat,pName);
}

uint32 CFXDB::GetProjectileObjectFlags(HRECORD hProjectile)
{
	uint32 dwObjectFlags    = 0;
	if (GetBool(hProjectile,FXDB_bGravity))
	{
		dwObjectFlags |= FLAG_GRAVITY;
	}
	return dwObjectFlags;
}

uint16 CFXDB::GetProjectileFlags(HRECORD hProjectile)
{
	if (!hProjectile) return 0;

	uint16 nFlags = 0;
	if (GetBool(hProjectile,FXDB_bFlySound))
	{
		nFlags |= PFX_FLYSOUND;
	}
	return nFlags;
};

uint32 CFXDB::GetProjectileFXFlags(HRECORD hProjectile)
{
	uint32 dwFXFlags    = 0;

	if (GetBool(hProjectile,FXDB_bFXLoop))
	{
		dwFXFlags |= FXFLAG_LOOP;
	}

	if (!GetBool(hProjectile,FXDB_bFXSmoothShutdown))
	{
		dwFXFlags |= FXFLAG_NOSMOOTHSHUTDOWN;
	}

	return dwFXFlags;
}

HRECORD CFXDB::GetSpearClassData(HRECORD hProjectile)
{
	return g_pLTDatabase->GetRecord(m_hSpearClassDataCat,GetString(hProjectile,FXDB_sClassData));
}

HRECORD CFXDB::GetProximityClassData(HRECORD hProjectile)
{
	return g_pLTDatabase->GetRecord(m_hProximityClassDataCat,GetString(hProjectile,FXDB_sClassData));
}

HRECORD CFXDB::GetRemoteClassData(HRECORD hProjectile)
{
	return g_pLTDatabase->GetRecord(m_hRemoteClassDataCat,GetString(hProjectile,FXDB_sClassData));
}

#if defined( _SERVERBUILD )

/////////////////////////////////////////////////////////////////////////////
//
//	S E R V E R - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
//
// CFXDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use FXDB
//
////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXDBPlugin::CFXDBPlugin()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CFXDBPlugin::CFXDBPlugin()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXDBPlugin::~CFXDBPlugin()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CFXDBPlugin::~CFXDBPlugin()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXDBPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

bool CFXDBPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
										  const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return false;
	LTASSERT(aszStrings && pcStrings, "Invalid string list population call encountered");

	// Add an entry for each impact fx

	uint32 nNumImpactFX = g_pFXDB->GetNumImpactFX();
	LTASSERT(nNumImpactFX < INVALID_GAME_DATABASE_INDEX, "Game database not available");

	HRECORD hImpactFX = NULL;

	for (uint32 i=0; i < nNumImpactFX; i++)
	{
		LTASSERT(cMaxStrings > (*pcStrings) + 1, "Too many FX records encountered");

		hImpactFX = g_pFXDB->GetImpactFX(i);
		if (!hImpactFX)
			continue;
		
		const char* pszName = g_pFXDB->GetRecordName(hImpactFX);
		if (!pszName)
			continue;

		uint32 dwImpactFXNameLen = LTStrLen(pszName);

		if (dwImpactFXNameLen &&
			dwImpactFXNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pszName);
		}
	}

	return true;
}

#endif // _SERVERBUILD
