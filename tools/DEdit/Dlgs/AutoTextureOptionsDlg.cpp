// AutoTextureOptionsDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "autotextureoptionsdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoTextureOptionsDlg dialog


CAutoTextureOptionsDlg::CAutoTextureOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAutoTextureOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAutoTextureOptionsDlg)
	m_bOffset = TRUE;
	m_bScale = TRUE;
	m_nStyle = 0;
	m_bRestrictDir = FALSE;
	//}}AFX_DATA_INIT
}

// Load up the options from the registry
void CAutoTextureOptionsDlg::LoadOptionsFromReg()
{
	CDEditOptions *pOptions = &(GetApp()->GetOptions());
	if (!pOptions)
		return;

	m_nStyle = pOptions->GetDWordValue("TextureWrap.Style", 0);
	m_bOffset = pOptions->GetBoolValue("TextureWrap.Offset", TRUE);
	m_bScale = pOptions->GetBoolValue("TextureWrap.Scale", TRUE);
	m_bRestrictDir = pOptions->GetBoolValue("TextureWrap.RestrictDir", FALSE);
}

// Save the options to the registry
void CAutoTextureOptionsDlg::SaveOptionsToReg()
{
	CDEditOptions *pOptions = &(GetApp()->GetOptions());
	if (!pOptions)
		return;

	pOptions->SetDWordValue("TextureWrap.Style", m_nStyle);
	pOptions->SetBoolValue("TextureWrap.Offset", m_bOffset);
	pOptions->SetBoolValue("TextureWrap.Scale", m_bScale);
	pOptions->SetBoolValue("TextureWrap.RestrictDir", m_bRestrictDir);
}

void CAutoTextureOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAutoTextureOptionsDlg)
	DDX_Check(pDX, IDC_OFFSET, m_bOffset);
	DDX_Check(pDX, IDC_SCALE, m_bScale);
	DDX_Radio(pDX, IDC_STYLE_LINEAR, m_nStyle);
	DDX_Check(pDX, IDC_RESTRICT_DIR, m_bRestrictDir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAutoTextureOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CAutoTextureOptionsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutoTextureOptionsDlg message handlers
