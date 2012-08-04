// ExportObjFile.cpp : implementation file
//

// #include "stdafx.h"
#include "bdefs.h"
#include "dedit.h"
#include "ExportObjFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportObjFile dialog


CExportObjFile::CExportObjFile(CWnd* pParent /*=NULL*/)
	: CDialog(CExportObjFile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportObjFile)
	m_exportHiddenBrushes = FALSE;
	m_exportFor3DSMax = TRUE;
	m_exportForMaya = FALSE;
	m_exportForSoft = FALSE;
	m_reverseNormals = FALSE;
	//}}AFX_DATA_INIT
	m_isOK = FALSE;
}


void CExportObjFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportObjFile)
	DDX_Check(pDX, IDC_EXPORT_HIDDEN_BRUSHES_CHECK, m_exportHiddenBrushes);
	DDX_Check(pDX, IDC_EXPORT_OBJ_FOR_3DSMAX_CHECK, m_exportFor3DSMax);
	DDX_Check(pDX, IDC_EXPORT_OBJ_FOR_MAYA_CHECK, m_exportForMaya);
	DDX_Check(pDX, IDC_EXPORT_OBJ_FOR_SOFT_CHECK, m_exportForSoft);
	DDX_Check(pDX, IDC_EXPORT_OBJ_REVERSE_NORMALS, m_reverseNormals);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportObjFile, CDialog)
	//{{AFX_MSG_MAP(CExportObjFile)
	ON_BN_CLICKED(IDC_EXPORT_HIDDEN_BRUSHES_CHECK, OnExportHiddenBrushesCheck)
	ON_BN_CLICKED(IDC_EXPORT_OBJ_FOR_3DSMAX_CHECK, OnExportObjFor3dsmaxCheck)
	ON_BN_CLICKED(IDC_EXPORT_OBJ_FOR_MAYA_CHECK, OnExportObjForMayaCheck)
	ON_BN_CLICKED(IDC_EXPORT_OBJ_FOR_SOFT_CHECK, OnExportObjForSoftCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportObjFile message handlers

void CExportObjFile::OnExportHiddenBrushesCheck() 
{
	// TODO: Add your control notification handler code here
	
}

void CExportObjFile::OnOK() 
{
	// TODO: Add extra validation here
	m_isOK = TRUE;
	CDialog::OnOK();
}

void CExportObjFile::OnExportObjFor3dsmaxCheck() 
{	
	SetCheckBox(IDC_EXPORT_OBJ_FOR_3DSMAX_CHECK,	m_exportFor3DSMax = FALSE );
	SetCheckBox(IDC_EXPORT_OBJ_FOR_MAYA_CHECK,		m_exportForMaya = FALSE );
	SetCheckBox(IDC_EXPORT_OBJ_FOR_SOFT_CHECK,		m_exportForSoft = FALSE );

	SetCheckBox(IDC_EXPORT_OBJ_FOR_3DSMAX_CHECK,	m_exportFor3DSMax = TRUE );	
}

void CExportObjFile::OnExportObjForMayaCheck() 
{
	SetCheckBox(IDC_EXPORT_OBJ_FOR_3DSMAX_CHECK,	m_exportFor3DSMax = FALSE );
	SetCheckBox(IDC_EXPORT_OBJ_FOR_MAYA_CHECK,		m_exportForMaya = FALSE );
	SetCheckBox(IDC_EXPORT_OBJ_FOR_SOFT_CHECK,		m_exportForSoft = FALSE );

	SetCheckBox(IDC_EXPORT_OBJ_FOR_MAYA_CHECK,		m_exportForMaya = TRUE );	
}

void CExportObjFile::OnExportObjForSoftCheck() 
{
	SetCheckBox(IDC_EXPORT_OBJ_FOR_3DSMAX_CHECK,	m_exportFor3DSMax = FALSE );
	SetCheckBox(IDC_EXPORT_OBJ_FOR_MAYA_CHECK,		m_exportForMaya = FALSE );
	SetCheckBox(IDC_EXPORT_OBJ_FOR_SOFT_CHECK,		m_exportForSoft = FALSE );

	SetCheckBox(IDC_EXPORT_OBJ_FOR_SOFT_CHECK,		m_exportForSoft = TRUE );	
}

void CExportObjFile::SetCheckBox(UINT id, BOOL bCheck)
{
	CButton *pButton;

	pButton = (CButton*)GetDlgItem(id);
	if(pButton)
		pButton->SetCheck(bCheck);
}
