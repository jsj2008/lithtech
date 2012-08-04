#include "precompile.h"
#include "modeledit.h"
#include "PieceLODGenDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP( CPieceLODGenDlg, CDialog )
END_MESSAGE_MAP()


CPieceLODGenDlg::CPieceLODGenDlg( CWnd* parent /*=NULL*/ )
	: CDialog( CPieceLODGenDlg::IDD, parent )
{
	m_Distance = 0.0f;
	m_Percent = 0.0f;
	m_MaxEdgeLen = 0.0f;
	m_MinNumTris = 0;
}


void CPieceLODGenDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );

	DDX_Text( pDX, IDC_DISTANCE, m_Distance );
	DDV_MinMaxFloat( pDX, m_Distance, 0.0f, 10000000.0f );
	DDX_Text( pDX, IDC_TRI_PERCENT, m_Percent );
	DDV_MinMaxFloat( pDX, m_Percent, 0.0f, 100.0f );
	DDX_Text( pDX, IDC_MAX_EDGE_LENGTH, m_MaxEdgeLen );
	DDV_MinMaxFloat( pDX, m_MaxEdgeLen, 0.0f, 10000000.0f );
	DDX_Text( pDX, IDC_TRI_MIN, m_MinNumTris );
	DDV_MinMaxUInt( pDX, m_MinNumTris, 0, 100000000 );
}
