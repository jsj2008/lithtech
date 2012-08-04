// InvalidAnimDimsDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "invalidanimdimsdlg.h"
#include "model_ops.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInvalidAnimDimsDlg dialog


CInvalidAnimDimsDlg::CInvalidAnimDimsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInvalidAnimDimsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvalidAnimDimsDlg)
	//}}AFX_DATA_INIT
}


void CInvalidAnimDimsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvalidAnimDimsDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvalidAnimDimsDlg, CDialog)
	//{{AFX_MSG_MAP(CInvalidAnimDimsDlg)
	ON_EN_CHANGE(IDC_EDIT_DIMS_X, UpdateEnabled)
	ON_EN_CHANGE(IDC_EDIT_DIMS_Z, UpdateEnabled)

	ON_BN_CLICKED(IDC_BUTTON_USE_X, OnButtonUseX)
	ON_BN_CLICKED(IDC_BUTTON_USE_Z, OnButtonUseZ)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvalidAnimDimsDlg message handlers

void CInvalidAnimDimsDlg::OnOK() 
{
	//get the values and make sure that they are ok
	CString sX, sZ;
	GetDlgItem(IDC_EDIT_DIMS_X)->GetWindowText(sX);
	GetDlgItem(IDC_EDIT_DIMS_Z)->GetWindowText(sZ);

	//convert to floats
	m_pAnimInfo->m_vDims.x = (float)atof(sX);
	m_pAnimInfo->m_vDims.z = (float)atof(sZ);
	
	CDialog::OnOK();
}

void CInvalidAnimDimsDlg::UpdateEnabled()
{
	//get the values and make sure that they are ok
	CString sX, sZ;
	GetDlgItem(IDC_EDIT_DIMS_X)->GetWindowText(sX);
	GetDlgItem(IDC_EDIT_DIMS_Z)->GetWindowText(sZ);

	//convert to floats
	float fX = (float)atof(sX);
	float fZ = (float)atof(sZ);

	//make sure that they match
	GetDlgItem(IDOK)->EnableWindow(fabs(fX - fZ) < 0.1f);
}

BOOL CInvalidAnimDimsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//display the animation name
	CString sAnimName;
	sAnimName.Format("Anim: %s", m_pAnimInfo->m_pAnim->GetName());
	GetDlgItem(IDC_STATIC_ANIM_NAME)->SetWindowText(sAnimName);

	//now fill in the dimension fields
	CString sData;

	sData.Format("%.2f", m_pAnimInfo->m_vDims.x);
	GetDlgItem(IDC_EDIT_DIMS_X)->SetWindowText(sData);

	sData.Format("%.2f", m_pAnimInfo->m_vDims.y);
	GetDlgItem(IDC_EDIT_DIMS_Y)->SetWindowText(sData);

	sData.Format("%.2f", m_pAnimInfo->m_vDims.z);
	GetDlgItem(IDC_EDIT_DIMS_Z)->SetWindowText(sData);

	//update the enabled status
	UpdateEnabled();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInvalidAnimDimsDlg::OnButtonUseX()
{
	//copy the text from the X field to the Z field and just bail
	CString sX;
	GetDlgItem(IDC_EDIT_DIMS_X)->GetWindowText(sX);
	GetDlgItem(IDC_EDIT_DIMS_Z)->SetWindowText(sX);

	OnOK();
}

void CInvalidAnimDimsDlg::OnButtonUseZ()
{
	//copy the text from the Z field to the X field and just bail
	CString sZ;
	GetDlgItem(IDC_EDIT_DIMS_Z)->GetWindowText(sZ);
	GetDlgItem(IDC_EDIT_DIMS_X)->SetWindowText(sZ);

	OnOK();
}
