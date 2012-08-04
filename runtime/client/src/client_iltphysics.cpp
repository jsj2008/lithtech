#include "bdefs.h"

#include "iltphysics.h"
#include "de_objects.h"
#include "moveobject.h"
#include "clientmgr.h"
#include "cmoveabstract.h"

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);



class CLTPhysicsClient : public ILTClientPhysics {
public:
    declare_interface(CLTPhysicsClient);

    CLTPhysicsClient();

    LTRESULT SetVelocity(HOBJECT hObj, const LTVector *pVel);
    LTRESULT SetAcceleration(HOBJECT hObj, const LTVector *pAccel);
    LTRESULT MoveObject(HOBJECT hObj, const LTVector *pPos, uint32 flags);
    LTRESULT UpdateMovement(MoveInfo *pMoveInfo);
    LTRESULT SetObjectDims(HOBJECT hObj, LTVector *pNewDims, uint32 flags);
    LTRESULT GetGlobalForce(LTVector &vec);
    LTRESULT SetGlobalForce(const LTVector &vec);
    LTRESULT MovePushObjects(HOBJECT hToMove, const LTVector &newPos, HOBJECT *hPushObjects, uint32 nPushObjects);
    LTRESULT RotatePushObjects(HOBJECT hToMove, const LTRotation &newRot, HOBJECT *hPushObjects, uint32 nPushObjects);

    LTRESULT GetStairHeight(float &fHeight);
    LTRESULT SetStairHeight(float fHeight);

  private:
    float m_fStairHeight;
};

//instantiate our implementation.
instantiate_interface(CLTPhysicsClient, ILTPhysics, Client);


CLTPhysicsClient::CLTPhysicsClient() {
    m_fStairHeight = -1.0f;
}


LTRESULT CLTPhysicsClient::SetVelocity(HOBJECT hObj, const LTVector *pVel)
{
    LTObject *pObj;

    CHECK_PARAMS(hObj, CLTPhysicsClient::SetVelocity);

    pObj = (LTObject*)hObj;
    pObj->m_Velocity = *pVel;
    pObj->m_InternalFlags |= IFLAG_APPLYPHYSICS;
    return LT_OK;
}

LTRESULT CLTPhysicsClient::SetAcceleration(HOBJECT hObj, const LTVector *pAccel) {
    LTObject *pObj;

    CHECK_PARAMS(hObj, CLTPhysicsClient::SetAcceleration);

    pObj = (LTObject*)hObj;
    pObj->m_Acceleration = *pAccel;
    pObj->m_InternalFlags |= IFLAG_APPLYPHYSICS;
    return LT_OK;
}

LTRESULT CLTPhysicsClient::MoveObject(HOBJECT hObj, const LTVector *pPos, uint32 flags) {
    MoveState moveState;
    LTObject *pObj;
    uint32 moFlags;
    WorldTree *pWorldTree;

    CHECK_PARAMS(hObj, CLTPhysicsClient::MoveObject);
    pWorldTree = world_bsp_client->ClientTree();
    if (!pWorldTree)
    {
      RETURN_ERROR(3, CLTPhysicsClient::MoveObject, LT_NOTINITIALIZED);
    }

    pObj = (LTObject*)hObj;

	// Optimize out non-moving calls
	if (*pPos == pObj->GetPos())
		return LT_OK;

    moveState.Setup(pWorldTree, g_pClientMgr->m_MoveAbstract, pObj, pObj->m_BPriority);

    moFlags = MO_DETACHSTANDING;
    if (flags & MOVEOBJECT_TELEPORT)
        moFlags |= MO_TELEPORT;

    ::MoveObject(&moveState, *pPos, moFlags);
    return LT_OK;
}

LTRESULT CLTPhysicsClient::UpdateMovement(MoveInfo *pMoveInfo)
{
    MotionInfo *pInfo;

    CHECK_PARAMS(pMoveInfo && pMoveInfo->m_hObject, CLTPhysicsClient::UpdateMovement);

    pInfo = &g_pClientMgr->m_MotionInfo;

    if (!(pMoveInfo->m_hObject->m_InternalFlags & IFLAG_APPLYPHYSICS))
	{
      return LT_OK;
    }

    CalcMotion(pInfo,
                pMoveInfo->m_hObject,
                pMoveInfo->m_Offset,
                pMoveInfo->m_hObject->m_Velocity,
                pMoveInfo->m_hObject->m_Acceleration,
                (pMoveInfo->m_hObject->m_Flags & FLAG_GRAVITY) != 0,
                pMoveInfo->m_dt);
    return LT_OK;
}

LTRESULT CLTPhysicsClient::SetObjectDims(HOBJECT hObj, LTVector *pNewDims, uint32 flags) {
    LTVector newDims;
    LTObject *pObj;
    MoveState moveState;
    WorldTree *pWorldTree;


    CHECK_PARAMS(hObj && pNewDims, SPhysicsLT::SetObjectDims);
    pWorldTree = world_bsp_client->ClientTree();
    if (!pWorldTree) {
        RETURN_ERROR(1, CLTPhysicsClient::SetObjectDims, LT_INVALIDPARAMS);
    }

    newDims = *pNewDims;
    pObj = (LTObject*)hObj;

    // Not allowed to change a WorldModel's dimensions!
    if (pObj->m_ObjectType != OT_CONTAINER && pObj->m_ObjectType != OT_WORLDMODEL) {
        moveState.Setup(pWorldTree, g_pClientMgr->m_MoveAbstract, pObj, pObj->m_BPriority);
        if (ChangeObjectDimensions(&moveState, newDims, !!(flags & SETDIMS_PUSHOBJECTS))) {
            return LT_OK;
        }
        else {
            *pNewDims = newDims;
            return LT_ERROR;
        }
    }
    else {
        return LT_INVALIDPARAMS;
    }
}

LTRESULT CLTPhysicsClient::GetGlobalForce(LTVector &vec) {
    vec = g_pClientMgr->m_MotionInfo.m_Force;
    return LT_OK;
}

LTRESULT CLTPhysicsClient::SetGlobalForce(const LTVector &vec) {
    g_pClientMgr->m_MotionInfo.SetForce(&vec);
    return LT_OK;
}

LTRESULT CLTPhysicsClient::MovePushObjects(HOBJECT hToMove, const LTVector &newPos,
    HOBJECT *hPushObjects, uint32 nPushObjects)
{
    FN_NAME(CLTPhysicsClient::MovePushObjects);

    LTObject *pObj;
    MoveState moveState;
    WorldTree *pWorldTree;


    CHECK_PARAMS2(hToMove && hPushObjects);

    pWorldTree = world_bsp_client->ClientTree();

    if (!pWorldTree){
      ERR(3, LT_NOTINITIALIZED);
    }

    pObj = (LTObject*)hToMove;
    moveState.Setup(pWorldTree, g_pClientMgr->m_MoveAbstract, pObj, pObj->m_BPriority);
    moveState.m_CustomTestObjects = (LTObject**)hPushObjects;
    moveState.m_nCustomTestObjects = nPushObjects;
    ::MoveObject(&moveState, newPos, MO_DETACHSTANDING|MO_MOVESTANDINGONS|MO_GOTHRUWORLD);
    return LT_OK;
}


LTRESULT CLTPhysicsClient::RotatePushObjects(HOBJECT hToMove, const LTRotation &newRot,
    HOBJECT *hPushObjects, uint32 nPushObjects)
{
    FN_NAME(CLTPhysicsClient::RotatePushObjects);

    LTObject *pObj;
    MoveState moveState;
    WorldTree *pWorldTree;

    CHECK_PARAMS2(hToMove && hPushObjects);

    pWorldTree = world_bsp_client->ClientTree();

    if (!pWorldTree) {
      ERR(3, LT_NOTINITIALIZED);
    }

    pObj = (LTObject*)hToMove;
    if (pObj->m_ObjectType != OT_WORLDMODEL && pObj->m_ObjectType != OT_CONTAINER)
    {
      RETURN_ERROR(3, CLTPhysicsClient::RotatePushObjects, LT_INVALIDPARAMS);
    }

    moveState.Setup(pWorldTree, g_pClientMgr->m_MoveAbstract, pObj, pObj->m_BPriority);
    moveState.m_CustomTestObjects = (LTObject**)hPushObjects;
    moveState.m_nCustomTestObjects = nPushObjects;

    RotateWorldModel(&moveState, newRot);
    return LT_OK;
}

LTRESULT CLTPhysicsClient::GetStairHeight(float &fHeight) {
    fHeight = m_fStairHeight;
    return LT_OK;
}

LTRESULT CLTPhysicsClient::SetStairHeight(float fHeight) {
    m_fStairHeight = fHeight;
    return LT_OK;
}


