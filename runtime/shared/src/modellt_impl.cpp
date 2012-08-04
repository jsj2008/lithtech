#include "bdefs.h"

#include "iltmodel.h"
#include "de_objects.h"
#include "objectmgr.h"
#include "impl_common.h"
#include "animtracker.h"

// ---------------------------------------------------------------------------


LTRESULT ILTModel::GetNextNode( HOBJECT hObject, HMODELNODE hNode, HMODELNODE &Next )
{
	Model *pModel;
	uint32 index;

	if(!hObject || hObject->m_ObjectType != OT_MODEL)
		RETURN_ERROR(1, GetNextModelNode, LT_INVALIDPARAMS);

	pModel = ToModel(hObject)->GetModelDB();

	if(hNode == INVALID_MODEL_NODE)
		index = (uint32)-1;
	else
		index = hNode;

	if((index+1) >= pModel->NumNodes())
	{
		return LT_FINISHED;
	}
	else
	{
		Next = index + 1;
		return LT_OK;
	}
}


LTRESULT ILTModel::GetRootNode( HOBJECT hObj, HMODELNODE &hNode )
{
	return GetNextNode(hObj,INVALID_MODEL_NODE,hNode);
}


LTRESULT ILTModel::GetParent( HOBJECT hObj, HMODELNODE node, HMODELNODE &parent )
{
	FN_NAME(ILTModel::GetParent);
    CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);

	parent = hObj->ToModel()->NodeGetParent(node);

	return LT_OK;
}


LTRESULT ILTModel::GetChild( HOBJECT hObj, HMODELNODE parent, uint32 index , HMODELNODE &child)
{
	FN_NAME(ILTModel::GetChild);
    CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);

	child = hObj->ToModel()->NodeGetChild(parent,index);

	return LT_OK;
}


LTRESULT ILTModel::GetNumChildren( HOBJECT hObj, HMODELNODE hNode, uint32 &NumChildren )
{
	FN_NAME(ILTModel::GetNumChildren);
    CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);

	NumChildren = hObj->ToModel()->NodeGetNumChildren(hNode);

	return LT_OK;
}

// GetNodeName( mi, node, ret-name, ret-name-buf-size )
LTRESULT ILTModel::GetNodeName( HOBJECT hObj, HMODELNODE hNode, char *name, uint32 maxLen )
{
	FN_NAME(ILTModel::GetNodeName);
    CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);

	if( maxLen == 0 || (hNode > hObj->ToModel()->GetModelDB()->NumNodes()) )
		RETURN_ERROR(1, GetNodeName, LT_INVALIDPARAMS);

	LTStrCpy(name, hObj->ToModel()->GetModelDB()->GetNode(hNode)->GetName(), maxLen );

	return LT_OK;
}


LTRESULT ILTModel::GetNumNodes( HOBJECT hObj, uint32 &num_nodes )
{
	FN_NAME(ILTModel::GetNumNodes);
    CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);

	if( hObj->ToModel()->GetModelDB())
	{
		num_nodes = hObj->ToModel()->GetModelDB()->NumNodes();
		return LT_OK;
	}

	return LT_ERROR ;
}

LTRESULT ILTModel::GetBindPoseNodeTransform( HOBJECT hObj, HMODELNODE node, LTMatrix &mat )
{
		FN_NAME(ILTModel::GetNumNodes);
    CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);

	Model *pModel = hObj->ToModel()->GetModelDB();

	if( pModel )
	{
		ModelNode *pNode = pModel->GetNode(node);
		mat = pNode->GetGlobalTransform();
		return LT_OK;
	}

	return LT_ERROR;
}

LTRESULT ILTModel::AddNodeControlFn( HOBJECT hObj, HMODELNODE hNode, NodeControlFn pFn, void *pUserData)
{
	FN_NAME(ILTModel::SetNodeControlFn);
    ModelInstance *pInst;

    CHECK_PARAMS2(hObj && (hObj->m_ObjectType == OT_MODEL) && (hNode != INVALID_MODEL_NODE));

	pInst = hObj->ToModel();
	pInst->AddNodeControlFn(hNode, pFn, pUserData);

	return LT_OK ;
}

LTRESULT ILTModel::AddNodeControlFn( HOBJECT hObj, NodeControlFn pFn, void *pUserData)
{
	FN_NAME(ILTModel::SetNodeControlFn);
    ModelInstance *pInst;

    CHECK_PARAMS2(hObj && (hObj->m_ObjectType == OT_MODEL));

	pInst = hObj->ToModel();
	pInst->AddNodeControlFn(pFn, pUserData);

	return LT_OK ;
}

LTRESULT ILTModel::RemoveNodeControlFn( HOBJECT hObj, HMODELNODE hNode, NodeControlFn pFn, void* pUserData)
{
	FN_NAME(ILTModel::SetNodeControlFn);
    ModelInstance *pInst;

    CHECK_PARAMS2(hObj && (hObj->m_ObjectType == OT_MODEL) && (hNode != INVALID_MODEL_NODE));

	pInst = hObj->ToModel();
	pInst->RemoveNodeControlFn(hNode, pFn, pUserData);

	//we need to re-evaluate the nodes since we have changed the tracker
	pInst->ResetCachedTransformNodeStates();

	return LT_OK ;
}

LTRESULT ILTModel::RemoveNodeControlFn( HOBJECT hObj, NodeControlFn pFn, void* pUserData)
{
	FN_NAME(ILTModel::SetNodeControlFn);
    ModelInstance *pInst;

    CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);

	pInst = hObj->ToModel();
	pInst->RemoveNodeControlFn(pFn, pUserData);

	//we need to re-evaluate the nodes since we have changed the tracker
	pInst->ResetCachedTransformNodeStates();

	return LT_OK ;
}

LTRESULT ILTModel::GetSocket(HOBJECT hObj, const char *pSocketName, HMODELSOCKET &hSocket)
{
    ModelInstance *pInst;
    Model *pModel;

    hSocket = INVALID_MODEL_SOCKET;

    CHECK_PARAMS(hObj && pSocketName && hObj->m_ObjectType == OT_MODEL, ILTModel::GetSocket);

    pInst = (ModelInstance*)hObj;
    pModel = pInst->GetModelDB();
    if (!pModel)
    {
        RETURN_ERROR(1, ILTModel::GetSocket, LT_NOTINITIALIZED);
    }

    if (pModel->FindSocket(pSocketName, &hSocket))
    {
        return LT_OK;
    }
    else
    {
        RETURN_ERROR(1, ILTModel::GetSocket, LT_NOTFOUND);
    }
}


LTRESULT ILTModel::GetNode(HOBJECT hObj, const char *pNodeName, HMODELNODE &hNode)
{
    ModelInstance *pInst;
    Model *pModel;


    hNode = INVALID_MODEL_NODE;

    CHECK_PARAMS(hObj && pNodeName && hObj->m_ObjectType == OT_MODEL, ILTModel::GetNode);

    pInst = (ModelInstance*)hObj;
    pModel = pInst->GetModelDB();
    if (!pModel)
    {
        RETURN_ERROR(1, ILTModel::GetNode, LT_NOTINITIALIZED);
    }

    if (pModel->FindNode(pNodeName, &hNode))
    {
        return LT_OK;
    }
    else
    {
        RETURN_ERROR(1, ILTModel::GetNode, LT_NOTFOUND);
    }
}

LTRESULT ILTModel::GetNumPieces( HOBJECT hObj, uint32 & NumPieces )
{
	FN_NAME(ILTModel::GetPiece);
    CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);

	NumPieces = hObj->ToModel()->GetModelDB()->m_Pieces.GetSize();

	return LT_OK ;

}

LTRESULT ILTModel::GetPiece(HOBJECT hObj, const char *pName, HMODELPIECE &hPiece)
{
    FN_NAME(ILTModel::GetPiece);
    ModelInstance *pInst;
    Model *pModel;


    hPiece = INVALID_MODEL_PIECE;

    CHECK_PARAMS2(hObj && pName && hObj->m_ObjectType == OT_MODEL);

    pInst = (ModelInstance*)hObj;
    pModel = pInst->GetModelDB();
    if (!pModel)
    {
        ERR(1, LT_NOTINITIALIZED);
    }

    if (pModel->FindPiece(pName, &hPiece))
    {
        return LT_OK;
    }
    else
    {
        ERR(1, LT_NOTFOUND);
    }
}


LTRESULT ILTModel::FindWeightSet(HOBJECT hObj, const char *pName, HMODELWEIGHTSET &hSet)
{
    FN_NAME(ILTModel::GetWeightSet);
    ModelInstance *pInst;
    Model *pModel;

    hSet = INVALID_MODEL_WEIGHTSET;

    CHECK_PARAMS2(hObj && pName && hObj->m_ObjectType == OT_MODEL);

    pInst = (ModelInstance*)hObj;
    pModel = pInst->GetModelDB();
    if (!pModel)
    {
        ERR(1, LT_NOTINITIALIZED);
    }

    if (pModel->FindWeightSet(pName, &hSet))
    {
        return LT_OK;
    }
    else
    {
        ERR(1, LT_NOTFOUND);
    }
}



LTRESULT ILTModel::GetPieceHideStatus(HOBJECT pObj, HMODELPIECE hPiece, bool &bHidden)
{
    FN_NAME(ILTModel::GetPieceHideStatus);
    ModelInstance *pInst;


    CHECK_PARAMS2(pObj && pObj->m_ObjectType == OT_MODEL);

    pInst = ToModel(pObj);
    bHidden = pInst->IsPieceHidden(hPiece);

    return LT_OK;
}


LTRESULT ILTModel::SetPieceHideStatus(HOBJECT pObj, HMODELPIECE hPiece, bool bHidden)
{
    FN_NAME(ServerModelLT::SetPieceHideStatus);
    ModelInstance *pInst;

    CHECK_PARAMS2(pObj && pObj->m_ObjectType == OT_MODEL);
    pInst = ToModel(pObj);

    bHidden = !!bHidden;
    if (bHidden == pInst->IsPieceHidden(hPiece))
    {
        return LT_NOCHANGE;
    }
    else
    {
        if (bHidden)
        {
            pInst->m_HiddenPieces[hPiece / 32] |= (1 << (hPiece % 32));
        }
        else
        {
            pInst->m_HiddenPieces[hPiece / 32] &= ~(1 << (hPiece % 32));
        }

        return LT_OK;
    }
}



LTRESULT ILTModel::GetNodeTransform( HOBJECT hObj, HMODELNODE hNode,
									 LTransform &transform, bool bWorldSpace)
{
    FN_NAME(ILTModel::GetNodeTransform);
    ModelInstance *pInst;

    CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);


    pInst = (ModelInstance*)hObj;

	if( pInst->GetNodeTransform(hNode, transform, bWorldSpace) )
		return LT_OK ;
	else
		return LT_ERROR ;

}


LTRESULT ILTModel::ApplyAnimations(HOBJECT hObj)
{
	FN_NAME(ILTModel::ApplyAnimations);
	CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);

	hObj->ToModel()->ForceUpdateCachedTransforms();

	return LT_OK;
}



LTRESULT ILTModel::GetSocketTransform(HOBJECT hObj,
									  HMODELSOCKET hSocket,
									  LTransform &transform,
									  bool bWorldSpace)
{
    FN_NAME(ILTModel::GetSocketTransform);
    ModelInstance *pInst;

    CHECK_PARAMS2(hObj && hObj->m_ObjectType == OT_MODEL);

    pInst = (ModelInstance*)hObj;

	if(pInst->GetSocketTransform( hSocket, transform, bWorldSpace ))
		return LT_OK ;
	else
		return LT_ERROR ;


    return LT_OK;
}

LTRESULT ILTModel::UpdateMainTracker(HOBJECT pObj, float fUpdateDelta)
{
    FN_NAME(ILTModel::UpdateMainTracker);
    CHECK_PARAMS2(pObj && pObj->m_ObjectType == OT_MODEL);

	ModelInstance *pInst = pObj->ToModel();

	//if the model isn't currently paused, dispatch to the update
	if(!pObj->IsPaused())
	{
		uint32 nElapsedTime = (uint32)(fUpdateDelta * 1000.0f);

		if(pInst->GetCSType() == ClientType )
			pInst->ClientUpdate(nElapsedTime);
		else
			pInst->ServerUpdate(nElapsedTime);
	}

	return LT_OK;
}


LTRESULT ILTModel::GetMainTracker(HOBJECT hModel, ANIMTRACKERID &TrackerID)
{
	TrackerID = MAIN_TRACKER;
	return LT_OK;
}


LTRESULT ILTModel::GetPlaybackState(HOBJECT hModel, ANIMTRACKERID TrackerID,
                                      uint32 &flags)
{
	FN_NAME(ILTModel::GetPlaybackState);
	flags = 0;
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

    if (trk_IsStopped(pTracker))
        flags |= MS_PLAYDONE;

	return LT_OK;
}

LTRESULT ILTModel::AddTracker(HOBJECT hModel, ANIMTRACKERID TrackerID)
{
	ASSERT( TrackerID != MAIN_TRACKER );

	FN_NAME(ILTModel::AddTracker);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	if( TrackerID == MAIN_TRACKER || pModel->GetTracker(TrackerID) )
		return LT_ALREADYEXISTS;

	// find the end of the tracker list
	// (and also count trackers to check for overflow)
    ASSERT( pModel->m_AnimTrackers );

	LTAnimTracker **ppLast, *pTest;
	int trackerCount = 0;

    ppLast = &pModel->m_AnimTrackers;
    for(pTest=pModel->m_AnimTrackers; pTest; pTest=pTest->GetNext())
    {
        ppLast = (LTAnimTracker**)&pTest->m_Link.m_pNext;
		++trackerCount;
    }

	if( trackerCount >= MAX_TRACKERS_PER_MODEL )
	{
		ERR(3, LT_OVERFLOW);
	}


	// Allocate the tracker.
	LTAnimTracker* pTracker = (LTAnimTracker*)sb_Allocate(&pModel->m_pObjectMgr->m_TrackerBank);
	if(!pTracker)
	{
		return LT_ERROR;
	}

    // Init.
    trk_Init(pTracker, pModel->GetModelDB(), 0);
    pTracker->SetModelInstance(pModel);
	pTracker->m_ID = TrackerID;

    // Add to the list.
    pTracker->SetNext(LTNULL);
    *ppLast = pTracker;

	//we need to re-evaluate the nodes since we have changed the tracker
	pModel->ResetCachedTransformNodeStates();

	//The tracker is now dirty.
	pTracker->m_bDirty = true;

    return LT_OK;
}



LTRESULT ILTModel::RemoveTracker(HOBJECT hModel, ANIMTRACKERID TrackerID)
{
	FN_NAME(ILTModel::RemoveTracker);

	if( TrackerID == MAIN_TRACKER )
	{
        ERR(3, LT_INVALIDPARAMS);
	}

	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

    LTAnimTracker **ppPrev;
	LTAnimTracker *pTracker = pModel->GetTracker(TrackerID, ppPrev);
	if( !pTracker )
	{
        ERR(3, LT_NOTFOUND);
	}
	else
    {
		// unlink
        *ppPrev = pTracker->GetNext();
		// deallocate
		sb_Free(&pModel->m_pObjectMgr->m_TrackerBank, pTracker);

		//we need to re-evaluate the nodes since we have changed the tracker
		pModel->ResetCachedTransformNodeStates();

        return LT_OK;
    }
}

// ------------------------------------------------------------------------
// GetAnimIndex( hmodel, anim-name, return-anim-index-value )
// utility function for getting the name to index association in the model
// db.
// ------------------------------------------------------------------------
LTRESULT ILTModel::GetAnimIndex( HOBJECT hModel, const char *pAnimName , uint32 & ret_anim_index )
{
	FN_NAME(ILTModel::GetAnimIndex);
	CHECK_PARAMS2(hModel);
	ret_anim_index = ic_GetAnimIndex(hModel, pAnimName);

	if( ret_anim_index == (uint32)-1)
	{
		return LT_INVALIDPARAMS;
	}

	return LT_OK ;
}

LTRESULT ILTModel::GetCurAnim(HOBJECT hModel, ANIMTRACKERID TrackerID,
                                HMODELANIM &hAnim)
{
	FN_NAME(ILTModel::GetCurAnim);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);
    hAnim = (HMODELANIM)trk_GetCurAnimIndex(pTracker);
	return LT_OK;
}

LTRESULT ILTModel::SetCurAnim(HOBJECT hModel, ANIMTRACKERID TrackerID,
                              HMODELANIM hAnim)
{
	FN_NAME(ILTModel::SetCurAnim);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

	bool bAnimationSet = false;
	if(!trk_SetCurAnim(pTracker, hAnim, true))
		return LT_NOTFOUND;

	//we need to re-evaluate the nodes since we have changed the tracker
	pModel->ResetCachedTransformNodeStates();

	//The tracker is now dirty.
	pTracker->m_bDirty = true;

	return LT_OK;
}

LTRESULT ILTModel::ResetAnim(HOBJECT hModel, ANIMTRACKERID TrackerID)
{
	FN_NAME(ILTModel::ResetAnim);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

    trk_Reset(pTracker);

	//we need to re-evaluate the nodes since we have changed the tracker
	pModel->ResetCachedTransformNodeStates();

	//The tracker is now dirty.
	pTracker->m_bDirty = true;

    return LT_OK;
}

LTRESULT ILTModel::GetLooping(HOBJECT hModel, ANIMTRACKERID TrackerID)
{
	FN_NAME(ILTModel::GetLooping);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

    if (pTracker->m_Flags & AT_LOOPING)
        return LT_YES;
    else
        return LT_NO;
}

LTRESULT ILTModel::SetLooping(HOBJECT hModel, ANIMTRACKERID TrackerID, bool bLooping)
{
	FN_NAME(ILTModel::SetLooping);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

	trk_Loop(pTracker, bLooping != 0);

	//The tracker is now dirty.
	pTracker->m_bDirty = true;

    return LT_OK;
}

LTRESULT ILTModel::GetPlaying(HOBJECT hModel, ANIMTRACKERID TrackerID)
{
	FN_NAME(ILTModel::GetPlaying);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

    if (pTracker->m_Flags & AT_PLAYING)
        return LT_YES;
    else
        return LT_NO;
}

LTRESULT ILTModel::SetPlaying(HOBJECT hModel, ANIMTRACKERID TrackerID, bool bPlaying)
{
	FN_NAME(ILTModel::SetPlaying);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

    trk_Play(pTracker, bPlaying != 0);

	//The tracker is now dirty.
	pTracker->m_bDirty = true;

    return LT_OK;
}

LTRESULT ILTModel::GetAnimLength(HOBJECT hModel, HMODELANIM hAnim,
	uint32 &length)
{
	length = 0;
	FN_NAME(ILTModel::GetCurAnimLength);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);
	CHECK_PARAMS2(hAnim < pModel->GetModelDB()->NumAnims());

    ModelAnim *pAnim = pModel->GetModelDB()->GetAnim(hAnim);
    if (!pAnim)
    {
        ERR(1, LT_NOTINITIALIZED);
    }

    length = pAnim->GetAnimTime();
    return LT_OK;
}

LTRESULT ILTModel::GetCurAnimLength(HOBJECT hModel, ANIMTRACKERID TrackerID,
	uint32 &length)
{
	length = 0;
	FN_NAME(ILTModel::GetCurAnimLength);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
	CHECK_PARAMS2(pTracker);

    ModelAnim *pAnim = pTracker->GetCurAnim();
    if (!pAnim)
    {
        ERR(1, LT_NOTINITIALIZED);
    }

    length = pAnim->GetAnimTime();
    return LT_OK;
}

LTRESULT ILTModel::GetCurAnimTime(HOBJECT hModel, ANIMTRACKERID TrackerID, 
		uint32 &curTime)
{
	curTime = 0;
	FN_NAME(ILTModel::GetCurAnimTime);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

    curTime = pTracker->m_TimeRef.m_Cur.m_Time;
	return LT_OK;
}

LTRESULT ILTModel::SetCurAnimTime(
                   HOBJECT hModel, ANIMTRACKERID TrackerID, 
	               uint32 curTime)
{
	FN_NAME(ILTModel::SetCurAnimTime);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

    trk_SetCurTime(pTracker, curTime, false);

	//we need to re-evaluate the nodes since we have changed the tracker
	pModel->ResetCachedTransformNodeStates();

	//The tracker is now dirty.
	pTracker->m_bDirty = true;

	return LT_OK;
}

// Set Movement Encoding transform hint node.
LTRESULT ILTModel::SetHintNode(
                   HOBJECT hModel, ANIMTRACKERID TrackerID, 
			       HMODELNODE hNode)
{
	FN_NAME(ILTModel::SetCurAnimTime);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

    pTracker->m_hHintNode = hNode;

	//The tracker is now dirty.
	pTracker->m_bDirty = true;

	return LT_OK;
}


LTRESULT ILTModel::SetAnimRate( HOBJECT hModel, ANIMTRACKERID TrackerID, float fRate )
{
	FN_NAME(ILTModel::SetAnimRate);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

	pTracker->m_RateModifier = fRate;

	//The tracker is now dirty.
	pTracker->m_bDirty = true;

    return LT_OK;
}

LTRESULT ILTModel::GetAnimRate( HOBJECT hModel, ANIMTRACKERID TrackerID, float &fRate )
{
	fRate = 0.0f;

	FN_NAME(ILTModel::GetAnimRate);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);
   
	fRate = pTracker->m_RateModifier;
    return LT_OK;
}

LTRESULT ILTModel::GetWeightSet(
                   HOBJECT hModel, ANIMTRACKERID TrackerID,
                   HMODELWEIGHTSET &hSet)
{
	FN_NAME(ILTModel::GetWeightSet);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

    hSet = pTracker->m_TimeRef.m_iWeightSet;
    return LT_OK;
}

LTRESULT ILTModel::SetWeightSet(HOBJECT hModel, ANIMTRACKERID TrackerID, 
                                  HMODELWEIGHTSET hSet)
{
	FN_NAME(ILTModel::SetWeightSet);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

	LTAnimTracker* pTracker = pModel->GetTracker(TrackerID);
    CHECK_PARAMS2(pTracker);

	pTracker->m_TimeRef.m_iWeightSet = hSet;

	//we need to re-evaluate the nodes since we have changed the tracker
	pModel->ResetCachedTransformNodeStates();

	//The tracker is now dirty.
	pTracker->m_bDirty = true;

	return LT_OK;
}

LTRESULT ILTModel::GetNumLODs( HOBJECT hModel, HMODELPIECE hPiece, uint32 & num_lods)
{
    FN_NAME(ILTModel::GetNumLODs);
    
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

    if(!pModel->GetNumLOD(hPiece, num_lods))
        return LT_INVALIDPARAMS;// bad hpiece index val.
    else return LT_OK;
}

LTRESULT ILTModel::GetLODValFromDist( HOBJECT hModel, HMODELPIECE hPiece, float dist , uint32 & lod_index )
{
    FN_NAME(ILTModel::GetLODValFromDist);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModel = hModel->ToModel();
	CHECK_PARAMS2(pModel);

    if(!pModel->GetLODValFromDist( hPiece, dist, lod_index ))
    {
        return LT_INVALIDPARAMS;
    }
       return LT_OK;   
}


// Model OBB stuff 
LTRESULT ILTModel::GetNumModelOBBs( HOBJECT hObj, uint32 & num_obbs ) 
{
	FN_NAME(ILTModel::GetNumModelOBBs);
    LTRESULT dResult = LT_OK ;
    ModelInstance *pInst;

    pInst = hObj->ToModel();
    CHECK_PARAMS2(pInst );
#if(MODEL_OBB)
	num_obbs = pInst->NumCollisionObjects ();

    return dResult;
#endif
return LT_ERROR ;
}
	
LTRESULT ILTModel::GetModelOBBCopy( HOBJECT hObj, ModelOBB *model_obbs  )
{
	FN_NAME(ILTModel::GetModelOBBCopy);
    LTRESULT dResult = LT_OK ;

#if(MODEL_OBB)
	 ModelInstance *pInst;
    pInst = hObj->ToModel();
    CHECK_PARAMS2(pInst );

	pInst->GetCollisionObjects( model_obbs );
	return dResult ;
#endif
	return LT_ERROR;
	
}

LTRESULT ILTModel::UpdateModelOBB( HOBJECT hObj, ModelOBB *model_obbs ) 
{
	FN_NAME(ILTModel::GetModelOBBCopy);
    LTRESULT dResult = LT_OK ;


#if(MODEL_OBB)
	ModelInstance *pInst;
    pInst = hObj->ToModel();
    CHECK_PARAMS2(pInst );

	pInst->UpdateCollisionObjects( model_obbs );
	return dResult ;
#endif
	return LT_ERROR ;
}













