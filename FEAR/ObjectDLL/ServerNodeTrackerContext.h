#ifndef __SERVER_NODETRACKERCONTEXT_H__
#define __SERVER_NODETRACKERCONTEXT_H__

#include "NodeTrackerContext.h"

// Server records target settings for save/load.

struct TrackerTargetRecord
{
	CNodeTracker::ETargetType	eTrackerTarget;		// Type of targeting (model, object, position).
	LTObjRef					hTarget;			// Object or model being tracked.
	HMODELNODE					hTargetNode;		// Handle to node being tracked.
	LTVector					vTarget;			// Offset or position being tracked.
};


//
// CServerNodeTrackerContext:
// Manages the current node tracking settings per model instance
// on the server.
//

class CServerNodeTrackerContext : public CNodeTrackerContext
{
	typedef CNodeTrackerContext super;

public:
	// Creation/destruction.

			 CServerNodeTrackerContext();
	virtual	~CServerNodeTrackerContext();

	// Save / Load

	void			Save( ILTMessage_Write *pMsg );
    void			Load( ILTMessage_Read *pMsg );

	// Activation.

	virtual void	SetActiveTrackerGroups( uint32 dwActiveTrackerGroups );

	// Targeting.

	void			SetTrackedTarget( HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset );
	void			SetTrackedTarget( HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset );
	void			SetTrackedTarget( HOBJECT hObject, const LTVector& vOffset );
	void			SetTrackedTarget( const LTVector& vPosition );

	virtual void	SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset );
	virtual void	SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset );
	virtual void	SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hObject, const LTVector& vOffset );
	virtual void	SetTrackedTarget( EnumNodeTrackerGroup eGroup, const LTVector& vPosition );

	// Server update.

	void			UpdateNodeTrackers();

	// Client syncing.

	void			UpdateClient();

protected:

	void			UpdateClientTarget( EnumNodeTrackerGroup eGroup );

protected:

	bool					m_bRefreshClient;
	double					m_fLastUpdateTime;
	TrackerTargetRecord		m_TrackedNodeTargets[kTrackerGroup_Count];		// Record of last SetTarget.
};


#endif

