#ifndef __SERVER_TRACKEDNODECONTEXT_H__
#define __SERVER_TRACKEDNODECONTEXT_H__

#include "TrackedNodeContext.h"


// Server records target settings for save/load.

struct TrackTargetRecord
{
	EnumTrackTarget	eTrackingTarget;	// Type of targeting (model, object, position).
	LTObjRef		hTarget;			// Object or model being tracked.
	HMODELNODE		hTargetNode;		// Handle to node being tracked.
	LTVector		vTarget;			// Offset or position being tracked.
};


//
// CServerTrackedNodeContext:
// Manages the current node tracking settings per model instance
// on the server.
//

class CServerTrackedNodeContext : public CTrackedNodeContext
{
	typedef CTrackedNodeContext super;

public:
	// Creation/destruction.

			 CServerTrackedNodeContext();
	virtual	~CServerTrackedNodeContext();

	// Save / Load

	void	Save(ILTMessage_Write *pMsg);
    void	Load(ILTMessage_Read *pMsg);

	// Activation.

	virtual void SetActiveTrackingGroup(EnumTrackedNodeGroup eGroup);

	// Targeting.

	virtual bool SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset);
	virtual bool SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset);
	virtual bool SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hObject, const LTVector& vOffset);
	virtual bool SetTrackedTarget(EnumTrackedNodeGroup eGroup, const LTVector& vPosition);

	// Server update.

	void Update();

	// Client syncing.

	inline bool NeedToRefreshClient() const { return m_bRefreshClient; }
	void UpdateClient();

protected:

	void UpdateClientTarget(EnumTrackedNodeGroup eGroup);

protected:

	bool					m_bRefreshClient;
	TrackTargetRecord		m_TrackedNodeTargets[kTrack_Count];		// Record of last SetTarget.
};


#endif

