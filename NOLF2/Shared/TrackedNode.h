
#ifndef __TRACKEDNODE_H__
#define __TRACKEDNODE_H__

#include <time.h>
#include "LTObjRef.h"

//CTrackedNode :
//	holds all information for a tracked node. This is a behind the scenes class
// that is not intended for use outside of the interface provided by the TrackedNodeMgr.
// It holds all data necessary to manipulate a tracked node from the NodeController
// callback function.

//forward declaration of the manager class
class CTrackedNodeMgr;

class CTrackedNode
{
public:

	//constructor
	CTrackedNode();

	//different tracking modes
	enum ETrackMode	{	TRACK_OBJECT,
						TRACK_NODE,
						TRACK_WORLDPOS,
						TRACK_LOCALPOS,
						TRACK_OBJSPACEPOS,
						TRACK_ANIMATION
					};

	//the type of tracking mode we are in
	ETrackMode		m_eTrackMode;

	//the model that this node belongs to
	LTObjRef		m_hModel;

	//the node that it is attached to
	HMODELNODE		m_hNode;

	//the object that is being tracked NULL if none, in which case
	//the origin is always used
	LTObjRef		m_hTrackObject;

	//If HOBJECT is supposed to be treated as a model and not just an object,
	//this will refer to the node that is supposed to be tracked on the model
	HMODELNODE		m_hTrackNode;

	//the offset from the object's position to track
	LTVector		m_vTrackOffset;

	//this is the transform to convert from a space where the desired pointing vector is Z to
	//the aligned space where Z is always forward
	LTMatrix		m_mInvTargetTransform;

	//the space we were in at the end of the last update
	LTVector		m_vActualUp;
	LTVector		m_vActualRight;
	LTVector		m_vActualForward;
	LTVector		m_vActualPos;

	//vector for aligning the target vector to prevent roll
	LTVector		m_vAlignUp;

	//flags indicating what state we were in during the last update
	
	//determine if we were looking at the target
	bool			m_bLookingAtTarget;

	//determine if we were in the discomfort zone
	bool			m_bInDiscomfort;

	//determine if the max threshold was hit
	bool			m_bAtMaxThreshold;

	//flag indicating whether or not this is even enabled
	bool			m_bEnabled;

	//flag indicating that when this node reaches the target look at state, it should disable itself
	bool			m_bAutoDisable;

	//this flag indicates that the parent's animation transform should be competely disregarded so
	//that the constraints will never be altered by it. This is commonly done for higher up nodes
	//to give complete control over how they move
	bool			m_bIgnoreParentAnimation;

	//flag indicating if this node alignment should be relative to the animation orienation with
	//respect to the model orienation. This is used for things such as aiming guns sideways, in which
	//the node that should be aligned is constantly changing with how the gun is oriented towards
	//the forward of the model. 
	bool			m_bOrientFromAnim;

	//flag indicating that this update will be the first after a deactivation period. This indicates
	//that the current orientation should be grabbed from the animation to prevent popping and
	//will be cleared after the orientation is grabbed
	bool			m_bSyncOrientation;

	//the child node, can be NULL
	CTrackedNode*	m_pChild;

	//the object that we are cloning for orientation
	CTrackedNode*	m_pMimicNode;

	//cosine of the above angles
	float			m_fTanXDiscomfort;
	float			m_fTanYDiscomfort;
	float			m_fTanXThreshold;
	float			m_fTanYThreshold;

	//maxiumum angular velocity
	float			m_fMaxAngVel;

	//the last time that this node was updated
	clock_t			m_LastUpdate;

	//the node holds a pointer back to its manager, this normally isn't
	//a good thing since it results in cyclic dependencies, but it is
	//an opaque pointer and is needed due to the callback nature of the
	//node control
	CTrackedNodeMgr *m_pNodeMgr;
};


#endif
