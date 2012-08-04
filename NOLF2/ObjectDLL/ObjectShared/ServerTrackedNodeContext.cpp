// ----------------------------------------------------------------------- //
//
// MODULE  : ServerTrackedNodeContext.cpp
//
// PURPOSE : ServerTrackedNodeContext implementation
//           Manages the current node tracking settings per model instance
//           on the server.
//
// CREATED : 3/27/02
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerTrackedNodeContext.h"
#include "MsgIDs.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerTrackedNodeContext::CServerTrackedNodeContext
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CServerTrackedNodeContext::CServerTrackedNodeContext()
{
	for( uint32 iGroup=0; iGroup < kTrack_Count; ++iGroup )
	{
		m_TrackedNodeTargets[iGroup].eTrackingTarget = kTrackTarget_None;
		m_TrackedNodeTargets[iGroup].hTarget = LTNULL;
		m_TrackedNodeTargets[iGroup].hTargetNode = LTNULL;
	}

	m_bRefreshClient = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerTrackedNodeContext::~CServerTrackedNodeContext
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerTrackedNodeContext::~CServerTrackedNodeContext()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerTrackedNodeContext::SetActiveTrackingGroup
//
//	PURPOSE:	Turn on a tracking group, and turn off previous group.
//
// ----------------------------------------------------------------------- //

void CServerTrackedNodeContext::SetActiveTrackingGroup(EnumTrackedNodeGroup eGroup)
{
	if( eGroup != m_eActiveTrackingGroup )
	{
		super::SetActiveTrackingGroup( eGroup );

		// Tell the client to SetActiveTrackingGroup.

		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8( MID_SFX_MESSAGE );
		cClientMsg.Writeuint8( SFX_CHARACTER_ID );
		cClientMsg.WriteObject( m_hModel );
		cClientMsg.Writeuint8( CFX_TRACK_TARGET_MSG );
		cClientMsg.Writeuint32( kTrackMsg_ActivateGroup );
		cClientMsg.Writeuint32( eGroup );
		g_pLTServer->SendToClient( cClientMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerTrackedNodeContext::SetTarget
//
//	PURPOSE:	Set a target for a tracking group.
//              Overloads for targeting models, objects, or positions.
//
// ----------------------------------------------------------------------- //

bool CServerTrackedNodeContext::SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset)
{
	// Find the specified model node.

	HMODELNODE hNode = LTNULL;
	if( hModel && pszNodeName )
	{
		g_pModelLT->GetNode( hModel, (char*)pszNodeName, hNode );
	}

	if( SetTrackedTarget( eGroup, hModel, hNode, vOffset ) )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //

bool CServerTrackedNodeContext::SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset)
{
	if( super::SetTrackedTarget( eGroup, hModel, hNode, vOffset ) )
	{
		// Record targeting parameters.

		m_TrackedNodeTargets[eGroup].eTrackingTarget = kTrackTarget_Model;
		m_TrackedNodeTargets[eGroup].hTarget = hModel;
		m_TrackedNodeTargets[eGroup].hTargetNode = hNode;
		m_TrackedNodeTargets[eGroup].vTarget = vOffset;

		// Tell the client to SetTarget.

		UpdateClientTarget( eGroup );
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //

bool CServerTrackedNodeContext::SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hObject, const LTVector& vOffset)
{
	if( super::SetTrackedTarget( eGroup, hObject, vOffset ) )
	{
		// Record targeting parameters.

		m_TrackedNodeTargets[eGroup].eTrackingTarget = kTrackTarget_Object;
		m_TrackedNodeTargets[eGroup].hTarget = hObject;
		m_TrackedNodeTargets[eGroup].vTarget = vOffset;

		// Tell the client to SetTarget.

		UpdateClientTarget( eGroup );
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //

bool CServerTrackedNodeContext::SetTrackedTarget(EnumTrackedNodeGroup eGroup, const LTVector& vPosition)
{
	if( super::SetTrackedTarget( eGroup, vPosition ) )
	{
		// Record targeting parameters.

		m_TrackedNodeTargets[eGroup].eTrackingTarget = kTrackTarget_Position;
		m_TrackedNodeTargets[eGroup].vTarget = vPosition;

		// Tell the client to SetTarget.

		UpdateClientTarget( eGroup );
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerTrackedNodeContext::UpdateClientTarget
//
//	PURPOSE:	Update targeting changes from server to client.
//
// ----------------------------------------------------------------------- //

void CServerTrackedNodeContext::UpdateClientTarget(EnumTrackedNodeGroup eGroup)
{
	// Tell the client to SetTarget.

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8( MID_SFX_MESSAGE );
	cClientMsg.Writeuint8( SFX_CHARACTER_ID );
	cClientMsg.WriteObject( m_hModel );
	cClientMsg.Writeuint8( CFX_TRACK_TARGET_MSG );
	cClientMsg.Writeuint32( kTrackMsg_SetTarget );
	cClientMsg.Writeuint32( eGroup );
	cClientMsg.Writeuint32( m_TrackedNodeTargets[eGroup].eTrackingTarget );
	cClientMsg.WriteObject( m_TrackedNodeTargets[eGroup].hTarget );
	cClientMsg.Writeuint32( m_TrackedNodeTargets[eGroup].hTargetNode );
	cClientMsg.WriteLTVector( m_TrackedNodeTargets[eGroup].vTarget );

	g_pLTServer->SendToClient( cClientMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerTrackedNodeContext::UpdateClient
//
//	PURPOSE:	Update changes from server to client.
//
// ----------------------------------------------------------------------- //

void CServerTrackedNodeContext::UpdateClient()
{
	for( uint32 iGroup=0; iGroup < kTrack_Count; ++iGroup )
	{
		// Reset targeting.

		switch( m_TrackedNodeTargets[iGroup].eTrackingTarget )
		{
			case kTrackTarget_Model:
				SetTrackedTarget( (EnumTrackedNodeGroup)iGroup, m_TrackedNodeTargets[iGroup].hTarget, m_TrackedNodeTargets[iGroup].hTargetNode, m_TrackedNodeTargets[iGroup].vTarget );
				break;

			case kTrackTarget_Object:
				SetTrackedTarget( (EnumTrackedNodeGroup)iGroup, m_TrackedNodeTargets[iGroup].hTarget, m_TrackedNodeTargets[iGroup].vTarget );
				break;

			case kTrackTarget_Position:
				SetTrackedTarget( (EnumTrackedNodeGroup)iGroup, m_TrackedNodeTargets[iGroup].vTarget );
				break;
		}
	}

	// Re-activate tracking group.

	SetActiveTrackingGroup( m_eActiveTrackingGroup );

	m_bRefreshClient = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerTrackedNodeContext::Update
//
//	PURPOSE:	Check limits.
//
// ----------------------------------------------------------------------- //

void CServerTrackedNodeContext::Update()
{
	for( uint32 iGroup=0; iGroup < kTrack_Count; ++iGroup )
	{
		if( m_cTrackedNodesPerGroup[iGroup] == 0 )
		{
			continue;
		}

		EnumTrackedNodeGroup eGroup = (EnumTrackedNodeGroup)iGroup;

if( iGroup != 1 ) continue;

		if( !IsAtDiscomfort( eGroup ) )
		{
			TRACE( "DISCOMFORT: %f\n", g_pLTServer->GetTime() );
		}
		else {
			TRACE( "NOT DISCOMFORT: %f\n", g_pLTServer->GetTime() );
		}

//		if( !IsAtLimit( eGroup ) )
//		{
//			continue;
//		}

/**
		// Get the orientation of the first child, to see if the hierarchy
		// of nodes has hit its Y-rotation limit.

		LTVector vRight, vUp, vForward, vPos;
		m_pTrackedNodeMgr->GetBasisSpace( 
			m_aTrackedNodeGroups[iGroup][0],
			vRight, vUp, vForward, vPos);

		vForward.y = 0.f;

		LTRotation rRot;
		g_pLTServer->GetObjectRotation( m_hModel, &rRot);
		LTVector vModelForward = rRot.Forward();

		LTFLOAT fMaxY = g_pModelButeMgr->GetTrackingNodeMaxAngleY( m_eTrackingNodes[eGroup], (ModelTrackingNode)0 );

		LTFLOAT fDot = vForward.Dot( vModelForward );
LTFLOAT f1 = (LTFLOAT)acos(fDot);
LTFLOAT f2 = DEG2RAD(fMaxY);

		if( acos(fDot) >= DEG2RAD(fMaxY) )
		{
			TRACE( "HIT LIMIT\n" ); 
		}
**/
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerTrackedNodeContext::Save / Load
//
//	PURPOSE:	Save / Load.
//
// ----------------------------------------------------------------------- //

void CServerTrackedNodeContext::Save(ILTMessage_Write *pMsg)
{
	// Some values get reset by Init(). Intentionally not saving:
	// m_pTrackedNodeMgr
	// m_hModel
	// m_aTrackedNodeGroups[kTrack_Count]
	// m_cTrackedNodesPerGroup[kTrack_Count]
	// m_bTrackedNodesInitialized

	SAVE_DWORD(	m_eActiveTrackingGroup );

	for( uint32 iGroup=0; iGroup < kTrack_Count; ++iGroup )
	{
		SAVE_DWORD( m_TrackedNodeTargets[iGroup].eTrackingTarget );
		SAVE_HOBJECT( m_TrackedNodeTargets[iGroup].hTarget );
		SAVE_DWORD( m_TrackedNodeTargets[iGroup].hTargetNode );
		SAVE_VECTOR( m_TrackedNodeTargets[iGroup].vTarget );
	}
}

// ----------------------------------------------------------------------- //

void CServerTrackedNodeContext::Load(ILTMessage_Read *pMsg)
{
	// Some values get reset by Init(). Intentionally not loading:
	// m_pTrackedNodeMgr
	// m_hModel
	// m_aTrackedNodeGroups[kTrack_Count]
	// m_cTrackedNodesPerGroup[kTrack_Count]
	// m_bTrackedNodesInitialized

	LOAD_DWORD_CAST( m_eActiveTrackingGroup, EnumTrackedNodeGroup );

	for( uint32 iGroup=0; iGroup < kTrack_Count; ++iGroup )
	{
		LOAD_DWORD_CAST( m_TrackedNodeTargets[iGroup].eTrackingTarget, EnumTrackTarget );
		LOAD_HOBJECT( m_TrackedNodeTargets[iGroup].hTarget );
		LOAD_DWORD( m_TrackedNodeTargets[iGroup].hTargetNode );
		LOAD_VECTOR( m_TrackedNodeTargets[iGroup].vTarget );
	}

	m_bRefreshClient = true;
}


