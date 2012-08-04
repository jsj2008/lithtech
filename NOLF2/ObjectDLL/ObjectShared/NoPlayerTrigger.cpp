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
	
	#include "stdafx.h"
	#include "PlayerObj.h"
	#include "NoPlayerTrigger.h"


LINKFROM_MODULE( NoPlayerTrigger );

BEGIN_CLASS( NoPlayerTrigger )

	// Overrides
	
	ADD_BOOLPROP_FLAG(PlayerTriggerable, 0, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(AITriggerable, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(AITriggerName, "", PF_HIDDEN)
	ADD_BOOLPROP_FLAG(BodyTriggerable, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(BodyTriggerName, "", PF_HIDDEN)

END_CLASS_DEFAULT( NoPlayerTrigger, Trigger, NULL, NULL )

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

uint32 NoPlayerTrigger::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
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

LTBOOL NoPlayerTrigger::Update()
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
		return LTFALSE;

	// Count the number of players in the trigger and activate only if
	// NO players in the game are within the trigger...

	HOBJECT hObj;
	uint32	nPlayersInGame = CPlayerObj::GetNumberPlayersWithClients( );
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
		return LTFALSE;

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

LTBOOL NoPlayerTrigger::Activate()
{
	// Start the update

	SetNextUpdate( UPDATE_DELTA );

	return LTTRUE;
}