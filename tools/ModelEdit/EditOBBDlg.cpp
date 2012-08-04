// EditOBBDlg.cpp : implementation file
// 


#include "precompile.h"

#include "modeledit.h"
#include "solidrectwnd.h"
#include "lteulerangles.h"
#include "EditOBBDlg.h"

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditOBBDlg dialog


//CEditOBBDlg::CEditOBBDlg(const SOBB &obb,CWnd* pParent /*=NULL*/)
	//: CDialog(CEditOBBDlg::IDD, pParent), m_OBB(obb)
CEditOBBDlg::CEditOBBDlg(ModelNode *pNode ,CWnd* pParent /*=NULL*/)
	: CDialog(CEditOBBDlg::IDD, pParent)
{
	m_Initialized = false;
	m_InitOBB = m_OBB = pNode->GetOBB();
	m_InitEnable = pNode->IsOBBEnabled();
	m_CurEA = Eul_FromQuat( m_OBB.m_Orientation, EulOrdXYZs );
	m_CurEA.x = RadiansToDegrees( m_CurEA.x );
	m_CurEA.y = RadiansToDegrees( m_CurEA.y );
	m_CurEA.z = RadiansToDegrees( m_CurEA.z );

	m_pModelNode = pNode;

	//{{AFX_DATA_INIT(CEditOBBDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditOBBDlg::DoDataExchange(CDataExchange* pDX)
{
	EulerAngles ea;
	
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditOBBDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
		DDX_Text( pDX, IDC_EDIT_OBB_POS_X, m_OBB.m_Pos.x);
		DDX_Text( pDX, IDC_EDIT_OBB_POS_Y, m_OBB.m_Pos.y);
		DDX_Text( pDX, IDC_EDIT_OBB_POS_Z, m_OBB.m_Pos.z);

		DDX_Text( pDX, IDC_EDIT_OBB_ROT_X, ea.x);
		DDX_Text( pDX, IDC_EDIT_OBB_ROT_Y, ea.y);
		DDX_Text( pDX, IDC_EDIT_OBB_ROT_Z, ea.z);

		DDX_Text( pDX, IDC_EDIT_OBB_SIZ_X, m_OBB.m_Size.x);
		DDX_Text( pDX, IDC_EDIT_OBB_SIZ_Y, m_OBB.m_Size.y);
		DDX_Text( pDX, IDC_EDIT_OBB_SIZ_Z, m_OBB.m_Size.z);

	
	//}}AFX_DATA_MAP

	if(pDX->m_bSaveAndValidate)
	{
		if( ((CButton*)GetDlgItem(IDC_ENABLE_OBB))->GetCheck() == 0 )
			m_pModelNode->DisableOBB();
		else
			m_pModelNode->EnableOBB();

		ea.x = DegreesToRadians( ea.x );
		ea.y = DegreesToRadians( ea.y );
		ea.z = DegreesToRadians( ea.z );
		m_OBB.m_Orientation = Eul_ToQuat(ea);

		m_pModelNode->SetOBB(m_OBB);
	}
}

void CEditOBBDlg::OnCancel( void )
{
	m_pModelNode->SetOBB( m_InitOBB );
	if( m_InitEnable )
		m_pModelNode->EnableOBB();
	else
		m_pModelNode->DisableOBB();

	CDialog::OnCancel();
}


BEGIN_MESSAGE_MAP(CEditOBBDlg, CDialog)
	//{{AFX_MSG_MAP(CEditOBBDlg)
	ON_EN_CHANGE(IDC_EDIT_OBB_POS_X,OBBChange)
	ON_EN_CHANGE(IDC_EDIT_OBB_POS_Y,OBBChange)
	ON_EN_CHANGE(IDC_EDIT_OBB_POS_Z,OBBChange)
	ON_EN_CHANGE(IDC_EDIT_OBB_ROT_X,OBBChange)
	ON_EN_CHANGE(IDC_EDIT_OBB_ROT_Y,OBBChange)
	ON_EN_CHANGE(IDC_EDIT_OBB_ROT_Z,OBBChange)
	ON_EN_CHANGE(IDC_EDIT_OBB_SIZ_X,OBBChange)
	ON_EN_CHANGE(IDC_EDIT_OBB_SIZ_Y,OBBChange)
	ON_EN_CHANGE(IDC_EDIT_OBB_SIZ_Z,OBBChange)
	ON_BN_CLICKED(IDC_ENABLE_OBB,OBBChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditOBBDlg message handlers

BOOL CEditOBBDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Initialized = false;

	// Initialized dialog from obb.
	char buf[128];
	sprintf(buf,"%f",m_OBB.m_Pos.x);
	((CEdit*)GetDlgItem(IDC_EDIT_OBB_POS_X))->SetWindowText(buf);
	sprintf(buf,"%f",m_OBB.m_Pos.y);
	((CEdit*)GetDlgItem(IDC_EDIT_OBB_POS_Y))->SetWindowText(buf); 
	sprintf(buf,"%f",m_OBB.m_Pos.z);
	((CEdit*)GetDlgItem(IDC_EDIT_OBB_POS_Z))->SetWindowText(buf);
	
	sprintf(buf,"%f",m_CurEA.x);
	((CEdit*)GetDlgItem(IDC_EDIT_OBB_ROT_X))->SetWindowText(buf);
	sprintf(buf,"%f",m_CurEA.y);
	((CEdit*)GetDlgItem(IDC_EDIT_OBB_ROT_Y))->SetWindowText(buf);
	sprintf(buf,"%f",m_CurEA.z);
	((CEdit*)GetDlgItem(IDC_EDIT_OBB_ROT_Z))->SetWindowText(buf);
	
	sprintf(buf,"%f",m_OBB.m_Size.x);
	((CEdit*)GetDlgItem(IDC_EDIT_OBB_SIZ_X))->SetWindowText(buf);
	sprintf(buf,"%f",m_OBB.m_Size.y);
	((CEdit*)GetDlgItem(IDC_EDIT_OBB_SIZ_Y))->SetWindowText(buf);
	sprintf(buf,"%f",m_OBB.m_Size.z);
	((CEdit*)GetDlgItem(IDC_EDIT_OBB_SIZ_Z))->SetWindowText(buf);	

	((CButton*)GetDlgItem(IDC_ENABLE_OBB))->SetCheck(m_pModelNode->IsOBBEnabled()) ;

	UINT CtlIDs[] = { IDC_EDIT_OBB_POS_X_TRACK, IDC_EDIT_OBB_POS_Y_TRACK, IDC_EDIT_OBB_POS_Z_TRACK,
					  IDC_EDIT_OBB_ROT_X_TRACK, IDC_EDIT_OBB_ROT_Y_TRACK, IDC_EDIT_OBB_ROT_Z_TRACK,
					  IDC_EDIT_OBB_SIZ_X_TRACK, IDC_EDIT_OBB_SIZ_Y_TRACK, IDC_EDIT_OBB_SIZ_Z_TRACK };
	COLORREF RGBVals[] = { RGB(255,0,0), RGB(0,255,0), RGB(0,0,255),
						   RGB(255,0,0), RGB(0,255,0), RGB(0,0,255),
						   RGB(255,0,0), RGB(0,255,0), RGB(0,0,255) };

	for( int i = 0; i < 9; i++ )
	{
		trackers[i].m_CtlID = CtlIDs[i];
		trackers[i].m_ParentDlg = this;

		CRect rect;
		CWnd* wnd = GetDlgItem( CtlIDs[i] );
		wnd->GetClientRect( &rect );
		wnd->ClientToScreen( &rect );
		ScreenToClient( &rect );

		trackers[i].Create( NULL, "RectTracker", WS_CHILD|WS_VISIBLE, rect, this, -1, NULL );
		trackers[i].ModifyStyle( 0, WS_CHILD|WS_VISIBLE );
		trackers[i].SetColor( RGBVals[i] );
	}

	m_Initialized = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditOBBDlg::OBBChange( void )
{
	if( m_Initialized )
		UpdateData();
}

// called by the mouse trackers with value offset
void CEditOBBDlg::TrackerCallback( UINT ctlID, float offset )
{
	UINT editCtlID;
	float newEditVal;
	char buf[128];

	switch( ctlID )
	{
	case IDC_EDIT_OBB_POS_X_TRACK: newEditVal = m_OBB.m_Pos.x + offset; editCtlID = IDC_EDIT_OBB_POS_X; break;
	case IDC_EDIT_OBB_POS_Y_TRACK: newEditVal = m_OBB.m_Pos.y + offset; editCtlID = IDC_EDIT_OBB_POS_Y; break;
	case IDC_EDIT_OBB_POS_Z_TRACK: newEditVal = m_OBB.m_Pos.z + offset; editCtlID = IDC_EDIT_OBB_POS_Z; break;
	case IDC_EDIT_OBB_ROT_X_TRACK: newEditVal = (m_CurEA.x += offset); editCtlID = IDC_EDIT_OBB_ROT_X; break;
	case IDC_EDIT_OBB_ROT_Y_TRACK: newEditVal = (m_CurEA.y += offset); editCtlID = IDC_EDIT_OBB_ROT_Y; break;
	case IDC_EDIT_OBB_ROT_Z_TRACK: newEditVal = (m_CurEA.z += offset); editCtlID = IDC_EDIT_OBB_ROT_Z; break;
	case IDC_EDIT_OBB_SIZ_X_TRACK: newEditVal = m_OBB.m_Size.x + offset; editCtlID = IDC_EDIT_OBB_SIZ_X; break;
	case IDC_EDIT_OBB_SIZ_Y_TRACK: newEditVal = m_OBB.m_Size.y + offset; editCtlID = IDC_EDIT_OBB_SIZ_Y; break;
	case IDC_EDIT_OBB_SIZ_Z_TRACK: newEditVal = m_OBB.m_Size.z + offset; editCtlID = IDC_EDIT_OBB_SIZ_Z; break;
	default: ASSERT(0); return;
	}

	sprintf( buf, "%f", newEditVal );
	((CEdit*)GetDlgItem( editCtlID ))->SetWindowText( buf );

	UpdateData();
}



BEGIN_MESSAGE_MAP( COBBTracker, SolidRectWnd )
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()

COBBTracker::COBBTracker()
{
	m_HasCapture = false;
}

COBBTracker::~COBBTracker()
{
	ASSERT( !m_HasCapture );
}

void COBBTracker::OnLButtonDown( UINT nFlags, CPoint point )
{
	ASSERT( !m_HasCapture );
	SetCapture();
	m_HasCapture = true;
	m_InitPoint = point;
	ShowCursor( false );

	SolidRectWnd::OnLButtonDown( nFlags, point );
}

void COBBTracker::OnLButtonUp( UINT nFlags, CPoint point )
{
	if( m_HasCapture )
	{
		ReleaseCapture();
	}

	SolidRectWnd::OnLButtonUp( nFlags, point );
}

void COBBTracker::OnRButtonDown( UINT nFlags, CPoint point )
{
	if( m_HasCapture )
	{
		ReleaseCapture();
	}

	SolidRectWnd::OnRButtonDown( nFlags, point );
}

void COBBTracker::OnCaptureChanged( CWnd* pWnd )
{
	if( m_HasCapture )
	{
		ShowCursor( true );
		m_HasCapture = false;
	}
}

void COBBTracker::OnMouseMove( UINT nFlags, CPoint point )
{
	if( m_HasCapture && (point != m_InitPoint) )
	{
		CPoint newPoint = m_InitPoint;
		ClientToScreen( &newPoint );
		SetCursorPos( newPoint.x, newPoint.y );

		float diff = (float)(point.x - m_InitPoint.x);
		m_ParentDlg->TrackerCallback( m_CtlID, diff );
	}

	SolidRectWnd::OnMouseMove( nFlags, point );
}
