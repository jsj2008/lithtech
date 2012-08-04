//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ResNewDir.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "resnewdir.h"
#include "resourcemgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResNewDir dialog


CResNewDir::CResNewDir(CWnd* pParent /*=NULL*/)
	: CDialog(CResNewDir::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResNewDir)
	m_szNewDir = _T("");
	//}}AFX_DATA_INIT

	m_cDirType = RESTYPE_WORLD;
}


void CResNewDir::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResNewDir)
	DDX_Text(pDX, IDC_ENTEREDTEXT, m_szNewDir);
	DDV_MaxChars(pDX, m_szNewDir, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResNewDir, CDialog)
	//{{AFX_MSG_MAP(CResNewDir)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResNewDir message handlers
