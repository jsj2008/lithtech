// WeightSetSelectDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "WeightSetSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// WeightSetSelectDlg dialog


WeightSetSelectDlg::WeightSetSelectDlg(Model *pModel, DWORD iStartingSel, CWnd* pParent /*=NULL*/)
	: CDialog(WeightSetSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(WeightSetSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pModel = pModel;
	m_SelectedSet = iStartingSel;
}


void WeightSetSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WeightSetSelectDlg)
	DDX_Control(pDX, IDC_WEIGHTSETS, m_WeightSets);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(WeightSetSelectDlg, CDialog)
	//{{AFX_MSG_MAP(WeightSetSelectDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WeightSetSelectDlg message handlers

void WeightSetSelectDlg::OnOK() 
{
	int iSel;

	iSel = m_WeightSets.GetCurSel();
	if(iSel >= 0 && iSel < (int)m_pModel->NumWeightSets())
	{
		m_SelectedSet = (DWORD)iSel;
	}
	else
	{
		m_SelectedSet = INVALID_MODEL_WEIGHTSET;
	}
	
	CDialog::OnOK();
}

BOOL WeightSetSelectDlg::OnInitDialog() 
{
	DWORD i;
	
	CDialog::OnInitDialog();

	for(i=0; i < m_pModel->NumWeightSets(); i++)
	{
		m_WeightSets.AddString(m_pModel->GetWeightSet(i)->GetName());
	}
	m_WeightSets.SetCurSel((int)m_SelectedSet);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
