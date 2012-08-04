// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerTrigger.cpp
//
// PURPOSE : PlayerTrigger - Implementation
//
// CREATED : 3/26/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "stdafx.h"
	#include "PlayerObj.h"
	#include "PlayerTrigger.h"


LINKFROM_MODULE( PlayerTrigger );

BEGIN_CLASS( PlayerTrigger )

	// Overrides
	
	ADD_BOOLPROP_FLAG(PlayerTriggerable, 1, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(AITriggerable, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(AITriggerName, "", PF_HIDDEN)
	ADD_BOOLPROP_FLAG(BodyTriggerable, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(BodyTriggerName, "", PF_HIDDEN)

	// New Props...

	ADD_LONGINTPROP( PlayerInsideID, -1 )
	ADD_LONGINTPROP( PlayerOutsideID, -1 )

END_CLASS_DEFAULT( PlayerTrigger, Trigger, NULL, NULL )

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

uint32 PlayerTrigger::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct *)pData;
			if (!pStruct) return 0;

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProps(pStruct);
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

bool PlayerTrigger::ReadProps( ObjectCreateStruct *pOCS )
{
	if( !pOCS )
		return false;

	GenericProp gProp;

	if( g_pLTServer->GetPropGeneric( "PlayerInsideID", &gProp ) == LT_OK )
	{
		m_TCS.nPlayerInsideID = (uint32)gProp.m_Long;
	}

	if( g_pLTServer->GetPropGeneric( "PlayerOutsideID", &gProp ) == LT_OK )
	{
		m_TCS.nPlayerOutsideID = (uint32)gProp.m_Long;
	}

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

LTBOOL PlayerTrigger::Activate()
{
	if (g_pGameServerShell->IsPaused())
	{
		return LTFALSE;
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
		return LTFALSE;

	// Count the number of players in the trigger and activate only if
	// ALL players in the game are within the trigger...

	HOBJECT hObj;
	uint32	nPlayersInGame = CPlayerObj::GetNumberPlayersWithClients( );

	// Don't trigger if there are no players in the game.
	if( nPlayersInGame == 0 )
		return LTFALSE;

	uint32	nPlayersInTrigger = 0;

	ObjectLink *pLink = pObjList->m_pFirstLink;
	while( pLink )
	{
		hObj = pLink->m_hObject;
		CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hObj ));

		// Only count live players with loaded clients.
		if( pPlayerObj && !pPlayerObj->IsDead( ) && pPlayerObj->IsClientLoaded( ))
		{
			++nPlayersInTrigger;
		}
	
		pLink = pLink->m_pNext;
	}
	
	g_pLTServer->RelinquishList( pObjList );

	if( nPlayersInTrigger != nPlayersInGame )
		return LTFALSE;

	// Ok! All the players are acounted for.
	// Let the base Trigger object activate.

	return Trigger::Activate();
}