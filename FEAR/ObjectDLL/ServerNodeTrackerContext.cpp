// ----------------------------------------------------------------------- //
//
// MODULE  : ServerNodeTrackerContext.cpp
//
// PURPOSE : ServerNodeTrackerContext implementation
//           Manages the current node tracking settings per model instance
//           on the server.
//
// CREATED : 11/17/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ServerNodeTrackerContext.h"
#include "MsgIDs.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::CServerNodeTrackerContext
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CServerNodeTrackerContext::CServerNodeTrackerContext()
{
	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		m_TrackedNodeTargets[iGroup].eTrackerTarget = CNodeTracker::eTarget_Aimer;
		m_TrackedNodeTargets[iGroup].hTarget = NULL;
		m_TrackedNodeTargets[iGroup].hTargetNode = (HMODELNODE)NULL;
	}

	m_fLastUpdateTime = 0.f;
	m_bRefreshClient = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::~CServerNodeTrackerContext
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerNodeTrackerContext::~CServerNodeTrackerContext()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::SetActiveTrackerGroups
//
//	PURPOSE:	Turn on/off tracker groups.
//
// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::SetActiveTrackerGroups( uint32 dwActiveTrackerGroups )
{
	// Bail if nothing has changed.

	if( dwActiveTrackerGroups == m_bitsActiveTrackingFlags.to_ulong() )
	{
		return;
	}

	super::SetActiveTrackerGroups( dwActiveTrackerGroups );

	// Tell the client to SetActiveTrackerGroups.

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8( MID_SFX_MESSAGE );
	cClientMsg.Writeuint8( SFX_CHARACTER_ID );
	cClientMsg.WriteObject( m_hModel );
	cClientMsg.WriteBits(CFX_TRACKER_TARGET_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cClientMsg.Writeuint32( kTrackerMsg_ToggleGroups );
	cClientMsg.Writeuint32( dwActiveTrackerGroups );
	g_pLTServer->SendToClient( cClientMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::SetTarget
//
//	PURPOSE:	Set a target for all tracker groups.
//              Overloads for targeting models, objects, or positions.
//
// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::SetTrackedTarget( HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset )
{
	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		if( m_NodeTrackerInstances[iGroup].m_pNodeTracker )
		{
			SetTrackedTarget( (EnumNodeTrackerGroup)iGroup, hModel, pszNodeName, vOffset );
		}
	}
}

// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::SetTrackedTarget( HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset )
{
	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		if( m_NodeTrackerInstances[iGroup].m_pNodeTracker )
		{
			SetTrackedTarget( (EnumNodeTrackerGroup)iGroup, hModel, hNode, vOffset );
		}
	}
}

// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::SetTrackedTarget( HOBJECT hObject, const LTVector& vOffset )
{
	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		if( m_NodeTrackerInstances[iGroup].m_pNodeTracker )
		{
			SetTrackedTarget( (EnumNodeTrackerGroup)iGroup, hObject, vOffset );
		}
	}
}

// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::SetTrackedTarget( const LTVector& vPosition )
{
	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		if( m_NodeTrackerInstances[iGroup].m_pNodeTracker )
		{
			SetTrackedTarget( (EnumNodeTrackerGroup)iGroup, vPosition );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::SetTarget
//
//	PURPOSE:	Set a target for a tracker group.
//              Overloads for targeting models, objects, or positions.
//
// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset )
{
	// Find the specified model node.

	HMODELNODE hNode = (HMODELNODE)NULL;
	if( hModel && pszNodeName )
	{
		g_pModelLT->GetNode( hModel, (char*)pszNodeName, hNode );
	}

	SetTrackedTarget( eGroup, hModel, hNode, vOffset );
}

// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset )
{
	super::SetTrackedTarget( eGroup, hModel, hNode, vOffset );

	// Record targeting parameters.

	m_TrackedNodeTargets[eGroup].eTrackerTarget = CNodeTracker::eTarget_Node;
	m_TrackedNodeTargets[eGroup].hTarget = hModel;
	m_TrackedNodeTargets[eGroup].hTargetNode = hNode;
	m_TrackedNodeTargets[eGroup].vTarget = vOffset;

	// Tell the client to SetTarget.

	UpdateClientTarget( eGroup );
}

// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hObject, const LTVector& vOffset )
{
	super::SetTrackedTarget( eGroup, hObject, vOffset );

	// Record targeting parameters.

	m_TrackedNodeTargets[eGroup].eTrackerTarget = CNodeTracker::eTarget_Object;
	m_TrackedNodeTargets[eGroup].hTarget = hObject;
	m_TrackedNodeTargets[eGroup].vTarget = vOffset;

	// Tell the client to SetTarget.

	UpdateClientTarget( eGroup );
}

// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::SetTrackedTarget( EnumNodeTrackerGroup eGroup, const LTVector& vPosition )
{
	super::SetTrackedTarget( eGroup, vPosition );

	// Record targeting parameters.

	m_TrackedNodeTargets[eGroup].eTrackerTarget = CNodeTracker::eTarget_World;
	m_TrackedNodeTargets[eGroup].vTarget = vPosition;

	// Tell the client to SetTarget.

	UpdateClientTarget( eGroup );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::UpdateClientTarget
//
//	PURPOSE:	Update targeting changes from server to client.
//
// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::UpdateClientTarget( EnumNodeTrackerGroup eGroup )
{
	// Tell the client to SetTarget.

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8( MID_SFX_MESSAGE );
	cClientMsg.Writeuint8( SFX_CHARACTER_ID );
	cClientMsg.WriteObject( m_hModel );
	cClientMsg.WriteBits(CFX_TRACKER_TARGET_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cClientMsg.Writeuint32( kTrackerMsg_SetTarget );
	cClientMsg.Writeuint32( eGroup );
	cClientMsg.Writeuint32( m_TrackedNodeTargets[eGroup].eTrackerTarget );
	cClientMsg.WriteObject( m_TrackedNodeTargets[eGroup].hTarget );
	cClientMsg.Writeuint32( m_TrackedNodeTargets[eGroup].hTargetNode );
	cClientMsg.WriteLTVector( m_TrackedNodeTargets[eGroup].vTarget );

	g_pLTServer->SendToClient( cClientMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::UpdateClient
//
//	PURPOSE:	Update changes from server to client.
//
// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::UpdateClient()
{
	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		// Reset targeting.

		switch( m_TrackedNodeTargets[iGroup].eTrackerTarget )
		{
			case CNodeTracker::eTarget_Node:
				SetTrackedTarget( (EnumNodeTrackerGroup)iGroup, m_TrackedNodeTargets[iGroup].hTarget, m_TrackedNodeTargets[iGroup].hTargetNode, m_TrackedNodeTargets[iGroup].vTarget );
				break;

			case CNodeTracker::eTarget_Object:
				SetTrackedTarget( (EnumNodeTrackerGroup)iGroup, m_TrackedNodeTargets[iGroup].hTarget, m_TrackedNodeTargets[iGroup].vTarget );
				break;

			case CNodeTracker::eTarget_World:
				SetTrackedTarget( (EnumNodeTrackerGroup)iGroup, m_TrackedNodeTargets[iGroup].vTarget );
				break;
		}
	}

	// Re-activate tracker groups.

	SetActiveTrackerGroups( m_bitsActiveTrackingFlags.to_ulong() );

	m_bRefreshClient = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::UpdateNodeTrackers
//
//	PURPOSE:	Update node trackers.
//
// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::UpdateNodeTrackers()
{
	if( m_bRefreshClient )
	{
		UpdateClient();
	}

	// Calculate time delta.

	double fTimeCur = g_pLTServer->GetTime();
	float fTimeDelta = (float)(fTimeCur - m_fLastUpdateTime);
	m_fLastUpdateTime = fTimeCur;

	super::UpdateNodeTrackers( fTimeDelta );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::Save / Load
//
//	PURPOSE:	Save / Load.
//
// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::Save( ILTMessage_Write *pMsg )
{
	// Some values get reset by Init(). Intentionally not saving:
	// m_hModel
	// m_NodeTrackerInstances[kTracker_Count]

	SAVE_DWORD(	m_bitsActiveTrackingFlags.to_ulong() );

	SAVE_DOUBLE( m_fLastUpdateTime );

	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		SAVE_DWORD( m_TrackedNodeTargets[iGroup].eTrackerTarget );
		SAVE_HOBJECT( m_TrackedNodeTargets[iGroup].hTarget );
		SAVE_DWORD( m_TrackedNodeTargets[iGroup].hTargetNode );
		SAVE_VECTOR( m_TrackedNodeTargets[iGroup].vTarget );
	}
}

// ----------------------------------------------------------------------- //

void CServerNodeTrackerContext::Load( ILTMessage_Read *pMsg )
{
	// Some values get reset by Init(). Intentionally not loading:
	// m_hModel
	// m_NodeTrackerInstances[kTracker_Count]

	uint32 dwActiveTrackingGroups;
	LOAD_DWORD(	dwActiveTrackingGroups );
	m_bitsActiveTrackingFlags = NODE_TRACKER_FLAGS( dwActiveTrackingGroups );

	LOAD_DOUBLE( m_fLastUpdateTime );

	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		LOAD_DWORD_CAST( m_TrackedNodeTargets[iGroup].eTrackerTarget, CNodeTracker::ETargetType );
		LOAD_HOBJECT( m_TrackedNodeTargets[iGroup].hTarget );
		LOAD_DWORD( m_TrackedNodeTargets[iGroup].hTargetNode );
		LOAD_VECTOR( m_TrackedNodeTargets[iGroup].vTarget );
	}

	m_bRefreshClient = true;
}


