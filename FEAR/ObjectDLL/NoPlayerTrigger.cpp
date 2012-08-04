// ----------------------------------------------------------------------- //
//
// MODULE  : NoPlayerTrigger.cpp
//
// PURPOSE : NoPlayerTrigger - Implementation
//
// CREATED : 4/5/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "Stdafx.h"
	#include "PlayerObj.h"
	#include "NoPlayerTrigger.h"


LINKFROM_MODULE( NoPlayerTrigger );

BEGIN_CLASS( NoPlayerTrigger )

	// Overrides
	
	ADD_BOOLPROP_FLAG(PlayerTriggerable, 0, PF_HIDDEN, "This flag toggles whether or not the players bounding box intersecting that of the Trigger will activate it.")
	ADD_BOOLPROP_FLAG(AITriggerable, 0, PF_HIDDEN, "This flag toggles whether or not an AIs bounding box intersecting that of the Trigger will activate it.")
	ADD_STRINGPROP_FLAG(AITriggerName, "", PF_HIDDEN, "Triggers that are set to be AI triggerable can be set up to only respond to a specific AI. Enter the name of the specific AI that you want to activate the trigger in this field.")
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_HIDDEN, "Not implemented yet for NoPlayerTrigger yet.")

END_CLASS( NoPlayerTrigger, Trigger, "NoPlayerTrigger objects are Triggers that will activate only if no players are within the trigger." )

CMDMGR_BEGIN_REGISTER_CLASS( NoPlayerTrigger )
CMDMGR_END_REGISTER_CLASS( NoPlayerTrigger, Trigger )

#define UPDATE_DELTA					0.1f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NoPlayerTrigger::NoPlayerTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

NoPlayerTrigger::NoPlayerTrigger() : Trigger()
{
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NoPlayerTrigger::~NoPlayerTrigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

NoPlayerTrigger::~NoPlayerTrigger()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NoPlayerTrigger::~NoPlayerTrigger()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 NoPlayerTrigger::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			uint32 dwRet = Trigger::EngineMessageFn(messageID, pData, fData);
			
			Update();

			return dwRet;
		}
		break;

		default : break;
	}


	return Trigger::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

bool NoPlayerTrigger::Update()
{
	SetNextUpdate( UPDATE_DELTA );
	
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
	// NO players in the game are within the trigger...

	HOBJECT hObj;
	bool	bPlayersInTrigger = false;

	ObjectLink *pLink = pObjList->m_pFirstLink;
	while( pLink )
	{
		hObj = pLink->m_hObject;
		
		if( hObj && IsPlayer( hObj ))
		{
			bPlayersInTrigger = true;
			break;
		}
	
		pLink = pLink->m_pNext;
	}
	
	g_pLTServer->RelinquishList( pObjList );

	if( bPlayersInTrigger )
		return false;

	SetNextUpdate( UPDATE_NEVER );

	// Ok! All the players are acounted for.
	// Let the base Trigger object activate.

	return Trigger::Activate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NoPlayerTrigger::~Activate()
//
//	PURPOSE:	Activate the trigger if appropriate...
//
// ----------------------------------------------------------------------- //

bool NoPlayerTrigger::Activate()
{
	// Start the update

	SetNextUpdate( UPDATE_DELTA );

	return true;
}
