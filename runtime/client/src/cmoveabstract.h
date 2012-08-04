
// Implements the client-side movement abstraction.

#ifndef __CMOVEABSTRACT_H__
#define __CMOVEABSTRACT_H__

#ifndef __MOVEOBJECT_H__
#include "moveobject.h"
#endif


class CClientMgr;


class CMoveAbstract : public MoveAbstract
{
public:

					CMoveAbstract() {};
	
	void			SetObjectChangeFlags(LTObject *pObj, uint32 flags);
	CollisionInfo *&GetCollisionInfo();
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

#endif  // __CMOVEABSTRACT_H__


