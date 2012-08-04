// ----------------------------------------------------------------------- //
//
// MODULE  : ClientTrackedNodeContext.cpp
//
// PURPOSE : ClientTrackedNodeContext implementation
//           Manages the current node tracking settings per model instance
//           on the client.
//
// CREATED : 3/27/02
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientTrackedNodeContext.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientTrackedNodeContext::CClientTrackedNodeContext
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CClientTrackedNodeContext::CClientTrackedNodeContext()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientTrackedNodeContext::~CClientTrackedNodeContext
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CClientTrackedNodeContext::~CClientTrackedNodeContext()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientTrackedNodeContext::HandleServerMessage
//
//	PURPOSE:	Handle server message, changing context settings.
//
// ----------------------------------------------------------------------- //

void CClientTrackedNodeContext::HandleServerMessage(ILTMessage_Read *pMsg)
{
	EnumTrackMsg eTrackMsg = (EnumTrackMsg)pMsg->Readuint32();

	switch( eTrackMsg )
	{
		case kTrackMsg_ActivateGroup:
			HandleActivateGroup( pMsg );
			break;

		case kTrackMsg_SetTarget:
			HandleSetTarget( pMsg );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientTrackedNodeContext:: Message handlers
//
//	PURPOSE:	Handle server messages, changing context settings.
//
// ----------------------------------------------------------------------- //

void CClientTrackedNodeContext::HandleActivateGroup(ILTMessage_Read *pMsg)
{
	EnumTrackedNodeGroup eGroup = (EnumTrackedNodeGroup)pMsg->Readuint32();
	SetActiveTrackingGroup( eGroup );
}

// ----------------------------------------------------------------------- //

void CClientTrackedNodeContext::HandleSetTarget(ILTMessage_Read *pMsg)
{
	EnumTrackedNodeGroup eGroup = (EnumTrackedNodeGroup)pMsg->Readuint32();
	EnumTrackTarget eTrackingTarget = (EnumTrackTarget)pMsg->Readuint32();
	HOBJECT hTarget = pMsg->ReadObject();
	HMODELNODE hNode = pMsg->Readuint32();
	LTVector vTarget = pMsg->ReadLTVector();

	// Set targeting.

	switch( eTrackingTarget )
	{
		case kTrackTarget_Model:
			SetTrackedTarget( eGroup, hTarget, hNode, vTarget );
			break;

		case kTrackTarget_Object:
			SetTrackedTarget( eGroup, hTarget, vTarget );
			break;

		case kTrackTarget_Position:
			SetTrackedTarget( eGroup, vTarget );
			break;
	}
}

