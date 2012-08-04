//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ClassList.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "classlist.h"
#include "editprojectmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClassList

CClassList::CClassList()
{
}

CClassList::~CClassList()
{
}

/************************************************************************/
// Updates the contents of the listbox
void CClassList::UpdateContents(BOOL bShowTemplates)
{
	CEditProjectMgr *pProject=GetProject();
	if(!pProject)
		return;

	// Remove the current contents
	ResetContent();

	// Add the classes to the listbox
	int i;
	for(i=0; i < pProject->m_nClassDefs; i++)
	{
		if (!(pProject->m_ClassDefs[i].m_ClassFlags & CF_HIDDEN))
		{
			AddString(pProject->m_ClassDefs[i].m_ClassName);
		}
	}

	// Include the template classes
	if (bShowTemplates)
	{
		for(i=0; i < pProject->m_TemplateClasses; i++)
		{
			AddString(pProject->m_TemplateClasses[i]->m_ClassName);			
		}
	}
}

/************************************************************************/
// Selects a class in the listbox
BOOL CClassList::SelectClass(CString sClass)
{
	// Select the class in the listbox control
	int nSelString=FindString(-1, sClass);
	if (nSelString != LB_ERR)
	{
		// Set the current selection to the index of the class
		SetCurSel(nSelString);
		return TRUE;
	}
	else
	{
		// Clear the selection
		SetCurSel(-1);
		return FALSE;
	}
}

/************************************************************************/
// Gets the selected class from the listbox
CString CClassList::GetSelectedClass()
{
	// Get the current selection
	int nSelString=GetCurSel();
	if (nSelString != LB_ERR)
	{
		// Get the string for the current selection
		CString sString;
		GetText(nSelString, sString);

		// Return the string
		return sString;
	}
	else
	{
		// Return an empty string
		return "";
	}
}

BEGIN_MESSAGE_MAP(CClassList, CListBox)
	//{{AFX_MSG_MAP(CClassList)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClassList message handlers
