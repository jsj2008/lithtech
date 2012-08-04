//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#include "bdefs.h"
#include "editregion.h"
#include "node_ops.h"


BOOL g_bCheckNodes = TRUE;

#ifdef DIRECTEDITOR_BUILD
	#include "regiondoc.h"
	#include "edithelpers.h"
	#include "nodeview.h"
#endif



// LandExport takes FOREVER while debugging unless we remove the asserts.
#ifdef NODEOPS_NO_ASSERT
	#define NODEOPS_CHECK(x)
#else
	#define NODEOPS_CHECK(x) ASSERT(x)
#endif


void SetupBrushProperties(CEditBrush *pBrush, CEditRegion *pRegion)
{
	CBaseProp	*pProp;

	SetupWorldNode("Brush", pBrush->AsNode(), pRegion);

	pProp = pBrush->m_PropList.GetProp("BrushType");
	if(pProp && (pProp->m_Type == LT_PT_LONGINT))
		((CRealProp*)pProp)->m_Value = 129.0f;

	pProp = pBrush->m_PropList.GetProp("FrictionCoefficient");
	if(pProp && (pProp->m_Type == LT_PT_REAL))
		((CRealProp*)pProp)->m_Value = 1.0f;
}


void no_DetachNode(CEditRegion *pRegion, CWorldNode *pNode, BOOL bAttachToRoot)
{
	if(pNode->GetParent())
	{
		pNode->GetParent()->m_Children.RemoveAt(pNode);
	}

	if(bAttachToRoot)
	{
		pNode->SetParent(&pRegion->m_RootNode);
		
		#ifdef DIRECTEDITOR_BUILD
			GetNodeView()->SetNode(pNode);
		#endif
		
		pRegion->m_RootNode.m_Children.Append(pNode);
	}
	else
	{
		pNode->SetParent(NULL);
	}
}


// Attaches the node to the parent.
void no_AttachNode(CEditRegion *pRegion, CWorldNode *pChild, CWorldNode *pParent)
{
	if(!pParent)
		pParent = &pRegion->m_RootNode;

	no_DetachNode(pRegion, pChild, FALSE);
	pChild->SetParent(pParent);

	NODEOPS_CHECK(pParent->m_Children.FindElement(pChild) == BAD_INDEX);
	
	// Really slows down LandExport in debug so LandExport turns this option off.
	if(g_bCheckNodes)
	{
		NODEOPS_CHECK(pRegion->CheckNodes(pParent) == NULL);
	}

	pParent->m_Children.Append(pChild);

	if(g_bCheckNodes)
	{
		NODEOPS_CHECK(pRegion->CheckNodes(pParent) == NULL);
	}

#ifdef DIRECTEDITOR_BUILD
	// Set the node in the node view if we are working on the active region
	if (pRegion == GetActiveRegionDoc()->GetRegion())
	{
		GetNodeView( )->SetNode(pChild);
	}
#endif
}


void no_InitializeNewNode(CEditRegion *pRegion, CWorldNode *pNode, CWorldNode *pParent)
{
	if(pNode->GetType() == Node_Brush)
	{
		pRegion->AddBrush(pNode->AsBrush());
	}
	else if(pNode->GetType() == Node_Object)
	{
		pRegion->AddObject(pNode->AsObject());
	}

	// Set it up in the node view.
	no_AttachNode(pRegion, pNode, pParent);

	// Select it if it was selected.
	if(pNode->IsFlagSet(NODEFLAG_SELECTED))
		pRegion->SelectNode(pNode);
}


CEditBrush* no_CreateNewBrush(CEditRegion *pRegion, CWorldNode *pParent)
{
	CEditBrush *pBrush;
	
	pBrush = new CEditBrush;
	SetupBrushProperties(pBrush, pRegion);
	no_InitializeNewNode(pRegion, pBrush, pParent);
	return pBrush;
}


void no_DestroyNode(CEditRegion *pRegion, CWorldNode *pNode, BOOL bDeleteChildren)
{
	GPOS pos;

	NODEOPS_CHECK(pRegion->CheckNodes() == NULL);

	#ifdef DIRECTEDITOR_BUILD
		GetNodeView()->RemoveFromTree(pNode);
		pNode->RemoveFromTree();
		GetNodeView()->DeleteNode(pNode);
	#endif

	if(pNode->IsFlagSet(NODEFLAG_SELECTED))
		pRegion->UnselectNode(pNode);

	pRegion->RemoveNodeFromPath(pNode);

	// Either delete or reattach the children.
	if(bDeleteChildren)
	{
		for(pos=pNode->m_Children; pos; )
		{
			no_DestroyNode(pRegion, pNode->m_Children.GetNext(pos), TRUE);
		}
	}
	else
	{
		// Just attach the children to the parent node.
		for(pos=pNode->m_Children; pos; )
		{
			no_AttachNode(pRegion, pNode->m_Children.GetNext(pos), pNode->GetParent());
		}
	}
	
	NODEOPS_CHECK(pNode->m_Children.GetSize() == 0);

	// Detach from its parent.
	no_DetachNode(pRegion, pNode, FALSE);

	NODEOPS_CHECK(pRegion->CheckNodes() == NULL);

	// Remove it from a CEditRegion list if it's in one.	
	if(pNode->GetType() == Node_Brush)
	{
		pRegion->RemoveBrush(pNode->AsBrush());
	}
	else if(pNode->GetType() == Node_Object)
	{
		pRegion->RemoveObject(pNode->AsObject());
	}

	NODEOPS_CHECK(pRegion->CheckNodes() == NULL);
	delete pNode;
	NODEOPS_CHECK(pRegion->CheckNodes() == NULL);
}



