// ModifyDlgBase.cpp: implementation of the CModifyDlgBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ButeEdit.h"
#include "ModifyDlgBase.h"
#include "ButeEditDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModifyDlgBase::CModifyDlgBase()
{
	m_bNameReadOnly=TRUE;
}

CModifyDlgBase::~CModifyDlgBase()
{

}

/************************************************************************/
// Call this (after DoModal) to update the states of the common controls
void CModifyDlgBase::UpdateCommonControlStates(CDialog *pDialog)
{	
	// Set the name control to read only if necessary
	CEdit *pEdit=(CEdit *)pDialog->GetDlgItem(IDC_EDIT_NAME);
	if (pEdit)
	{
		pEdit->SetReadOnly(m_bNameReadOnly);
	}

	// Hide the "replace all keys" checkbox if the name is not read only
	if (!m_bNameReadOnly)
	{
		pDialog->GetDlgItem(IDC_CHECK_REPLACE_ALL)->ShowWindow(SW_HIDE);
	}
}

/************************************************************************/
// Call this to see if it is okay to close the dialog.  Note that some
// messages boxes might popup.
BOOL CModifyDlgBase::IsOkayToClose(CDialog *pDialog, CString sName)
{
	// Perform a name check if we are in the state to modify the name
	if (IsEnableModifyName())
	{
		// Make sure that there is a name
		if (sName.GetLength() <= 0)
		{
			DisplayNameEnterMessage(pDialog);
			return FALSE;
		}

		// Make sure that there aren't invalid characters in the name
		if (!IsNameValidCharacters(sName))
		{
			DisplayInvalidNameCharactersMessage(pDialog);
			return FALSE;
		}

		// Check to see if the key name is already defined
		if (IsKeyDefined(sName))
		{
			// Display the error message
			DisplayDuplicateKeyMessage(pDialog);
			return FALSE;
		}		
	}

	return TRUE;
}

/************************************************************************/
// Call this to see if a key is already defined
BOOL CModifyDlgBase::IsKeyDefined(CString sKey)
{
	// Get the main window
	CMainFrame *pFrame=(CMainFrame *)AfxGetMainWnd();

	// Get the document
	CButeEditDoc *pDoc=(CButeEditDoc *)pFrame->GetActiveDocument();

	// Get the bute manager
	CButeMgr *pButeMgr=pDoc->GetButeMgr();

	// Check to see if this key exists
	if (pButeMgr->Exist(pDoc->GetSelectedTag(), sKey))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/************************************************************************/
// Check to see if the key name contains valid characters
BOOL CModifyDlgBase::IsNameValidCharacters(CString sKey)
{
	// Look at each character in the string
	int i;
	for (i=0; i < sKey.GetLength(); i++)
	{
		// Make sure that the character is valid
		if (!isalpha(sKey[i]) && !isdigit(sKey[i]))
		{
			// An invalid character was found
			return FALSE;
		}
	}

	// No invalid characters were found
	return TRUE;
}

/************************************************************************/
// Display the invalid characters name message
void CModifyDlgBase::DisplayInvalidNameCharactersMessage(CWnd *pParent)
{
	pParent->MessageBox("Invalid characters were found in the name.  Please use numbers and letters only.",
						"Invalid Characters", MB_OK | MB_ICONERROR);
}

/************************************************************************/
// Displays a message box indicating that the key is already defined
void CModifyDlgBase::DisplayDuplicateKeyMessage(CWnd *pParent)
{
	pParent->MessageBox("A key by that name already exists.  Please chose a different name.",
						"Key Found", MB_OK | MB_ICONERROR);
}

/************************************************************************/
// Displays a message box indicating that a name must be entered
void CModifyDlgBase::DisplayNameEnterMessage(CWnd *pParent)
{
	pParent->MessageBox("You must enter a name for the value.", "Name Error", MB_OK | MB_ICONERROR);
}