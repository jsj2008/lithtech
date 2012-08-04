
#include "stdafx.h"
#include "MatrixConfigDlg.h"

BEGIN_MESSAGE_MAP(CMatrixConfigDlg, CDialog)
    //{{AFX_MSG_MAP(CMatrixConfigDlg)
	ON_EN_CHANGE(IDC_MATRIXPARAM_00, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_01, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_02, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_03, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_10, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_11, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_12, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_13, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_20, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_21, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_22, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_23, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_30, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_31, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_32, OnMatrixParamChanged)
	ON_EN_CHANGE(IDC_MATRIXPARAM_33, OnMatrixParamChanged)
	ON_BN_CLICKED(IDC_PROJECTED, OnProjectEnable)
	ON_CBN_SELCHANGE(IDC_TEXCOORDCOUNT, OnTexCoordCountChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMatrixConfigDlg::OnInitDialog()
{
	InitializeDialogControls();					// Grab the Dialog's controls...
	SetDialogControls_From_RenderStyleData();	// Setup the controls with the passed in data...

	return true;
}

// Grab the Dialog's controls...
void CMatrixConfigDlg::InitializeDialogControls()
{
	m_pEB_MatrixParam[0]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_00);					assert(m_pEB_MatrixParam[0]);
	m_pEB_MatrixParam[1]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_01);					assert(m_pEB_MatrixParam[1]);
	m_pEB_MatrixParam[2]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_02);					assert(m_pEB_MatrixParam[2]);
	m_pEB_MatrixParam[3]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_03);					assert(m_pEB_MatrixParam[3]);
	m_pEB_MatrixParam[4]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_10);					assert(m_pEB_MatrixParam[4]);
	m_pEB_MatrixParam[5]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_11);					assert(m_pEB_MatrixParam[5]);
	m_pEB_MatrixParam[6]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_12);					assert(m_pEB_MatrixParam[6]);
	m_pEB_MatrixParam[7]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_13);					assert(m_pEB_MatrixParam[7]);
	m_pEB_MatrixParam[8]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_20);					assert(m_pEB_MatrixParam[8]);
	m_pEB_MatrixParam[9]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_21);					assert(m_pEB_MatrixParam[9]);
	m_pEB_MatrixParam[10]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_22);					assert(m_pEB_MatrixParam[10]);
	m_pEB_MatrixParam[11]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_23);					assert(m_pEB_MatrixParam[11]);
	m_pEB_MatrixParam[12]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_30);					assert(m_pEB_MatrixParam[12]);
	m_pEB_MatrixParam[13]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_31);					assert(m_pEB_MatrixParam[13]);
	m_pEB_MatrixParam[14]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_32);					assert(m_pEB_MatrixParam[14]);
	m_pEB_MatrixParam[15]						= (CEdit*)GetDlgItem(IDC_MATRIXPARAM_33);					assert(m_pEB_MatrixParam[15]);
	m_pBn_ProjectEnable							= (CButton*)GetDlgItem(IDC_PROJECTED);						assert(m_pBn_ProjectEnable);
	m_pCB_TexCoordCount							= (CComboBox*)GetDlgItem(IDC_TEXCOORDCOUNT);				assert(m_pCB_TexCoordCount);

	m_pCB_TexCoordCount->InsertString(0, "1");
	m_pCB_TexCoordCount->InsertString(1, "2");
	m_pCB_TexCoordCount->InsertString(2, "3");
	m_pCB_TexCoordCount->InsertString(3, "4");
}

// Setup the controls with the passed in data...
void CMatrixConfigDlg::SetDialogControls_From_RenderStyleData()
{
	char szTmp[64];
	for (uint32 i = 0; i < 16; ++i) {
		sprintf(szTmp,"%5.3f",m_pData->pMatrix[i]); m_pEB_MatrixParam[i]->SetWindowText(szTmp); }
	m_pBn_ProjectEnable->SetCheck(*m_pData->ProjectUVEnable);

	CString sVal;
	sVal.Format("%d", *m_pData->TexCoordCount);
	m_pCB_TexCoordCount->SelectString(-1, sVal);

	m_bReady = true;
}

void CMatrixConfigDlg::OnTexCoordCountChanged()
{
	if(!m_bReady)
		return;

	CString sVal;
	m_pCB_TexCoordCount->GetWindowText(sVal);

	*m_pData->TexCoordCount = max(1, min(4, atoi(sVal))); 
	g_AppFormView->RenderStyleDataChanged();
}

