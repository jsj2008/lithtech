#include "bdefs.h"

#include "iltphysics.h"
#include "de_objects.h"
#include "serverobj.h"
#include "smoveabstract.h"
#include "servermgr.h"
#include "s_object.h"
#include "packetdefs.h"

//IWorldServerBSP interface.
#include "world_server_bsp.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);


class CLTPhysicsServer : public ILTPhysics {
public:
    declare_interface(CLTPhysicsServer);

    CLTPhysicsServer();

    LTRESULT SetVelocity(HOBJECT hObj, const LTVector *pVel);
    LTRESULT SetAcceleration(HOBJECT hObj, const LTVector *pAccel);
    LTRESULT MoveObject(HOBJECT hObj, const LTVector *pNewPos, uint32 flags);
    LTRESULT SetObjectDims(HOBJECT hObj, LTVector *pNewDims, uint32 flags);
    LTRESULT GetGlobalForce(LTVector &vec);
    LTRESULT SetGlobalForce(const LTVector &vec);

    LTRESULT GetStairHeight(float &fHeight);
    LTRESULT SetStairHeight(float fHeight);

private:
    float m_fStairHeight;

};

//instantiate our implementation.
instantiate_interface(CLTPhysicsServer, ILTPhysics, Server);




CLTPhysicsServer::CLTPhysicsServer() {
    m_fStairHeight = -1.0f;
}



LTRESULT CLTPhysicsServer::SetVelocity(HOBJECT hObj, const LTVector *pVel)
{
    LTObject *pObj;

    if (!hObj || !pVel)
	{
        RETURN_ERROR(1, CLTPhysicsServer::SetVelocity, LT_INVALIDPARAMS);
    }

    pObj = HandleToServerObj(hObj);

    // No change?
    if (pObj->m_Velocity.DistSqr(*pVel) < 0.001f) {
        return LT_OK;
    }

    pObj->m_InternalFlags |= IFLAG_APPLYPHYSICS;

    pObj->m_Velocity = *pVel;
    return LT_OK;
}


LTRESULT CLTPhysicsServer::SetAcceleration(HOBJECT hObj, const LTVector *pAccel) {
    LTObject *pObj;

    if (!hObj)
    {
        RETURN_ERROR(1, CLTPhysicsServer::SetAcceleration, LT_INVALIDPARAMS);
    }

    pObj = HandleToServerObj(hObj);

    // No change?
    if (pObj->m_Acceleration.DistSqr(*pAccel) < 0.001f)
        return LT_OK;

    pObj->m_InternalFlags |= IFLAG_APPLYPHYSICS;
    pObj->m_Acceleration = *pAccel;
    return LT_OK;
}


LTRESULT CLTPhysicsServer::MoveObject(HOBJECT hObj, const LTVector *pNewPos, uint32 flags) {
    LTObject *pObj;

    if (!hObj)
    {
        RETURN_ERROR(1, CLTPhysicsServer::MoveObject, LT_INVALIDPARAMS);
    }

    pObj = HandleToServerObj(hObj);

	// Optimize out non-moving calls
	if (*pNewPos == pObj->GetPos())
		return LT_OK;

    FullMoveObject(pObj, pNewPos, MO_DETACHSTANDING|MO_SETCHANGEFLAG|MO_MOVESTANDINGONS);
    return LT_OK;
}

LTRESULT CLTPhysicsServer::SetObjectDims(HOBJECT hObj, LTVector *pNewDims, uint32 flags) {
    LTVector newDims;
    LTObject *pObj;
    MoveState moveState;


    newDims = *pNewDims;

    pObj = HandleToServerObj(hObj);
    CHECK_PARAMS(hObj && pNewDims, SPhysicsLT::SetObjectDims);

    // Not allowed to change a WorldModel's dimensions!
    if (pObj->m_ObjectType != OT_CONTAINER && pObj->m_ObjectType != OT_WORLDMODEL)
    {
		LTVector vOldDims = pObj->GetDims();
        moveState.Setup(world_bsp_server->ServerTree(), g_pServerMgr->m_MoveAbstract, pObj, pObj->m_BPriority);
        if (ChangeObjectDimensions(&moveState, newDims, !!(flags & SETDIMS_PUSHOBJECTS)))
        {
			if (pObj->GetDims() != vOldDims)
				SetObjectChangeFlags(pObj, CF_DIMS);
            return LT_OK;
        }
        else
        {
            *pNewDims = newDims;
            return LT_ERROR;
        }
    }
    else
    {
        return LT_INVALIDPARAMS;
    }
}


LTRESULT CLTPhysicsServer::GetGlobalForce(LTVector &vec) {
    vec = g_pServerMgr->m_MotionInfo.m_Force;
    return LT_OK;
}

LTRESULT CLTPhysicsServer::SetGlobalForce(const LTVector &vec) {
    g_pServerMgr->m_MotionInfo.SetForce(&vec);
    return LT_OK;
}

LTRESULT CLTPhysicsServer::GetStairHeight(float &fHeight) {
    fHeight = m_fStairHeight;
    return LT_OK;
}

LTRESULT CLTPhysicsServer::SetStairHeight(float fHeight) {
    m_fStairHeight = fHeight;
    return LT_OK;
}

