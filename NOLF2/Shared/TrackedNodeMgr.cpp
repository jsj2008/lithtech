#include "stdafx.h"
#include "TrackedNode.h"
#include "TrackedNodeMgr.h"

#include <algorithm>


//the function we will use for handling the callbacks
extern void TrackedNodeControlFn(const NodeControlData& Data, void*);


CTrackedNodeMgr::CTrackedNodeMgr(ILTCSBase* pILTBase) :
	m_pILTBase(pILTBase)
{
}

CTrackedNodeMgr::~CTrackedNodeMgr()
{
	//remove all of the nodes
	DestroyAllNodes();
}

// GetNumNodes
//   Returns the current number of nodes managed by this manager

uint32 CTrackedNodeMgr::GetNumNodes() const
{
	return m_cNodes.size();
}


// SetBaseInterface
//   Allows the changing of the interface used with the node manager. Note
// that this will not work if there are any existing nodes already created.

bool CTrackedNodeMgr::SetBaseInterface(ILTCSBase* pILTBase)
{
	//setup the model interface, we can only do this though if we don't
	//have any nodes created (because we wouldn't know how to destroy
	//them then)
	if(GetNumNodes() > 0)
		return false;

	m_pILTBase = pILTBase;
	return true;
}

// GetBaseInterface
//	 Provides access to the base interface used by this manager

ILTCSBase* CTrackedNodeMgr::GetBaseInterface()
{
	return m_pILTBase;
}


// CreateTrackingNode
//	 Creates a new tracking node that will control the specified node
// of the specified model. If the TrackNode ID is INVALID_TRACKEDNODE,
// the node could not be successfully created and use of the ID will
// result in undefined behavior. Newly created nodes are not set to
// actively track, they must be enabled through EnableTracking.

HTRACKEDNODE CTrackedNodeMgr::CreateTrackingNode( HOBJECT hModel, const char* pszNodeName)
{
	//first check our input parameters
	if(!m_pILTBase || (hModel == NULL) || (pszNodeName == NULL))
		return INVALID_TRACKEDNODE;

	//ok, now make sure that it is indeed a model
	uint32 nType;
	if(m_pILTBase->Common()->GetObjectType(hModel, &nType) != LT_OK)
		return INVALID_TRACKEDNODE;

	if(nType != OT_MODEL)
		return INVALID_TRACKEDNODE;

	//ok, we have a model, and a node name, so let us find that node
	HMODELNODE hNode;
	if(m_pILTBase->GetModelLT()->GetNode(hModel, (char*)pszNodeName, hNode) != LT_OK)
		return INVALID_TRACKEDNODE;

	if(nType != OT_MODEL)
		return INVALID_TRACKEDNODE;

	//alright, we have the model, and the node, now all we need to do is create
	//our internal tracked node
	CTrackedNode* pNewNode = debug_new(CTrackedNode);

	//check the allocation
	if(!pNewNode)
		return INVALID_TRACKEDNODE;

	//Ok, now fill out all the information
	pNewNode->m_hModel		= hModel;
	pNewNode->m_hNode		= hNode;
	pNewNode->m_pNodeMgr	= this;

	//now install it on the model itself
	if(m_pILTBase->GetModelLT()->AddNodeControlFn(hModel, hNode, TrackedNodeControlFn, (void*)pNewNode) != LT_OK)
	{
		//free memory and bail
		debug_delete(pNewNode);
		return INVALID_TRACKEDNODE;
	}

	//alright, just add it to our list and success
	m_cNodes.push_back(pNewNode);

	return (HTRACKEDNODE)pNewNode;
}

// DestroyTrackingNode
//	 Removes the specified tracking node as well as all references to it.
// This will return false if that node is not an actual tracking node.

bool CTrackedNodeMgr::DestroyTrackingNode( HTRACKEDNODE ID )
{
	//make sure everything is valid
	if(!CheckValidity(ID))
		return false;

	CTrackedNode* pNode = (CTrackedNode*)ID;

	NodeList::iterator Pos = std::find(m_cNodes.begin(), m_cNodes.end(), pNode);
	if(Pos == m_cNodes.end())
	{
		//this isn't our iterator, this isn't my house
		return false;
	}

	//now uninstall the function
	m_pILTBase->GetModelLT()->RemoveNodeControlFn(pNode->m_hModel, pNode->m_hNode, TrackedNodeControlFn, (void*)pNode);

	//remove it from our lists
	m_cNodes.erase(Pos);

	//now go through and clean up any dependencies
	for(uint32 nCurrNode = 0; nCurrNode < m_cNodes.size(); nCurrNode++)
	{
		CTrackedNode* pCheckNode = m_cNodes[nCurrNode];

		if(pCheckNode->m_pChild == pNode)
			pCheckNode->m_pChild = NULL;
		if(pCheckNode->m_pMimicNode == pNode)
			pCheckNode->m_pMimicNode = NULL;
	}

	//clean up any used memory
	debug_delete(pNode);

	//success
	return true;
}

// DestroyNodesOnModel
//	 Given a model, it will destroy all nodes that are attached to it. It
// will return the number of nodes destroyed.

uint32 CTrackedNodeMgr::DestroyNodesOnModel( HOBJECT hModel)
{
	//count the num removed
	uint32 nNumRemoved = 0;

	//go backwards removing all the items
	for(int32 nCurrNode = m_cNodes.size() - 1; nCurrNode >= 0; nCurrNode--)
	{
		if(m_cNodes[nCurrNode]->m_hModel == hModel)
		{
			if(DestroyTrackingNode(m_cNodes[nCurrNode]))
				nNumRemoved++;
		}
	}

	//all done
	return nNumRemoved;
}

// DestroyAllNodes
//	 This will remove all nodes from the list and all references. It will
// return the number of nodes destroyed.

uint32 CTrackedNodeMgr::DestroyAllNodes()
{
	uint32 nNumRemoved = m_cNodes.size();

	for(NodeList::iterator it = m_cNodes.begin(); it != m_cNodes.end(); it++)
	{
		//first off, remove it from the model
		m_pILTBase->GetModelLT()->RemoveNodeControlFn((*it)->m_hModel, (*it)->m_hNode, TrackedNodeControlFn, (void*)(*it));

		//now free the memory
		debug_delete(*it);
	}

	//clear out the vector
	m_cNodes.resize(0);

	return nNumRemoved;
}

// SetNodeConstraints
//	 Given a node, this will allow for the user to specify the angular
// constraints of the node. This will be restricted to an ellipsoid
// pattern around the cone axis with the half angles designating the
// size of each ellipse dimension. The angles divide the area into two
// sections, a comfort area and a maximum area, as long as the orientation
// lies within the comfort area it will not try and propagate changes up
// the linking tree, but will try to once it reaches the discomfort area.
// This will return false if the angles are invalid, the up axis is linear
// with the cone axis or if the angles are invalid. All angles are in radians.

bool CTrackedNodeMgr::SetNodeConstraints(	HTRACKEDNODE ID,
							const LTVector& vMovConeAxis, 
							const LTVector& vMovConeUp,
							float fXDiscomfortAngle,
							float fYDiscomfortAngle,
							float fXMaxAngle,
							float fYMaxAngle,
							float fMaxAngVel
						)
{
	//sanity checks
	if(!CheckValidity(ID))
		return false;

	//ok, we have a valid ID, so let us setup the parameters
	CTrackedNode* pNode = (CTrackedNode*)ID;

	//see if the up and forward vectors are valid
	LTVector vForward	= vMovConeAxis;
	LTVector vUp		= vMovConeUp;

	//ensure proper scale
	vForward.Normalize();
	vUp.Normalize();

	//ensure they form a valid space (and not a plane)
	if(vUp.Dot(vForward) > 0.99f)
	{
		//not valid, we need to try a different up, our preference is the world up
		vUp.Init(0.0f, 1.0f, 0.0f);

		if(vUp.Dot(vForward) > 0.99f)
		{
			//ok, forward is already taking the up....so, tilt us back
			vUp.Init(0.0f, 0.0f, -1.0f);
		}
	}

	//now generate the right, and ensure orthogonality
	LTVector vRight = vForward.Cross(vUp);
	vUp = vRight.Cross(vForward);

	vRight.Normalize();
	vUp.Normalize();

	//setup this as the basis space
	pNode->m_mInvTargetTransform.SetBasisVectors(&vRight, &vUp, &vForward);
	pNode->m_mInvTargetTransform.Transpose();

	//we need to make sure that their angular constraints are valid (meaning that they are positive and
	//less than 90 deg)
	fXMaxAngle			= LTCLAMP(fXMaxAngle,			0.0f, DEG2RAD(89.0f));
	fYMaxAngle			= LTCLAMP(fYMaxAngle,			0.0f, DEG2RAD(89.0f));
	fXDiscomfortAngle	= LTCLAMP(fXDiscomfortAngle,	0.0f, fXMaxAngle);
	fYDiscomfortAngle	= LTCLAMP(fYDiscomfortAngle,	0.0f, fYMaxAngle);

	//now precompute the tangent of those values (used for finding the height of the cone created which
	//is used in the threshold determination code)
	pNode->m_fTanXDiscomfort = (float)tan(fXDiscomfortAngle);
	pNode->m_fTanYDiscomfort = (float)tan(fYDiscomfortAngle);
	pNode->m_fTanXThreshold  = (float)tan(fXMaxAngle);
	pNode->m_fTanYThreshold  = (float)tan(fYMaxAngle);

	//handle setting up the maximum angular velocity
	pNode->m_fMaxAngVel		= (float)fabs(fMaxAngVel);

	//and we are ready for primetime
	return true;
}

// SetNodeConstraints
//	 This function acts as the same as above but creates the ellipse around the standard
// orientation of Y being forward and X running along the bone

bool CTrackedNodeMgr::SetNodeConstraints(	HTRACKEDNODE ID,
											float fXDiscomfortAngle,
											float fYDiscomfortAngle,
											float fXMaxAngle,
											float fYMaxAngle,
											float fMaxAngVel
										)
{
	//just forward this onto the more elaborate version
	return SetNodeConstraints(ID,	LTVector(0.0f, 1.0f, 0.0f), LTVector(-1.0f, 0.0f, 0.0f), 
									fXDiscomfortAngle, fYDiscomfortAngle,
									fXMaxAngle, fYMaxAngle,
									fMaxAngVel);
}

// LinkNodeOrientation
//	 Given a node, it will link its orienation to the other specified node and will
// blindly copy the orientation instead of evaluating. This is useful for nodes that
// must match direction such as eyes. Returns false if either ID is invalid. This
// will remove any old angular restrictions and also any existing links.

bool CTrackedNodeMgr::LinkNodeOrientation(	HTRACKEDNODE ID,
											HTRACKEDNODE CopyFrom )
{
	//check the params and parent object
	if(!CheckValidity(ID) || !CheckValidity(CopyFrom))
		return false;

	//ok, setup the link pointer
	ID->m_pMimicNode = CopyFrom;

	//success
	return true;
}

// EnableTracking
//	 Allows the enabling and disabling of node tracking for a specified node.
// When disabled it will use the animation position and will only propagate
// up the linking tree but will not modify itself. Returns false if the ID is
// invalid

bool CTrackedNodeMgr::EnableTracking( HTRACKEDNODE ID, bool bTrack )
{
	if(!CheckValidity(ID))
		return false;

	//see if we are actually switching state
	if(ID->m_bEnabled != bTrack)
	{
		//we are indeed switching state, store our new state
		ID->m_bEnabled = bTrack;

		//we will also need to resync our orienation when we update again, so flag that
		ID->m_bSyncOrientation = true;
	}
	return true;
}


// IsTracking
//	 Returns whether or not the specified node is currently tracking or not

bool CTrackedNodeMgr::IsTracking( HTRACKEDNODE ID )
{
	if(!CheckValidity(ID))
		return false;

	return ID->m_bEnabled;
}

// LinkNodes
//	 Sets a node as a parent, which will be notified to be updated as soon as the
// child link enters its area of discomfort. Only one parent can be specified per
// node. Nodes that are linked in orienation will not notify parents.

bool CTrackedNodeMgr::LinkNodes( HTRACKEDNODE ChildID, HTRACKEDNODE ParentID )
{
	//make sure the nodes are valid
	if(!CheckValidity(ChildID) || !CheckValidity(ParentID))
		return false;

	ParentID->m_pChild = ChildID;
	return true;
}

// IsAtLimit
//	 Returns whether or not the node reached the maximum extent during its last
// update.

bool CTrackedNodeMgr::IsAtLimit( HTRACKEDNODE ID )
{
	if(!CheckValidity(ID))
		return false;

	return ID->m_bAtMaxThreshold;
}


// IsAtDiscomfort
//	 Returns whether or not the node reached the discomfort extent during its last
// update.

bool CTrackedNodeMgr::IsAtDiscomfort( HTRACKEDNODE ID)
{
	if(!CheckValidity(ID))
		return false;

	return ID->m_bInDiscomfort;
}

// IsLookingAtTarget
//	 Given a node it will return whether or not it was looking directly
// at the target at the end of its last update.

bool CTrackedNodeMgr::IsLookingAtTarget( HTRACKEDNODE ID)
{
	if(!CheckValidity(ID))
		return false;

	return ID->m_bLookingAtTarget;
}

// SetTarget
//	 Tells the specified node to track the node of the specified model with a bit
// of an offset. It will then do its best to follow that point. Returns false
// on an invalid ID, object, or node.

bool CTrackedNodeMgr::SetTarget( HTRACKEDNODE ID, HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset )
{
	if(!CheckValidity(ID))
		return false;

	//make sure that the model pointed to is not ourself, this can cause infinite
	//recursion
	assert(hModel != ID->m_hModel);

	//make sure that that is actually a valid model
	uint32 nType;
	if(m_pILTBase->Common()->GetObjectType(hModel, &nType) != LT_OK)
		return false;

	if(nType != OT_MODEL)
		return false;

	//ok, we have a model, and a node name, so let us find that node
	HMODELNODE hNode;
	if(m_pILTBase->GetModelLT()->GetNode(hModel, (char*)pszNodeName, hNode) != LT_OK)
		return false;

	//setup this target
	ID->m_eTrackMode	= CTrackedNode::TRACK_NODE;
	ID->m_hTrackObject	= hModel;
	ID->m_hTrackNode	= hNode;
	ID->m_vTrackOffset	= vOffset;

	//success
	return true;
}

bool CTrackedNodeMgr::SetTarget( HTRACKEDNODE ID, HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset )
{
	if(!CheckValidity(ID))
		return false;

	//make sure that the model pointed to is not ourself, this can cause infinite
	//recursion
	assert(hModel != ID->m_hModel);

	//make sure that that is actually a valid model
	uint32 nType;
	if(m_pILTBase->Common()->GetObjectType(hModel, &nType) != LT_OK)
		return false;

	if(nType != OT_MODEL)
		return false;

	//make sure we have a valid node
	if(hNode == INVALID_MODEL_NODE)
		return false;

	//setup this target
	ID->m_eTrackMode	= CTrackedNode::TRACK_NODE;
	ID->m_hTrackObject	= hModel;
	ID->m_hTrackNode	= hNode;
	ID->m_vTrackOffset	= vOffset;

	//success
	return true;
}

// SetTarget
//	 Tells the specified node to track the specified object's origin plus the
// specified offset. Returns false on invalid ID or object.

bool CTrackedNodeMgr::SetTarget( HTRACKEDNODE ID, HOBJECT hObject, const LTVector& vOffset )
{
	if(!CheckValidity(ID))
		return false;

	//setup this target
	ID->m_eTrackMode	= CTrackedNode::TRACK_OBJECT;
	ID->m_hTrackObject	= hObject;
	ID->m_vTrackOffset	= vOffset;

	return true;
}

// SetTargetWorld
//	 Tells the specified node to track the a specified point in world space.
// Returns false on an invalid ID. 

bool CTrackedNodeMgr::SetTargetWorld( HTRACKEDNODE ID, const LTVector& vPosition )
{
	if(!CheckValidity(ID))
		return false;

	//setup this target
	ID->m_eTrackMode	= CTrackedNode::TRACK_WORLDPOS;
	ID->m_vTrackOffset	= vPosition;

	return true;
}

// SetTargetLocal
//	 Tells the specified node to track the a specified point in node space.
// Returns false on an invalid ID. This is good for lining up in a relative 
// direction to the animation

bool CTrackedNodeMgr::SetTargetLocal( HTRACKEDNODE ID, const LTVector& vPosition )
{
	if(!CheckValidity(ID))
		return false;

	//setup this target
	ID->m_eTrackMode	= CTrackedNode::TRACK_LOCALPOS;
	ID->m_vTrackOffset	= vPosition;

	return true;
}

// SetTargetObject
//	 Tells the specified node to track the a specified point in object space.
// Returns false on an invalid ID. 

bool CTrackedNodeMgr::SetTargetObject( HTRACKEDNODE ID, const LTVector& vPosition )
{
	if(!CheckValidity(ID))
		return false;

	//setup this target
	ID->m_eTrackMode	= CTrackedNode::TRACK_OBJSPACEPOS;
	ID->m_vTrackOffset	= vPosition;

	return true;
}

// TargetAnimation
//	 Tells the specified node to look where the animation is looking. This is
// commonly used for fading out the tracker by having it look where the animation
// is looking, and then when it is looking at the target, disabling the tracker.

bool CTrackedNodeMgr::TargetAnimation( HTRACKEDNODE ID, const LTVector& vOffset )
{
	if(!CheckValidity(ID))
		return false;

	//setup the target
	ID->m_eTrackMode	= CTrackedNode::TRACK_ANIMATION;
	ID->m_vTrackOffset	= vOffset;

	//success
	return true;
}


// GetBasisSpace
//	 Given a node, it will fill in the appropriate vectors specifying the
// orienation and position of the node. Returns false on an invalid ID.

bool CTrackedNodeMgr::GetBasisSpace( HTRACKEDNODE ID,	LTVector& vRight, 
														LTVector& vUp, 
														LTVector& vForward,
														LTVector& vPos)
{
	if(!CheckValidity(ID))
		return false;

	vRight		= ID->m_vActualRight;
	vUp			= ID->m_vActualUp;
	vForward	= ID->m_vActualForward;
	vPos		= ID->m_vActualPos;

	return true;
}

// CheckValidity
//   This will check the validity of the specified tracked node as well as the
// state of the object, and will return false if either is in a state
// that it cannot be operated upon. In debug it will also check and see if
// the tracked node properly belongs to this manager

bool CTrackedNodeMgr::CheckValidity( HTRACKEDNODE ID )
{
	//first check the manager's state
	if(!m_pILTBase)
		return false;

	//now check the handle's state
	if(ID == INVALID_TRACKEDNODE)
		return false;

	//make sure the parent matches
	if(ID->m_pNodeMgr != this)
		return false;	

	//success
	return true;
}

// GetOrientationSpace
//	 Given a node, it will fill in the appropriate vectors specifying the
// orienation space as set on the node with the forward and up vectors

bool CTrackedNodeMgr::GetOrientationSpace( HTRACKEDNODE ID,	LTVector& vRight, 
											LTVector& vUp, 
											LTVector& vForward)
{
	if(!CheckValidity(ID))
		return false;

	//read in the vectors
	LTMatrix mTargetTransform = ID->m_mInvTargetTransform;
	mTargetTransform.Transpose();

	mTargetTransform.GetBasisVectors(&vRight, &vUp, &vForward);
	return true;
}


// SetAutoDisable
//	 Tells the specified node to disable itself when it reaches the point where
// it is looking at the specified target. This is commonly used in conjunction with
// TargetAnimation in order to fade the animation out

bool CTrackedNodeMgr::SetAutoDisable( HTRACKEDNODE ID, bool bAutoDisable )
{
	if(!CheckValidity(ID))
		return false;

	//just set the flag on the node
	ID->m_bAutoDisable = bAutoDisable;
	return true;
}

// SetOrientOnAnim
// 	Sets the flag indicating if this node alignment should be relative to the animation orienation with
// respect to the model orienation. This is used for things such as aiming guns sideways, in which
// the node that should be aligned is constantly changing with how the gun is oriented towards
// the forward of the model. 

bool CTrackedNodeMgr::SetOrientOnAnim( HTRACKEDNODE ID, bool bTrackOnAnim )
{
	if(!CheckValidity(ID))
		return false;

	//just set the flag on the node
	ID->m_bOrientFromAnim = bTrackOnAnim;
	return true;
}

// SetIgnoreParentAnimation
// 	Sets the flag indicating if the animation for the parent node should be completely ignored for
// the constraints. This essentially makes it so that the constraints are constant regardless of
// what animation is playing or how the animation has any higher up nodes oriented. This can
// cause the node to go into odd orientations if the animation does large rotations, but gives
// constant aligned constraints so it is useful to counteract minor rotations

bool CTrackedNodeMgr::SetIgnoreParentAnimation( HTRACKEDNODE ID, bool bIgnoreParentAnimation )
{
	if(!CheckValidity(ID))
		return false;

	//just set the flag on the node
	ID->m_bIgnoreParentAnimation = bIgnoreParentAnimation;
	return true;
}
