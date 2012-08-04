// ImportAnimation.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "importanimation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportAnimation

IMPLEMENT_DYNAMIC(CImportAnimation, CFileDialog)

CImportAnimation::CImportAnimation(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_bImportAnimations = TRUE;
	m_bImportUserDims = FALSE;
	m_bImportTranslations = FALSE;
	m_bUseUVCoords = FALSE;
	m_bImportSockets = FALSE;
	m_bImportWeightSets = FALSE;
	m_ofn.Flags |= OFN_ENABLETEMPLATE;
	m_ofn.lpTemplateName = MAKEINTRESOURCE( IDD_IMPORTANIM );
	m_ofn.Flags |= OFN_ALLOWMULTISELECT;
}

BEGIN_MESSAGE_MAP(CImportAnimation, CFileDialog)
	//{{AFX_MSG_MAP(CImportAnimation)
	ON_BN_CLICKED(IDC_IMPORT_USEUVCOORDS, OnCheck)
	ON_BN_CLICKED(IDC_IMPORT_ANIMATIONS, OnCheck)
	ON_BN_CLICKED(IDC_IMPORT_USERDIMS, OnCheck)
	ON_BN_CLICKED(IDC_IMPORT_TRANSLATIONS, OnCheck)
	ON_BN_CLICKED(IDC_IMPORT_SOCKETS, OnCheck)
	ON_BN_CLICKED(IDC_IMPORT_WEIGHTSETS, OnCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CImportAnimation::OnInitDone( )
{
	CButton *pCheck;

	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_ANIMATIONS );
	if( pCheck )
		pCheck->SetCheck( m_bImportAnimations );
	
	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_USEUVCOORDS );
	if( pCheck )
		pCheck->SetCheck( m_bUseUVCoords );

	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_USERDIMS );
	if( pCheck )
		pCheck->SetCheck( m_bImportUserDims );

	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_TRANSLATIONS );
	if( pCheck )
		pCheck->SetCheck( m_bImportTranslations );

	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_SOCKETS );
	if( pCheck )
		pCheck->SetCheck( m_bImportSockets );

	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_WEIGHTSETS );
	if( pCheck )
		pCheck->SetCheck( m_bImportWeightSets );
}

void CImportAnimation::OnCheck() 
{
	CButton *pCheck;

	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_ANIMATIONS );
	if( pCheck )
		m_bImportAnimations = pCheck->GetCheck( );
	
	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_USEUVCOORDS );
	if( pCheck )
		m_bUseUVCoords = pCheck->GetCheck( );

	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_USERDIMS );
	if( pCheck )
		m_bImportUserDims = pCheck->GetCheck( );

	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_TRANSLATIONS );
	if( pCheck )
		m_bImportTranslations = pCheck->GetCheck( );

	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_SOCKETS );
	if( pCheck )
		m_bImportSockets = pCheck->GetCheck( );

	pCheck = ( CButton * )GetDlgItem( IDC_IMPORT_WEIGHTSETS );
	if( pCheck )
		m_bImportWeightSets = pCheck->GetCheck( );
}
