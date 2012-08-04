//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// AdvancedSelect.cpp: implementation of the CAdvancedSelect class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "advancedselect.h"
#include "regiondoc.h"
#include "edithelpers.h"
#include "nodeview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAdvancedSelect::CAdvancedSelect()
{

}

CAdvancedSelect::~CAdvancedSelect()
{

}

/************************************************************************/
// Initialization
BOOL CAdvancedSelect::Init(CRegionDoc *pRegionDoc)
{
	if (pRegionDoc)
	{
		m_pRegionDoc=pRegionDoc;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/************************************************************************/
// Performs a selection based on the current criteria
void CAdvancedSelect::Select(BOOL bIncludeChildren, BOOL bShowResults)
{
	// Return if there isn't any selection criteria
	if (m_classCriteria.GetSize() == 0 && m_nameCriteria.GetSize()		== 0
									   && m_propertyCriteria.GetSize()	== 0)
	{
		return;
	}
	
	int nNumFound = RecurseAndSelect(m_pRegionDoc->GetRegion()->GetRootNode(), TRUE, bIncludeChildren);

	// Make sure that the root node didn't get selected
	m_pRegionDoc->GetRegion()->UnselectNode(m_pRegionDoc->GetRegion()->GetRootNode());	
	m_pRegionDoc->NotifySelectionChange();

	// Display the results
	if (bShowResults)
	{
		char sResult[255];
		sprintf(sResult, "Selected %d objects", nNumFound);
		MessageBox(NULL, sResult, "Done", MB_OK | MB_TASKMODAL);
	}
}

/************************************************************************/
// Performs an unselection based on the current criteria
void CAdvancedSelect::Unselect(BOOL bIncludeChildren, BOOL bShowResults)
{
	// Return if there isn't any selection criteria
	if (m_classCriteria.GetSize() == 0 && m_nameCriteria.GetSize()		== 0
									   && m_propertyCriteria.GetSize()	== 0)
	{
		return;
	}

	int nNumFound = RecurseAndSelect(m_pRegionDoc->GetRegion()->GetRootNode(), FALSE, bIncludeChildren);
	m_pRegionDoc->NotifySelectionChange();

	// Display the results
	if (bShowResults)
	{
		char sResult[255];
		sprintf(sResult, "Un-Selected %d objects", nNumFound);
		MessageBox(NULL, sResult, "Done", MB_OK | MB_TASKMODAL);
	}
}

/************************************************************************/
// Recurses and selects nodes based on the criteria.
int CAdvancedSelect::RecurseAndSelect(CWorldNode *pParentNode, bool bSelect, bool bIncludeChildren)
{
	int nResult = 0;

	// Select the node if it meets the criteria
	if (IsPassClassCriteria(pParentNode) && IsPassNameCriteria(pParentNode) &&
		IsPassPropertyCriteria(pParentNode))
	{
		++nResult;

		// Check to see if the selection should be done recursively
		if (bIncludeChildren)
		{
			// Select the node and its children
			if (bSelect)
			{
				m_pRegionDoc->GetRegion()->RecurseAndSelect(pParentNode);
				
				// Highlight the node in the node view
				GetNodeView()->HighlightNode(pParentNode);
			}
			else
			{
				m_pRegionDoc->GetRegion()->RecurseAndUnselect(pParentNode);		
			}
			return nResult;
		}
		else
		{
			// Select the node
			if (bSelect)
			{
				m_pRegionDoc->GetRegion()->SelectNode(pParentNode);		

				// Highlight the node in the node view
				GetNodeView()->HighlightNode(pParentNode);
			}
			else
			{
				m_pRegionDoc->GetRegion()->UnselectNode(pParentNode);		
			}
		}
	}

	// Recurse through the children
	GPOS pos=pParentNode->m_Children;
	while (pos)
	{
		nResult += RecurseAndSelect(pParentNode->m_Children.GetNext(pos), bSelect, bIncludeChildren);
	}

	return nResult;
}

/************************************************************************/
// Returns true if the node passes the class criteria.  True is also
// returned if there isn't any class criteria.
BOOL CAdvancedSelect::IsPassClassCriteria(CWorldNode *pNode)
{
	// Determine if this node meets the class criteria
	if (m_classCriteria.GetSize() > 0)
	{		
		int i;
		for (i=0; i < m_classCriteria.GetSize(); i++)
		{
			// Compare the class names
			if (stricmp(m_classCriteria[i].GetClass(), pNode->GetClassName()) == 0)
			{
				return TRUE;				
			}

			//This is not the best approach, but since the prefab references 
			//don't actually have corresponding classes, we need to look for the automatically
			//added in class PrefabRef for selection
			if(	(pNode->GetType() == Node_PrefabRef) && 
				(stricmp(m_classCriteria[i].GetClass(), "PrefabRef") == 0))
			{
				return TRUE;
			}
		}
	}
	else
	{
		return TRUE;
	}

	// The class name was not found
	return FALSE;
}

/************************************************************************/
// Returns true if the node passes the name criteria.  True is also
// returned if there isn't any naming criteria.
BOOL CAdvancedSelect::IsPassNameCriteria(CWorldNode *pNode)
{
	// Get the name
	CString sName;

	// Special case container nodes and get their name from the label
	if (pNode->m_Type == Node_Null)
	{
		sName=pNode->GetNodeLabel();
	}
	else
	{
		CBaseProp *pProp = pNode->m_PropList.GetProp(g_NameName, FALSE);
		if(pProp && pProp->m_Type == LT_PT_STRING)
		{
			sName=((CStringProp*)pProp)->m_String;
		}				
	}

	// Determine if the node meets the name criteria
	if (m_nameCriteria.GetSize() > 0)
	{		
		int i;
		for (i=0; i < m_nameCriteria.GetSize(); i++)
		{
			// Determine if we should just search on a partial string
			if (m_nameCriteria[i].IsPartialString())
			{				
				CString sSearch=m_nameCriteria[i].GetName();	// The string to search for

				sName.MakeUpper();
				sSearch.MakeUpper();

				// Search for the string
				if (sName.Find(sSearch) != -1)
				{
					return TRUE;					
				}
			}
			else
			{
				// Check to see if we are comparing on an empty string
				if (m_nameCriteria[i].GetName().GetLength() == 0)
				{
					if(sName.GetLength() == 0)
					{
						return TRUE;
					}
				}
				// Compare the two strings
				else if (stricmp(m_nameCriteria[i].GetName(), sName) == 0)
				{
					return TRUE;
				}
			}
		}
	}
	else
	{
		return TRUE;
	}

	// The name was not found
	return FALSE;
}

/************************************************************************/
// Returns true if the node passes the property criteria.  True is also
// returned if there isn't any naming criteria.
BOOL CAdvancedSelect::IsPassPropertyCriteria(CWorldNode *pNode)
{
	// Storage for the property description
	CBaseProp* pProp;


	if (pNode == NULL)  return false;

	// Special case container nodes have no properties

	if (pNode->m_Type == Node_Null)  return false;


	// Determine if the node meets the name criteria
	if (m_propertyCriteria.GetSize() > 0)
	{		
		int i, j;
		for (i=0; i < m_propertyCriteria.GetSize(); i++)
		{
			pProp = m_propertyCriteria[i].GetProp();

			if (pProp != NULL)
			for (j=0; j < pNode->GetPropertyList()->GetSize(); j++) 
			{
				// check if the prop in the node is the same as the one in the criteria
				//
				// note: casting doesn't work right so we do a dumb switch - David C.

				if (m_propertyCriteria[i].GetMatchValue())
				{
				switch (pProp->GetType()) 
				{				
				case LT_PT_STRING:
					{
					if (((CStringProp*)pProp)->SameAs((CStringProp*)pNode->GetPropertyList()->GetAt(j)))  return true;
					break;
					}
				case LT_PT_REAL:
					{
					if (((CRealProp*)pProp)->SameAs((CRealProp*)pNode->GetPropertyList()->GetAt(j)))  return true;
					break;
					}
				case LT_PT_LONGINT:
					{
					if (((CRealProp*)pProp)->SameAs((CRealProp*)pNode->GetPropertyList()->GetAt(j)))  return true;
					break;
					}
				case LT_PT_FLAGS:
					{
					if (((CRealProp*)pProp)->SameAs((CRealProp*)pNode->GetPropertyList()->GetAt(j)))  return true;
					break;
					}
				case LT_PT_VECTOR:
					{
					if (((CVectorProp*)pProp)->SameAs((CVectorProp*)pNode->GetPropertyList()->GetAt(j)))  return true;
					break;
					}
				case LT_PT_ROTATION:
					{
					if (((CRotationProp*)pProp)->SameAs((CRotationProp*)pNode->GetPropertyList()->GetAt(j)))  return true;
					break;
					}
				case LT_PT_COLOR:
					{
					if (((CColorProp*)pProp)->SameAs((CColorProp*)pNode->GetPropertyList()->GetAt(j)))  return true;
					break;
					}
				case LT_PT_BOOL:
					{
					if (((CBoolProp*)pProp)->SameAs((CBoolProp*)pNode->GetPropertyList()->GetAt(j)))  return true;
					break;
					}
				default:
					{
					ASSERT(FALSE);			
					}
				}
				}
				else
				{
					if (_stricmp(pNode->GetPropertyList()->GetAt(j)->GetName(), pProp->GetName()) == 0)  return true;
				}
			}
		}
	}
	else
	{
		return true;
	}

	// The name was not found
	return false;
}

/************************************************************************/
// Adds a class type selection criteria
void CAdvancedSelect::AddClassCriteria(CString sClassType)
{
	// Setup the criteria data
	CClassCriteria criteria;
	criteria.SetClass(sClassType);

	// Add the critera
	m_classCriteria.Add(criteria);
}

/************************************************************************/
// Adds a class name criteria
void CAdvancedSelect::AddNameCriteria(CString sName, BOOL bPartialString)
{
	// Setup the criteria data
	CNameCriteria criteria;
	criteria.SetName(sName);
	criteria.SetPartialString(bPartialString);

	// Add the criteria
	m_nameCriteria.Add(criteria);
}

/************************************************************************/
// Adds a class name criteria
void CAdvancedSelect::AddPropertyCriteria(CBaseProp* pProp, bool matchValue)
{
	// Setup the criteria data
	CPropertyCriteria criteria;
	criteria.SetProp(pProp);
	criteria.SetMatchValue(matchValue);

	// Add the criteria
	m_propertyCriteria.Add(criteria);
}

/************************************************************************/
// Clears the current selection criteria
void CAdvancedSelect::ClearCriteria()
{
	m_classCriteria.RemoveAll();
	m_nameCriteria.RemoveAll();

	// Clean up dynamic data
	for (int i=0; i < m_propertyCriteria.GetSize(); i++) 
	{
		m_propertyCriteria[i].Cleanup();
	}

	m_propertyCriteria.RemoveAll();
}

