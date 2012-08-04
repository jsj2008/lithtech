//-----------------------------------------------------------------------------
// NodeTracker.h
//
// Provides the definition for the node tracking system. This system serves the role 
// of allowing orienting nodes within a model so that it appears to be tracking a 
// specified target. 
//
//-----------------------------------------------------------------------------
#ifndef __NODETRACK_H__
#define __NODETRACK_H__

class CNodeTracker : public ILTObjRefReceiver
{
public:

	//constructs a node tracker that is associated with the specified object. Note that this
	//assumes that the object will not be going away from underneath this controller
	CNodeTracker();
	~CNodeTracker();

	//called to set the object associated with this node tracker. This object must be a model
	//and this call should be set prior to setting any node information with this tracker.
	HOBJECT			GetObject() const					{ return m_hObject; }
	void			SetObject(HOBJECT hObject);


	//----------------------------------
	// Controlled node settings

	//called to determine the number of controlled nodes exist within this simulation
	uint32			GetNumControlledNodes() const			{ return m_nNumControlledNodes; }

	//called to set the number of controlled nodes that this node tracker controls. Note that
	//all settings and nodes currently associated with this tracker will be discarded during
	//this operation.
	bool			SetNumControlledNodes(uint32 nNumNodes);

	//called to set the node that is controlled. The provided index must be in the range
	//of [0..Num controlled nodes)
	HMODELNODE		GetControlledNode(uint32 nNode) const;
	bool			SetControlledNode(uint32 nNode, HMODELNODE hNode);

	//called to set the weight associated with the node. The index should be in the range
	//of [0..Num controlled nodes), and the weight should be [0..1] 
	float			GetControlledNodeWeight(uint32 nNode) const;
	bool			SetControlledNodeWeight(uint32 nNode, float fWeight);

	//called to access the blend weight of the entire system. This is in the range
	//of [0..1] where 0 means the system is effectively disabled, and 1 is full movement
	float			GetSystemBlendWeight() const			{ return m_fSystemBlendWeight; }
	void			SetSystemBlendWeight(float fWeight);

	//----------------------------------
	// Aimer node settings

	//sets the model node to the provided aimer node
	HMODELNODE		GetAimerNode() const					{ return m_hAimerNode; }
	void			SetAimerNode(HMODELNODE hAimer);

	//access to the limits of the aimer node in radians for each direction. The X is limited to
	//(-PI..PI), and Y is limited to (-PI/2..PI/2)
	const LTRect2f&	GetAimerLimits() const					{ return m_rAimerLimits; }
	void			SetAimerLimits(const LTRect2f& rLimits);

	//access to the maximum speed that the aimer can turn at. This is measured in radians per second
	//and should be >= 0
	float			GetMaxSpeed() const						{ return m_fMaxSpeedRadPerS; }
	void			SetMaxSpeed(float fMaxSpeed);

	//----------------------------------
	// Target settings

	//the different types of targeting that can be done
	enum ETargetType 
	{
		eTarget_World,		//target a world space position
		eTarget_Object,		//target an object
		eTarget_Node,		//target a node on a model
		eTarget_Aimer		//target an offset in aimer space
	};
	
	//clears the targeting information, causing the aimer to simply try to return to the 
	//animation's indicated facing
	void			ClearTarget();

	//called to track a local offset in the node tracker space
	void			SetTargetAimer(const LTVector& vOffset);

	//this behaves the same as SetTargetAimer, but takes in extents as a parameter and converts
	//that to the target position based upon the current extents of the node
	void			SetTargetAimerExtents(const LTPolarCoord& polarExtents);

	//called to set the target to a world space position
	void			SetTargetWorld(const LTVector& vPos);

	//called to set the target to follow a specified object. If the object is invalid, then nothing
	//will be tracked
	void			SetTargetObject(HOBJECT hObj, const LTVector& vOffset);

	//called to set the target to follow a specific model node. If either the model or node is invalid,
	//then nothing will be tracked
	void			SetTargetNode(HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset);

	//called to access the different parts of the tracking information. Note that the relevant
	//information can be NULL if not applicable, such as if it is a world space position, the HOBJECT
	//will be NULL.
	ETargetType		GetTargetType() const			{ return m_eTargetType; }
	HOBJECT			GetTargetObject() const			{ return m_hTargetObject; }
	HMODELNODE		GetTargetNode()	const			{ return m_hTargetNode; }
	const LTVector&	GetTargetOffset() const			{ return m_vTargetOffset; }

	//----------------------------------
	// Current status

	//called to determine if during the last update, the aimer was able to aim directly
	//at the target
	bool			DidAimAtTarget() const			{ return m_bDidAimAtTarget; }

	//called to get the parameterized extents of the current aiming direction. This is in the range
	//of [-1..1] with respect to the aiming node extents
	const LTPolarCoord&	GetCurrentExtents() const	{ return m_polarAimExtents; }

	//called to update the current status of the node controller based upon the update time that
	//is provided in seconds. This will return false if the object was in an invalid state
	//and could not be updated.
	bool			Update(float fElapsedTimeS);

	//----------------------------------
	// Serialization

	//called to save the current state and parameters of this node tracker to the provided stream
	void			Save(ILTMessage_Write* pOutFile);

	//called to load the current state of this node tracker from the provided stream. Note that all
	//current status and parameters will be lost during this operation.
	bool			Load(ILTMessage_Read* pInFile);

	//----------------------------------
	// ILTObjRefReceiver

	// when HOBJECT ref points to gets deleted.
	virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

private:

	PREVENT_OBJECT_COPYING(CNodeTracker);

	//----------------------------------
	// Node Controller Support

	//called to handle registering/unregistering a node for the node tracker callback. These
	//will do nothing if the specified node is invalid, or there is no associated object
	void		RegisterNode(uint32 nNode);
	void		UnregisterNode(HOBJECT hObj, uint32 nNode);

	//the actual node controller function that will be called to apply influence to the nodes
	static void NodeControllerCB(const NodeControlData& NodeData, void* pUserData);

	//----------------------------------
	// Internal utilities

	//called to clean up and destroy the controlled node list
	void		FreeControlledNodes();

	//called to get a vector that represents the target orientation in aimer space
	LTVector	GetTargetPosAimerSpace(const LTRigidTransform& tInvAimerSpace) const;

	//--------------------
	// System parameters
	//--------------------

	//the object that this node tracker is associated with
	LTObjRefNotifier m_hObject;

	//-----------
	// Controlled node parameters

	//a structure that represents all the data necessary for a controlled node
	struct SControlledNode
	{
		SControlledNode();

		//a pointer back to the containing node. This is done in order to allow for these
		//structures to be provided to the node controller without having to create
		//intermediate objects
		CNodeTracker*	m_pOwner;

		//the blending weight used with this node
		float			m_fWeight;

		//the node that this controlled node controls in the model
		HMODELNODE		m_hNode;
	};

	//the listing of nodes that this system controls
	SControlledNode*		m_pControlledNodes;
	uint32					m_nNumControlledNodes;

	//the overall blend weight of this system. Useful for ramping the system on or off
	float					m_fSystemBlendWeight;

	//-----------
	// Aimer node parameters

	//the node that we will use as the aimer
	HMODELNODE				m_hAimerNode;

	//the limits on the aimer node, measured in radians
	LTRect2f				m_rAimerLimits;

	//the maximum speed of this aimer in radians per second
	float					m_fMaxSpeedRadPerS;

	//-----------
	// Target parameters

	//the type of targeting that is being done
	ETargetType				m_eTargetType;

	//the current object that is being targeted (NULL if it is the world)
	LTObjRef				m_hTargetObject;

	//the current node that is being targeted (NULL if none)
	HMODELNODE				m_hTargetNode;

	//the offset vector in the target space to allow for offsetting of the target
	LTVector				m_vTargetOffset;

	//--------------------
	// Current State
	//--------------------

	//flag indicating whether or not the node tracking was able to successfully target
	//the target the last update
	bool					m_bDidAimAtTarget;

	//the parameterized extents of the current orientation with respect to the limits [-1..1]
	LTPolarCoord			m_polarAimExtents;

	//the object space orientation of the aimer node during the last update. This is necessary
	//since other object space nodes must be converted to aimer space to have the rotation applied
	LTRotation				m_rAimerObjectRot;

	//the current unit forward vector in the aimer node space
	LTVector				m_vAimDirection;
};


#endif
