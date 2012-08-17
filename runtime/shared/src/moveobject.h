#ifndef __MOVEOBJECT_H__
#define __MOVEOBJECT_H__


class MoveState;
class ILTPhysics;
class WorldTree;
class Node;
class WorldModelInstance;


void RetransformWorldModel(WorldModelInstance *pWorldModel);


// Flags to MoveObject.
#define MO_DETACHSTANDING	(1<<0)
#define MO_SETCHANGEFLAG	(1<<1)
#define MO_MOVESTANDINGONS	(1<<2)
#define MO_TELEPORT			(1<<3)
#define MO_GOTHRUWORLD		(1<<4) // Don't do world collision.
#define MO_NOSLIDING		(1<<5) // Don't slide (override for FLAG_NOSLIDING)


class MoveAbstract
{
public:
	virtual void			SetObjectChangeFlags(LTObject *pObj, uint32 flags)=0;
	virtual CollisionInfo *&GetCollisionInfo()=0;
	virtual void			DoTouchNotify(LTObject *pMain, LTObject *pTouching, LTVector &stopVel, float force)=0;
	virtual void			PutObjectInContainer(LTObject *pObj, LTObject *pContainer)=0;
	virtual void			BreakContainerLinks(LTObject *pObj)=0;
	virtual void			MoveAttachments(MoveState *pState)=0;
	virtual LTBOOL			ShouldPushObject(MoveState *pState, LTObject *pPusher, LTObject *pPushee)=0;
	virtual void			DoCrush(LTObject *pObject, LTObject *pCrusher)=0;
	virtual void			CheckMaxPos(MoveState *pState, LTVector *pPos)=0;
	virtual uint32			IsServer()=0;
	virtual LTBOOL			CanOptimizeObject(LTObject *pObject)=0;
	virtual char*			GetObjectClassName(LTObject *pObject)=0;
	virtual ILTPhysics *	GetPhysics()=0;
};


class MoveState
{
public:
	MoveState()
	{
		m_pWorldTree = LTNULL;
		m_pAbstract = LTNULL;
		m_pObj = LTNULL;
		m_nRestart = 0;
	}

		
	#ifdef _DEBUG
		// Used so we can make sure calls are being made correctly.
		LTBOOL Verify() {return !!m_pWorldTree && !!m_pAbstract && !!m_pObj;}
	#endif

	void			SetupCall()
	{
	#ifdef _DEBUG
		ASSERT(Verify());
	#endif
		m_bServer = m_pAbstract->IsServer();
	}

	void Setup(WorldTree *pWorldTree,
		MoveAbstract *pAbstract, LTObject *pObj, uint32 bPriority)
	{
		m_pWorldTree = pWorldTree;
		m_pAbstract = pAbstract;
		m_pObj = pObj;
		m_BPriority = bPriority;
		m_CustomTestObjects = LTNULL;
		m_nCustomTestObjects = 0;
	}

	void			Inherit(MoveState *pOther, LTObject *pObj)
	{
		Setup(pOther->m_pWorldTree,
			pOther->m_pAbstract, pObj, pOther->m_BPriority);
	}

// Set these before calling MoveObject.
	WorldTree		*m_pWorldTree;
	MoveAbstract	*m_pAbstract;
	LTObject		*m_pObj;
	uint32			m_BPriority; // What blocking priority are we using?
	
	LTObject		**m_CustomTestObjects; // Tells it to only test these objects for collision.
	uint32			m_nCustomTestObjects;

// Used internally, don't set.
	uint32		m_bServer;
	LTVector	m_vStartPos;
	LTVector	m_vDestPos;
	LTVector	m_vDeltaPos;
	LTVector	m_vMoveCenter;
	float		m_fMoveRadius;
	LTVector	m_vMoveMin;
	LTVector	m_vMoveMax;
	LTMatrix	*m_pWMObjectTransform;
	int			m_nRestart;
};


// Sets up the necessary structures to make pObj stand on pStandingOn.
void SetObjectStanding(LTObject *pObj, LTObject *pStandingOn, const Node *pNode);

// Detach this object from whatever it's standing on.
void DetachObjectStanding(LTObject *pObj);

// Detach any objects standing on this object.
void DetachObjectsStandingOn(LTObject *pObj);

// Is the object solid? Dependent on client/server
LTBOOL IsSolid( uint32 dwFlags, LTBOOL bServer );

// Returns TRUE if the object should be treated as a physical object.
inline LTBOOL IsPhysical(uint32 flags, LTBOOL bServer)
{
	return (IsSolid(flags, bServer) || flags & (FLAG_TOUCH_NOTIFY|FLAG_CONTAINER));
}

// Called when a WorldModel is created to setup its bounding box for its initial rotation.
void InitialWorldModelRotate(WorldModelInstance *pInstance);


// THE function to move an object.  
void MoveObject(MoveState *pState, LTVector moveTo, uint32 flags);

// Change the object's dimensions.
// The real way this function should work is have:
//		bCollide -- tries to push other objects .. if they can't be pushed, return False.
LTBOOL ChangeObjectDimensions(MoveState *pState, LTVector &newDims, LTBOOL bCollide, 
	LTBOOL bTrivialReject = LTTRUE);

// Rotates the world model and moves objects out of the way.
// The client doesn't do collisions automatically when a worldmodel rotates.
void RotateWorldModel(MoveState *pState, 
	const LTRotation &cRotation, 
	LTBOOL bDoCollisions = LTTRUE);

// Process a non-solid collision
void DoNonsolidCollision(MoveAbstract *pAbstract, LTObject *pObj1, LTObject *pObj2);

#endif  // __MOVEOBJECT_H__

