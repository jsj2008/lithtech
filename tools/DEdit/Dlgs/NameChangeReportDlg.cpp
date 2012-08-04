// NameChangeReportDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "namechangereportdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNameChangeReportDlg dialog


CNameChangeReportDlg::CNameChangeReportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNameChangeReportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNameChangeReportDlg)
	//}}AFX_DATA_INIT

	// Initialize object member variables
	m_pOriginalNameArray=NULL;
	m_pUpdatedNameArray=NULL;

	m_pPropertyNameArray=NULL;
	m_pPropertyOriginalValueArray=NULL;
	m_pPropertyUpdatedValueArray=NULL;
}


void CNameChangeReportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNameChangeReportDlg)
	DDX_Control(pDX, IDC_LIST_PROPERTIES, m_listProperties);
	DDX_Control(pDX, IDC_LIST_NAMES, m_listNames);
	//}}AFX_DATA_MAP
}

/************************************************************************/
// Sets the name arrays.  Call this before calling DoModal
void CNameChangeReportDlg::SetObjectNameArrays(CStringArray *pOriginal, CStringArray *pUpdated)
{
	m_pOriginalNameArray=pOriginal;
	m_pUpdatedNameArray=pUpdated;
}

/************************************************************************/
// Sets the property name arrays.  Call this before calling DoModal
void CNameChangeReportDlg::SetPropertyNameArrays(CStringArray *pNameArray, CStringArray *pOriginal, CStringArray *pUpdated)
{
	m_pPropertyNameArray=pNameArray;
	m_pPropertyOriginalValueArray=pOriginal;
	m_pPropertyUpdatedValueArray=pUpdated;
}

BEGIN_MESSAGE_MAP(CNameChangeReportDlg, CDialog)
	//{{AFX_MSG_MAP(CNameChangeReportDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNameChangeReportDlg message handlers

BOOL CNameChangeReportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Fill in the column controls
	BuildObjectNameCtrl(m_listNames, m_pOriginalNameArray, m_pUpdatedNameArray);
	BuildPropertyValueCtrl(m_listProperties, m_pPropertyNameArray, m_pPropertyOriginalValueArray, m_pPropertyUpdatedValueArray);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/************************************************************************/
// This fills in the object name column control
void CNameChangeReportDlg::BuildObjectNameCtrl(CColumnCtrl &columnCtrl, CStringArray *pOriginal, CStringArray *pUpdated)
{
	// Add the columns to the object name control
	columnCtrl.AddColumn("Original Name");
	columnCtrl.AddColumn("Updated Name");

	// Make sure that we have pointers
	if (!pOriginal || !pUpdated)
	{
		ASSERT(FALSE);
		return;
	}

	// Verify that the arrays are the same size
	ASSERT(pOriginal->GetSize() == pUpdated->GetSize());

	// Add the text to each control
	int i;
	for (i=0; i < pOriginal->GetSize(); i++)
	{
		CString sRowText;
		sRowText.Format("%s\t%s", pOriginal->GetAt(i), pUpdated->GetAt(i));
		columnCtrl.AddRow(sRowText);
	}
}

/************************************************************************/
// This fills in the property value column control
void CNameChangeReportDlg::BuildPropertyValueCtrl(CColumnCtrl &columnCtrl, CStringArray *pNameArray, CStringArray *pOriginal, CStringArray *pUpdated)
{
	// Add the columns to the property name control
	columnCtrl.AddColumn("Property Name");
	columnCtrl.AddColumn("Original Value");
	columnCtrl.AddColumn("Updated Value");

	// Make sure that we have pointers
	if (!pNameArray || !pOriginal || !pUpdated)
	{
		ASSERT(FALSE);
		return;
	}

	// Verify that the arrays are the same size
	ASSERT(pNameArray->GetSize() == pOriginal->GetSize() && pNameArray->GetSize() == pUpdated->GetSize());

	// Add the text to each control
	int i;
	for (i=0; i < pOriginal->GetSize(); i++)
	{
		CString sRowText;
		sRowText.Format("%s\t%s\t%s", pNameArray->GetAt(i), pOriginal->GetAt(i), pUpdated->GetAt(i));
		columnCtrl.AddRow(sRowText);
	}
}
