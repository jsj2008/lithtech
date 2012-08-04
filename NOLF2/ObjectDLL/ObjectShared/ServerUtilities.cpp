// ----------------------------------------------------------------------- //
//
// MODULE  : ServerUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997 - 2002 Monolith Productions, Inc.  All Rights Reserved
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
#include "UberAssert.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "FXDefs.h"
#include "GameBaseLite.h"
#include "ParsedMsg.h"

extern	CGameServerShell*   g_pGameServerShell;

// Print out the passed in object's flags and flags2 to the console in a 
// human readable format...
void PrintObjectFlags(HOBJECT hObj, char* pMsg/*=LTNULL*/)
{
#ifdef _DEBUG
	if (!hObj)
	{
		g_pLTServer->CPrint("Invalid Object passed to PrintObjectFlags(%s)!", pMsg ? pMsg : "NULL");
		return;
	}

	// For now just print out the normal flags...in the future this should
	// probably be updated to support printing out the user, and client
	// flags..

	char szName[64];
	szName[0] = '\0';
	g_pLTServer->GetObjectName(hObj, szName, sizeof(szName));
	g_pLTServer->CPrint("%s (%s) Flags:", (pMsg ? pMsg : "Object"), szName);

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);

	if (!dwFlags)
	{
		g_pLTServer->CPrint("  No Flags Set!");
		return;
	}

	// Yes this could be cleaner, but I'm in a hurry...
	if (dwFlags & FLAG_VISIBLE)
	{
		g_pLTServer->CPrint("  Visible");
	}
	if (dwFlags & FLAG_SHADOW)
	{
		g_pLTServer->CPrint("  Shadow");
	}
	if (dwFlags & FLAG_SPRITEBIAS)
	{
		g_pLTServer->CPrint("  Sprite Bias");
	}
	if (dwFlags & FLAG_ROTATABLESPRITE)
	{
		g_pLTServer->CPrint("  Rotatable Sprite");
	}
	if (dwFlags & FLAG_SPRITE_NOZ)
	{
		g_pLTServer->CPrint("  Sprite No Z");
	}
	if (dwFlags & FLAG_GLOWSPRITE)
	{
		g_pLTServer->CPrint("  Glow Sprite");
	}
	if (dwFlags & FLAG_REALLYCLOSE)
	{
		g_pLTServer->CPrint("  Really Close");
	}
	if (dwFlags & FLAG_FOGDISABLE)
	{
		g_pLTServer->CPrint("  Fog Disable");
	}
	if (dwFlags & FLAG_FULLPOSITIONRES)
	{
		g_pLTServer->CPrint("  Full Position Res");
	}
	if (dwFlags & FLAG_NOLIGHT)
	{
		g_pLTServer->CPrint("  No Light");
	}
	if (dwFlags & FLAG_PAUSED)
	{
		g_pLTServer->CPrint("  Paused");
	}
	if (dwFlags & FLAG_YROTATION)
	{ 
		g_pLTServer->CPrint("  Y Rotation");
	}
	if (dwFlags & FLAG_RAYHIT)
	{
		g_pLTServer->CPrint("  Ray Hit");
	}
	if (dwFlags & FLAG_SOLID)
	{
		g_pLTServer->CPrint("  Solid");
	}
	if (dwFlags & FLAG_BOXPHYSICS)
	{
		g_pLTServer->CPrint("  Box Physics");
	}
	if (dwFlags & FLAG_CLIENTNONSOLID)
	{
		g_pLTServer->CPrint("  Client Non Solid");
	}
	if (dwFlags & FLAG_TOUCH_NOTIFY)
	{
		g_pLTServer->CPrint("  Touch Notify");
	}
	if (dwFlags & FLAG_GRAVITY)
	{
		g_pLTServer->CPrint("  Gravity");
	}
	if (dwFlags & FLAG_STAIRSTEP)
	{
		g_pLTServer->CPrint("  Stair Step");
	}
	if (dwFlags & FLAG_GOTHRUWORLD)
	{
		g_pLTServer->CPrint("  Go Thru World");
	}
	if (dwFlags & FLAG_DONTFOLLOWSTANDING)
	{
		g_pLTServer->CPrint("  Don't Follow Standing");
	}
	if (dwFlags & FLAG_NOSLIDING)
	{
		g_pLTServer->CPrint("  No Sliding");
	}
	if (dwFlags & FLAG_POINTCOLLIDE)
	{
		g_pLTServer->CPrint("  Point Collide");
	}
	if (dwFlags & FLAG_MODELKEYS)
	{
		g_pLTServer->CPrint("  Model Keys");
	}
	if (dwFlags & FLAG_TOUCHABLE)
	{
		g_pLTServer->CPrint("  Touchable");
	}
	if (dwFlags & FLAG_FORCECLIENTUPDATE)
	{
		g_pLTServer->CPrint("  Force Client Update");
	}
	if (dwFlags & FLAG_REMOVEIFOUTSIDE)
	{
		g_pLTServer->CPrint("  Remove If Outside");
	}
	if (dwFlags & FLAG_FORCEOPTIMIZEOBJECT)
	{
		g_pLTServer->CPrint("  Force Optimize Object");
	}
	if (dwFlags & FLAG_CONTAINER)
	{
		g_pLTServer->CPrint("  Container");
	}

	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags2, dwFlags);

	if (!dwFlags)
	{
		g_pLTServer->CPrint(" No Flags 2 Set!");
		return;
	}
	else
	{
		g_pLTServer->CPrint(" Flags 2:");
	}

	if (dwFlags & FLAG2_FORCEDYNAMICLIGHTWORLD)
	{
		g_pLTServer->CPrint("  Force Dynamic Light World");
	}
	if (dwFlags & FLAG2_ADDITIVE)
	{
		g_pLTServer->CPrint("  Additive");
	}
	if (dwFlags & FLAG2_MULTIPLY)
	{
		g_pLTServer->CPrint("  Multiply");
	}
	if (dwFlags & FLAG2_PLAYERCOLLIDE)
	{
		g_pLTServer->CPrint("  Player Collide");
	}
	if (dwFlags & FLAG2_DYNAMICDIRLIGHT)
	{
		g_pLTServer->CPrint("  Dynamic Dir Light");
	}
	if (dwFlags & FLAG2_SKYOBJECT)
	{
		g_pLTServer->CPrint("  Sky Object");
	}
	if (dwFlags & FLAG2_FORCETRANSLUCENT)
	{
		g_pLTServer->CPrint("  Force Translucent");
	}

#endif // _DEBUG
}

LTRESULT FindNamedObject(const char* szName, ILTBaseClass *& pObject, LTBOOL bMultipleOkay)
{
	if ( !szName || !*szName ) return LT_NOTFOUND;

	GameBaseLite *pLiteObject = g_pGameServerShell->GetLiteObjectMgr()->FindObjectByName(szName);
	if (pLiteObject)
	{
		pObject = (ILTBaseClass*)pLiteObject;
		return LT_OK;
	}

	pObject = LTNULL;
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


void ObjectCPrint(HOBJECT hObject, const char *pMsg, ...)
{
	UBER_ASSERT( g_pLTServer, "ObjectCPrint: No server." );

	va_list marker;
	char msg[500];
	const int msg_length = sizeof(msg);

	va_start(marker, pMsg);
	_vsnprintf(msg, msg_length, pMsg, marker);
	va_end(marker);
	msg[msg_length] = '\0';

	char szName[64];
	if(hObject)
	{
		g_pLTServer->GetObjectName(hObject, szName, sizeof(szName));
	}

	g_pLTServer->CPrint("%f %s : %s",
		g_pLTServer->GetTime(),
		hObject ? szName : "",
		msg);
}

void ObjectCPrint(const char *pName, const char *pMsg, ...)
{
	UBER_ASSERT( g_pLTServer, "ObjectCPrint: No server." );

	va_list marker;
	char msg[500];
	const int msg_length = sizeof(msg);

	va_start(marker, pMsg);
	_vsnprintf(msg, msg_length, pMsg, marker);
	va_end(marker);
	msg[msg_length] = '\0';

	g_pLTServer->CPrint("%f %s : %s",
		g_pLTServer->GetTime(),
		pName,
		msg);
}

static void ParseTriggerMsg(const char* szMsg, char* aszMsgs[256], uint32* pcMsgs)
{
	static char szMsgCopy[1024];
	strcpy(szMsgCopy, szMsg);

	// Start with no messages
	*pcMsgs = 0;

	// We're done if it's empty
	if (!*szMsg)
		return;

	// Remember where the first message is
	aszMsgs[(*pcMsgs)++] = szMsgCopy;

	// How deep in parenthesis are we?
	uint32 nParenDepth = 0;
	// Are we in quotes?
	bool bInQuote = false;
	// Where are we in the message?
	char *pFinger = szMsgCopy;

	// Scan...
	while (*pFinger)
	{
		// What character are we looking at?
		switch (*pFinger)
		{
			// Increase the parenthesis depth unless we're in quotes
			case '(' :
			{
				if (bInQuote)
					break;
				++nParenDepth;
				break;
			}
			// Decrease the parenthesis depth unless we're in quotes
			case ')' :
			{
				if (bInQuote || !nParenDepth)
					break;
				--nParenDepth;
				break;
			}
			// Toggle the quote state
			case '"' :
			{
				bInQuote = !bInQuote;
				break;
			}
			// End the message if we're not in quotes or parentheses
			case ';' :
			{
				if (bInQuote || nParenDepth)
					break;
				*pFinger = 0;
				aszMsgs[(*pcMsgs)++] = pFinger + 1;
				break;
			}
			default :
				break;
		}

		// Next!
		++pFinger;
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

    const char* pMsg = g_pLTServer->GetStringData(hMsg);

    const char* pName = g_pLTServer->GetStringData(hName);

	return SendTriggerMsgToObjects(pSender, pName, pMsg);
}

// This version is used to support shared code between projects...

LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, const char* pName, const char* pMsg)
{
	// Check parameters.
	if( !pMsg )
	{
		ASSERT( !"SendTriggerMsgToObjects:  Invalid parameters." );
		return LTFALSE;
	}

	// Some commandmgr messages don't need a target.
	if( !pName || !pName[0] )
	{
		if (g_pCmdMgr->IsValidCmd(pMsg))
		{
			return g_pCmdMgr->Process( pMsg, pSender ? pSender->m_hObject : NULL, LTNULL );
		}
		else
		{
			ASSERT( !"SendTriggerMsgToObjects:  Invalid parameters." );
			return LTFALSE;
		}
	}


	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
	bool bSendToActivePlayer	= false;
	bool bSendToOtherPlayers	= false;
	bool bSendToPlayer			= false;

	// If the sender is a player, then store it
	// as the ActivePlayer.  Triggers can now be sent to an object called "ActivePlayer"
	// and it will go to this object.  Triggers sent to "OtherPlayers" will go
	// to all players except the "ActivePlayer".
	CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( pSender );
	if( pPlayerObj )
	{
		g_pGameServerShell->SetActivePlayer( pSender->m_hObject );
	}

	// See if the message is getting sent to the active player or the other players.
	// This is the way LD can send messages to specific players instead of all players.
	// The active player is set whenever a player sends a trigger message.  The "otherplayers"
	// are the players other than the "activeplayer".
	if( stricmp( pName, "ActivePlayer" ) == 0 )
	{
		bSendToActivePlayer = true;
	}
	else if( stricmp( pName, "OtherPlayers" ) == 0 )
	{
		bSendToOtherPlayers = true;
	}
	
	if( bSendToActivePlayer || bSendToOtherPlayers ||
		(stricmp( pName, "player" ) == 0) )
	{
		// We're trying to send to a player...

		bSendToPlayer = true;
	}

	// Get the active player.
	HOBJECT hActivePlayer = g_pGameServerShell->GetActivePlayer( );

	// Send to the active player.
	if( bSendToActivePlayer )
	{
		if( hActivePlayer )
			objArray.AddObject( hActivePlayer );
	}
	// Send to all players except the active player.
	else if( bSendToOtherPlayers )
	{
		// Get the active player if there is any.
		CPlayerObj* pActivePlayer = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hActivePlayer ));

		// Go through all the players and add them to the list except 
		// the active player.
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
		{
			CPlayerObj* pPlayerObj = *iter;

			// Send the message if this is not the active player.
			if( pPlayerObj && pPlayerObj != pActivePlayer )
			{
				objArray.AddObject( pPlayerObj->m_hObject );
			}

			iter++;
		}
	}
	// Do default name processing.
	else
	{
		g_pLTServer->FindNamedObjects(pName, objArray);

		// Process the message as a command if it is a valid command...

		if (g_pCmdMgr->IsValidCmd(pMsg))
		{
			return g_pCmdMgr->Process( pMsg, pSender ? pSender->m_hObject : NULL, 
				objArray.NumObjects( ) ? objArray.GetObject(0) : LTNULL );
		}
	}

	int nNumObjects = objArray.NumObjects( );

	if( (nNumObjects == 0) && bSendToPlayer )
	{
		static CParsedMsg::CToken s_cTok_Objective("Objective");
		static CParsedMsg::CToken s_cTok_Option("Option");
		static CParsedMsg::CToken s_cTok_Parameter("Parameter");

		// We want to send to a player but we couldn't find one.
		// Send the message to the server shell and see if we can process it there...

		ConParse parse;
		parse.Init( pMsg );

		while( g_pCommonLT->Parse( &parse ) == LT_OK )
		{
			// Don't parse empty messages
			if( !parse.m_nArgs || !parse.m_Args[0] )
				continue;

			CParsedMsg cCurMsg( parse.m_nArgs, parse.m_Args );

			if( cCurMsg.GetArg(0) == s_cTok_Objective ||
				cCurMsg.GetArg(0) == s_cTok_Option ||
				cCurMsg.GetArg(0) == s_cTok_Parameter )
			{
				g_pGameServerShell->ProcessObjectiveMessage( cCurMsg, LTNULL );
			}
		}
	}

	// Send the trigger to all the found objects.
	for (int i = 0; i < nNumObjects; i++)
	{
		SendTriggerMsgToObject(pSender, objArray.GetObject(i), 0, pMsg);
	}


	// LiteObject's aren't in the same name space as normal objects, so don't
	// bother looking if we already found an object above...
	if (0 == nNumObjects)
	{
		// Send the trigger to all lite objects with that name
		ILTBaseClass* pLiteObject = g_pGameServerShell->GetLiteObjectMgr()->FindObjectByName(pName);
		if (pLiteObject)
		{
			SendTriggerMsgToObject(pSender, (ILTBaseClass*)pLiteObject, 0, pMsg);
		}
	}

    return LTTRUE;
}

void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg)
{
    const char* szMessage = g_pLTServer->GetStringData(hMsg);

	SendTriggerMsgToObject( pSender, hObj, FALSE, szMessage );
}

// This version is used to support shared code between projects...

void SendTriggerMsgToObject(LPBASECLASS pSender, LPBASECLASS pObj, LTBOOL, const char* szMessage)
{
	// Check parameters.
	if( !szMessage )
	{
		ASSERT( !"SendTriggerMsgToObject:  Invalid parameters." );
		return;
	}

	// Some commandmgr messages don't require a target.
	if( !pObj )
	{
		if (g_pCmdMgr->IsValidCmd(szMessage))
		{
			g_pCmdMgr->Process(szMessage, pSender, pObj);
		}
		else
		{
			ASSERT( !"SendTriggerMsgToObject:  Invalid parameters." );
		}

		return;
	}



	// If the sender is a player, then store it
	// as the ActivePlayer.  Triggers can now be sent to an object called "ActivePlayer"
	// and it will go to this object.  Triggers sent to "OtherPlayers" will go
	// to all players except the "ActivePlayer".
	// Even though this function gets called by SendTriggerMsgToObjects and that function
	// also sets the active player, plenty of objects call this function directly, so
	// we have to do this here too.
	CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( pSender );
	if( pPlayerObj )
	{
		g_pGameServerShell->SetActivePlayer( pSender->m_hObject );
	}

	// Process the message as a command if it is a valid command...

	if (g_pCmdMgr->IsValidCmd(szMessage))
	{
		g_pCmdMgr->Process(szMessage, pSender, pObj);
		return;
	}

	if (g_ShowTriggersTrack.GetFloat() != 0.0f)
	{
		// [KLS 9/7/02] Can't call GetObjectName() for two different variables used in
		// the same scope as that function uses a static buffer.  ALWAYS BEWARE a function
		// that returns a char* (or even const char*) they are almost ALWAYS EVIL!

		// Use szSenderName as a temp buffer for pSendName...so pSendName and pRecvName
		// won't always be the same name (well, except for when "Command Manager" is used
		// for pSendName ;)

		char szSenderName[256] = {0};
		const char *pSendName;
	
		if (pSender)
		{
			g_pLTServer->GetObjectName(pSender->m_hObject, szSenderName, sizeof(szSenderName));
			pSendName = szSenderName;
		}
		else
		{
			pSendName = "Command Manager";
		}

		// Okay to call GetObjectName() here as we'll use it only for pRecvName...
        const char* pRecvName = GetObjectName(pObj);
		const char* pFilter   = g_ShowTriggersFilter.GetStr();

		// Filter out displaying any unwanted messages...

        LTBOOL bPrintMsg = (!pFilter || !pFilter[0]);
		if (!bPrintMsg)
		{
            bPrintMsg = (szMessage ? !strstr(pFilter, szMessage) : LTTRUE);
		}

		if (bPrintMsg)
		{
            g_pLTServer->CPrint(" ");
            g_pLTServer->CPrint("Message:    %s", szMessage ? szMessage : "NULL");
			g_pLTServer->CPrint("Sent from: '%s' to '%s'", pSendName, pRecvName ? pRecvName : "OBJECT NOT FOUND");
            g_pLTServer->CPrint(" ");
		}
	}

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_TRIGGER);
	cMsg.Writeuint32((uint32)szMessage);
	pObj->ObjectMessageFn((pSender) ? pSender->m_hObject : LTNULL, cMsg.Read());
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

LTBOOL IsCharacter( ILTBaseClass *pObject )
{
    static HCLASS hCharacterTest = g_pLTServer->GetClass( "CCharacter" );
	HCLASS hClass;
	if (pObject && !pObject->m_hObject)
		hClass = ((GameBaseLite*)pObject)->GetClass();
	else
		hClass = pObject ? g_pLTServer->GetObjectClass(pObject->m_hObject) : LTNULL;
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
// IsBody
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
// IsExplosion
//
// Checks if handle is a handle to an Explosion
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsExplosion( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "Explosion" );
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
// IsWorldModel
//
// Checks if handle is a handle to a WorldModel
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsWorldModel( HOBJECT hObject )
{
	static HCLASS hTest = g_pLTServer->GetClass( "WorldModel" );
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
	if (!IsTeamGameType())
		return INVALID_TEAM;

	CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( hPlayer ));
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
	if (!IsTeamGameType())
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
	static int nLen = strlen( szTeam );
	uint32 nTeamId = INVALID_TEAM;
	if( !_strnicmp( pszTeamString, szTeam, nLen ))
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
// MoveObjectToFloor
//
// Move the object down to the floor (or down to rest on an object)
// Arguments:
//		hObject - handle to object to move
// Return:
//		True if object was moved
//-------------------------------------------------------------------------------------------

LTBOOL MoveObjectToFloor(HOBJECT hObj, HOBJECT *pFilterList, ObjectFilterFn pFilterFn)
{
    if (!hObj) return LTFALSE;

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
            const char* sValue = g_pLTServer->GetVarValueString(hVar);
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

void Warn(const char* szFormat, ...)
{
#ifndef _FINAL
	static char szBuffer[4096];
	va_list val;
	va_start(val, szFormat);
	vsprintf(szBuffer, szFormat, val);
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

const char *GetObjectName(HOBJECT hObj)
{
	static char aName[256];
	ASSERT(g_pLTServer);
	LTRESULT nResult = g_pLTServer->GetObjectName(hObj, aName, sizeof(aName));
	return nResult == LT_OK ? aName : LTNULL;
}

const char *GetObjectName(ILTBaseClass *pObj)
{
	if (!pObj)
		return LTNULL;
	else if (!pObj->m_hObject)
		return ((GameBaseLite*)pObj)->GetName();
	else
		return GetObjectName(pObj->m_hObject);
}

char g_szString[4096];
GenericProp g_gp;

// Check if in multiplayer game.
bool IsMultiplayerGame()
{
    return !!(g_pGameServerShell->GetGameType() != eGameTypeSingle);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayClientFX
//
//	PURPOSE:	Plays a fire-and-forget clientFX at a specific location 
//				in the world.
//
//	NOTE:		(pvPos is ignored if hParent is passed in)
//
//				Since these are fire & forget, dwFlags should never
//				include looping
// ----------------------------------------------------------------------- //
void PlayClientFX(char* szFXName, HOBJECT hParent, LTVector* pvPos, LTRotation *prRot, uint32 dwFlags)
{
	PlayClientFX( szFXName, hParent, LTNULL, pvPos, prRot, LTNULL, dwFlags );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayClientFX
//
//	PURPOSE:	Plays a fire-and-forget clientFX at a specific location 
//				in the world using data from a target object.
//
//	NOTE:		(pvPos is ignored if hParent is passed in)
//
//				Since these are fire & forget, dwFlags should never
//				include looping
// ----------------------------------------------------------------------- //
void PlayClientFX(char* szFXName, HOBJECT hParent, HOBJECT hTarget, LTVector* pvPos, LTRotation *prRot, LTVector *pvTargetPos, uint32 dwFlags)
{
	LTVector vPos;
	LTRotation rRot;
	if(hParent)
	{
		g_pLTServer->GetObjectPos(hParent, &vPos);
		g_pLTServer->GetObjectRotation(hParent, &rRot);
	}
	else if( !pvPos || !prRot )
	{
			// [KLS 05/19/02] Don't do the fx if the position is bogus...
		g_pLTServer->CPrint("ERROR in PlayClientFX()!  Invalid position specified for FX: %s!", szFXName);
		return;
	}
	else
	{
		vPos = *pvPos;
		rRot = *prRot;
	}
	

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_CLIENTFXGROUPINSTANT);
	cMsg.WriteString(szFXName);
	cMsg.Writeuint32(dwFlags);
	cMsg.WriteCompPos(vPos);
	cMsg.WriteCompLTRotation(rRot);
	cMsg.WriteObject(hParent);

	// Write the target information if we have any...
	
	if( hTarget || pvTargetPos )
	{
		cMsg.Writeuint8( true );
		cMsg.WriteObject( hTarget );

		LTVector vTargetPos;
		if( hTarget )
		{
			g_pLTServer->GetObjectPos( hTarget, &vTargetPos );
		}
		else
		{
			vTargetPos = *pvTargetPos;
		}

		cMsg.WriteCompPos( vTargetPos );
	}
	else
	{
		cMsg.Writeuint8( false );
	}

	g_pLTServer->SendSFXMessage(cMsg.Read(), vPos, 0);
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

	CPlayerObj* pPlayerObj = ( CPlayerObj* )g_pLTServer->GetClientUserData( hClient );
	if( !pPlayerObj )
		return NULL;

	return pPlayerObj;
}

