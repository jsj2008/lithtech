#ifndef __TRACKEDNODEMGR_H__
#define __TRACKEDNODEMGR_H__

//declarations for internally used classes
class CTrackedNode;

//define a handle type for users to refer to tracked nodes
typedef CTrackedNode*				HTRACKEDNODE;

//an invalid handle for users to compare for validation checks
#define INVALID_TRACKEDNODE  NULL

class CTrackedNodeMgr
{
public:

	CTrackedNodeMgr(ILTCSBase* pILTBase = NULL);
	virtual ~CTrackedNodeMgr();

	// SetBaseInterface
	//   Allows the changing of the interface used with the node manager. Note
	// that this will not work if there are any existing nodes already created.

	bool SetBaseInterface(ILTCSBase* pILTBase);

	// GetBaseInterface
	//	 Provides access to the base interface used by this manager

	ILTCSBase* GetBaseInterface();

	// GetNumNodes
	//   Returns the current number of nodes managed by this manager

	uint32  GetNumNodes() const;

	// CreateTrackingNode
	//	 Creates a new tracking node that will control the specified node
	// of the specified model. If the TrackNode ID is INVALID_TRACKEDNODE,
	// the node could not be successfully created and use of the ID will
	// result in undefined behavior. Newly created nodes are not set to
	// actively track, they must be enabled through EnableTracking.

	HTRACKEDNODE CreateTrackingNode( HOBJECT hModel, const char* pszNodeName);

	// DestroyTrackingNode
	//	 Removes the specified tracking node as well as all references to it.
	// This will return false if that node is not an actual tracking node.

	bool DestroyTrackingNode( HTRACKEDNODE ID );

	// DestroyNodesOnModel
	//	 Given a model, it will destroy all nodes that are attached to it. It
	// will return the number of nodes destroyed.

	uint32 DestroyNodesOnModel( HOBJECT hModel);

	// DestroyAllNodes
	//	 This will remove all nodes from the list and all references. It will
	// return the number of nodes destroyed.

	uint32 DestroyAllNodes();

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

	bool  SetNodeConstraints(	HTRACKEDNODE ID,
								const LTVector& vMovConeAxis, 
								const LTVector& vMovConeUp,
								float fXDiscomfortAngle,
								float fYDiscomfortAngle,
								float fXMaxAngle,
								float fYMaxAngle,
								float fMaxAngVel
							);

	// SetNodeConstraints
	//	 This function acts as the same as above but creates the ellipse around the standard
	// orientation of Y being forward and X running along the bone

	bool  SetNodeConstraints(	HTRACKEDNODE ID,
								float fXDiscomfortAngle,
								float fYDiscomfortAngle,
								float fXMaxAngle,
								float fYMaxAngle,
								float fMaxAngVel
							);

	// LinkNodeOrientation
	//	 Given a node, it will link its orienation to the other specified node and will
	// blindly copy the orientation instead of evaluating. This is useful for nodes that
	// must match direction such as eyes. Returns false if either ID is invalid. This
	// will remove any old angular restrictions and also any existing links.

	bool  LinkNodeOrientation(	HTRACKEDNODE ID,
								HTRACKEDNODE CopyFrom );

	// EnableTracking
	//	 Allows the enabling and disabling of node tracking for a specified node.
	// When disabled it will use the animation position and will only propagate
	// up the linking tree but will not modify itself. Returns false if the ID is
	// invalid

	bool  EnableTracking( HTRACKEDNODE ID, bool bTrack );

	// IsTracking
	//	 Returns whether or not the specified node is currently tracking or not

	bool  IsTracking( HTRACKEDNODE ID );

	// LinkNodes
	//	 Sets a node as a parent, which will be notified to be updated as soon as the
	// child link enters its area of discomfort. Only one parent can be specified per
	// node. Nodes that are linked in orienation will not notify parents.

	bool  LinkNodes( HTRACKEDNODE ChildID, HTRACKEDNODE ParentID );

	// IsAtLimit
	//	 Returns whether or not the node reached the maximum extent during its last
	// update.

	bool  IsAtLimit( HTRACKEDNODE ID );

	// IsAtDiscomfort
	//	 Returns whether or not the node reached the discomfort extent during its last
	// update.

	bool  IsAtDiscomfort( HTRACKEDNODE ID);

	// IsLookingAtTarget
	//	 Given a node it will return whether or not it was looking directly
	// at the target at the end of its last update.

	bool  IsLookingAtTarget( HTRACKEDNODE ID);

	// SetTarget
	//	 Tells the specified node to track the node of the specified model with a bit
	// of an offset. It will then do its best to follow that point. Returns false
	// on an invalid ID, object, or node.

	bool  SetTarget( HTRACKEDNODE ID, HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset );
	bool  SetTarget( HTRACKEDNODE ID, HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset );

	// SetTarget
	//	 Tells the specified node to track the specified object's origin plus the
	// specified offset. Returns false on invalid ID or object.

	bool  SetTarget( HTRACKEDNODE ID, HOBJECT hObject, const LTVector& vOffset );

	// SetTargetWorld
	//	 Tells the specified node to track the a specified point in world space.
	// Returns false on an invalid ID. 

	bool  SetTargetWorld( HTRACKEDNODE ID, const LTVector& vPosition );

	// SetTargetLocal
	//	 Tells the specified node to track the a specified point in node space.
	// Returns false on an invalid ID. This is good for lining up in a relative 
	// direction to the animation

	bool  SetTargetLocal( HTRACKEDNODE ID, const LTVector& vPosition );

	// SetTargetObject
	//	 Tells the specified node to track the a specified point in object space.
	// Returns false on an invalid ID. 

	bool  SetTargetObject( HTRACKEDNODE ID, const LTVector& vPosition );

	// TargetAnimation
	//	 Tells the specified node to look where the animation is looking. This is
	// commonly used for fading out the tracker by having it look where the animation
	// is looking, and then when it is looking at the target, disabling the tracker.

	bool  TargetAnimation( HTRACKEDNODE ID, const LTVector& vOffset );

	// SetAutoDisable
	//	 Tells the specified node to disable itself when it reaches the point where
	// it is looking at the specified target. This is commonly used in conjunction with
	// TargetAnimation in order to fade the animation out

	bool  SetAutoDisable( HTRACKEDNODE ID, bool bAutoDisable );


	// SetOrientOnAnim
	// 	Sets the flag indicating if this node alignment should be relative to the animation orienation with
	// respect to the model orienation. This is used for things such as aiming guns sideways, in which
	// the node that should be aligned is constantly changing with how the gun is oriented towards
	// the forward of the model. 

	bool  SetOrientOnAnim( HTRACKEDNODE ID, bool bTrackOnAnim );

	// SetIgnoreParentAnimation
	// 	Sets the flag indicating if the animation for the parent node should be completely ignored for
	// the constraints. This essentially makes it so that the constraints are constant regardless of
	// what animation is playing or how the animation has any higher up nodes oriented. This can
	// cause the node to go into odd orientations if the animation does large rotations, but gives
	// constant aligned constraints so it is useful to counteract minor rotations

	bool  SetIgnoreParentAnimation( HTRACKEDNODE ID, bool bIgnoreParentAnimation );


	// GetBasisSpace
	//	 Given a node, it will fill in the appropriate vectors specifying the
	// orienation and position of the node. Returns false on an invalid ID.

	bool  GetBasisSpace( HTRACKEDNODE ID,	LTVector& vRight, 
											LTVector& vUp, 
											LTVector& vForward,
											LTVector& vPos);

	// GetOrientationSpace
	//	 Given a node, it will fill in the appropriate vectors specifying the
	// orienation space as set on the node with the forward and up vectors

	bool  GetOrientationSpace( HTRACKEDNODE ID,	LTVector& vRight, 
												LTVector& vUp, 
												LTVector& vForward);


private:

	// CheckValidity
	//   This will check the validity of the specified tracked node as well as the
	// state of the object, and will return false if either is in a state
	// that it cannot be operated upon. In debug it will also check and see if
	// the tracked node properly belongs to this manager
	
	bool CheckValidity( HTRACKEDNODE ID );


	//the model interface we are using.
	ILTCSBase*					m_pILTBase;

	typedef std::vector<CTrackedNode*> NodeList;
	//our list of nodes
	NodeList					m_cNodes;
};


#endif

