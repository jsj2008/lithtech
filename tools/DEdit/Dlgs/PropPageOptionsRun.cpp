//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// PropPageOptionsRun.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "proppageoptionsrun.h"
#include "optionsrun.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun property page

IMPLEMENT_DYNCREATE(CPropPageOptionsRun, CPropertyPage)

CPropPageOptionsRun::CPropPageOptionsRun() : CPropertyPage(CPropPageOptionsRun::IDD)
{
	//{{AFX_DATA_INIT(CPropPageOptionsRun)
	m_sExecutable = _T("");
	m_sProgramArguments = _T("");
	m_sWorkingDirectory = _T("");
	//}}AFX_DATA_INIT

	m_bLoaded = FALSE;
}

CPropPageOptionsRun::~CPropPageOptionsRun()
{
}

void CPropPageOptionsRun::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageOptionsRun)
	DDX_Text(pDX, IDC_EDIT_EXECUTABLE, m_sExecutable);
	DDX_Text(pDX, IDC_EDIT_PROGRAM_ARGUMENTS, m_sProgramArguments);
	DDX_Text(pDX, IDC_EDIT_WORKING_DIRECTORY, m_sWorkingDirectory);
	//}}AFX_DATA_MAP
}


/************************************************************************/
// Loads the options
void CPropPageOptionsRun::LoadOptions()
{
	// Get the options class
	COptionsRun *pOptions=GetApp()->GetOptions().GetRunOptions();
	if (pOptions)
	{
		// Retrieve the options
		m_sExecutable=pOptions->GetExecutable();
		m_sProgramArguments = pOptions->GetProgramArguments();
		m_sWorkingDirectory = pOptions->GetWorkingDirectory();
		m_bLoaded = TRUE;
	}
}

/************************************************************************/
// Saves the options
void CPropPageOptionsRun::SaveOptions()
{
	if (!m_bLoaded)
		return;

	// Get the options class
	COptionsRun *pOptions=GetApp()->GetOptions().GetRunOptions();
	if (pOptions)
	{
		// Set the options
		pOptions->SetExecutable(m_sExecutable);
		pOptions->SetProgramArguments(m_sProgramArguments);
		pOptions->SetWorkingDirectory(m_sWorkingDirectory);
	}
}

BEGIN_MESSAGE_MAP(CPropPageOptionsRun, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageOptionsRun)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun message handlers


void CPropPageOptionsRun::OnButtonBrowse() 
{
	// Update the member variables data
	UpdateData();

	// Create the file dialog
	CFileDialog fileDialog(TRUE, "exe", m_sExecutable, NULL, "Executable (*.exe)|*.exe||");	

	// Display the file dialog
	if (fileDialog.DoModal() == IDOK)
	{
		// Get the pathname and place it in the member variable
		m_sExecutable=fileDialog.GetPathName();

		// Update the dialog data
		UpdateData(FALSE);
	}
}


BOOL CPropPageOptionsRun::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// Load the options
	LoadOptions();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
