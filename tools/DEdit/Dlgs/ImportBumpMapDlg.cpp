// AddPatchDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "importbumpmapdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportBumpMapDlg dialog


CImportBumpMapDlg::CImportBumpMapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImportBumpMapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportBumpMapDlg)
	m_fHeight			= 0.1f;

	m_nHeightChannel	= 0;
	m_nHeightChannel	= ALPHA;
	m_nLuminanceChannel	= NONE;

	m_sImageFile = "";
	//}}AFX_DATA_INIT
}


void CImportBumpMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportBumpMapDlg)
	DDX_Text(pDX, IDC_EDIT_TGA_NAME, m_sImageFile);
	DDX_Text(pDX, IDC_EDIT_HEIGHT_SCALE, m_fHeight);

	DDX_CBIndex(pDX, IDC_COMBO_HEIGHT_CHANNEL, m_nHeightChannel);
	DDX_CBIndex(pDX, IDC_COMBO_LUMINANCE_CHANNEL, m_nLuminanceChannel);
	//}}AFX_DATA_MAP
}

BOOL CImportBumpMapDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//disable the luminance channel if the user does not want that
	if(!m_sLuminanceText.IsEmpty())
	{
		GetDlgItem(IDC_STATIC_LUMINANCE_CHANNEL)->SetWindowText(m_sLuminanceText);
	}
	
	return TRUE;
}

void CImportBumpMapDlg::OnButtonBrowse()
{
	//bring up the file dialog...
	CFileDialog Dlg(TRUE, "tga", NULL, OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, "Targa Files (*.tga)|*.tga|All Files (*.*)|*.*||");

	if(Dlg.DoModal() == IDOK)
	{
		POSITION Pos = Dlg.GetStartPosition();

		while(Pos)
		{
			if(!m_sImageFile.IsEmpty())
				m_sImageFile += ';';

			m_sImageFile += Dlg.GetNextPathName(Pos);
		}

		//update the data
		UpdateData(false);
	}
}


BEGIN_MESSAGE_MAP(CImportBumpMapDlg, CDialog)
	//{{AFX_MSG_MAP(CImportBumpMapDlg)
		ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

