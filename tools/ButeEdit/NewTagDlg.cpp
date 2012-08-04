// NewTagDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "NewTagDlg.h"
#include "ButeEditDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewTagDlg dialog


CNewTagDlg::CNewTagDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewTagDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewTagDlg)
	m_sName = _T("");
	//}}AFX_DATA_INIT
}


void CNewTagDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewTagDlg)
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewTagDlg, CDialog)
	//{{AFX_MSG_MAP(CNewTagDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewTagDlg message handlers

void CNewTagDlg::OnOK() 
{
	// Update the data
	UpdateData();

	// Get the main window
	CMainFrame *pFrame=(CMainFrame *)AfxGetMainWnd();

	// Get the document
	CButeEditDoc *pDoc=(CButeEditDoc *)pFrame->GetActiveDocument();

	// Make sure that there is a name
	if (m_sName.GetLength() <= 0)
	{
		MessageBox("You must enter a name for the value.", "Name Error", MB_OK | MB_ICONERROR);
		return;
	}

	// Make sure that there aren't any invalid characters in the tag name
	int i;
	for (i=0; i < m_sName.GetLength(); i++)
	{
		// Make sure that the character is valid
		if (!isalpha(m_sName[i]) && !isdigit(m_sName[i]))
		{
			// Display the invalid characters dialog
			MessageBox("Invalid characters were found in the name.  Please use numbers and letters only.",
					   "Invalid Characters", MB_OK | MB_ICONERROR);
			return;
		}
	}

	// Make sure that this name is unique
	for (i=0; i < pDoc->GetNumTags(); i++)
	{
		// Compare the tags
		if (m_sName == pDoc->GetTag(i))
		{
			MessageBox("A tag by that name already exists.  Please chose a different name.",
					   "Key Found", MB_OK | MB_ICONERROR);
			return;
		}
	}
	
	CDialog::OnOK();
}
