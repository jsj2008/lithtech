//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#include "bdefs.h"
#include "regiondoc.h"
#include "edithelpers.h"
#include "propertyhelpers.h"
#include "nodeview.h"

static CPropList	g_PropList;


// ---------------------------------------------------------- //
// Adds the property to the list of it doesn't already exist.
// ---------------------------------------------------------- //

static void MaybeAddToPropList(CPropList *pList, CBaseProp *pProp)
{
	CBaseProp	*pTestProp, *pNewProp;


	// Does a property with the same name already exist?
	if(pTestProp = pList->GetProp(pProp->m_Name))
	{
	}
	else
	{
		pNewProp = pProp->CreateSameKind();
		pNewProp->Copy(pProp);
		
		strcpy(pNewProp->m_Name, pProp->m_Name);
		pNewProp->m_Type = pProp->m_Type;
		pNewProp->m_PropFlags = pProp->m_PropFlags;
		
		pList->m_Props.Append(pNewProp);
	}
}


// ---------------------------------------------------------- //
// Goes thru all the properties in pMain and removes any
// non-matching properties in pTest.
// ---------------------------------------------------------- //

static void RemoveNonMatching(CPropList *pMain, CPropList *pTest)
{
	DWORD		i;
	CBaseProp	*pMainProp, *pTestProp;

	for(i=0; i < pMain->m_Props; i++)
	{
		pMainProp = pMain->m_Props[i];
		pTestProp = pTest->GetProp(pMainProp->m_Name);

		if(pTestProp)
		{
			if(pMainProp->m_Type==pTestProp->m_Type)
			{
			}
			else
			{
				// Ok, they don't match.  Remove it.
				pMain->m_Props.Remove(i);
				delete pMainProp;
				--i;
			}
		}
	}
}


// ---------------------------------------------------------- //
// Finds all the common properties in all the selected nodes,
// and sets up g_PropList accordingly.
// ---------------------------------------------------------- //

CPropList* CreateMainPropertyList(CRegionDoc *pDoc)
{
	DWORD		i, j;
	CWorldNode	*pNode;
	CEditRegion	*pRegion = pDoc->GetRegion();

	
	g_PropList.Term();

	// First get all of the properties in there (ignoring null nodes).
	for(i=0; i < pRegion->m_Selections; i++)
	{
		pNode = pRegion->m_Selections[i];
		if(pNode->GetType() != Node_Null)
		{
			for(j=0; j < pNode->m_PropList.m_Props; j++)
			{
				MaybeAddToPropList(&g_PropList, pNode->m_PropList.m_Props[j]);
			}
		}
	}

	// Now remove ones that any of the objects don't have (ignoring null nodes).
	for(i=0; i < pRegion->m_Selections; i++)
	{
		pNode = pRegion->m_Selections[i];
		if(pNode->GetType() != Node_Null)
		{
			RemoveNonMatching(&g_PropList, &pNode->m_PropList);
		}
	}

	return &g_PropList;
}
		

// ---------------------------------------------------------- //
// Goes through all the currently selected nodes, and sets
// their properties based on g_PropList.
// ---------------------------------------------------------- //

void ReadPropertiesIntoSelections(CRegionDoc *pDoc)
{
	CEditRegion		*pRegion = pDoc->GetRegion();
	CBaseProp		*pMainProp, *pDestProp;
	CWorldNode		*pNode;
	DWORD			i, sel;
	
	BOOL bRedrawViews=FALSE;
	for(i=0; i < g_PropList.m_Props; i++)
	{
		pMainProp = g_PropList.m_Props[i];

		for(sel=0; sel < pRegion->m_Selections; sel++)
		{
			pNode = pRegion->m_Selections[sel];

			if(pNode->GetType() != Node_Null)
			{
				pDestProp = pNode->m_PropList.GetMatchingProp(pMainProp->m_Name, pMainProp->m_Type);
				if(pDestProp)
				{
					if(pDestProp->m_Type==pMainProp->m_Type)
					{
						pDestProp->Copy(pMainProp);

						// {BP 1/7/98}
						// This could be a model Filename.  The dims searching values
						// need to be reset so the drawing functions know to reread this prop...
						if( pNode->GetType( ) == Node_Object )
						{
							if (pDestProp->m_Type == LT_PT_STRING || pDestProp->m_Type == LT_PT_VECTOR)
							{
								((CBaseEditObj * )pNode)->SetSearchForDims(TRUE);
								bRedrawViews=TRUE;
							}
						}
					}
					else
					{
						// RemoveNonMatching should have filtered this case out!
						ASSERT(FALSE);
					}
				}
			}
		}
	}

	// Redraw the views
	if (bRedrawViews)
	{
		pDoc->RedrawAllViews( );
	}
}


// ---------------------------------------------------------- //
// 
//	ReadPropertyIntoSelections
//
//  Sets property into selections that apply.  Any selection
//	that has the property is updated.
// 
// ---------------------------------------------------------- //

void ReadPropertyIntoSelections( CRegionDoc *pDoc, CBaseProp *pMainProp )
{
	CEditRegion		*pRegion = pDoc->GetRegion();
	CBaseProp		*pDestProp;
	CWorldNode		*pNode;
	DWORD			sel;
	
	// Indicates if the views need to be redrawn
	BOOL bRedrawViews=FALSE;

	for(sel=0; sel < pRegion->m_Selections; sel++)
	{
		pNode = pRegion->m_Selections[sel];

		if(pNode->GetType() != Node_Null)
		{
			pDestProp = pNode->m_PropList.GetMatchingProp(pMainProp->m_Name, pMainProp->m_Type);
			if(pDestProp)
			{
				if(pDestProp->m_Type==pMainProp->m_Type)
				{
					pDestProp->Copy(pMainProp);

					// {BP 1/7/98}
					// This could be a model Filename.  The dims searching values
					// need to be reset so the drawing functions know to reread this prop...
					if( pNode->GetType( ) == Node_Object )
					{
						if (pDestProp->m_Type == LT_PT_STRING || pDestProp->m_Type == LT_PT_VECTOR)
						{
							((CBaseEditObj * )pNode)->SetSearchForDims(TRUE);						
							bRedrawViews=TRUE;
						}
					}
				}
				else
				{
					// RemoveNonMatching should have filtered this case out!
					ASSERT(FALSE);
				}
			}

			// Update the node label
			CNodeView *pNodeView=GetNodeView();
			if (pNodeView)
			{
				pNodeView->UpdateTreeItemLabel(pNode);
			}
		}
	}

	// Redraw the views if necessary
	if (bRedrawViews)
	{
		pDoc->RedrawAllViews();
	}
}
