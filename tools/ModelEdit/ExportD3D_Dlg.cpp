// ExportD3D_Dlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "ExportD3D_Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportD3D_Dlg
IMPLEMENT_DYNAMIC(CExportD3D_Dlg, CFileDialog)

CExportD3D_Dlg::CExportD3D_Dlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_ofn.Flags |= OFN_ENABLETEMPLATE;
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_EXPORT_D3D);
	m_bDoneWithUpdate = false;
	m_CompressionType = RELEVANT ;

	m_bStreamOptionsPosition	= true;
	m_bStreamOptionsNormal		= true;
	m_bStreamOptionsColor		= false;
	m_bStreamOptionsTangent		= false;
	m_bStreamOptionsUV1			= true;
	m_bStreamOptionsUV2			= false;
	m_bStreamOptionsUV3			= false;
	m_bStreamOptionsUV4			= false;
}

BEGIN_MESSAGE_MAP(CExportD3D_Dlg, CFileDialog)
	//{{AFX_MSG_MAP(CImportAnimation)
	ON_BN_CLICKED(IDC_EXPORT_D3D_FORCE_BONES_PER_VERT, OnDataChange)
	ON_EN_CHANGE(IDC_EXPORT_D3D_BONES_PER_VERT, OnDataChange)
	ON_BN_CLICKED(IDC_EXPORT_D3D_FORCE_BONES_PER_TRI, OnDataChange)
	ON_BN_CLICKED(IDC_EXPORT_PAUSE_AFTER_COMPILE, OnDataChange)
	ON_BN_CLICKED(IDC_NO_GEOM, OnDataChange)

	ON_EN_CHANGE(IDC_EXPORT_D3D_BONES_PER_TRI, OnDataChange)
	ON_BN_CLICKED(IDC_EXPORT_USED3DTNL, OnDataChange)
	ON_BN_CLICKED(IDC_EXPORT_USEMATRIXPALETTES, OnDataChange)
	ON_EN_CHANGE(IDC_EXPORT_MINBONEWEIGHT, OnDataChange)
	ON_CBN_SELCHANGE(IDC_COMBO_COMPRESSION_TYPE, OnDataChange)

	ON_BN_CLICKED(IDC_STREAM0_OPTIONS_POSITION, OnDataChange)
	ON_BN_CLICKED(IDC_STREAM0_OPTIONS_NORMAL, OnDataChange)
	ON_BN_CLICKED(IDC_STREAM0_OPTIONS_COLOR, OnDataChange)
	ON_BN_CLICKED(IDC_STREAM0_OPTIONS_TANGENT, OnDataChange)
	ON_BN_CLICKED(IDC_STREAM0_OPTIONS_UV1, OnDataChange)
	ON_BN_CLICKED(IDC_STREAM0_OPTIONS_UV2, OnDataChange)
	ON_BN_CLICKED(IDC_STREAM0_OPTIONS_UV3, OnDataChange)
	ON_BN_CLICKED(IDC_STREAM0_OPTIONS_UV4, OnDataChange)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CExportD3D_Dlg::OnInitDone()
{
	// Set initial data...
	CButton* pCheck = (CButton*)GetDlgItem(IDC_EXPORT_D3D_FORCE_BONES_PER_VERT);
	if (pCheck) pCheck->SetCheck(m_bMaxBonesPerVert);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EXPORT_D3D_BONES_PER_VERT);
	if (pEdit) 
	{ 
		char szBuff[32]; sprintf(szBuff,"%d",m_iMaxBonesPerVert);
		pEdit->SetWindowText(szBuff); 
	}

	if (pEdit) pEdit->EnableWindow(m_bMaxBonesPerVert);

	
	pCheck = (CButton*)GetDlgItem(IDC_EXPORT_D3D_FORCE_BONES_PER_TRI);
	if (pCheck) pCheck->SetCheck(m_bMaxBonesPerTri);
	pEdit = (CEdit*)GetDlgItem(IDC_EXPORT_D3D_BONES_PER_TRI);
	if (pEdit) { 
		char szBuff[32]; sprintf(szBuff,"%d",m_iMaxBonesPerTri);
		pEdit->SetWindowText(szBuff); }
	if (pEdit) pEdit->EnableWindow(m_bMaxBonesPerTri);

	
	pCheck = (CButton*)GetDlgItem(IDC_EXPORT_USEMATRIXPALETTES);
	if (pCheck) pCheck->SetCheck(m_bUseMatrixPalettes);

	pEdit = (CEdit *)GetDlgItem(IDC_EXPORT_MINBONEWEIGHT);
	if (pEdit)
	{
		char sTmp[32];
		sprintf(sTmp, "%4.2f", m_fMinBoneWeight);
		pEdit->SetWindowText(sTmp);
		pEdit->EnableWindow(true);
	}

	pCheck = (CButton*)GetDlgItem(IDC_EXPORT_PAUSE_AFTER_COMPILE);
	if (pCheck) pCheck->SetCheck(m_bPauseAfterCompile);
	

	pCheck = (CButton*)GetDlgItem(IDC_NO_GEOM);
	if(pCheck) pCheck->SetCheck(m_bExcludeGeometry);


	CComboBox *pComboBox = (CComboBox*)GetDlgItem( IDC_COMBO_COMPRESSION_TYPE );

	// setup compression type options. make relevant only default case.
	// note: any compression removes va animated stuff.
	if( pComboBox ) 
	{
		int cur_sel ;
		pComboBox->AddString( "No compression");
		pComboBox->AddString( "(RLE)");
		cur_sel = pComboBox->AddString( "(RLE16) ");	
		pComboBox->AddString( "(RLE16) Player View " );
		cur_sel = m_CompressionType ;
		pComboBox->SetCurSel(cur_sel);
		pComboBox->UpdateData(FALSE);
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_POSITION);
	if (pCheck) 
	{
		pCheck->SetCheck(true);
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_NORMAL);
	if (pCheck) 
	{
		pCheck->SetCheck(true);
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_UV1);
	if (pCheck) 
	{
		pCheck->SetCheck(true);
	}
		
	m_bDoneWithUpdate = true;
}

void CExportD3D_Dlg::OnDataChange() 
{
	if (!m_bDoneWithUpdate) return;

	CButton* pCheck = (CButton*)GetDlgItem(IDC_EXPORT_D3D_FORCE_BONES_PER_VERT);
	
	if (pCheck) m_bMaxBonesPerVert = pCheck->GetCheck() ? true : false;

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EXPORT_D3D_BONES_PER_VERT);
	if (pEdit) 
	{ 
		CString szBuff; pEdit->GetWindowText(szBuff); 
		m_iMaxBonesPerVert = atoi(szBuff); 
	}

	if (pEdit) 
		pEdit->EnableWindow(m_bMaxBonesPerVert);
	
	pCheck = (CButton*)GetDlgItem(IDC_EXPORT_D3D_FORCE_BONES_PER_TRI);

	if (pCheck) m_bMaxBonesPerTri = pCheck->GetCheck() ? true : false;

	pEdit = (CEdit*)GetDlgItem(IDC_EXPORT_D3D_BONES_PER_TRI);

	if (pEdit) 
	{ 
		CString szBuff; pEdit->GetWindowText(szBuff); 
		m_iMaxBonesPerTri = atoi(szBuff); 
	}
	
	if (pEdit) pEdit->EnableWindow(m_bMaxBonesPerTri);


	pCheck = (CButton*)GetDlgItem(IDC_EXPORT_USEMATRIXPALETTES);
	if (pCheck) m_bUseMatrixPalettes = pCheck->GetCheck() ? true : false;

	pEdit = (CEdit *)GetDlgItem(IDC_EXPORT_MINBONEWEIGHT);
	if (pEdit)
	{		
		char sTmp[32];
		pEdit->GetWindowText(sTmp, 32);

		m_fMinBoneWeight = (float)atof(sTmp);
	}

	pCheck = (CButton*)GetDlgItem(IDC_NO_GEOM);
	if (pCheck) m_bExcludeGeometry = pCheck->GetCheck() ? true : false ;

	pCheck = (CButton*)GetDlgItem(IDC_EXPORT_PAUSE_AFTER_COMPILE);
	if (pCheck) m_bPauseAfterCompile = pCheck->GetCheck() ? true : false;


	// get the compression type.
	CComboBox *pComboBox = (CComboBox*)GetDlgItem( IDC_COMBO_COMPRESSION_TYPE );
	if( pComboBox )
	{
		int val = pComboBox->GetCurSel();
		switch (val) {
		case 0:
			m_CompressionType = NONE ; break ;
		case 1:
			m_CompressionType = RELEVANT;break;
		case 2:
			m_CompressionType = RELEVANT_16bit ; break ;
		case 3:
			m_CompressionType = REL_PV16 ; break ;
		};
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_POSITION);
	if (pCheck) 
	{
		m_bStreamOptionsPosition = pCheck->GetCheck() ? true : false;
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_NORMAL);
	if (pCheck) 
	{
		m_bStreamOptionsNormal = pCheck->GetCheck() ? true : false;
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_COLOR);
	if (pCheck) 
	{
		m_bStreamOptionsColor = pCheck->GetCheck() ? true : false;
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_TANGENT);
	if (pCheck) 
	{
		m_bStreamOptionsTangent = pCheck->GetCheck() ? true : false;
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_UV1);
	if (pCheck) 
	{
		m_bStreamOptionsUV1 = pCheck->GetCheck() ? true : false;
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_UV2);
	if (pCheck) 
	{
		m_bStreamOptionsUV2 = pCheck->GetCheck() ? true : false;
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_UV3);
	if (pCheck) 
	{
		m_bStreamOptionsUV3 = pCheck->GetCheck() ? true : false;
	}

	pCheck = (CButton*)GetDlgItem(IDC_STREAM0_OPTIONS_UV4);
	if (pCheck) 
	{
		m_bStreamOptionsUV4 = pCheck->GetCheck() ? true : false;
	}

}
