//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// WorldClassDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "classdlg.h"
#include "editprojectmgr.h"
#include "regmgr.h"
#include "optionsclassdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CClassDlg dialog


CClassDlg::CClassDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClassDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClassDlg)
	m_bShowTree = TRUE;
	m_bBindIndividually = FALSE;
	//}}AFX_DATA_INIT
	m_nMaxRecentClasses=10;
	m_bEnableBindIndividually=FALSE;
}


void CClassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClassDlg)
	DDX_Control(pDX, IDC_CLASSTREE, m_treeClasses);
	DDX_Control(pDX, IDC_LIST_CLASSES, m_listClasses);
	DDX_Control(pDX, IDC_LIST_RECENT_CLASSES, m_listRecent);
	DDX_Check(pDX, IDC_CHECK_SHOW_HIERARCHY, m_bShowTree);
	DDX_Check(pDX, IDC_CHECK_BIND_INDIVIDUALLY, m_bBindIndividually);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CClassDlg, CDialog)
	//{{AFX_MSG_MAP(CClassDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_SHOW_HIERARCHY, OnCheckShowHierarchy)
	ON_NOTIFY(NM_KILLFOCUS, IDC_CLASSTREE, OnKillfocusClasstree)
	ON_LBN_SELCHANGE(IDC_LIST_CLASSES, OnSelchangeListClasses)
	ON_LBN_DBLCLK(IDC_LIST_CLASSES, OnDblclkListClasses)
	ON_LBN_DBLCLK(IDC_LIST_RECENT_CLASSES, OnDblclkListRecentClasses)
	ON_NOTIFY(NM_DBLCLK, IDC_CLASSTREE, OnDblclkClasstree)
	ON_LBN_SELCHANGE(IDC_LIST_RECENT_CLASSES, OnSelchangeListRecentClasses)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClassDlg message handlers

BOOL CClassDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Load the registry settings
	LoadSettings();

	// Update the controls
	UpdateData(FALSE);

	// Update the tree control
	m_treeClasses.UpdateContents(TRUE);

	// Update the listbox control	
	m_listClasses.UpdateContents(TRUE);

	// Make sure that there is an intial class
	if (m_sInitialClass.GetLength() == 0)
	{		
		m_sInitialClass=m_treeClasses.GetItemText(m_treeClasses.GetRootItem());
	}

	// Select the class
	SelectClass(m_sInitialClass);
	
	// Set the window title
	SetWindowText( m_sTitleString );
	
	// Update which controls (tree or list) to display
	UpdateControlVisibility();	

	// Build the recent class listbox
	int i;
	for (i=0; i < m_recentClasses.GetSize(); i++)
	{
		m_listRecent.AddString(m_recentClasses[i]);
	}

	// Disable the remove button
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CClassDlg::OnDestroy() 
{		
	CDialog::OnDestroy();	
}


/************************************************************************/
// Selects a class in both controls
void CClassDlg::SelectClass(CString sClass)
{
	m_sSelectedClass=sClass;

	// Select the class in the tree control
	m_treeClasses.SelectClass(sClass);
	
	// Select the class in the listbox control
	m_listClasses.SelectClass(sClass);
}

/************************************************************************/
// Update which controls (tree or list) to display
void CClassDlg::UpdateControlVisibility()
{
	UpdateData();

	if (m_bShowTree)
	{
		// Show the tree and hide the listbox
		m_treeClasses.ShowWindow(SW_SHOW);
		m_listClasses.ShowWindow(SW_HIDE);
	}
	else
	{
		// Hide the tree and show the listbox
		m_treeClasses.ShowWindow(SW_HIDE);
		m_listClasses.ShowWindow(SW_SHOW);
	}

	if (m_bEnableBindIndividually)
	{
		GetDlgItem(IDC_CHECK_BIND_INDIVIDUALLY)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_BIND_INDIVIDUALLY)->EnableWindow(FALSE);
	}
}

/************************************************************************/
// Updates the currently selected class string
void CClassDlg::UpdateSelectedClass()
{
	if ( m_bShowTree)
	{
		// Get the selected class from the tree
		m_sSelectedClass=m_treeClasses.GetSelectedClass();	

		// Select the class in the listbox
		m_listClasses.SelectClass(m_sSelectedClass);
	}
	else
	{
		// Get the selected class from the listbox
		m_sSelectedClass=m_listClasses.GetSelectedClass();

		// Select the class in the tree
		m_treeClasses.SelectClass(m_sSelectedClass);
	}
}

/************************************************************************/
// The show hierarchy checkbox was pressed
void CClassDlg::OnCheckShowHierarchy() 
{
	// Update which controls (tree or list) to display
	UpdateControlVisibility();	
}

/************************************************************************/
// The tree control has lost focus
void CClassDlg::OnKillfocusClasstree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Updates the selected class
	UpdateSelectedClass();

	*pResult = 0;
}

/************************************************************************/
// The selection has changed in the list box
void CClassDlg::OnSelchangeListClasses() 
{
	UpdateSelectedClass();	
}

/************************************************************************/
// The class list was double clicked on
void CClassDlg::OnDblclkListClasses() 
{
	// Update the selected class
	UpdateSelectedClass();
	
	// Save the settings
	SaveSettings();

	// Return from the dialog
	EndDialog(IDOK);
}

/************************************************************************/
// The selection of the recent classes changed
void CClassDlg::OnSelchangeListRecentClasses() 
{
	// Get the new selection
	int nSelIndex=m_listRecent.GetCurSel();

	// Select the class
	if (nSelIndex != LB_ERR)
	{
		// Get the selected string
		CString sString;
		m_listRecent.GetText(nSelIndex, sString);

		// Select the class
		SelectClass(sString);

		// Enable the remove button
		GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(TRUE);
	}	
}

/************************************************************************/
// The recent class list was double clicked on
void CClassDlg::OnDblclkListRecentClasses() 
{
	// Save the settings
	SaveSettings();

	// End the dialog
	EndDialog(IDOK);	
}

/************************************************************************/
// The class tree was double clicked on
void CClassDlg::OnDblclkClasstree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Update the selected class
	UpdateSelectedClass();
	
	// Save the settings
	SaveSettings();

	// Return from the dialog
	EndDialog(IDOK);
	
	*pResult = 0;
}


/************************************************************************/
// Load settings from the registry
void CClassDlg::LoadSettings()
{
	// Get the options class
	COptionsClassDialog *pOptions=GetApp()->GetOptions().GetClassDialogOptions();
	if (!pOptions)
	{
		return;
	}

	// Array to hold the loaded class names
	CStringArray tempClassArray;

	// Get the recent classes from the registry
	pOptions->GetRecentClasses(tempClassArray);
	
	// Get the max number of recent classes
	m_nMaxRecentClasses=pOptions->GetMaxRecentClasses();

	// Load the "show heirarchy" value
	m_bShowTree=pOptions->GetShowTree();
	
	// Load the "bind individually" value
	m_bBindIndividually=pOptions->GetBindIndividually();
	
	// Build the recent class array
	BuildRecentClassArray(tempClassArray, m_recentClasses);	
}

/************************************************************************/
// Save settings to the registry
void CClassDlg::SaveSettings()
{
	// Update the data
	UpdateData();

	// Get the options class
	COptionsClassDialog *pOptions=GetApp()->GetOptions().GetClassDialogOptions();
	if (!pOptions)
	{
		return;
	}	

	// Check to see if the selected class already exists in the array
	unsigned int i;
	for (i=0; i < m_recentClasses.GetSize(); i++)
	{
		if (m_recentClasses[i] == m_sSelectedClass)
		{
			// Delete the item from the array as it will be added to the top of the list below
			m_recentClasses.RemoveAt(i);
			break;
		}
	}

	// Add the selected class to the recent class array
	m_recentClasses.InsertAt(0, m_sSelectedClass);

	// Make sure that the recent class list isn't too big
	if (m_recentClasses.GetSize() > m_nMaxRecentClasses)
	{
		// Remove the last item
		m_recentClasses.RemoveAt(m_nMaxRecentClasses-1);
	}

	// Set the recent classes from the registry
	pOptions->SetRecentClasses(m_recentClasses);
	
	// Set the max number of recent classes
	pOptions->SetMaxRecentClasses(m_nMaxRecentClasses);

	// Save the "show heirarchy" value
	pOptions->SetShowTree(m_bShowTree);
	
	// Save the "bind individually" value
	pOptions->SetBindIndividually(m_bBindIndividually);	
}

/************************************************************************/
// Builds the recent class array from an array of strings.  The method
// compares the given strings with the classes defined in the project.
void CClassDlg::BuildRecentClassArray(CStringArray &srcArray, CStringArray &destArray)
{
	// Get the project
	CEditProjectMgr *pProject=GetProject();

	// Add each loaded class to the array if it exists in the project
	int i;
	for (i=0; i < srcArray.GetSize(); i++)
	{
		BOOL bFound=FALSE;

		// Search the class list
		int n;
		for(n=0; n < pProject->m_nClassDefs; n++)
		{
			if (srcArray[i] == pProject->m_ClassDefs[n].m_ClassName)
			{
				// Add the class name to the array
				destArray.Add(srcArray[i]);
				bFound=TRUE;
				break;
			}			
		}

		// Search the template classes if it was not found in the regular class array
		if (!bFound)
		{			
			for(n=0; n < pProject->m_TemplateClasses; n++)
			{
				// Compare the class name to the template class name
				if (srcArray[i] == pProject->m_TemplateClasses[n]->m_ClassName)
				{
					// Add the class name to the array
					destArray.Add(srcArray[i]);
					break;
				}			
			}
		}
	}		
}

/************************************************************************/
// The OK button was pressed
void CClassDlg::OnOK() 
{		
	// Update the selected class
	UpdateSelectedClass();

	// Save the settings
	SaveSettings();

	CDialog::OnOK();
}

/************************************************************************/
// The remove button was pressed
void CClassDlg::OnButtonRemove() 
{	
	// Get the current selection
	int nCurSel=m_listRecent.GetCurSel();
	if (nCurSel != LB_ERR)
	{
		// Remove the item from the array
		m_recentClasses.RemoveAt(nCurSel);

		// Remove the selection from the listbox
		m_listRecent.DeleteString(nCurSel);

		// If there are remaining items, update current selection
		int nNumItems=m_listRecent.GetCount();
		if (nNumItems > 0)
		{
			// Keep the same selection or select the last item in the list, whichever is smaller.
			m_listRecent.SetCurSel(LTMIN(nCurSel, nNumItems-1));

			// Indicate a selection change
			OnSelchangeListRecentClasses();
		}
		else
		{
			// Disable the remove button
			GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(FALSE);
		}
	}
}
