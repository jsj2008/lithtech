// ----------------------------------------------------------------------- //
//
// MODULE  : TO2VersionMgr.cpp
//
// PURPOSE : Implementation of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2VersionMgr.h"
#if defined(_CLIENTBUILD)
#include "ClientResShared.h"
#endif

#ifdef _TO2DEMO

// {E77DA1EB-C293-499a-8C8C-0B0784897A9F}
LTGUID GAMEGUID = 
{ 0xe77da1eb, 0xc293, 0x499a, { 0x8c, 0x8c, 0xb, 0x7, 0x84, 0x89, 0x7a, 0x9f } };

#else

// {BEF696D3-E5DC-4db5-B3D6-70AFBD0D2ADD}
LTGUID GAMEGUID = 
{ 0xbef696d3, 0xe5dc, 0x4db5, { 0xb3, 0xd6, 0x70, 0xaf, 0xbd, 0xd, 0x2a, 0xdd } };

#endif

// This is used for folder paths and registry entries.  Don't put invalid characters into the name.
#ifdef _TO2DEMO
char const GAME_NAME[] = "No One Lives Forever 2 (Official Demo)";
#else
char const GAME_NAME[] = "No One Lives Forever 2";
#endif

int const GAME_HANDSHAKE_VER_MAJOR = 1;
int const GAME_HANDSHAKE_VER_MINOR = 3;
int const GAME_HANDSHAKE_VER = ((GAME_HANDSHAKE_VER_MAJOR << 8) + GAME_HANDSHAKE_VER_MINOR);


#ifdef _TO2DEMO
static const char* s_szVersion		= "Demo v1.0";
#elif defined( _FINAL )
static const char* s_szVersion		= "v1.3";
#else
static const char* s_szVersion		= "NOLF 2 Development Build 124 (DO NOT DISTRIBUTE)";
#endif // _TO2DEMO

static const char* s_szBuild		= "Build 124";
static const uint32 s_nBuildNumber = 124;
static const char* s_szNetVersion	= "1.0.0.3";
static const char* s_szNetGameName	= "NOLF2";

const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__1_1 = 72;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__1_2 = 90;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__1_3 = s_nBuildNumber;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__CurrentBuild = s_nBuildNumber;

CVersionMgr* g_pVersionMgr = NULL;


// Creates a build guid based on the game guid.
#define MAKEBUILDGUID( _a, _b, _c, _d0, _d1, _d2, _d3, _d4, _d5, _d6, _d7 )										\
static const LTGUID s_BuildGuid =																				\
{																												\
	(_a) + GAMEGUID.guid.a, (_b) + GAMEGUID.guid.b, (_c) + GAMEGUID.guid.c,										\
	{																											\
		(_d0) + GAMEGUID.guid.d[0], (_d1) + GAMEGUID.guid.d[1], (_d2) + GAMEGUID.guid.d[2], (_d3) + GAMEGUID.guid.d[3],	\
		(_d4) + GAMEGUID.guid.d[4], (_d5) + GAMEGUID.guid.d[5], (_d6) + GAMEGUID.guid.d[6], (_d7) + GAMEGUID.guid.d[7]	\
	}																										\
};																											\


#ifdef _TO2DEMO
// Official Demo guid.
// {BFF2EBA7-3F13-473c-9B48-81A724FF579B}
MAKEBUILDGUID( 0xbff2eba7, 0x3f13, 0x473c, 0x9b, 0x48, 0x81, 0xa7, 0x24, 0xff, 0x57, 0x9b );
//#define BUILDGUID_SAMANTHA
#elif defined(BUILDGUID_SAMANTHA)
// Samantha Build guid.
// {B0118155-BACA-40d9-AE52-AFC5A7D65257}
MAKEBUILDGUID( 0xb0118155, 0xbaca, 0x40d9, 0xae, 0x52, 0xaf, 0xc5, 0xa7, 0xd6, 0x52, 0x57 );
#else // Normal
// Normal retail build guid.
// {D097EFE1-354D-4ba1-9457-5A64CB836AA8}
MAKEBUILDGUID( 0xd097efe1, 0x354d, 0x4ba1, 0x94, 0x57, 0x5a, 0x64, 0xcb, 0x83, 0x6a, 0xa8 );
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2VersionMgr::CTO2VersionMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTO2VersionMgr::CTO2VersionMgr() : CVersionMgr()
{
	g_pVersionMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2VersionMgr::GetVersion()
//
//	PURPOSE:	Get the current version string
//
// ----------------------------------------------------------------------- //

const char* CTO2VersionMgr::GetVersion()
{
	return s_szVersion;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2VersionMgr::GetSaveVersion()
//
//	PURPOSE:	Get the current save version 
//
// ----------------------------------------------------------------------- //

const uint32 CTO2VersionMgr::GetSaveVersion()
{
	return s_nBuildNumber;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2VersionMgr:GetNetVersion()
//
//	PURPOSE:	Get the current net version 
//
// ----------------------------------------------------------------------- //

const char* CTO2VersionMgr::GetNetVersion()
{
	return s_szNetVersion;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2VersionMgr:GetNetGameName()
//
//	PURPOSE:	Get the net game name.
//
// ----------------------------------------------------------------------- //

const char* CTO2VersionMgr::GetNetGameName()
{
	return s_szNetGameName;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2VersionMgr::GetBuild()
//
//	PURPOSE:	Get the current build string
//
// ----------------------------------------------------------------------- //

const char* CTO2VersionMgr::GetBuild()
{
	return s_szBuild;
}




void CTO2VersionMgr::Update()
{
#if defined(_CLIENTBUILD)
	if (stricmp(LoadTempString(IDS_ALLOW_GORE),"TRUE") != 0)
	{
		m_bLowViolence = true;
	}

	//override for debugging purposes
	float fVal = 0.0f;

    if (!g_pLTClient) return;

    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LowViolence");
	if(hVar)
	{
        fVal = g_pLTClient->GetVarValueFloat(hVar);
		if (fVal >= 1.0f)
		{
			m_bLowViolence = true;
		}

	}
#endif

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2VersionMgr::GetBuildGuid()
//
//	PURPOSE:	Gets a guid based on a build.  Allows us to tag special 
//				builds sent to people.
//
// ----------------------------------------------------------------------- //

const LTGUID* CTO2VersionMgr::GetBuildGuid()
{
	return &s_BuildGuid;
}
