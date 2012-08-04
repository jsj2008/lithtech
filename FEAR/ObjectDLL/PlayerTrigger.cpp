// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerTrigger.cpp
//
// PURPOSE : PlayerTrigger - Implementation
//
// CREATED : 3/26/02
//
// (c) 2002-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "Stdafx.h"
	#include "PlayerObj.h"
	#include "PlayerTrigger.h"
	#include "StringUtilities.h"


LINKFROM_MODULE( PlayerTrigger );

BEGIN_CLASS( PlayerTrigger )

	// Overrides
	
	ADD_BOOLPROP_FLAG(PlayerTriggerable, 1, PF_HIDDEN, "This flag toggles whether or not the players bounding box intersecting that of the Trigger will activate it.")
	ADD_BOOLPROP_FLAG(AITriggerable, 0, PF_HIDDEN, "This flag toggles whether or not an AIs bounding box intersecting that of the Trigger will activate it.")
	ADD_STRINGPROP_FLAG(AITriggerName, "", PF_HIDDEN, "Triggers that are set to be AI triggerable can be set up to only respond to a specific AI. Enter the name of the specific AI that you want to activate the trigger in this field.")

	// New Props...

	ADD_STRINGIDPROP_FLAG( PlayerInsideID, "", 0, "Specifies a string ID that will be displayed as a message to players that are inside the trigger." )
	ADD_STRINGIDPROP_FLAG( PlayerOutsideID, "", 0, "Specifies a string ID that will be displayed as a message to players that are outside the trigger." )

END_CLASS( PlayerTrigger, Trigger, "PlayerTrigger objects are Triggers that will activate only when all players in a level are within the trigger.  Used for multiplayer." )

CMDMGR_BEGIN_REGISTER_CLASS( PlayerTrigger )
CMDMGR_END_REGISTER_CLASS( PlayerTrigger, Trigger )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerTrigger::PlayerTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PlayerTrigger::PlayerTrigger() : Trigger()
{
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerTrigger::~PlayerTrigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

PlayerTrigger::~PlayerTrigger()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PlayerTrigger::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct *)pData;
			if (!pStruct) return 0;

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProps(&pStruct->m_cProperties);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if( fData != INITIALUPDATE_SAVEGAME )
			{
				InitialUpdate();
			}
		}
		break;

		default : break;
	}
	
	return Trigger::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerTrigger::ReadProps
//
//	PURPOSE:	Read in the property values...
//
// ----------------------------------------------------------------------- //

bool PlayerTrigger::ReadProps( const GenericPropList *pProps )
{
	if( !pProps )
		return false;

	CreateTriggerCreateStruct( );
	if( !m_pTriggerCS )
		return false;

	m_pTriggerCS->nPlayerInsideID = IndexFromStringID( pProps->GetStringID("PlayerInsideID", "") );
	m_pTriggerCS->nPlayerOutsideID = IndexFromStringID( pProps->GetStringID("PlayerOutsideID", "") );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerTrigger::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

bool PlayerTrigger::InitialUpdate()
{
	// Always send the specialfx message for PlayerTriggers...
	
	m_bSendTriggerFXMsg = true;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerTrigger::~Activate()
//
//	PURPOSE:	Activate the trigger if appropriate...
//
// ----------------------------------------------------------------------- //

bool PlayerTrigger::Activate()
{
	if (g_pGameServerShell->IsPaused())
	{
		return false;
	}

	// Find all the objects within the trigger...

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	LTVector vMin = vPos - vDims;
	LTVector vMax = vPos + vDims;

	ObjectList *pObjList = g_pLTServer->GetBoxIntersecters( &vMin, &vMax );
	if( !pObjList )
		return false;

	// Count the number of players in the trigger and activate only if
	// ALL players in the game are within the trigger...

	HOBJECT hObj;
	uint32	nPlayersInGame = CPlayerObj::GetNumberPlayersWithClients( );

	// Don't trigger if there are no players in the game.
	if( nPlayersInGame == 0 )
		return false;

	uint32	nPlayersInTrigger = 0;

	ObjectLink *pLink = pObjList->m_pFirstLink;
	while( pLink )
	{
		hObj = pLink->m_hObject;
		pLink = pLink->m_pNext;

		// Only count live players
		CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hObj ));
		if( !pPlayerObj || pPlayerObj->GetPlayerState( ) != ePlayerState_Alive )
			continue;

		++nPlayersInTrigger;
	}
	
	g_pLTServer->RelinquishList( pObjList );

	if( nPlayersInTrigger != nPlayersInGame )
		return false;

	// Ok! All the players are acounted for.
	// Let the base Trigger object activate.

	return Trigger::Activate();
}
