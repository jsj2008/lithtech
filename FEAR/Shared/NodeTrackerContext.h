#ifndef __NODETRACKERCONTEXT_H__
#define __NODETRACKERCONTEXT_H__

#include "NodeTracker.h"

// Groups of tracking nodes exist for different purposes.

enum EnumNodeTrackerGroup
{
	kTrackerGroup_None = -1,
	kTrackerGroup_LookAt,
	kTrackerGroup_AimAt,
	kTrackerGroup_Arm,
	kTrackerGroup_Count,
};

typedef std::bitset<kTrackerGroup_Count> NODE_TRACKER_FLAGS;

enum EnumNodeTrackerFlag
{
	kTrackerFlag_None	= 0x00,
	kTrackerFlag_LookAt	= 0x01,
	kTrackerFlag_AimAt	= 0x02,
	kTrackerFlag_Arm	= 0x04,
};

// Server needs to tell client tracking settings.

enum EnumTrackerMsg
{
	kTrackerMsg_ToggleGroups,
	kTrackerMsg_SetTarget,
};

enum EnumTrackerState
{
	kTrackerState_Updating,
	kTrackerState_ShuttingDown,
	kTrackerState_Idle,
};


//
// SNodeTrackerInstance:
//

struct SNodeTrackerInstance
{
	CNodeTracker*				m_pNodeTracker;			// Node tracker object.
	ModelsDB::HTRACKERNODEGROUP	m_hModelTrackerGroup;	// Modelbutes Tracker nodes group.
	EnumTrackerState			m_eTrackerState;		// Stae of the Tracker.
};


//
// CNodeTrackerContext:
// Manages the current node tracking settings per model instance
// on the client or server.
//

class CNodeTrackerContext
{
public:
	// Creation/destruction.

			 CNodeTrackerContext();
	virtual ~CNodeTrackerContext();

	void Init( HOBJECT hModel, ModelsDB::HSKELETON hSkeleton );
	void Term();

	// Activation.

	virtual void	SetActiveTrackerGroups( uint32 dwActiveTrackerGroups );
	virtual void	EnableTrackerGroup( EnumNodeTrackerGroup eGroup );
	virtual void	DisableTrackerGroup( EnumNodeTrackerGroup eGroup, bool bImmediate );
	bool			IsTrackerGroupActive( EnumNodeTrackerGroup eGroup ) const;

	// Targeting.

	virtual void SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset );
	virtual void SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset );
	virtual void SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hObject, const LTVector& vOffset );
	virtual void SetTrackedTarget( EnumNodeTrackerGroup eGroup, const LTVector& vPosition );
	virtual void SetTrackedTarget( EnumNodeTrackerGroup eGroup, const LTPolarCoord& polarExtents );

	// Update.

	void		UpdateNodeTrackers( float fElapsedTimeS );

	// Limits.

	bool DidAimAtTarget( EnumNodeTrackerGroup eGroup );
	bool GetCurrentExtents( EnumNodeTrackerGroup eGroup, LTPolarCoord& polarExtents );

	// Settings... These are initialy set in Init() through the model skeleton but can be overriden with these...

	virtual void SetMaxSpeed( EnumNodeTrackerGroup eGroup, float fMaxSpeed );

	// Revert back to the default max speed for the node tracker associated with the given group...
	virtual void SetDefaultMaxSpeed( EnumNodeTrackerGroup eGroup );
	
	// Specify the limits for the node tracker associated with the given group...
	virtual void SetLimits( EnumNodeTrackerGroup eGroup, const LTRect2f &rLimits );
	
	// Retrieve the limits for the node tracker associated with the given group...
	virtual bool GetLimits( EnumNodeTrackerGroup eGroup, LTRect2f &rLimits );
	
	// Revert back to the default limits for the node tracker associated with the given group...
	virtual void SetDefaultLimits( EnumNodeTrackerGroup eGroup );


protected:

	void _Init( EnumNodeTrackerGroup eGroup );

protected:

	// The following members do not need to be saved (on the server)

	LTObjRef				m_hModel;										// Model instance.
	SNodeTrackerInstance	m_NodeTrackerInstances[kTrackerGroup_Count];	// Node tracker instances.
	
	// The following members DO need to be saved (on the server)

	NODE_TRACKER_FLAGS		m_bitsActiveTrackingFlags;
};


#endif

