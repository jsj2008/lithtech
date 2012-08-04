#ifndef __RAGDOLL_H__
#define __RAGDOLL_H__

class CRagDollNode;
class CModelNode;
class CRagDollConstraint;

//A handle to a ragdoll node
typedef uint32				HRAGDOLLNODE;
#define INVALIDRAGDOLLNODE	((HRAGDOLLNODE)-1)

class CRagDoll
{
public:

	//the different states that the ragdoll can be in
	enum ERagDollState	{	eRagDollState_Listen,
							eRagDollState_Active	
						};

	CRagDoll(HOBJECT hModel, uint32 nMaxNodes, uint32 nMaxConstraints, uint32 nMaxModelNodes);
	~CRagDoll();

	//this returns the positions of the nodes in the current frame
	uint32			GetCurrentPosition();

	//this returns the positions of the nodes from the previous frame
	uint32			GetPreviousPosition();

	//accesses the coefficient of restitution (fraction of remaining energy after a bounce) of the model
	float			GetCOR() const;
	void			SetCOR(float fCOR);

	//accesses the number of iterations to perform on this model
	uint32			GetNumIterations() const;
	void			SetNumIterations(uint32 nNumIterations);

	//gets the stat that the ragdoll is currently in
	ERagDollState	GetState() const;

	//accessors for the global acceleration that will be applied on the model each frame
	const LTVector& GetAccel() const;
	void			SetAccel(const LTVector& vAccel);

	//creates a node from the specified model node
	HRAGDOLLNODE	CreateNode(const char* pszNodeName, float fRadius, float fWeight);

	//creates a link from a model node to a ragdoll node that will use the passed in nodes in order
	//to determine its position and orientation
	bool			CreateModelNode(const char* pszNodeName, HRAGDOLLNODE hPos, HRAGDOLLNODE hPrimaryLink, HRAGDOLLNODE hSecondaryLink);

	//creates a distance constraint between two nodes
	bool			AddConstraint(const CRagDollConstraint& Constraint);

	//called to finalize the creation of a ragdoll. This must be called before update is, otherwise
	//update will fail
	bool			ActivateRagDoll();

	//called to update this model. This should be called before any accessing of the nodes since it
	//will only evaluate itself on the first call of the frame
	bool			Update();

	//gets the distance between two ragdoll nodes. This assumes the nodes have had their positions updated
	float			GetDistance(HRAGDOLLNODE hNode1, HRAGDOLLNODE hNode2);

	//gets a specified node, returns NULL if out of range
	CRagDollNode*	GetNode(HRAGDOLLNODE hNode);

	//specifies a node to use as the position for the model. Each update the model will be moved
	//to the position of the node for purposes of visibility, hit detection, etc
	bool			SetMovementNode(HRAGDOLLNODE hNode);

	//the amount of drag on the velocities (ie .3 would have 30% of the velocity energy removed per second)
	void			SetDragAmount(float fAmount);

	//the frictional constant. The higher this is the stronger the force of friction
	void			SetFrictionConstant(float fVal);


private:

	void			SetState(ERagDollState eState);

	//this tells the doll to swap the status of the positions (so current becomes previous, etc)
	void			SwapPositions();

	//during a listening state, this will just run through and grab new positions
	bool			UpdateListen();

	//applies the forces onto each node
	bool			ApplyForces(float fCurrFrameTime);

	//runs through all the constraints and applies them the specified number of times
	bool			ApplyConstraints(float fNewFrameTime);

	//handles nodes intersecting other node bounding spheres
	bool			ApplyInterNodeCollisions();

	//runs through the nodes looking for nodes that should have their velocity set to 0 (prevents jitters)
	bool			ClampVelocities();

	//this function will actually handle the updating of the ragdoll positions based upon
	//the forces
	bool			UpdateRagDoll(float fCurrFrameTime);

	//cleans up all allocated memory
	void			Free();

	//the model that this is ragdolling
	HOBJECT			m_hModel;

	//coefficient of restitution for the model
	float			m_fCOR;

	//number of iterations to perform
	uint32			m_nNumIterations;

	//the time delta between the last and current frame
	float			m_fPrevFrameDelta;

	//initial update flag...this is just for testing (applying arbitrary forces)
	//TODO:JO Remove this eventually
	bool			m_bFirstUpdate;

	//determines if the last time update value is valid
	bool			m_bValidLastUpdateTime;

	//the index into the position list for the current frame
	uint32			m_nCurrentIndex;

	//current state of the ragdoll
	ERagDollState	m_eState;

	//the actual nodes that belong to this ragdoll
	CRagDollNode*	m_pNodes;

	//the number of nodes that we have allocated
	uint32			m_nNumNodes;
	uint32			m_nMaxNodes;

	//the amount of drag on the velocities (ie .3 would have 30% of the velocity energy removed per second)
	float			m_fDragAmount;

	//the frictional constant
	float			m_fFrictionConstant;

	//the distance constraints on the model
	CRagDollConstraint**	m_pConstraints;

	//the number of nodes that we have allocated
	uint32			m_nNumConstraints;
	uint32			m_nMaxConstraints;

	//the tyings from the model nodes to the ragdoll nodes
	CModelNode*		m_pModelNodes;

	//the number of nodes that we have allocated
	uint32			m_nNumModelNodes;
	uint32			m_nMaxModelNodes;

	//the acceleration on this ragdoll
	LTVector		m_vAccel;

	//the node that should be used for the position of the model
	HRAGDOLLNODE	m_hMovementNode;

};

#endif

