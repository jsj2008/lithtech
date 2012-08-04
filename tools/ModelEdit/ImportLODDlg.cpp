#include "precompile.h"
#include "modeledit.h"
#include "ImportLODDlg.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP( CImportLODDlg, CDialog )
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_IMPORTLODS_EXPANDALL, OnPieceExpandAll)
	ON_COMMAND(ID_IMPORTLODS_COLLAPSEALL, OnPieceCollapseAll)
END_MESSAGE_MAP()


CImportLODDlg::CImportLODDlg( CWnd* parent /*=NULL*/ )
	: CDialog( CImportLODDlg::IDD, parent )
{
}


void CImportLODDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );

	// load the dialog data
	if( !pDX->m_bSaveAndValidate )
	{
	}
	// retrieve the dialog data
	else
	{
		m_ModelEditDlg->GetSelectedPieceLODs( &m_PieceList, m_ImportModel, m_Selection );
	}
}


BOOL CImportLODDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	// Set up the lod list
	m_PieceList.SubclassDlgItem( IDC_PIECES, this );
	m_PieceList.EnableMultiSelect( TRUE );
	m_PieceList.EnableEditText( FALSE );

	m_ModelEditDlg->FillPieceList( &m_PieceList, m_ImportModel );

	UpdateData( FALSE );

	return TRUE;
}


void CImportLODDlg::OnContextMenu( CWnd* pWnd, CPoint point ) 
{
	CMenu menu;
	
	if( pWnd->m_hWnd == m_PieceList.m_hWnd )
	{
		VERIFY( menu.LoadMenu( IDR_IMPORTLODS ) );
	}
	else return;

	CMenu* popup = menu.GetSubMenu( 0 );
	ASSERT( popup );

	popup->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
}


void CImportLODDlg::OnPieceExpandAll( void )
{
	m_PieceList.ExpandAll();
}


void CImportLODDlg::OnPieceCollapseAll( void )
{
	m_PieceList.CollapseAll();
	m_PieceList.ClearSelection();
}
