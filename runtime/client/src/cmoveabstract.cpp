#include "bdefs.h"

#include "cmoveabstract.h"
#include "clientmgr.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);






ILTPhysics *CMoveAbstract::GetPhysics() { 
    return ilt_client->Physics(); 
}

void CMoveAbstract::SetObjectChangeFlags(LTObject *pObj, uint32 flags) {

}


CollisionInfo *&CMoveAbstract::GetCollisionInfo() {
    return g_pClientMgr->m_pCollisionInfo;
}


void CMoveAbstract::DoTouchNotify(LTObject *pMain, LTObject *pTouching, LTVector &stopVel, float forceMag) {
    if (g_pClientMgr->m_pCollisionInfo) {
        g_pClientMgr->m_pCollisionInfo->m_vStopVel = stopVel;
        g_pClientMgr->m_pCollisionInfo->m_hObject = (HOBJECT)pTouching;

        if (i_client_shell != NULL) {
            i_client_shell->OnTouchNotify((HOBJECT)pMain, g_pClientMgr->m_pCollisionInfo, forceMag);
        }
    }
}


void CMoveAbstract::DoCrush(LTObject *pObject, LTObject *pCrusher) {

}


void CMoveAbstract::PutObjectInContainer(LTObject *pObj, LTObject *pContainer) {

}


void CMoveAbstract::BreakContainerLinks(LTObject *pObj) {

}

void CMoveAbstract::MoveAttachments(MoveState *pState) {

}


LTBOOL CMoveAbstract::ShouldPushObject(MoveState *pState, LTObject *pPusher, LTObject *pPushee) {
    return pState->m_CustomTestObjects && pState->m_pObj == pPusher && pPushee->IsMoveable();
}


void CMoveAbstract::CheckMaxPos(MoveState *pState, LTVector *pPos) {

}


uint32 CMoveAbstract::IsServer() {
    return LTFALSE;
}


LTBOOL CMoveAbstract::CanOptimizeObject(LTObject *pObj) {
    return LTFALSE;
}


char* CMoveAbstract::GetObjectClassName(LTObject *pObject) {
    return "CLIENT-OBJECT";
}

//EOF
