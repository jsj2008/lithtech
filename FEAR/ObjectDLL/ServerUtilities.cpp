// ----------------------------------------------------------------------- //
//
// MODULE  : ServerUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997 - 2004 Monolith Productions, Inc.  All Rights Reserved.
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include <stdlib.h>
#include "ServerUtilities.h"
#include "iltserver.h"
#include "MsgIDs.h"
#include "SoundTypes.h"
#include "VarTrack.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "ltserverobj.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "FxDefs.h"
#include "ParsedMsg.h"
#include "iltmodel.h"
#include "ServerConnectionMgr.h"
#include "GameModeMgr.h"
#include "TeamMgr.h"
#include "lttimeutils.h"

extern	CGameServerShell*   g_pGameServerShell;

// Temp buffer...(moved from CommonUtilities.cpp as it's only used on the
// server).  TO DO: Remove use of this altogether....
char s_FileBuffer[_MAX_PATH];


LTRESULT FindNamedObject(const char* szName, ILTBaseClass *& pObject, bool bMultipleOkay)
{
	if ( !szName || !*szName ) return LT_NOTFOUND;

	pObject = NULL;
	ObjArray<HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects((char*)szName,objArray);

	switch ( objArray.NumObjects() )
	{
		case 1:
		{
			pObject = g_pLTServer->HandleToObject(objArray.GetObject(0));
			return LT_OK;
		}
		case 0:
		{
			return LT_NOTFOUND;
		}
		default:
		{
			if ( bMultipleOkay )
			{
				pObject = g_pLTServer->HandleToObject(objArray.GetObject(0));
				return LT_OK;
			}
			else
			{
                g_pLTServer->CPrint("Error, %d objects named \"%s\" present in level", objArray.NumObjects(), szName);
				return LT_ERROR;
			}
		}
	}
}

// The following two functions should be used to determine how long a block
// of code takes to execute.  For example:
//
// StartTimingCounterServer();
// float p1 = 30.0f, p2 = 50.0f;
// Function(p1, p2);
// EndTimingCounterServer("Function(%.2f, %.2f)", p1, p2);
//
// If "Function" took 1000 ticks to execute, the above code would print in
// the console:
//		Function(30.00, 50.00) : 1000 ticks
//
// NOTE:  The timing information is only printed to the console if the server
// console variable "ShowTiming" is set to 1. (i.e., serv showtiming 1)

extern VarTrack	g_ShowTimingTrack;
static TLTPrecisionTime s_StartTimer;

void StartTimingCounterServer()
{
    if (!g_pLTServer || g_ShowTimingTrack.GetFloat() < 1.0f) return;

	s_StartTimer = LTTimeUtils::GetPrecisionTime();
}

void EndTimingCounterServer(char *msg, ...)
{
    if (!g_pLTServer || g_ShowTimingTrack.GetFloat() < 1.0f) return;

	double fElapsedMS = LTTimeUtils::GetPrecisionTimeIntervalMS(s_StartTimer, LTTimeUtils::GetPrecisionTime());

	// parse the message

	char pMsg[256];
	va_list marker;
	va_start(marker, msg);
	LTVSNPrintF(pMsg, LTARRAYSIZE(pMsg), msg, marker);
	va_end(marker);

	g_pLTServer->CPrint("%s : %8.4f ms", pMsg, fElapsedMS );
}


void ObjectCPrint(HOBJECT hObject, const char *pMsg, ...)
{
	LTASSERT( g_pLTServer, "ObjectCPrint: No server." );

	va_list marker;
	char msg[500];

	va_start(marker, pMsg);
	LTVSNPrintF(msg, LTARRAYSIZE(msg), pMsg, marker);
	va_end(marker);

	char szName[64];
	if(hObject)
	{
		g_pLTServer->GetObjectName(hObject, szName, LTARRAYSIZE(szName));
	}

	g_pLTServer->CPrint("%f %s : %s",
		g_pLTServer->GetTime(),
		hObject ? szName : "",
		msg);
}

void ObjectCPrint(const char *pName, const char *pMsg, ...)
{
	LTASSERT( g_pLTServer, "ObjectCPrint: No server." );

	va_list marker;
	char msg[500];

	va_start(marker, pMsg);
	LTVSNPrintF(msg, LTARRAYSIZE(msg), pMsg, marker);
	va_end(marker);

	g_pLTServer->CPrint("%f %s : %s",
		g_pLTServer->GetTime(),
		pName,
		msg);
}

//-------------------------------------------------------------------------------------------
// IsPlayer
//
// Checks if handle is a handle to a CPlayerObj
// Arguments:
//		hObject - handle to object to test
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool IsPlayer( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

	HCLASS hPlayerTest = g_pLTServer->GetClass( "CPlayerObj" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hPlayerTest ));
}


//-------------------------------------------------------------------------------------------
// IsCharacter
//
// Checks if handle is a handle to a CCharacter
// Arguments:
//		hObject - handle to object to test
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool IsCharacter( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

    HCLASS hCharacterTest = g_pLTServer->GetClass( "CCharacter" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hCharacterTest ));
}

bool IsCharacter( ILTBaseClass *pObject )
{
	if ( NULL == pObject )
		return false;

	HCLASS hCharacterTest = g_pLTServer->GetClass( "CCharacter" );
	HCLASS hClass = pObject ? g_pLTServer->GetObjectClass(pObject->m_hObject) : NULL;
    return ( g_pLTServer->IsKindOf( hClass, hCharacterTest ));
}

//-------------------------------------------------------------------------------------------
// IsCharacterHitBox
//
// Checks if handle is a handle to a CCharacterHitBox
// Arguments:
//		hObject - handle to object to test
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool IsCharacterHitBox( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

	HCLASS hTest  = g_pLTServer->GetClass( "CCharacterHitBox" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsTurret
//
// Checks if handle is a handle to a Turret
// Arguments:
//		hObject - handle to object to test
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool IsTurret( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

	HCLASS hTest = g_pLTServer->GetClass( "Turret" );
	HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
	return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsExplosion
//
// Checks if handle is a handle to an Explosion
// Arguments:
//		hObject - handle to object to test
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool IsExplosion( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

	HCLASS hTest  = g_pLTServer->GetClass( "Explosion" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsGameBase
//
// Checks if handle is a handle to a GameBase
// Arguments:
//		hObject - handle to object to test
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool IsGameBase( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

	HCLASS hTest = g_pLTServer->GetClass( "GameBase" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsWorldModel
//
// Checks if handle is a handle to a WorldModel
// Arguments:
//		hObject - handle to object to test
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool IsWorldModel( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

	HCLASS hTest = g_pLTServer->GetClass( "WorldModel" );
	HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
	return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsKindOf
//
// Checks if hObject is of a particular class
// Arguments:
//		hObject - handle to object to test
//		szClass - the class
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool IsKindOf( HOBJECT hObject, const char* szClass )
{
	if ( NULL == hObject )
		return false;

	HCLASS hClassTest = g_pLTServer->GetClass( (char*)szClass );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hClassTest ));
}

//-------------------------------------------------------------------------------------------
// IsKindOf
//
// Checks if hObject is of a particular class
// Arguments:
//		hObject  - handle to object to test
//		hObject2 - handle to test agains
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool IsKindOf( HOBJECT hObject, HOBJECT hObject2 )
{
	if ( NULL == hObject )
		return false;

	HCLASS hClassTest = g_pLTServer->GetObjectClass( hObject2 );
    HCLASS hClass     = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hClassTest ));
}


//-------------------------------------------------------------------------------------------
// GetPlayerTeamId
//
// Gets the teamid from an HOBJECT that represents a player.
//		Returns INVALID_TEAM if the object isn't a player or
//		if it's not a team gametype.
// Arguments:
//		hPlayer  - handle to a player
// Return:
//      uint8 - teamid.
//-------------------------------------------------------------------------------------------

uint8 GetPlayerTeamId( HOBJECT hPlayer )
{
	if (!GameModeMgr::Instance( ).m_grbUseTeams)
		return INVALID_TEAM;

	CPlayerObj *pPlayer = CPlayerObj::DynamicCast( hPlayer );
	if( !pPlayer )
		return INVALID_TEAM;

	return pPlayer->GetTeamID();
}
	


//-------------------------------------------------------------------------------------------
// AreSameTeam()
//
// Checks if hPlayer1 and hPlayer2 are on the same team
//		returns false if either object is not a player
//		returns true only if the players are on the same team
// Arguments:
//		hPlayer1  - handle to first player
//		hPlayer2 - handle to second player
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool AreSameTeam( HOBJECT hPlayer1, HOBJECT hPlayer2 )
{
	if (!GameModeMgr::Instance( ).m_grbUseTeams)
		return false;

	return ( GetPlayerTeamId( hPlayer1 ) == GetPlayerTeamId( hPlayer2 ));
}
	
//-------------------------------------------------------------------------------------------
// TeamIdToString
//
// Returns a string representation of the teamid.
// Arguments:
//		nTeamId - The teamid to convert..
// Return:
//      char const* - "NoTeam", "Team0", "Team1"
//-------------------------------------------------------------------------------------------

char const* TeamIdToString( uint8 nTeamId )
{
	switch( nTeamId )
	{
		case 0:
			return "Team0";
			break;
		case 1:
			return "Team1";
			break;
		default:
			return "NoTeam";
			break;
	}
}
	
//-------------------------------------------------------------------------------------------
// TeamStringToTeamId
//
// Returns a teamid based on a string.
// Arguments:
//		pszTeamString - team string to convert.
// Return:
//      uint8 - team id.
//-------------------------------------------------------------------------------------------

uint8 TeamStringToTeamId( char const* pszTeamString )
{
	if( !pszTeamString )
		return INVALID_TEAM;

	static char const szTeam[] = "Team";
	static int nLen = LTStrLen( szTeam );
	uint32 nTeamId = INVALID_TEAM;
	if( !LTSubStrICmp( pszTeamString, szTeam, nLen ))
	{
		nTeamId = atoi( &pszTeamString[ nLen ] );
		if( nTeamId >= MAX_TEAMS )
		{
			nTeamId = INVALID_TEAM;
		}
	}

	return nTeamId;
}


//-------------------------------------------------------------------------------------------
// TeamPopulateEditStringList
//
// Populates an EditStringList with list of available teams.
//
//-------------------------------------------------------------------------------------------

void TeamPopulateEditStringList( 
								char** aszStrings,
								uint32* pcStrings,
								const uint32 cMaxStrings,
								const uint32 cMaxStringLength
								)
{
	char szTeam[32] = {0};

	LTASSERT(cMaxStrings > (*pcStrings) + 1, "Too many teams for stringlist.");
	if( *pcStrings < cMaxStrings )
	{
		LTStrCpy( aszStrings[(*pcStrings)++], "NoTeam", cMaxStringLength );

		for( int i = 0; i < MAX_TEAMS; ++i )
		{
			LTASSERT(cMaxStrings > (*pcStrings) + 1, "Too many teams for stringlist.");
			// exit out early if we can't hold any more strings
			if( *pcStrings >= cMaxStrings )
				return;

			LTSNPrintF( szTeam, ARRAY_LEN(szTeam), "Team%i", i );
			if( (LTStrLen( szTeam ) < cMaxStringLength) && ((*pcStrings) + 1 < cMaxStrings) )
			{
				LTStrCpy( aszStrings[(*pcStrings)++], szTeam, cMaxStringLength );
			}
		}
	}

	// Sort the list so turret types are easier to find.  Skip the first item
	// since it's the none selection.
	qsort( aszStrings + 1, *pcStrings - 1, sizeof(char *), CaseInsensitiveCompare );
}

//-------------------------------------------------------------------------------------------
// MoveObjectToFloor
//
// Move the object down to the floor (or down to rest on an object)
// Arguments:
//		hObject - handle to object to move
// Return:
//		True if object was moved
//-------------------------------------------------------------------------------------------

bool MoveObjectToFloor(HOBJECT hObj, HOBJECT *pFilterList, ObjectFilterFn pFilterFn)
{
    if (!hObj) return false;

	// Intersect down to find the poly / object to move down to...

    LTVector vPos, vDims;
	g_pLTServer->GetObjectPos(hObj, &vPos);
	g_pPhysicsLT->GetObjectDims(hObj, &vDims);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From	= vPos;
	IQuery.m_To		= vPos + LTVector(0.0f, -10000.0f, 0.0f);

	IQuery.m_Flags	   = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	IQuery.m_FilterFn  = pFilterFn ? pFilterFn : ObjListFilterFn;
	IQuery.m_pUserData = pFilterList;

    if (g_pLTServer->IntersectSegment(IQuery, &IInfo))
	{
        float fDist = vPos.y - IInfo.m_Point.y;
		if (fDist > vDims.y)
		{
			vPos.y -= (fDist - (vDims.y + 0.1f));
			g_pLTServer->SetObjectPos(hObj, vPos);
            return true;
		}
	}

    return false;
}

void ShowObjectAttachments(HOBJECT hObj, bool bShow)
{
	if (!hObj) return;

	HLOCALOBJ attachList[20];
    uint32 dwListSize = 0;
    uint32 dwNumAttach = 0;

    g_pCommonLT->GetAttachments(hObj, attachList, 20, dwListSize, dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;

	for (int i=0; i < nNum; i++)
	{
		if (bShow)
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
		}
		else
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
		}

		// Hide/Show this attachment's attachments...
		ShowObjectAttachments(attachList[i],bShow);
	}

}


// Get the intersection point and see if it's inside the other dimensions.
#define DO_PLANE_TEST(MinBox, MaxBox, planeCoord, coord0, coord1, coord2, normalDirection) \
	t = (planeCoord - Point1.coord0) / (Point2.coord0 - Point1.coord0);\
	testCoords[0] = Point1.coord1 + ((Point2.coord1 - Point1.coord1) * t);\
	if(testCoords[0] > MinBox.coord1-.05f && testCoords[0] < MaxBox.coord1+.05f)\
	{\
		testCoords[1] = Point1.coord2 + ((Point2.coord2 - Point1.coord2) * t);\
		if(testCoords[1] > MinBox.coord2-.05f && testCoords[1] < MaxBox.coord2+.05f)\
		{\
			if ( pIntersectPt ) \
			{ \
				pIntersectPt->coord0 = planeCoord;\
				pIntersectPt->coord1 = testCoords[0];\
				pIntersectPt->coord2 = testCoords[1];\
			} \
			if ( pIntersectPlane ) \
			{ \
				pIntersectPlane->m_Normal.x = normalDirection;\
				pIntersectPlane->m_Normal.y = 0.0f;\
				pIntersectPlane->m_Normal.z = 0.0f;\
				pIntersectPlane->m_Dist = MinBox.x * normalDirection;\
			} \
            return true;\
		}\
	}

bool DoesSegmentIntersectAABB(const LTVector& Point1, const LTVector& Point2, const LTVector& MinBox, const LTVector& MaxBox, LTVector *pIntersectPt /*= NULL*/, LTPlane *pIntersectPlane /*= NULL*/)
{
	float t;
	float testCoords[2];

	// Left/Right.
	if(Point1.x < MinBox.x)
	{
		if(Point2.x < MinBox.x)
            return false;

		DO_PLANE_TEST(MinBox, MaxBox, MinBox.x, x, y, z, -1.0f);
	}
	else if(Point1.x > MaxBox.x)
	{
		if(Point2.x > MaxBox.x)
            return false;

		DO_PLANE_TEST(MinBox, MaxBox, MaxBox.x, x, y, z, 1.0f);
	}

	// Top/Bottom.
	if(Point1.y < MinBox.y)
	{
		if(Point2.y < MinBox.y)
            return false;

		DO_PLANE_TEST(MinBox, MaxBox, MinBox.y, y, x, z, -1.0f);
	}
	else if(Point1.y > MaxBox.y)
	{
		if(Point2.y > MaxBox.y)
            return false;

		DO_PLANE_TEST(MinBox, MaxBox, MaxBox.y, y, x, z, 1.0f);
	}

	// Front/Back.
	if(Point1.z < MinBox.z)
	{
		if(Point2.z < MinBox.z)
            return false;

		DO_PLANE_TEST(MinBox, MaxBox, MinBox.z, z, x, y, -1.0f);
	}
	else if(Point1.z > MaxBox.z)
	{
		if(Point2.z > MaxBox.z)
            return false;

		DO_PLANE_TEST(MinBox, MaxBox, MaxBox.z, z, x, y, 1.0f);
	}

    return false;
}

bool SAABB::IntersectPoint( const LTVector& vPoint )
{
	if( ( vMin.y > vPoint.y ) ||
		( vMax.y < vPoint.y ) ||
		( vMin.x > vPoint.x ) ||
		( vMax.x < vPoint.x ) ||
		( vMin.z > vPoint.z ) ||
		( vMax.z < vPoint.z ) )
	{
		return false;
	}

	return true;
}


bool IsVector(const char* szString)
{
	const char* pch = szString;
	while ( *pch )
	{
		if ( isalpha(*pch) )
		{
			return false;
		}

		pch++;
	}

	return true;
}

char const* GetConsoleString(char* pszKey, char const* pszDefault)
{
    if (g_pLTServer)
	{
        HCONSOLEVAR hVar = g_pLTServer->GetConsoleVariable(pszKey);
		if (hVar)
		{
            const char* pszValue = g_pLTServer->GetConsoleVariableString(hVar);
			if (pszValue)
			{
				return pszValue;
			}
		}
	}

	return pszDefault;
}

int GetConsoleInt(char* sKey, int nDefault)
{
    if (g_pLTServer)
	{
        HCONSOLEVAR hVar = g_pLTServer->GetConsoleVariable(sKey);
		if (hVar)
		{
            float fValue = g_pLTServer->GetConsoleVariableFloat(hVar);
			return((int)fValue);
		}
	}

	return(nDefault);
}

float GetConsoleFloat(char* sKey, float fDefault)
{
    if (g_pLTServer)
	{
        HCONSOLEVAR hVar = g_pLTServer->GetConsoleVariable(sKey);
		if (hVar)
		{
            float fValue = g_pLTServer->GetConsoleVariableFloat(hVar);
			return(fValue);
		}
	}

	return(fDefault);
}

void WriteConsoleString(char const* sKey, char const* sValue)
{
    if (g_pLTServer)
	{
		g_pLTServer->SetConsoleVariableString(sKey, sValue);
	}
}

void WriteConsoleInt(char* sKey, int nValue)
{
    if (g_pLTServer)
	{
		g_pLTServer->SetConsoleVariableFloat(sKey, (float)nValue);
	}
}

void WriteConsoleFloat(char* sKey, float fValue)
{
    if (g_pLTServer)
	{
		g_pLTServer->SetConsoleVariableFloat(sKey, fValue);
	}
}

void Warn(const char* szFormat, ...)
{
#ifndef _FINAL
	if (GetConsoleInt("DebugLevel",0) < 1 ) return;
	static char szBuffer[4096];
	va_list val;
	va_start(val, szFormat);
	LTVSNPrintF(szBuffer, LTARRAYSIZE(szBuffer), szFormat, val);
	va_end(val);
//	_ASSERT(!szBuffer);
	g_pLTServer->CPrint("WARNING: %s", szBuffer);
#endif
}

LTRESULT SendEmptyClientMsg(uint32 nMsgID, HCLIENT hClient, uint32 nFlags)
{
	LTRESULT nResult;

	CAutoMessage cMsg;

	cMsg.Writeuint8(nMsgID);

	nResult = g_pLTServer->SendToClient(cMsg.Read(), hClient, nFlags);

	return nResult;
}

LTRESULT SendEmptyObjectMsg(uint32 nMsgID, HOBJECT hSource, HOBJECT hDest, uint32 nFlags)
{
	LTRESULT nResult;

	CAutoMessage cMsg;

	cMsg.Writeuint32(nMsgID);

	nResult = g_pLTServer->SendToObject(cMsg.Read(), hSource, hDest, nFlags);

	return nResult;
}


//////////////////////////////////////////////////////////////////////////
// Function name   : SendToClientsExcept
// Description     : Sends a message to all clients except the one specified.
// Return type     : LTRESULT - LT_OK on success. Will stop on failure.
// Argument        : ILTMessage_Read& msg - message to send.
// Argument        : HCLIENT hExceptClient - The client to skip.
// Argument        : uint32 nFlags - message flags.
//////////////////////////////////////////////////////////////////////////
LTRESULT SendToClientsExcept( ILTMessage_Read& msg, HCLIENT hExceptClient, uint32 nFlags )
{
	ServerConnectionMgr::GameClientDataList::iterator iter = ServerConnectionMgr::Instance( ).GetGameClientDataList( ).begin( );
	for( ; iter != ServerConnectionMgr::Instance( ).GetGameClientDataList( ).end( ); iter++ )
	{
		GameClientData* pGameClientData = *iter;
		if( pGameClientData->GetClient() == hExceptClient )
			continue;

		LTRESULT hRes = g_pLTServer->SendToClient( &msg, pGameClientData->GetClient(), nFlags );
		if( hRes != LT_OK )
			return hRes;

		// Start over.
		msg.Seek( 0 );
	}

	return LT_OK;
}


const char *GetObjectName(HOBJECT hObj)
{
	static char aName[256];
	ASSERT(g_pLTServer);
	LTRESULT nResult = g_pLTServer->GetObjectName(hObj, aName, sizeof(aName));
	return nResult == LT_OK ? aName : NULL;
}

// Check if in multiplayer game.
bool IsMultiplayerGameServer()
{
	// If we don't have a server (like when being run from worldedit), then assume no mp.
	if( !g_pLTServer )
		return false;

	return g_pLTServer->IsMultiplayerExe();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetPlayerFromClientId
//
//	PURPOSE:	Gets CPlayerObj from clientid.
//
// ----------------------------------------------------------------------- //
CPlayerObj* GetPlayerFromClientId( uint32 nClientId )
{
	HCLIENT hClient = g_pLTServer->GetClientHandle( nClientId );
	if( !hClient )
		return NULL;

	CPlayerObj* pPlayerObj = GetPlayerFromHClient(hClient);
	if( !pPlayerObj )
		return NULL;

	return pPlayerObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetObjectMaterialCount
//
//	PURPOSE:	Returns the number of materials on a model.
//
// ----------------------------------------------------------------------- //
int GetObjectMaterialCount(HOBJECT hObj)
{
	for (int i = 0; ; ++i)
	{
		char szGarbage[MAX_PATH];
		LTRESULT result = g_pModelLT->GetMaterialFilename(hObj, i, szGarbage, LTARRAYSIZE(szGarbage));
		if (result != LT_OK)
		{
			return i;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayWeaponModelPieces
//
//	PURPOSE:	Show or hide a list of weapon model pieces,
//				using the specified weapon object, weapon record, 
//				and model piece attribute list.
//
// ----------------------------------------------------------------------- //

void DisplayWeaponModelPieces( LTObjRef hObject, HWEAPON hWeapon, const char* ModelPieceListAtt, bool bShow, bool bUseAIWeapons )
{
	if( !hWeapon || !hObject )
		return;

	// get the model interface
	ILTModel *pModelLT = g_pLTServer->GetModelLT();
	ASSERT( 0 != pModelLT );

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, bUseAIWeapons);

	uint32 NumPieces = g_pWeaponDB->GetNumValues(hWpnData,ModelPieceListAtt);
	for (uint32 PieceIndex=0; PieceIndex < NumPieces; PieceIndex++)
	{
		const char* pszPiece = g_pWeaponDB->GetString(hWpnData, ModelPieceListAtt, PieceIndex);

		HMODELPIECE hPiece = 0;
		if( LT_OK == pModelLT->GetPiece( hObject, pszPiece, hPiece ) )
		{
			// show or hide the model piece
			LTRESULT ltResult = pModelLT->SetPieceHideStatus( hObject, hPiece, !bShow );
			ASSERT( ( LT_OK == ltResult) || ( LT_NOCHANGE == ltResult ) );
		}
	}
}

