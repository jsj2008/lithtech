// ----------------------------------------------------------------------- //
//
// MODULE  : ServerUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdlib.h>
#include "ServerUtilities.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "SoundTypes.h"
#include "CVarTrack.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "ltserverobj.h"

// The following two functions should be used to determine how long a block
// of code takes to execute.  For example:
//
// StartTimingCounter();
// float p1 = 30.0f, p2 = 50.0f;
// Function(p1, p2);
// EndTimingCounter("Function(%.2f, %.2f)", p1, p2);
//
// If "Function" took 1000 ticks to execute, the above code would print in
// the console:
//		Function(30.00, 50.00) : 1000 ticks
//
// NOTE:  The timing information is only printed to the console if the server
// console variable "ShowTiming" is set to 1. (i.e., serv showtiming 1)

extern CVarTrack	g_ShowTimingTrack;
extern CVarTrack	g_ShowTriggersTrack;
extern CVarTrack	g_ShowTriggersFilter;
static LTCounter     s_counter;

void StartTimingCounter()
{
    if (!g_pLTServer || g_ShowTimingTrack.GetFloat() < 1.0f) return;

    g_pLTServer->StartCounter(&s_counter);
}

void EndTimingCounter(char *msg, ...)
{
    if (!g_pLTServer || g_ShowTimingTrack.GetFloat() < 1.0f) return;

    uint32 dwTicks = g_pLTServer->EndCounter(&s_counter);

	// parse the message

	char pMsg[256];
	va_list marker;
	va_start(marker, msg);
	int nSuccess = vsprintf(pMsg, msg, marker);
	va_end(marker);

	if (nSuccess < 0) return;

    g_pLTServer->CPrint("%s : %d ticks", pMsg, dwTicks);
}

static void ParseTriggerMsg(const char* szMsg, char* aszMsgs[256], uint32* pcMsgs)
{
	static char szMsgCopy[1024];
	strcpy(szMsgCopy, szMsg);

	char* szToken = strtok(szMsgCopy, ";");
	while( szToken != NULL )
	{
		aszMsgs[(*pcMsgs)++] = szToken;
		szToken = strtok(NULL, ";");
	}
}

LTBOOL SendMixedTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg)
{
	uint32 cMsgs = 0;
	char* aszMsgs[256];

	ParseTriggerMsg(g_pLTServer->GetStringData(hMsg), aszMsgs, &cMsgs);

	LTBOOL bReturn = LTTRUE;
    for ( uint32 iMsg = 0 ; iMsg < cMsgs ; iMsg++ )
	{
		bReturn &= SendTriggerMsgToObjects(pSender, g_pLTServer->GetStringData(hName), aszMsgs[iMsg]);
	}

	return bReturn;
}

LTBOOL SendMixedTriggerMsgToObjects(LPBASECLASS pSender, const char* pName, const char* pMsg)
{
	uint32 cMsgs = 0;
	char* aszMsgs[256];

	ParseTriggerMsg(pMsg, aszMsgs, &cMsgs);

	LTBOOL bReturn = LTTRUE;
    for ( uint32 iMsg = 0 ; iMsg < cMsgs ; iMsg++ )
	{
		bReturn &= SendTriggerMsgToObjects(pSender, pName, aszMsgs[iMsg]);
	}

	return bReturn;
}

void SendMixedTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg)
{
	uint32 cMsgs = 0;
	char* aszMsgs[256];

	ParseTriggerMsg(g_pLTServer->GetStringData(hMsg), aszMsgs, &cMsgs);

	LTBOOL bReturn = LTTRUE;
    for ( uint32 iMsg = 0 ; iMsg < cMsgs ; iMsg++ )
	{
		SendTriggerMsgToObject(pSender, hObj, LTFALSE, aszMsgs[iMsg]);
	}
}

void SendMixedTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, LTBOOL bBogus, const char* pStr)
{
	uint32 cMsgs = 0;
	char* aszMsgs[256];

	ParseTriggerMsg(pStr, aszMsgs, &cMsgs);

	LTBOOL bReturn = LTTRUE;
    for ( uint32 iMsg = 0 ; iMsg < cMsgs ; iMsg++ )
	{
		SendTriggerMsgToObject(pSender, hObj, LTFALSE, aszMsgs[iMsg]);
	}
}

// Send hMsg string to all objects named hName...

LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg)
{
    if (!hMsg) return LTFALSE;

    char* pMsg = g_pLTServer->GetStringData(hMsg);

	// Process the message as a command if it is a valid command...

	if (g_pCmdMgr->IsValidCmd(pMsg))
	{
		return g_pCmdMgr->Process(pMsg);
	}

    if (!hName) return LTFALSE;

    char* pName = g_pLTServer->GetStringData(hName);

	return SendTriggerMsgToObjects(pSender, pName, pMsg);
}

// This version is used to support shared code between projects...

LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, const char* pName, const char* pMsg)
{
    if (!pMsg) return LTFALSE;

	// Process the message as a command if it is a valid command...

	if (g_pCmdMgr->IsValidCmd(pMsg))
	{
		return g_pCmdMgr->Process(pMsg);
	}

    if (!pName || pName[0] == '\0') return LTFALSE;

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
	// ILTServer::FindNameObjects does not destroy pName so this is safe
    g_pLTServer->FindNamedObjects((char*)pName, objArray);

	int numObjects = objArray.NumObjects();
    if (!numObjects) return LTFALSE;

	for (int i = 0; i < numObjects; i++)
	{
		SendTriggerMsgToObject(pSender, objArray.GetObject(i), 0, pMsg);
	}

    return LTTRUE;
}

void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg)
{
	HMESSAGEWRITE hMessage;
	char *pSendName, *pRecvName, *pFilter;

    char* szMessage = g_pLTServer->GetStringData(hMsg);


	// Process the message as a command if it is a valid command...

	if (g_pCmdMgr->IsValidCmd(szMessage))
	{
		g_pCmdMgr->Process(szMessage);
		return;
	}


    hMessage = g_pLTServer->StartMessageToObject(pSender, hObj, MID_TRIGGER);
	if (hMessage)
	{
		if (g_ShowTriggersTrack.GetFloat() != 0.0f)
		{
            if (pSender) pSendName = g_pLTServer->GetObjectName(pSender->m_hObject);
			else pSendName = "Command Manager";

            pRecvName   = g_pLTServer->GetObjectName(hObj);
			pFilter		= g_ShowTriggersFilter.GetStr();

			// Filter out displaying any unwanted messages...

            LTBOOL bPrintMsg = (!pFilter || !pFilter[0]);
			if (!bPrintMsg)
			{
                bPrintMsg = (szMessage ? !strstr(pFilter, szMessage) : LTTRUE);
			}

			if (bPrintMsg)
			{
                g_pLTServer->CPrint("Message: %s", szMessage ? szMessage : "NULL");
				g_pLTServer->CPrint("  Sent from '%s', to '%s'", pSendName, pRecvName);
			}
		}

        g_pLTServer->WriteToMessageDWord(hMessage, (uint32)g_pLTServer->GetStringData(hMsg));
        g_pLTServer->EndMessage(hMessage);
	}
}

// This version is used to support shared code between projects...

void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, LTBOOL, const char* szMessage)
{
	HMESSAGEWRITE hMessage;
	char *pSendName, *pRecvName, *pFilter;

	// Process the message as a command if it is a valid command...

	if (g_pCmdMgr->IsValidCmd(szMessage))
	{
		g_pCmdMgr->Process(szMessage);
		return;
	}


    hMessage = g_pLTServer->StartMessageToObject(pSender, hObj, MID_TRIGGER);
	if (hMessage)
	{
		if (g_ShowTriggersTrack.GetFloat() != 0.0f)
		{
            if (pSender) pSendName = g_pLTServer->GetObjectName(pSender->m_hObject);
			else pSendName = "Command Manager";

            pRecvName   = g_pLTServer->GetObjectName(hObj);
			pFilter		= g_ShowTriggersFilter.GetStr();

			// Filter out displaying any unwanted messages...

            LTBOOL bPrintMsg = (!pFilter || !pFilter[0]);
			if (!bPrintMsg)
			{
                bPrintMsg = (szMessage ? !strstr(pFilter, szMessage) : LTTRUE);
			}

			if (bPrintMsg)
			{
                g_pLTServer->CPrint("Message: %s", szMessage ? szMessage : "NULL");
				g_pLTServer->CPrint("  Sent from '%s', to '%s'", pSendName, pRecvName);
			}
		}

        g_pLTServer->WriteToMessageDWord(hMessage, (uint32)szMessage);
        g_pLTServer->EndMessage(hMessage);
	}
}

//-------------------------------------------------------------------------------------------
// IsPlayer
//
// Checks if handle is a handle to a CPlayerObj
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsPlayer( HOBJECT hObject )
{
    static HCLASS hPlayerTest = g_pLTServer->GetClass( "CPlayerObj" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hPlayerTest ));
}

//-------------------------------------------------------------------------------------------
// IsAI
//
// Checks if handle is a handle to a CAI object
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsAI( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "CAI" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsDog
//
// Checks if handle is a handle to a dog (AI_Dog, Poodle, etc)
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsDog ( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "AI_Dog" );
    HCLASS hClass = g_pLTServer->GetObjectClass(hObject);
    return g_pLTServer->IsKindOf(hClass, hTest);
}

//-------------------------------------------------------------------------------------------
// IsVehicle
//
// Checks if handle is a handle to a vehicle object
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsVehicle( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "Vehicle" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsCharacter
//
// Checks if handle is a handle to a CCharacter
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsCharacter( HOBJECT hObject )
{
    static HCLASS hCharacterTest = g_pLTServer->GetClass( "CCharacter" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hCharacterTest ));
}

//-------------------------------------------------------------------------------------------
// IsCharacterHitBox
//
// Checks if handle is a handle to a CCharacterHitBox
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsCharacterHitBox( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "CCharacterHitBox" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsCharacterHitBox
//
// Checks if handle is a handle to a Body
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsBody( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "Body" );
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
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsGameBase( HOBJECT hObject )
{
    static HCLASS hTest = g_pLTServer->GetClass( "GameBase" );
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
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsKindOf( HOBJECT hObject, const char* szClass )
{
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
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsKindOf( HOBJECT hObject, HOBJECT hObject2 )
{
    HCLASS hClassTest = g_pLTServer->GetObjectClass( hObject2 );
    HCLASS hClass     = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hClassTest ));
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

LTBOOL MoveObjectToFloor(HOBJECT hObj)
{
    if (!hObj) return LTFALSE;

	// Intersect with the world to find the poly we're standing on...

    LTVector vPos, vDims;
    g_pLTServer->GetObjectPos(hObj, &vPos);
    g_pLTServer->GetObjectDims(hObj, &vDims);

	LTVector vDir(0.0f, -10000.0f, 0.0f);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From	= vPos;
	IQuery.m_To		= vPos + vDir;

	IQuery.m_Flags	   = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	IQuery.m_FilterFn  = NULL;
	IQuery.m_pUserData = NULL;

    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
        LTFLOAT fDist = vPos.y - IInfo.m_Point.y;
		if (fDist > vDims.y)
		{
			vPos.y -= (fDist - (vDims.y + 0.1f));
            g_pLTServer->SetObjectPos(hObj, &vPos);
            return LTTRUE;
		}
	}

    return LTFALSE;
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
            return LTTRUE;\
		}\
	}

LTBOOL DoesSegmentIntersectAABB(const LTVector& Point1, const LTVector& Point2, const LTVector& MinBox, const LTVector& MaxBox, LTVector *pIntersectPt /*= NULL*/, LTPlane *pIntersectPlane /*= NULL*/)
{
	float t;
	float testCoords[2];

	// Left/Right.
	if(Point1.x < MinBox.x)
	{
		if(Point2.x < MinBox.x)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MinBox.x, x, y, z, -1.0f);
	}
	else if(Point1.x > MaxBox.x)
	{
		if(Point2.x > MaxBox.x)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MaxBox.x, x, y, z, 1.0f);
	}

	// Top/Bottom.
	if(Point1.y < MinBox.y)
	{
		if(Point2.y < MinBox.y)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MinBox.y, y, x, z, -1.0f);
	}
	else if(Point1.y > MaxBox.y)
	{
		if(Point2.y > MaxBox.y)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MaxBox.y, y, x, z, 1.0f);
	}

	// Front/Back.
	if(Point1.z < MinBox.z)
	{
		if(Point2.z < MinBox.z)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MinBox.z, z, x, y, -1.0f);
	}
	else if(Point1.z > MaxBox.z)
	{
		if(Point2.z > MaxBox.z)
            return LTFALSE;

		DO_PLANE_TEST(MinBox, MaxBox, MaxBox.z, z, x, y, 1.0f);
	}

    return LTFALSE;
}

LTBOOL IsVector(const char* szString)
{
	const char* pch = szString;
	while ( *pch )
	{
		if ( isalpha(*pch) )
		{
			return LTFALSE;
		}

		pch++;
	}

	return LTTRUE;
}

void GetConsoleString(char* sKey, char* sDest, char* sDefault)
{
    if (g_pLTServer)
	{
        HCONVAR hVar = g_pLTServer->GetGameConVar(sKey);
		if (hVar)
		{
            char* sValue = g_pLTServer->GetVarValueString(hVar);
			if (sValue)
			{
				strcpy(sDest, sValue);
				return;
			}
		}
	}

	strcpy(sDest, sDefault);
}

int GetConsoleInt(char* sKey, int nDefault)
{
    if (g_pLTServer)
	{
        HCONVAR hVar = g_pLTServer->GetGameConVar(sKey);
		if (hVar)
		{
            float fValue = g_pLTServer->GetVarValueFloat(hVar);
			return((int)fValue);
		}
	}

	return(nDefault);
}

LTFLOAT GetConsoleFloat(char* sKey, LTFLOAT fDefault)
{
    if (g_pLTServer)
	{
        HCONVAR hVar = g_pLTServer->GetGameConVar(sKey);
		if (hVar)
		{
            float fValue = g_pLTServer->GetVarValueFloat(hVar);
			return(fValue);
		}
	}

	return(fDefault);
}

void WriteConsoleString(char* sKey, char* sValue)
{
    if (g_pLTServer)
	{
		char sTemp[256];
        wsprintf(sTemp, "+%s \"%s\"", sKey, sValue);
        g_pLTServer->RunGameConString(sTemp);
	}
}

void WriteConsoleInt(char* sKey, int nValue)
{
    if (g_pLTServer)
	{
		char sTemp[256];
		wsprintf(sTemp, "+%s %i", sKey, nValue);
        g_pLTServer->RunGameConString(sTemp);
	}
}

void WriteConsoleFloat(char* sKey, LTFLOAT fValue)
{
    if (g_pLTServer)
	{
		char sTemp[256];
		sprintf(sTemp, "+%s %f", sKey, fValue);
        g_pLTServer->RunGameConString(sTemp);
	}
}

