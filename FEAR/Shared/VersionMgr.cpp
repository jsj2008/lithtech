// ----------------------------------------------------------------------- //
//
// MODULE  : VersionMgr.cpp
//
// PURPOSE : Implementation of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#define VERSIONMGR_EXPORTS
#include "Stdafx.h"
#include "VersionMgr.h"
#include "CommonUtilities.h"
#include "WinUtil.h"
#include "ltfileoperations.h"
#include "iltfilemgr.h"
#include "ltbuildversion.h"
#include "ltprofileutils.h"
#include "ltgamecfg.h"

uint16 const GAME_HANDSHAKE_VER_MASK = 0x1234;

LTGUID GAMEGUID = 
{ 0xdf560590, 0xd918, 0x4a51, { 0x9c, 0xb1, 0x38, 0xeb, 0xbe, 0x38, 0x79, 0xad } };

uint16 const GAME_HANDSHAKE_VER_MAJOR = 1;
uint16 const GAME_HANDSHAKE_VER_MINOR = 0;
uint16 const GAME_HANDSHAKE_VER = ((GAME_HANDSHAKE_VER_MAJOR << 8) + GAME_HANDSHAKE_VER_MINOR);

static const uint32 s_nBuildNumber = LT_VERSION_BUILD;
static const char* s_szNetVersion	= "FEAR v1.08";

static const char* s_szBuildVersion = LT_VERSION_PRODUCT_STRING;

const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__1_0 = 225;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__1_03 = 253;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__1_04 = 255;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__CurrentBuild = s_nBuildNumber;

CVersionMgr* g_pVersionMgr = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::CVersionMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CVersionMgr::CVersionMgr()
{
	CLIENT_CODE
	(
		// Determine what language version the game is using...
		m_bLowViolence	= false;
	)

	char szString[256];
	LTSNPrintF( szString, LTARRAYSIZE( szString ), "Build %d", s_nBuildNumber );
	m_sBuild = szString;

	m_sPatchVersion = GetNetVersion( );

	m_nCurrentSaveVersion = 0;

	g_pVersionMgr = this;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::Init()
//
//	PURPOSE:	Initialize data.
//
// ----------------------------------------------------------------------- //

bool CVersionMgr::Init( )
{
	char szGameSystemIniFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( "Game.ini", szGameSystemIniFile, LTARRAYSIZE( szGameSystemIniFile ));
	m_sGameSystemIniFile = szGameSystemIniFile;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::GetCurrentSaveVersion()
//
//	PURPOSE:	Grab the "current" save version.  Meaning the version
//				of the save file currently being loaded.
//
// ----------------------------------------------------------------------- //

CVersionMgr::TSaveVersion CVersionMgr::GetCurrentSaveVersion() const
{
	// The only time we care about old save versions is when we are restoring a
	// previously saved game.
	if( m_nLastLGFlags == LOAD_RESTORE_GAME )
		return m_nCurrentSaveVersion;

	return CVersionMgr::kSaveVersion__CurrentBuild;
}

#ifdef _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::GetCDKey
//
//	PURPOSE:	Gets the cdkey.
//
// ----------------------------------------------------------------------- //
void CVersionMgr::GetCDKey( char* pszCDKey, uint32 nCDKeySize )
{
	char szPath[MAX_PATH*2];
	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, "Key.ini", LTARRAYSIZE( szPath ));

	LTProfileUtils::ReadString( GAME_NAME, "CDKey", "", pszCDKey, nCDKeySize, szPath );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::SetCDKey
//
//	PURPOSE:	Sets the cdkey.
//
// ----------------------------------------------------------------------- //
void CVersionMgr::SetCDKey( char const* pszCDKey )
{
	char szPath[MAX_PATH*2];
	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, "Key.ini", LTARRAYSIZE( szPath ));

	LTProfileUtils::WriteString( GAME_NAME, "CDKey", pszCDKey, szPath );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::GetPreSaleCDKey
//
//	PURPOSE:	Gets the presale cdkey.
//
// ----------------------------------------------------------------------- //
void CVersionMgr::GetPreSaleCDKey( char* pszPreSaleCDKey, uint32 nSize )
{
	char szPath[MAX_PATH*2];
	LTFileOperations::GetUserDirectory( "FEARPreSale", szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, "Key.ini", LTARRAYSIZE( szPath ));

	LTProfileUtils::ReadString( "FEARPreSale", "CDKey", "", pszPreSaleCDKey, nSize, szPath );
}


#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::GetSaveVersion()
//
//	PURPOSE:	Get the current save version 
//
// ----------------------------------------------------------------------- //

const uint32 CVersionMgr::GetSaveVersion()
{
	return s_nBuildNumber;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr:GetDisplayVersion
//
//	PURPOSE:	Get the current display version 
//
// ----------------------------------------------------------------------- //

void CVersionMgr::GetDisplayVersion( wchar_t* pszDisplayVersion, uint32 nDisplayVersionLen ) const
{
#ifdef _CLIENTBUILD
	if (GetCurExecutionShellContext() == eExecutionShellContext_Client)
	{
#ifndef _FINAL
		LTSNPrintF( pszDisplayVersion, nDisplayVersionLen, L"v%S Build %d (DO NOT DISTRIBUTE)", s_szBuildVersion, s_nBuildNumber );
#else // _FINAL
		if( LTGameCfg::IsMPFreeProduct())
		{
			LTStrCpy( pszDisplayVersion, LoadString("IDS_GAME_VERSION_MPFree"), nDisplayVersionLen );
		}
		else
		{
			LTStrCpy( pszDisplayVersion, LoadString("IDS_GAME_VERSION"), nDisplayVersionLen );
		}
#endif // _FINAL
	}
#endif // _CLIENTBUILD
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr:GetNetVersion()
//
//	PURPOSE:	Get the current net version 
//
// ----------------------------------------------------------------------- //

const char* CVersionMgr::GetNetVersion() const
{
	return s_szNetVersion;
}

void CVersionMgr::Update()
{
	CLIENT_CODE
	(
		if (g_pLTBase != g_pLTClient)
		{
			return;
		}

		if (LTStrICmp(LoadString("IDS_ALLOW_GORE"),L"TRUE") != 0)
		{
			m_bLowViolence = true;
		}

		//override for debugging purposes
		float fVal = 0.0f;

		if (!g_pLTClient) return;

		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("LowViolence");
		if(hVar)
		{
			fVal = g_pLTClient->GetConsoleVariableFloat(hVar);
			if (fVal >= 1.0f)
			{
				m_bLowViolence = true;
			}

		}
	) // CLIENT_CODE
}


#ifdef __cplusplus
extern "C"
{
#endif
// ----------------------------------------------------------------------- //
// Function name   : GetBuildNumber
// Description     : Gets the build number for export.
// Return type     : uint32 
// ----------------------------------------------------------------------- //
MODULE_EXPORT uint32 GetBuildNumber( )
{
	return s_nBuildNumber;
}

#ifdef __cplusplus
}
#endif
