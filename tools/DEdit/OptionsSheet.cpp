//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsSheet.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "optionssheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsSheet

IMPLEMENT_DYNAMIC(COptionsSheet, CPropertySheet)

COptionsSheet::COptionsSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	CommonConstructor();
}

COptionsSheet::COptionsSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	CommonConstructor();
}

void COptionsSheet::CommonConstructor()
{
	m_psh.hIcon=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_psh.dwFlags |= PSP_USEHICON;
	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	//setup the icons for each of the tabs
	m_displayPage.m_psp.dwFlags |= PSP_USEHICON;
	m_displayPage.m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_OPTIONSVIEWPORT);

	m_d3dPage.m_psp.dwFlags |= PSP_USEHICON;
	m_d3dPage.m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_OPTIONSD3D);

	m_runPage.m_psp.dwFlags |= PSP_USEHICON;
	m_runPage.m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_OPTIONSRUN);

	m_clipboardPage.m_psp.dwFlags |= PSP_USEHICON;
	m_clipboardPage.m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_OPTIONSCLIPBOARD);

	m_undoPage.m_psp.dwFlags |= PSP_USEHICON;
	m_undoPage.m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_OPTIONSAUTOSAVE);

	m_modelsPage.m_psp.dwFlags |= PSP_USEHICON;
	m_modelsPage.m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_OPTIONSMODELS);

	m_controlsPage.m_psp.dwFlags |= PSP_USEHICON;
	m_controlsPage.m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_OPTIONSCONTROLS);

	m_miscPage.m_psp.dwFlags |= PSP_USEHICON;
	m_miscPage.m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_OPTIONSMISC);

	m_prefabsPage.m_psp.dwFlags |= PSP_USEHICON;
	m_prefabsPage.m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_WORLDS_TAB_ICON);

	AddPage(&m_displayPage);
	AddPage(&m_d3dPage);
	AddPage(&m_runPage);
	AddPage(&m_clipboardPage);
	AddPage(&m_undoPage);
	AddPage(&m_modelsPage);
	AddPage(&m_controlsPage);
	AddPage(&m_miscPage);
	AddPage(&m_prefabsPage);
}


COptionsSheet::~COptionsSheet()
{
}


BEGIN_MESSAGE_MAP(COptionsSheet, CPropertySheet)
	//{{AFX_MSG_MAP(COptionsSheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsSheet message handlers
