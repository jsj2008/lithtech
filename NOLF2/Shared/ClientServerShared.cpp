// ----------------------------------------------------------------------- //
//
// MODULE  : ClientServerShared.cpp
//
// PURPOSE : Utility functions used on client and server.
//
// CREATED : 10/21/02
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientServerShared.h"
#include "CommonUtilities.h"

#ifdef _CLIENTBUILD
	#include "ClientUtilities.h"
#endif //_CLIENTBUILD


void ButeToConsoleFloat( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar )
{
	float fVal;

	fVal = bute.GetFloat( pszButeTag, pszButeAttr );
	if( bute.Success( ))
		WriteConsoleFloat(( char * )pszConsoleVar, fVal );
}

void ButeToConsoleString( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar )
{
	char szVal[128];

	bute.GetString( pszButeTag, pszButeAttr, szVal, sizeof(szVal) );
	if( bute.Success( ))
		WriteConsoleString(( char * )pszConsoleVar, szVal );
}

void ConsoleToButeFloat( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar )
{
	float fVal;

	fVal = GetConsoleFloat(( char * )pszConsoleVar, 0.0f );
	bute.SetFloat( pszButeTag, pszButeAttr, fVal );
}

void ConsoleToButeString( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar )
{
	char szVal[MAX_PATH*2];

	GetConsoleString(( char * )pszConsoleVar, szVal, "" );
	bute.SetString( pszButeTag, pszButeAttr, szVal );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SetObjectRenderStyle
//
//	PURPOSE:	Sets a renderstyle of an object
//
// ----------------------------------------------------------------------- //
void SetObjectRenderStyle(HOBJECT hObj, uint8 nRenderStyleNum, char* szRenderStyleName)
{
	// Sanity checks
	if(nRenderStyleNum >= MAX_MODEL_RENDERSTYLES)
	{
		ASSERT(FALSE);
		return;
	}

	if(!szRenderStyleName)
		return;

	// Create a temp ocs
	ObjectCreateStruct ocs;
	strcpy(ocs.m_RenderStyleNames[nRenderStyleNum], szRenderStyleName);

	// And do it
	g_pCommonLT->SetObjectFilenames(hObj, &ocs);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetGameType
//
//  PURPOSE:	Gets the current gametype.
//
// ----------------------------------------------------------------------- //

GameType GetGameType( )
{
#ifdef _CLIENTBUILD
	return g_pGameClientShell->GetGameType();
#else // _CLIENTBUILD
	return g_pGameServerShell->GetGameType();
#endif // _CLIENTBUILD
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	IsCoopMultiplayerGameType
//
//  PURPOSE:	Checks the game type to see if it is a cooperative multiplayer game...
//
// ----------------------------------------------------------------------- //

bool IsCoopMultiplayerGameType()
{
	switch( GetGameType( ))
	{
		case eGameTypeSingle:
		case eGameTypeCooperative:
			return true;
		break;

		default:
			return false;
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	IsDeathmatchGameType
//
//  PURPOSE:	Checks the game type to see if it is a deathmatch or team deathmatch game...
//
// ----------------------------------------------------------------------- //

bool IsDeathmatchGameType()
{
	switch( GetGameType( ))
	{
		case eGameTypeSingle:
		case eGameTypeCooperative:
		case eGameTypeDoomsDay:
			return false;
		break;

		case eGameTypeDeathmatch:
		case eGameTypeTeamDeathmatch:
			return true;
		break;

		default:
			return false;
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	IsTeamGameType
//
//  PURPOSE:	Checks the game type to see if it is a team game...
//
// ----------------------------------------------------------------------- //

bool IsTeamGameType()
{
	switch( GetGameType( ))
	{
		case eGameTypeSingle:
		case eGameTypeCooperative:
		case eGameTypeDeathmatch:
			return false;
		break;

		case eGameTypeDoomsDay:
		case eGameTypeTeamDeathmatch:
			return true;
		break;

		default:
			return false;
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	IsDifficultyGameType
//
//  PURPOSE:	Checks the game type to see if it uses difficulty settings...
//
// ----------------------------------------------------------------------- //

bool IsDifficultyGameType()
{
	switch( GetGameType( ))
	{
		case eGameTypeSingle:
		case eGameTypeCooperative:
			return true;
		break;

		case eGameTypeDeathmatch:
		case eGameTypeDoomsDay:
		case eGameTypeTeamDeathmatch:
			return false;
		break;

		default:
			return false;
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	IsRevivePlayerGameType
//
//  PURPOSE:	Checks the game type to see if it supports reviving players.
//
// ----------------------------------------------------------------------- //

bool IsRevivePlayerGameType()
{
	switch( GetGameType( ))
	{
		case eGameTypeCooperative:
		case eGameTypeDoomsDay:
			return true;
		break;
		default:
			return false;
		break;
	}
}

