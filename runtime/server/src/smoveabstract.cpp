#include "bdefs.h"

#include "smoveabstract.h"
#include "s_object.h"
#include "servermgr.h"
#include "interlink.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------
//IWorld holder
#include "world_server_bsp.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);

//the ILTServer game interface
#include "iltserver.h"
static ILTServer *ilt_server;
define_holder(ILTServer, ilt_server);





extern float g_DebugMaxPos;

void SMoveAbstract::SetObjectChangeFlags(LTObject *pObj, uint32 flags)
{
    ::SetObjectChangeFlags(pObj, flags);
}


CollisionInfo *& SMoveAbstract::GetCollisionInfo()
{
    return g_pServerMgr->m_pCollisionInfo;
}


void SMoveAbstract::DoTouchNotify(LTObject *pMain, LTObject *pTouching, LTVector &stopVel, float forceMag)
{
    if (g_pServerMgr->m_pCollisionInfo)
    {
        g_pServerMgr->m_pCollisionInfo->m_vStopVel = stopVel;
        g_pServerMgr->m_pCollisionInfo->m_hObject = (HOBJECT)pTouching;
//      pMain->sd->m_pObject->EngineMessageFn(
//          MID_TOUCHNOTIFY, pTouching, forceMag);
		pMain->sd->m_pObject->OnTouch(pTouching, forceMag);
    }
}


void SMoveAbstract::DoCrush(LTObject *pObject, LTObject *pCrusher)
{
//    pObject->sd->m_pObject->EngineMessageFn(
//        MID_CRUSH, ServerObjToHandle(pCrusher), 0.0f);
	pObject->sd->m_pObject->OnCrush(pCrusher);
}


void SMoveAbstract::PutObjectInContainer(LTObject *pObj, LTObject *pContainer)
{
    pObj->m_InternalFlags |= IFLAG_APPLYPHYSICS;

    // Link them together.
    CreateInterLink(pContainer, pObj, LINKTYPE_CONTAINER);
}


void SMoveAbstract::BreakContainerLinks(LTObject *pObj)
{
    BreakInterLinks(pObj, LINKTYPE_CONTAINER, FALSE);
}


void SMoveAbstract::MoveAttachments(MoveState *pState)
{
    Attachment *pAttachment;
    LTObject *pAttachedObj;
    MoveState moveState;
    LTVector attachPos;

    // Move the attachments (but don't set their change flags if it's a model.. the client will set their
    // position automatically).
    pAttachment = pState->m_pObj->m_Attachments;
    while (pAttachment)
    {
        pAttachedObj = sm_FindObject(pAttachment->m_nChildID);
        if (pAttachedObj)
        {
            LTVector vNewPos = pAttachment->m_Offset.m_Pos;
            LTMatrix mTemp;
            pState->m_pObj->m_Rotation.ConvertToMatrix(mTemp);
            mTemp.Apply3x3(vNewPos);
            attachPos = pState->m_pObj->GetPos() + vNewPos;
            
            // Teleport the attachment to the right spot...
            moveState.Setup(pState->m_pWorldTree, pState->m_pAbstract, pAttachedObj, pAttachedObj->m_BPriority);
            
			// Set the MoveObject flags...
			uint32 dwFlags = MO_DETACHSTANDING | MO_MOVESTANDINGONS | MO_TELEPORT;
			if( pState->m_pObj->m_ObjectType != OT_MODEL )
			{
				// Set the change flags for everything but models...
				dwFlags |= MO_SETCHANGEFLAG;
			}

			MoveObject(&moveState, attachPos, dwFlags);

            // Update its rotation..
            pAttachedObj->m_Rotation = pState->m_pObj->m_Rotation * pAttachment->m_Offset.m_Rot;
        }

        pAttachment = pAttachment->m_pNext;
    }
}


LTBOOL SMoveAbstract::ShouldPushObject(MoveState *pState, LTObject *pPusher, LTObject *pPushee)
{
    return pState->m_BPriority > pPushee->m_BPriority && pPushee->IsMoveable(); 
}


void SMoveAbstract::CheckMaxPos(MoveState *pState, LTVector *pPos)
{
    if (g_DebugMaxPos > 0.0f && pPos->MagSqr() > g_DebugMaxPos * g_DebugMaxPos)
    {
        dsi_ConsolePrint("Server: position larger than 'DebugMaxPos'");
        dsi_ConsolePrint("Object class: %s, position (%.2f, %.2f, %.2f)", pState->m_pObj->sd->m_pClass->m_ClassName,
            pPos->x, pPos->y, pPos->z);
    }
}


uint32 SMoveAbstract::IsServer()
{
    return LTTRUE;
}


LTBOOL SMoveAbstract::CanOptimizeObject(LTObject *pObj)
{
    return ::CanOptimizeObject(pObj);
}


char* SMoveAbstract::GetObjectClassName(LTObject *pObject)
{
    return pObject->sd->m_pClass->m_ClassName;
}

void FullMoveObject(LTObject *pObj, const LTVector *pP1, const uint32 flags) {
    MoveState moveState;

    moveState.Setup(world_bsp_server->ServerTree(), g_pServerMgr->m_MoveAbstract, pObj, pObj->m_BPriority);

    MoveObject(&moveState, *pP1, flags);
}

ILTPhysics *SMoveAbstract::GetPhysics() { 
    return ilt_server->Physics(); 
}
//EOF
