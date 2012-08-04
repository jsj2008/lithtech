#ifndef __TRACKEDNODECONTEXT_H__
#define __TRACKEDNODECONTEXT_H__

#include "TrackedNodeMgr.h"

// Groups of tracking nodes exist for different purposes.

enum EnumTrackedNodeGroup
{
	kTrack_None = -1,
	kTrack_LookAt,
	kTrack_AimAt,
	kTrack_Count,
};

// Tracking nodes may track a model, object, or position.

enum EnumTrackTarget
{
	kTrackTarget_None,
	kTrackTarget_Model,
	kTrackTarget_Object,
	kTrackTarget_Position,
};

// Server needs to tell client tracking settings.

enum EnumTrackMsg
{
	kTrackMsg_ActivateGroup,
	kTrackMsg_SetTarget,
};


//
// CTrackedNodeContext:
// Manages the current node tracking settings per model instance
// on the client or server.
//

class CTrackedNodeContext
{
public:
	// Creation/destruction.

			 CTrackedNodeContext();
	virtual ~CTrackedNodeContext();

	void Init(HOBJECT hModel, ModelSkeleton eSkeleton, CTrackedNodeMgr* pTrackedNodeMgr);
	void Term();

	// Activation.

	virtual void SetActiveTrackingGroup(EnumTrackedNodeGroup eGroup);
	EnumTrackedNodeGroup GetActiveTrackingGroup() const { return m_eActiveTrackingGroup; }

	// Validity.

	bool IsValidTrackingGroup(EnumTrackedNodeGroup eGroup);

	// Targeting.

	virtual bool SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset);
	virtual bool SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset);
	virtual bool SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hObject, const LTVector& vOffset);
	virtual bool SetTrackedTarget(EnumTrackedNodeGroup eGroup, const LTVector& vPosition);

	// Basis Space

	bool GetRootBasis(EnumTrackedNodeGroup eGroup, LTVector& vRight, LTVector& vUp, LTVector& vForward,	LTVector& vPos);

	// Limits.

	bool IsAtDiscomfort(EnumTrackedNodeGroup eGroup);
	bool IsAtLimit(EnumTrackedNodeGroup eGroup);

protected:

	void _Init(EnumTrackedNodeGroup eGroup, bool bTrackOnAnim);

	// OrientOnAnim.

	bool SetOrientOnAnim(EnumTrackedNodeGroup eGroup, bool bTrackOnAnim);

	// Enable/Disable.

	void EnableTrackingGroup(EnumTrackedNodeGroup eGroup, bool bEnable);

protected:

	// The following members do not need to be saved (on the server)

	CTrackedNodeMgr*		m_pTrackedNodeMgr;						// Client or server TrackedNodeMgr.
	HOBJECT					m_hModel;								// Model instance.
	HTRACKEDNODE*			m_aTrackedNodeGroups[kTrack_Count];		// Array of tracked nodes per group.
	uint32					m_cTrackedNodesPerGroup[kTrack_Count];	// Number of tracked nodes per group.
	ModelTrackingNodeGroup	m_eTrackingNodes[kTrack_Count];			// Modelbutes Tracking nodes per group.
	
	// The following members DO need to be saved (on the server)

	EnumTrackedNodeGroup	m_eActiveTrackingGroup;					// Currently active group (only one at a time).
};


#endif

