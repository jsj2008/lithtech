// PropPageOptionsClipboard.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "proppageoptionsclipboard.h"
#include "optionsclipboard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsClipboard dialog


IMPLEMENT_DYNCREATE(CPropPageOptionsClipboard, CPropertyPage)

CPropPageOptionsClipboard::CPropPageOptionsClipboard() : CPropertyPage(CPropPageOptionsClipboard::IDD)
{
	//{{AFX_DATA_INIT(CPropPageOptionsClipboard)
	m_bDisplayReportOfChanges = FALSE;
	m_bGenerateUniqueNames = FALSE;
	m_bUpdateRefProps = FALSE;
	//}}AFX_DATA_INIT
}


void CPropPageOptionsClipboard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageOptionsClipboard)
	DDX_Check(pDX, IDC_CHECK_DISPLAY_NAME_CHANGE_REPORT, m_bDisplayReportOfChanges);
	DDX_Check(pDX, IDC_CHECK_GENERATE_UNIQUE_NAMES, m_bGenerateUniqueNames);
	DDX_Check(pDX, IDC_CHECK_UPDATE_REFERENCING_PROPERTIES, m_bUpdateRefProps);
	//}}AFX_DATA_MAP
}

/************************************************************************/
// Update the enabled status of the controls
void CPropPageOptionsClipboard::UpdateEnabledStatus()
{
	// Update data
	UpdateData();

	// Enable/Disable the appropriate controls
	if (m_bGenerateUniqueNames)
	{		
		GetDlgItem(IDC_CHECK_UPDATE_REFERENCING_PROPERTIES)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_DISPLAY_NAME_CHANGE_REPORT)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_UPDATE_REFERENCING_PROPERTIES)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_DISPLAY_NAME_CHANGE_REPORT)->EnableWindow(FALSE);	
	}
}


BEGIN_MESSAGE_MAP(CPropPageOptionsClipboard, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageOptionsClipboard)
	ON_BN_CLICKED(IDC_CHECK_GENERATE_UNIQUE_NAMES, OnCheckGenerateUniqueNames)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_REFERENCING_PROPERTIES, OnCheckUpdateReferencingProperties)
	ON_BN_CLICKED(IDC_CHECK_DISPLAY_NAME_CHANGE_REPORT, OnCheckDisplayNameChangeReport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsClipboard message handlers

/************************************************************************/
// Dialog initialization
BOOL CPropPageOptionsClipboard::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Load the options
	COptionsClipboard *pOptions=GetApp()->GetOptions().GetClipboardOptions();
	if (pOptions)
	{
		m_bDisplayReportOfChanges = pOptions->GetDisplayNameChangeReport();
		m_bGenerateUniqueNames = pOptions->GetGenerateUniqueNames();
		m_bUpdateRefProps = pOptions->GetUpdateRefProps();
	}
	
	UpdateData(FALSE);
	UpdateEnabledStatus();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/************************************************************************/
// The "generate unique names" checkbox was clicked
void CPropPageOptionsClipboard::OnCheckGenerateUniqueNames() 
{
	// Update data
	UpdateData();

	// Update the options
	GetApp()->GetOptions().GetClipboardOptions()->SetGenerateUniqueNames(m_bGenerateUniqueNames);

	// Update the enabled status of the controls
	UpdateEnabledStatus();
}

/************************************************************************/
// The "update referencing properties" checkbox was clicked
void CPropPageOptionsClipboard::OnCheckUpdateReferencingProperties() 
{
	// Update data
	UpdateData();

	// Update the options
	GetApp()->GetOptions().GetClipboardOptions()->SetUpdateRefProps(m_bUpdateRefProps);

	// Update the enabled status of the controls
	UpdateEnabledStatus();	
}

/************************************************************************/
// The "display name change report" checkbox was clicked
void CPropPageOptionsClipboard::OnCheckDisplayNameChangeReport() 
{
	// Update data
	UpdateData();

	// Update the options
	GetApp()->GetOptions().GetClipboardOptions()->SetDisplayNameChangeReport(m_bDisplayReportOfChanges);

	// Update the enabled status of the controls
	UpdateEnabledStatus();		
}
