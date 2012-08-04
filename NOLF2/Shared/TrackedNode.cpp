#include "stdafx.h"
#include "TrackedNode.h"

CTrackedNode::CTrackedNode() :
	m_hModel(NULL),
	m_hNode(INVALID_MODEL_NODE),
	m_hTrackObject(NULL),
	m_hTrackNode(INVALID_MODEL_NODE),
	m_pChild(NULL),
	m_pMimicNode(NULL),

	m_eTrackMode(TRACK_LOCALPOS),
	
	m_bLookingAtTarget(false),
	m_bInDiscomfort(false),
	m_bAtMaxThreshold(false),
	m_bEnabled(false),
	m_bAutoDisable(false),
	m_bOrientFromAnim(false),
	m_bSyncOrientation(true),
	m_bIgnoreParentAnimation(false),
	
	m_vActualUp(0.0f, 1.0f, 0.0f),
	m_vActualRight(1.0f, 0.0f, 0.0f),
	m_vActualForward(0.0f, 0.0f, 1.0f),
	m_vActualPos(0.0f, 0.0f, 0.0f),

	m_vAlignUp(0.0f, 1.0f, 0.0f),

	m_vTrackOffset(0.0f, 0.0f, 0.0f),

	m_fMaxAngVel(0.0f),
	
	m_fTanXDiscomfort(0.0f),
	m_fTanYDiscomfort(0.0f),
	m_fTanXThreshold(0.0f),
	m_fTanYThreshold(0.0f),

	m_pNodeMgr(NULL),

	m_LastUpdate(0)
{
	m_mInvTargetTransform.Identity();
}
