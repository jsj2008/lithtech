// ----------------------------------------------------------------------- //
//
// MODULE  : ClientNodeTrackerContext.cpp
//
// PURPOSE : ClientNodeTrackerContext implementation
//           Manages the current node tracking settings per model instance
//           on the client.
//
// CREATED : 11/17/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientNodeTrackerContext.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientNodeTrackerContext::CClientNodeTrackerContext
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CClientNodeTrackerContext::CClientNodeTrackerContext()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientNodeTrackerContext::~CClientNodeTrackerContext
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CClientNodeTrackerContext::~CClientNodeTrackerContext()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientNodeTrackerContext::HandleServerMessage
//
//	PURPOSE:	Handle server message, changing context settings.
//
// ----------------------------------------------------------------------- //

void CClientNodeTrackerContext::HandleServerMessage(ILTMessage_Read *pMsg)
{
	EnumTrackerMsg eTrackMsg = (EnumTrackerMsg)pMsg->Readuint32();

	switch( eTrackMsg )
	{
		case kTrackerMsg_ToggleGroups:
			HandleToggleGroups( pMsg );
			break;

		case kTrackerMsg_SetTarget:
			HandleSetTarget( pMsg );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientNodeTrackerContext:: Message handlers
//
//	PURPOSE:	Handle server messages, changing context settings.
//
// ----------------------------------------------------------------------- //

void CClientNodeTrackerContext::HandleToggleGroups(ILTMessage_Read *pMsg)
{
	SetActiveTrackerGroups( pMsg->Readuint32() );
}

// ----------------------------------------------------------------------- //

void CClientNodeTrackerContext::HandleSetTarget(ILTMessage_Read *pMsg)
{
	EnumNodeTrackerGroup eGroup = (EnumNodeTrackerGroup)pMsg->Readuint32();
	CNodeTracker::ETargetType eTrackingTarget = (CNodeTracker::ETargetType)pMsg->Readuint32();
	HOBJECT hTarget = pMsg->ReadObject();
	HMODELNODE hNode = pMsg->Readuint32();
	LTVector vTarget = pMsg->ReadLTVector();

	// Set targeting.

	switch( eTrackingTarget )
	{
		case CNodeTracker::eTarget_Node:
			SetTrackedTarget( eGroup, hTarget, hNode, vTarget );
			break;

		case CNodeTracker::eTarget_Object:
			SetTrackedTarget( eGroup, hTarget, vTarget );
			break;

		case CNodeTracker::eTarget_World:
			SetTrackedTarget( eGroup, vTarget );
			break;

		case CNodeTracker::eTarget_Aimer:
			SetTrackedTarget( eGroup, LTPolarCoord( vTarget.x, vTarget.y ));
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientNodeTrackerContext::UpdateNodeTrackers
//
//	PURPOSE:	Update node trackers.
//
// ----------------------------------------------------------------------- //

void CClientNodeTrackerContext::UpdateNodeTrackers()
{
	// Calculate time delta.
	
	float fTimeDelta = ObjectContextTimer( m_hModel ).GetTimerElapsedS( );
	super::UpdateNodeTrackers( fTimeDelta );
}


