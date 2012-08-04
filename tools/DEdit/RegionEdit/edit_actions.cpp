//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#include "bdefs.h"
#include "undo_mgr.h"
#include "regiondoc.h"
#include "edit_actions.h"
#include "node_ops.h"
#include "edithelpers.h"
#include "nodeview.h"

// MIKE 12/29/97 - Removed all the ACTION_MODIFYALL / FullBackup_ stuff.


// ------------------------------------------------------------------ //
// Specialized action classes.
// ------------------------------------------------------------------ //

class CNodeReplacement : public CEditAction
{
	public:

					CNodeReplacement()
					{
						m_pNode = NULL;
						m_ParentID = INVALID_NODE_ID;
						m_ActionType = EDITACTION_REPLACENODE;
					}

					~CNodeReplacement()
					{
						if(m_pNode)
							delete m_pNode;
					}

		CWorldNode	*m_pNode;
		DWORD		m_NodeID; // Stored here because m_pNode can be NULL.
		DWORD		m_ParentID;
		CMoDWordArray	m_ChildrenIDs;

};



// ------------------------------------------------------------------ //
// Helpers.
// ------------------------------------------------------------------ //

static void BackupWorldNode(CWorldNode *pInNode, CNodeReplacement *pRet)
{
	GPOS pos;
	CWorldNode *pChild;
	DWORD i;

	pRet->m_pNode = pInNode->AllocateSameKind();
	pRet->m_pNode->DoCopy(pInNode);
	pRet->m_pNode->ForceUniqueID(pInNode);

	pRet->m_ParentID = pInNode->GetParent()->GetUniqueID();
	pRet->m_ChildrenIDs.SetSize(pInNode->m_Children);
	
	i=0;
	for(pos=pInNode->m_Children; pos; )
	{
		pChild = pInNode->m_Children.GetNext(pos);
		pRet->m_ChildrenIDs[i] = pChild->GetUniqueID();
		i++;
	}
}


void AddToActionListIfNew(PreActionList *pList, CPreAction *pAction, BOOL bDeleteIfNot)
{
	GPOS pos;
	CPreAction *pTestAction;

	for(pos=pList->GetHeadPosition(); pos; )
	{
		pTestAction = pList->GetNext(pos);

		if(pTestAction->m_ActionType == pAction->m_ActionType && 
			pTestAction->m_pNode == pAction->m_pNode)
		{
			if(bDeleteIfNot)
				delete pAction;

			return;
		}
	}

	pList->AddTail(pAction);
}



// ------------------------------------------------------------------ //
// Action functions.
// ------------------------------------------------------------------ //

CEditAction* ModifyNode_CreatePreReverse(CUndoMgr *pMgr, CPreAction *pPreAction)
{
	CNodeReplacement *pRet;
	CWorldNode *pInNode;

	ASSERT(pPreAction->m_pNode);
	if(pPreAction->m_pNode->m_Flags & NODEFLAG_WORLDROOT)
	{
		// Don't backup the root.
		return NULL;
	}
	else
	{
		pRet = new CNodeReplacement;
		pInNode = pPreAction->m_pNode;

		// Backup the node and remember the IDs of its parent and children.
		pRet->m_NodeID = pInNode->GetUniqueID();
		BackupWorldNode(pInNode, pRet);
		return pRet;
	}
}


CEditAction* ReplaceNode_CreateEditReverse(CUndoMgr *pMgr, CEditAction *pInputAction)
{
	CNodeReplacement *pAction;
	CNodeReplacement *pRet;
	CWorldNode *pWorldNode;
	DWORD i;


	pRet = new CNodeReplacement;
	pAction = (CNodeReplacement*)pInputAction;

	pRet->m_NodeID = pAction->m_NodeID;

	// Backup the node and remember the IDs of its parent and children.
	// It may not exist, in which case this action will just remove a  pAction->m_NodeID
	// when it gets executed without replacing it.
	pWorldNode = pMgr->m_pDoc->m_Region.FindNodeByID(pAction->m_NodeID);
	if(pWorldNode)
	{
		BackupWorldNode(pWorldNode, pRet);
	}

	return pRet;
}


void ReplaceNode_Execute(CUndoMgr *pMgr, CEditAction *pInputAction)
{
	CWorldNode *pNode, *pOldNode, *pParent, *pChild;
	CNodeReplacement *pAction;
	DWORD i;
	CEditRegion *pRegion;


	pRegion = &pMgr->m_pDoc->m_Region;
	pAction = (CNodeReplacement*)pInputAction;
	pNode = pAction->m_pNode;


	ASSERT(pRegion->CheckNodes() == NULL);

	// Get rid of the node currently there.
	pOldNode = pRegion->FindNodeByID(pAction->m_NodeID);
	if(pOldNode)
	{
		no_DestroyNode(pRegion, pOldNode, FALSE);
	}


	ASSERT(pRegion->CheckNodes() == NULL);

	if(pNode)
	{
		// Initialize the replacement node.
		no_InitializeNewNode(pRegion, pNode, NULL);

		ASSERT(pRegion->CheckNodes() == NULL);

		// Parent/child relationship.
		pParent = pRegion->FindNodeByID(pAction->m_ParentID);
		//ASSERT(pParent);
		no_AttachNode(pRegion, pAction->m_pNode, pParent);

		ASSERT(pRegion->CheckNodes() == NULL);

		for(i=0; i < pAction->m_ChildrenIDs; i++)
		{
			pChild = pRegion->FindNodeByID(pAction->m_ChildrenIDs[i]);
			//ASSERT(pChild);
			if (pChild)
			{
				no_AttachNode(pRegion, pChild, pNode);
			}
		}

		// If we have children involved, we need to make sure they have proper tree items -DC
		if (pAction->m_ChildrenIDs > 0) 
		{
			GetNodeView()->AddItemsToTree(pNode);
		}

		
		ASSERT(pRegion->CheckNodes() == NULL);

		// Special stuff if we're a brush.
		if(pNode->GetType() == Node_Brush)
		{
			for(i=0; i < pNode->AsBrush()->m_Polies; i++)
			{
				for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
				{
					pNode->AsBrush()->m_Polies[i]->GetTexture(nCurrTex).UpdateTextureID();
				}
			}
		}

		ASSERT(pRegion->CheckNodes() == NULL);

		// Clear the action's node since we moved it into the world.
		pAction->m_pNode = NULL;
	}

	ASSERT(pRegion->CheckNodes() == NULL);
}


CEditAction* AddedNode_CreatePreReverse(CUndoMgr *pMgr, CPreAction *pPreAction)
{
	CNodeReplacement *pRet;
	CWorldNode *pInNode;

	ASSERT(pPreAction->m_pNode);

	// Leave m_pNode NULL since all we want it to do is remove the node when it's executed.
	pRet = new CNodeReplacement;
	pRet->m_NodeID = pPreAction->m_pNode->GetUniqueID();

	return pRet;
}





