// SocketEdit.cpp : implementation file
//

#include "precompile.h"
#include "stdafx.h"
#include "modeledit.h"
#include "socketedit.h"

#include "regmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Externs
extern TCHAR szRegKeyCompany[];
extern TCHAR szRegKeyApp[];
extern TCHAR szRegKeyVer[];


/////////////////////////////////////////////////////////////////////////////
// CSocketEdit dialog


CSocketEdit::CSocketEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CSocketEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSocketEdit)
	m_Attachment = _T("");
	m_Name = _T("");
	m_NodeName=_T("");
	m_PosX = 0.0f;
	m_PosY = 0.0f;
	m_PosZ = 0.0f;
	m_RotX = 0.0f;
	m_RotY = 0.0f;
	m_RotZ = 0.0f;
	m_SclX = 100.0f;
	m_SclY = 100.0f;
	m_SclZ = 100.0f;
	//}}AFX_DATA_INIT
}


void CSocketEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSocketEdit)
	DDX_Control(pDX, IDC_APPLY, m_cApply);
	DDX_Text(pDX, IDC_SOCKETEDIT_ATTACHMENT, m_Attachment);
	DDX_Text(pDX, IDC_SOCKETEDIT_NAME, m_Name);
	DDX_Text(pDX, IDC_SOCKETEDIT_NODENAME, m_NodeName);
	DDX_Text(pDX, IDC_SOCKETEDIT_POSX, m_PosX);
	DDX_Text(pDX, IDC_SOCKETEDIT_POSY, m_PosY);
	DDX_Text(pDX, IDC_SOCKETEDIT_POSZ, m_PosZ);
	DDX_Text(pDX, IDC_SOCKETEDIT_ROTX, m_RotX);
	DDX_Text(pDX, IDC_SOCKETEDIT_ROTY, m_RotY);
	DDX_Text(pDX, IDC_SOCKETEDIT_ROTZ, m_RotZ);
	DDX_Text(pDX, IDC_SOCKETEDIT_SCLX, m_SclX);
	DDX_Text(pDX, IDC_SOCKETEDIT_SCLY, m_SclY);
	DDX_Text(pDX, IDC_SOCKETEDIT_SCLZ, m_SclZ);
	//}}AFX_DATA_MAP

		//this just makes sure that if it is close to 0 that it is
	//cut to zero to prevent a certain side effect of numerical
	//instability that was producing weird values in the edit
	//box of the orientation fields.
	if(fabs(m_RotX) < 0.0001)
		m_RotX = 0;

	if(fabs(m_RotY) < 0.0001)
		m_RotY = 0;

	if(fabs(m_RotZ) < 0.0001)
		m_RotZ = 0;
}


BEGIN_MESSAGE_MAP(CSocketEdit, CDialog)
	//{{AFX_MSG_MAP(CSocketEdit)
	ON_BN_CLICKED(IDC_SOCKETEDIT_ATTACHMENT_LOAD, OnAttachmentLoad)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSocketEdit message handlers

void CSocketEdit::OnAttachmentLoad() 
{
	// Open up a load window for the socket attachment edit
	// Look up the open directory in the registry
	CRegMgr regMgr;
	CString csOpenDir;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "OpenDir", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = 256;
		regMgr.Get("Attachment", csOpenDir.GetBufferSetLength(dwSize), dwSize);
		csOpenDir.ReleaseBuffer(-1);
		// Default to the model directory if the attachment directory hasn't been defined
		if (csOpenDir.IsEmpty())
		{
			dwSize = 256;
			regMgr.Get("Model", csOpenDir.GetBufferSetLength(dwSize), dwSize);
			csOpenDir.ReleaseBuffer(-1);
		}
	}

	csOpenDir += "*.lta;*.ltc";

	// get the filename
	CString sFilter;
	sFilter.LoadString(IDS_LOADMODELFILTER);

	CFileDialog dlg (TRUE, "lta", (csOpenDir.GetLength()) ? (LPCTSTR)csOpenDir : NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, sFilter, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	// Store the open directory in the registry
	csOpenDir = dlg.GetPathName();
	csOpenDir = csOpenDir.Left(csOpenDir.GetLength() - dlg.GetFileName().GetLength());
	regMgr.Set("Attachment", (LPCTSTR)csOpenDir);

	// Update the dialog
	m_Attachment = dlg.GetPathName();
	UpdateData(FALSE);
}

void CSocketEdit::OnApply() 
{
	if (!UpdateData(TRUE))
	{
				//this just makes sure that if it is close to 0 that it is
	//cut to zero to prevent a certain side effect of numerical
	//instability that was producing weird values in the edit
	//box of the orientation fields.
	if(fabs(m_RotX) < 0.0001)
		m_RotX = 0;

	if(fabs(m_RotY) < 0.0001)
		m_RotY = 0;

	if(fabs(m_RotZ) < 0.0001)
		m_RotZ = 0;

		return;
	}

	EndDialog(IDC_APPLY);	
}

BOOL CSocketEdit::OnInitDialog() 
{
	//this just makes sure that if it is close to 0 that it is
	//cut to zero to prevent a certain side effect of numerical
	//instability that was producing weird values in the edit
	//box of the orientation fields.
	if(fabs(m_RotX) < 0.0001)
		m_RotX = 0;

	if(fabs(m_RotY) < 0.0001)
		m_RotY = 0;

	if(fabs(m_RotZ) < 0.0001)
		m_RotZ = 0;

	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
