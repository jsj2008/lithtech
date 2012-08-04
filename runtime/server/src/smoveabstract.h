
// Server-side movement abstraction.

#ifndef __SMOVEABSTRACT_H__
#define __SMOVEABSTRACT_H__

 
#ifndef __MOVEOBJECT_H__
#include "moveobject.h"
#endif

class CServerMgr;

class SMoveAbstract : public MoveAbstract
{
public:

	SMoveAbstract()
	{

    }
	
	void			SetObjectChangeFlags(LTObject *pObj, uint32 flags);
	CollisionInfo*&	GetCollisionInfo();
	void			DoTouchNotify(LTObject *pMain, LTObject *pTouching, LTVector &stopVel, float forceMag);
	void			PutObjectInContainer(LTObject *pObj, LTObject *pContainer);
	void			BreakContainerLinks(LTObject *pObj);
	void			MoveAttachments(MoveState *pState);
	LTBOOL			ShouldPushObject(MoveState *pState, LTObject *pPusher, LTObject *pPushee);
	void			DoCrush(LTObject *pObject, LTObject *pCrusher);
	void			CheckMaxPos(MoveState *pState, LTVector *pPos);
	uint32			IsServer();
	LTBOOL			CanOptimizeObject(LTObject *pObj);
	char*			GetObjectClassName(LTObject *pObject);
	ILTPhysics *	GetPhysics();
};


void FullMoveObject
(
	LTObject*		pObj,
	const LTVector*	pP1,
	const uint32	flags
);

#endif
