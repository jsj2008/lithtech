// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "ServerUtilities.h"
#include "AINodeMgr.h"
#include "VolumeBrushTypes.h"
#include "SurfaceFunctions.h"
#include "ctype.h"
#include "SFXMsgIDs.h"
#include "NodeLine.h"
#include "AIUtils.h"

// Globals

CAINodeMgr* g_pAINodeMgr = LTNULL;

// Statics

static CBankedList<CAINode> s_bankCAINode;

// Externs

extern int g_cIntersectSegmentCalls;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::CAINodeMgr
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAINodeMgr::CAINodeMgr()
{
	g_pAINodeMgr = this;

    m_bInitialized = LTFALSE;
    m_apNode = LTNULL;
	m_cNodes = 0;
	m_dwNextID = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Term
//
//	PURPOSE:	Terminates the AINodeMgr
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Term()
{
    RemoveNodeDebug(LTFALSE);

	if ( m_apNode )
	{
        for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
		{
			_ASSERT(m_apNode[iNode]);

			s_bankCAINode.Delete(m_apNode[iNode]);
            m_apNode[iNode] = LTNULL;
		}

		debug_deletea(m_apNode);
	}

    m_apNode = LTNULL;
	m_cNodes = 0;
	m_dwNextID = 0;
    m_bInitialized = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Init
//
//	PURPOSE:	Create a list of all the Nodes in the level.
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Init()
{
	Term();

	// First, we count up the number of Nodes in the level

    HCLASS  hAINode = g_pLTServer->GetClass("AINode");
    HOBJECT hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAINode))
		{
			m_cNodes++;
		}
	}

    hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAINode))
		{
			m_cNodes++;
		}
	}

	// Now we allocate our array of nodes

	m_apNode = debug_newa(CAINode*, m_cNodes);

	// Now we put the nodes into our array

    hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAINode))
		{
			CAINode* pNode = s_bankCAINode.New();

			// Generate an ID

            uint32 dwID = m_dwNextID++;

			// Setup the node

            pNode->Init(dwID, *(AINode*)g_pLTServer->HandleToObject(hCurObject));

			// Add the node to our internal list

			m_apNode[dwID] = pNode;

			// Remove the object

            g_pLTServer->RemoveObject(hCurObject);
		}
	}

    hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAINode))
		{
			CAINode* pNode = s_bankCAINode.New();

			// Generate an ID

            uint32 dwID = m_dwNextID++;

			// Setup the node

            pNode->Init(dwID, *(AINode*)g_pLTServer->HandleToObject(hCurObject));

			// Add the node to our internal list

			m_apNode[dwID] = pNode;

			// Remove the object

            g_pLTServer->RemoveObject(hCurObject);
		}
	}

#ifndef _FINAL
    g_pLTServer->CPrint("Added %d nodes", m_cNodes);
#endif

    m_bInitialized = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Verify
//
//	PURPOSE:	Verifies all our nodes
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Verify()
{
	if ( m_apNode )
	{
        for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
		{
			if ( m_apNode[iNode] )
			{
				m_apNode[iNode]->Verify();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Load
//
//	PURPOSE:	Restores the state of the AINodeMgr
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Load(HMESSAGEREAD hRead)
{
	Term();

	LOAD_DWORD(m_dwNextID);
	LOAD_DWORD(m_cNodes);
	LOAD_BOOL(m_bInitialized);

	m_apNode = debug_newa(CAINode*, m_cNodes);

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		m_apNode[iNode] = s_bankCAINode.New();
		m_apNode[iNode]->Load(hRead);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Save
//
//	PURPOSE:	Saves the state of the AINodeMgr
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Save(HMESSAGEWRITE hWrite)
{
	SAVE_DWORD(m_dwNextID);
	SAVE_DWORD(m_cNodes);
	SAVE_BOOL(m_bInitialized);

	if ( m_apNode )
	{
        for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
		{
			// We should never have a null pointer in this list

			_ASSERT(m_apNode[iNode]);

			m_apNode[iNode]->Save(hWrite);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::AddNodeDebug
//
//	PURPOSE:	Add the node models
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::AddNodeDebug()
{
    if (!g_pLTServer) return;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( m_apNode[iNode] )
		{
			ObjectCreateStruct theStruct;
			INIT_OBJECTCREATESTRUCT(theStruct);

			VEC_COPY(theStruct.m_Pos, m_apNode[iNode]->GetPos());
			SAFE_STRCPY(theStruct.m_Filename, "Models\\1x1_square.abc");

			theStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
			theStruct.m_ObjectType = OT_MODEL;

            HCLASS hClass = g_pLTServer->GetClass("BaseClass");
            LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
			if (!pModel) return;

            LTVector vScale;
			VEC_SET(vScale, 20.0f, 20.0f, 20.0f);
            g_pLTServer->ScaleObject(pModel->m_hObject, &vScale);

			if ( m_apNode[iNode]->IsCover() )
			{
                g_pLTServer->SetObjectColor(pModel->m_hObject, .75f, 0.0f, 0.0f, 1.0f);
			}
			else if ( m_apNode[iNode]->IsPanic() )
			{
                g_pLTServer->SetObjectColor(pModel->m_hObject, 0.0f, .75f, 0.0f, 1.0f);
			}
			else if ( m_apNode[iNode]->IsVantage() )
			{
                g_pLTServer->SetObjectColor(pModel->m_hObject, 0.0f, 0.0f, .75f, 1.0f);
			}
			else if ( m_apNode[iNode]->IsSearchable() )
			{
                g_pLTServer->SetObjectColor(pModel->m_hObject, 0.0f, 0.75f, .75f, 1.0f);
			}
			else
			{
                g_pLTServer->SetObjectColor(pModel->m_hObject, 1.0f, 1.0f, 1.0f, 1.0f);
			}

			m_listNodeModels.AddTail(pModel);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::RemoveNodeDebug
//
//	PURPOSE:	Removes the node models
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::RemoveNodeDebug(LTBOOL bRemoveObjects /*= LTFALSE */)
{
    if (!g_pLTServer) return;

	BaseClass** pClass= m_listNodeModels.GetItem(TLIT_FIRST);
	while ( pClass && *pClass )
	{
		if ( bRemoveObjects )
		{
            g_pLTServer->RemoveObject((*pClass)->m_hObject);
		}

		pClass = m_listNodeModels.GetItem(TLIT_NEXT);
	}

	m_listNodeModels.Clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindTailNode
//
//	PURPOSE:	Determines the tail node that one should be at
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindTailNode(const LTVector& vTargetPos, const LTVector& vPos, uint32* adwTailNodes, int cTailNodes)
{
    LTFLOAT fMinDistance = (float)INT_MAX;
	int iTailedNode = CAINode::kInvalidNodeID;

	// Find the node closest to the tailed object

	{for ( int iTailNode = 0 ; iTailNode < cTailNodes ; iTailNode++ )
	{
        LTFLOAT fDistance = VEC_DISTSQR(vTargetPos, m_apNode[adwTailNodes[iTailNode]]->GetPos());
		if ( fDistance < fMinDistance )
		{
			iTailedNode = iTailNode;
			fMinDistance = fDistance;
		}
	}}

	fMinDistance = (float)INT_MAX;
	int iTailerNode = CAINode::kInvalidNodeID;

	// Find the node closest to the tailer

	{for ( int iTailNode = 0 ; iTailNode < cTailNodes ; iTailNode++ )
	{
        LTFLOAT fDistance = VEC_DISTSQR(vPos, m_apNode[adwTailNodes[iTailNode]]->GetPos());
		if ( fDistance < fMinDistance )
		{
			iTailerNode = iTailNode;
			fMinDistance = fDistance;
		}
	}}

	// Figure out what the tail node is based on these two nodes

	int iTailNode = CAINode::kInvalidNodeID;

	// If the tailer is less than the tailed node, the tail is the tailed node minus 1

	if ( iTailerNode < iTailedNode )
	{
		iTailNode = Max<int>(0, iTailedNode-1);
	}

	// If the tailer is greater than the tailed node, the tail is the tailed node plus 1

	if ( iTailerNode > iTailedNode )
	{
		iTailNode = Min<int>(cTailNodes-1, iTailedNode+1);
	}

	// If the tail node is equal to the tailednode, then there is no good node to go to.

	if ( iTailerNode == iTailedNode )
	{
		return NULL;
	}
	else
	{
		return m_apNode[adwTailNodes[iTailNode]];
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestNode
//
//	PURPOSE:	Finds the nearest node to vPos
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestNode(const LTVector& vPos)
{
    LTFLOAT  fMinDistance = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
        LTFLOAT  fDistance = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

		if ( fDistance < fMinDistance )
		{
			fMinDistance = fDistance;
			pClosestNode = m_apNode[iNode];
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestPoodleNode
//
//	PURPOSE:	Finds the nearest poodle node to vPos
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestPoodleNode(const LTVector& vPos)
{
    LTFLOAT  fMinDistance = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( m_apNode[iNode]->IsPoodle() )
		{
            LTFLOAT  fDistance = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;
				pClosestNode = m_apNode[iNode];
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestCover
//
//	PURPOSE:	Finds the nearest cover node to vPos
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestCover(const LTVector& vPos)
{
    LTFLOAT  fMinDistanceSqr = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( !m_apNode[iNode]->IsLocked() && m_apNode[iNode]->IsCover() )
		{
            LTFLOAT fDistanceSqr = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( (fDistanceSqr < fMinDistanceSqr) && (fDistanceSqr < m_apNode[iNode]->GetCoverRadiusSqr()) )
			{
				fMinDistanceSqr = fDistanceSqr;
				pClosestNode = m_apNode[iNode];
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestCoverFromThreat
//
//	PURPOSE:	Finds the nearest cover node that protects us from hThreat
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestCoverFromThreat(const LTVector& vPos, HOBJECT hThreat)
{
    LTFLOAT  fMinDistanceSqr = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( !m_apNode[iNode]->IsLocked() && m_apNode[iNode]->IsCover() )
		{
            LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( (fDistanceSqr < fMinDistanceSqr) && (fDistanceSqr < m_apNode[iNode]->GetCoverRadiusSqr()) )
			{
				if ( eCoverStatusOk == m_apNode[iNode]->GetCoverStatus(vPos, hThreat) )
				{
					fMinDistanceSqr = fDistanceSqr;
					pClosestNode = m_apNode[iNode];
				}
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestPanicFromThreat
//
//	PURPOSE:	Finds the nearest Panic node that protects us from hThreat
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestPanicFromThreat(const LTVector& vPos, HOBJECT hThreat)
{
    LTFLOAT  fMinDistanceSqr = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( !m_apNode[iNode]->IsLocked() && m_apNode[iNode]->IsPanic() )
		{
            LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( (fDistanceSqr < fMinDistanceSqr) && (fDistanceSqr < m_apNode[iNode]->GetPanicRadiusSqr()) )
			{
				if ( m_apNode[iNode]->IsPanicFromThreat(vPos, hThreat) )
				{
					fMinDistanceSqr = fDistanceSqr;
					pClosestNode = m_apNode[iNode];
				}
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestPanic
//
//	PURPOSE:	Finds the nearest panic node to vPos
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestPanic(const LTVector& vPos)
{
    LTFLOAT  fMinDistanceSqr = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( !m_apNode[iNode]->IsLocked() && m_apNode[iNode]->IsPanic() )
		{
            LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( (fDistanceSqr < fMinDistanceSqr) && (fDistanceSqr < m_apNode[iNode]->GetPanicRadiusSqr()) )
			{
				fMinDistanceSqr = fDistanceSqr;
				pClosestNode = m_apNode[iNode];
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestVantage
//
//	PURPOSE:	Finds the nearest Vantage node to vPos
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestVantage(const LTVector& vPos)
{
    LTFLOAT  fMinDistance = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( !m_apNode[iNode]->IsLocked() && m_apNode[iNode]->IsVantage() )
		{
            LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( (fDistanceSqr < fMinDistance) && (fDistanceSqr < m_apNode[iNode]->GetVantageRadiusSqr()) )
			{
				fMinDistance = fDistanceSqr;
				pClosestNode = m_apNode[iNode];
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestVantageToThreat
//
//	PURPOSE:	Finds the nearest vantage node that lets us see the hThreat
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestVantageToThreat(const LTVector& vPos, HOBJECT hThreat)
{
    LTFLOAT  fMinDistance = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( !m_apNode[iNode]->IsLocked() && m_apNode[iNode]->IsVantage() )
		{
            LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( (fDistanceSqr < fMinDistance) && (fDistanceSqr < m_apNode[iNode]->GetVantageRadiusSqr()) )
			{
				if ( eVantageStatusOk == m_apNode[iNode]->GetVantageStatus(vPos, hThreat) )
				{
					fMinDistance = fDistanceSqr;
					pClosestNode = m_apNode[iNode];
				}
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestUseObject
//
//	PURPOSE:	Finds the nearest use object node to pos
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestUseObject(const LTVector& vPos, const char* szClass)
{
    LTFLOAT  fMinDistanceSqr = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( !m_apNode[iNode]->IsLocked() && m_apNode[iNode]->HasUseObject() )
		{
            LTFLOAT fDistanceSqr = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( (fDistanceSqr < fMinDistanceSqr) && (fDistanceSqr < m_apNode[iNode]->GetUseObjectRadiusSqr()) )
			{
				HOBJECT hObject;
				if ( LT_OK == FindNamedObject(m_apNode[iNode]->GetUseObject(), hObject) )
				{
                    HCLASS hClass = g_pLTServer->GetClass((char*)szClass);

                    if ( g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObject), hClass) )
					{
						fMinDistanceSqr = fDistanceSqr;
						pClosestNode = m_apNode[iNode];
					}
				}
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestPickupObject
//
//	PURPOSE:	Finds the nearest Pickup object node to pos
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestPickupObject(const LTVector& vPos, const char* szClass)
{
    LTFLOAT  fMinDistanceSqr = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( !m_apNode[iNode]->IsLocked() && m_apNode[iNode]->HasPickupObject() )
		{
            LTFLOAT fDistanceSqr = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( (fDistanceSqr < fMinDistanceSqr) && (fDistanceSqr < m_apNode[iNode]->GetPickupObjectRadiusSqr()) )
			{
				HOBJECT hObject;
				if ( LT_OK == FindNamedObject(m_apNode[iNode]->GetPickupObject(), hObject) )
				{
                    HCLASS hClass = g_pLTServer->GetClass((char*)szClass);

                    if ( g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObject), hClass) )
					{
						fMinDistanceSqr = fDistanceSqr;
						pClosestNode = m_apNode[iNode];
					}
				}
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestBackup
//
//	PURPOSE:	Finds the nearest Backup node to vPos
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestBackup(const LTVector& vPos)
{
    LTFLOAT  fMinDistanceSqr = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( !m_apNode[iNode]->IsLocked() && m_apNode[iNode]->HasBackupCmd() )
		{
            LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( (fDistanceSqr < fMinDistanceSqr) && (fDistanceSqr < m_apNode[iNode]->GetBackupRadiusSqr()) )
			{
				fMinDistanceSqr = fDistanceSqr;
				pClosestNode = m_apNode[iNode];
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestTrainingFailure
//
//	PURPOSE:	Finds the nearest Training Failure node to vPos
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::FindNearestTrainingFailure(const LTVector& vPos)
{
    LTFLOAT  fMinDistanceSqr = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		if ( !m_apNode[iNode]->IsLocked() && m_apNode[iNode]->HasTrainingFailureCmd() )
		{
            LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, m_apNode[iNode]->GetPos());

			if ( fDistanceSqr < fMinDistanceSqr )
			{
				fMinDistanceSqr = fDistanceSqr;
				pClosestNode = m_apNode[iNode];
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::UnlockNode
//
//	PURPOSE:	Unlocks the node for public use
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::UnlockNode(uint32 dwNode)
{
	if ( !IsInitialized() || (dwNode >= GetNumNodes()) || (dwNode < 0) ) return;
	_ASSERT( m_apNode[dwNode]->IsLocked() );

	m_apNode[dwNode]->Unlock();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::LockNode
//
//	PURPOSE:	Claims the node for exclusive use
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::LockNode(uint32 dwNode)
{
	if ( !IsInitialized() || (dwNode >= GetNumNodes()) || (dwNode < 0) ) return;
	_ASSERT( !m_apNode[dwNode]->IsLocked() );

	m_apNode[dwNode]->Lock();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::GetNode
//
//	PURPOSE:	Finds a node based on its name
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::GetNode(uint32 iNode)
{
	if ( !IsInitialized() || (iNode >= GetNumNodes()) || (iNode < 0) ) return LTNULL;

	return m_apNode[iNode];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::GetNode
//
//	PURPOSE:	Finds a node based on its name
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::GetNode(HSTRING hstrName)
{
    if ( !g_pLTServer ) return LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
        if ( g_pLTServer->CompareStringsUpper(m_apNode[iNode]->GetName(), hstrName) )
		{
			return m_apNode[iNode];
		}
	}

    return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::GetNode
//
//	PURPOSE:	Finds a node based on its name
//
// ----------------------------------------------------------------------- //

CAINode* CAINodeMgr::GetNode(const char *szName)
{
    if ( !g_pLTServer ) return LTNULL;

    for ( uint32 iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
        const char* szNodeName = g_pLTServer->GetStringData(m_apNode[iNode]->GetName());

		if ( !_stricmp(szNodeName, szName) )
		{
			return m_apNode[iNode];
		}
	}

    return LTNULL;
}