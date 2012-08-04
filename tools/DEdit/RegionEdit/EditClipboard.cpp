//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditClipboard.cpp
//
//	PURPOSE	  : Implements the CEditClipboard class.
//
//	CREATED	  : October 3 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "editclipboard.h"
#include "edithelpers.h"
#include "nodeview.h"
#include "projectbar.h"
#include "optionsclipboard.h"
#include "regionview.h"

CEditClipboard::CEditClipboard()
{
	m_Nodes.SetCacheSize(50);
}

CEditClipboard::~CEditClipboard()
{
	Term();
}


void CEditClipboard::Term()
{
	DWORD		i;

	CWorldNode::FindParentNodes( m_Nodes );

	for( i=0; i < m_Nodes; i++ )
	{
		DeleteLocalNode( m_Nodes[i] );
	}

	m_Nodes.SetSize(0);	
}

//-------------------------------------------------------------------------------
//
//  CEditClipboard::DeleteLocalNode()
//
//  Purpose:	Deletes local node.
//
//-------------------------------------------------------------------------------
void CEditClipboard::DeleteLocalNode( CWorldNode *pNode )
{
	if( pNode->GetParent( ))
	{
		pNode->GetParent( )->RemoveChild( pNode );
	}

	// Handle deleting brush child...
// Brushes and Objects are WorldNodes themselves now so no special puff here.
/*
	if( pNode->m_Type == Node_Brush )
	{
		index = 
		ASSERT( pNode->m_pBrush );
		pNode->m_pBrush->SetNode( NULL );
		delete pNode->m_pBrush;
		pNode->m_pBrush = NULL;
	}
	
	// Handle deleting object child...
	else if( pNode->m_Type == Node_Object )
	{
		ASSERT( pNode->m_pEditObject );
		pNode->m_pEditObject->SetNode( NULL );
		delete pNode->m_pEditObject;
		pNode->m_pEditObject = NULL;
	}
*/
	delete pNode;
}


void CEditClipboard::Copy( CEditRegion *pRegion )
{
	DWORD			i;
	CWorldNode		*pNode;


	Term();


	for( i=0; i < pRegion->m_Selections; i++ )
	{
		// Add the node to the array
		pNode = pRegion->m_Selections[i]->AllocateSameKind();
		m_Nodes.Append( pNode );

		ClipboardDuplicate( pRegion->m_Selections[i], pNode, NULL );
	}

	CopyParenthood( pRegion->m_Selections, m_Nodes );
}

/************************************************************************/
// Paste the clipboards nodes into the world
void CEditClipboard::Paste(CEditRegion *pRegion, PreActionList &undoList, BOOL bPasteAtMarker)
{	
	// Past the clipboard nodes
	PasteNodes(pRegion, m_Nodes, undoList, bPasteAtMarker);
}

/************************************************************************/
// This will paste an array of source nodes into the edit region
void CEditClipboard::PasteNodes(CEditRegion *pRegion, CMoArray<CWorldNode*> &sourceArray, PreActionList &undoList, BOOL bPasteAtMarker)
{	
	pRegion->ClearSelections();

	if (sourceArray.GetSize() <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	// Turn off the drawing of the node view
	if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
	{
		GetNodeView()->SetRedraw(FALSE);
	}

	// Set the wait cursor
	GetNodeView()->BeginWaitCursor();
	
	// Get the active parent node
	CWorldNode *pActiveParent=pRegion->GetActiveParentNode();

	// Calculate the upper left position of all of the bounding box enclosing all
	// of the nodes as they would appear in DEdit.	
	CVector vBoundingPos;
	vBoundingPos.x=0.0f;
	vBoundingPos.y=0.0f;
	vBoundingPos.z=0.0f;

	if (bPasteAtMarker)
	{
		// Set the initial bounding position
		int i;
		for( i=0; i < sourceArray.GetSize(); i++ )
		{
			if (sourceArray[i]->GetType() != Node_Null)
			{
				vBoundingPos=sourceArray[i]->GetUpperLeftCornerPos();
				break;
			}
		}
		
		// Find the upper left bounding position		
		for( i=0; i < sourceArray.GetSize(); i++ )
		{
			// Skip Null nodes
			if (sourceArray[i]->GetType() == Node_Null)
			{
				continue;
			}

			CVector vUpperLeftCorner=sourceArray[i]->GetUpperLeftCornerPos();

			if (vUpperLeftCorner.x < vBoundingPos.x)
			{
				vBoundingPos.x=vUpperLeftCorner.x;
			}
			if (vUpperLeftCorner.y > vBoundingPos.y)
			{
				vBoundingPos.y=vUpperLeftCorner.y;
			}
			if (vUpperLeftCorner.z > vBoundingPos.z)
			{
				vBoundingPos.z=vUpperLeftCorner.z;
			}		
		}
	}	

	int i;
	for( i=0; i < sourceArray.GetSize(); i++ )
	{
		// Create the new node
		CWorldNode *pNewNode = sourceArray[i]->AllocateSameKind();		
		ClipboardDuplicate( sourceArray[i], pNewNode, pRegion );

		// Move the node to the marker
		if (bPasteAtMarker)
		{
			// Move brush nodes
			if(pNewNode->GetType() == Node_Brush)
			{
				// Get the brush pointer...
				CEditBrush *pBrush = pNewNode->AsBrush();
				ASSERT(pBrush);

				// The offset to move the brush from the marker (used when multiple objects are pasted)
				CVector vMoveOffset=pBrush->GetUpperLeftCornerPos()-vBoundingPos;

				pBrush->MoveBrush(pRegion->m_vMarker+vMoveOffset);
			}		
			else if((pNewNode->GetType() == Node_Object) || 
					(pNewNode->GetType() == Node_PrefabRef))
			{
				// The offset to move the object from the marker (used when multiple objects are pasted)
				CVector vMoveOffset=pNewNode->GetUpperLeftCornerPos()-vBoundingPos;

				// Handle movement for object nodes...
				pNewNode->SetPos(pRegion->m_vMarker+vMoveOffset);
			}
		}

		pRegion->AttachNode( pNewNode, pActiveParent );

		undoList.AddTail(new CPreAction(ACTION_ADDEDNODE, pNewNode));
	}

	// Make sure the poly texture strings are in the right StringHolder.
	pRegion->UpdateTextureStrings();

	CopyParenthood( sourceArray, pRegion->m_Selections );

	pRegion->CleanupGeometry();

	ASSERT(pRegion->CheckNodes() == NULL);

	// Turn on the drawing of the node view
	if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
	{
		GetNodeView()->SetRedraw(TRUE);
		GetNodeView()->Invalidate();		
	}	

	// Generate unique names for the nodes if needed
	COptionsClipboard *pOptions=GetApp()->GetOptions().GetClipboardOptions();
	if (pOptions && pOptions->GetGenerateUniqueNames())
	{
		// Get the document
		CRegionDoc *pDoc=GetActiveRegionDoc();				

		// Get the region view
		POSITION pos = pDoc->GetFirstViewPosition();
		CRegionView* pView = (CRegionView *)pDoc->GetNextView( pos );
		if(!pView)
		{
			ASSERT(FALSE);
			return;
		}

		// Generate unique names
		pView->GenerateUniqueNamesForSelected(pOptions->GetUpdateRefProps(), TRUE, pOptions->GetDisplayNameChangeReport(), FALSE);

		// Update the properties dialog
		pDoc->SetupPropertiesDlg(FALSE);

		// Redraw all of the views
		pDoc->RedrawAllViews();				
	}

	// Turn off the wait cursor
	GetNodeView()->EndWaitCursor();
}

//-----------------------------------------------------------------------------------
// 
//  CEditClipboard::ClipboardDuplicate()
//
//  Purpose:	Make a temporary copy of a node on Copy, and place the temporary node
//				when pasting.
//
//-----------------------------------------------------------------------------------
void CEditClipboard::ClipboardDuplicate( CWorldNode *pSrc, CWorldNode *pDest, CEditRegion *pAddStuffTo )
{
	// Copy node information...
	ASSERT(pDest->GetType() == pSrc->GetType());

	if (pSrc->GetType() == Node_Brush)
	{
		pDest->AsBrush()->DoCopy(pSrc, &m_stringHolder);
	}
	else
	{
		pDest->DoCopy(pSrc);
	}
	

	
	// Handle brush copies...
	if(pAddStuffTo)
	{
		if( pSrc->GetType() == Node_Brush )
		{
			// Add the brush to the region's list...
			pAddStuffTo->AddBrush(pDest->AsBrush());
		}
		// Handle Object copies...
		else if( pSrc->GetType() == Node_Object )
		{
			// Add the object to the region's list...
			pAddStuffTo->AddObject(pDest->AsObject());
		}

		// Select the new node.
		pAddStuffTo->SelectNode( pDest );		
	}
}


//-----------------------------------------------------------------------------------
// 
//  CEditClipboard::CopyParenthood( )
//
//  Purpose:	Copy parenthood associations from src to dest.  This is done
//				by looping over the source nodes and looking for the one's with a
//				parent node.  If the parent node is also in src, then the indices
//				linking child to parent for src are identical for dest.  This
//				function only works if the number of elements in src and dest are
//				equal.
//
//-----------------------------------------------------------------------------------
void CEditClipboard::CopyParenthood( CMoArray< CWorldNode * > &src, CMoArray< CWorldNode * > &dest )
{
	DWORD dwChildIndex, dwParentIndex;
	CWorldNode *pSrcChildNode, *pSrcParentNode, *pDestChildNode, *pDestParentNode;

	// This only works if src and dest have same number of elements...
	if( src.GetSize( ) != dest.GetSize( ))
		return;

	// Loop through all src nodes...
	for( dwChildIndex = 0; dwChildIndex < src.GetSize( ); dwChildIndex++ )
	{
		// Get pointers to nodes and make sure a parent exists...
		pSrcChildNode = src[ dwChildIndex ];
		pSrcParentNode = pSrcChildNode->GetParent( );
		if( pSrcParentNode == NULL )
			continue;

		// Loop through all the src nodes again, skipping the current one...
		for( dwParentIndex = 0; dwParentIndex < src.GetSize( ); dwParentIndex++ )
		{
			if( dwParentIndex == dwChildIndex )
				continue;

			// The parent node is in src, so quit out of this loop...
			if( pSrcParentNode == src[ dwParentIndex ])
				break;
		}

		// If we found a parent in src, then use indices from src to set
		// parent in dest...
		if( dwParentIndex < src.GetSize( ))
		{
			pDestChildNode = dest[ dwChildIndex ];
			pDestParentNode = dest[ dwParentIndex ];
			if( pDestChildNode->GetParent( ))
				pDestChildNode->GetParent( )->RemoveChild( pDestChildNode );
			pDestChildNode->SetParent( pDestParentNode );
			// Update the tree item if it exists...
			if( pDestChildNode->GetItem( ))
				GetNodeView( )->SetNode( pDestChildNode );
			
			pDestParentNode->m_Children.Append( pDestChildNode );
		}
	}
}