// RenderWnd.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "renderwnd.h"
#include "modeleditdlg.h"
#include "geomroutines.h"
#include "regmgr.h"
#include "mmsystem.h" // for timegettime()
#include "windef.h"   // for max (?)
#include "tdguard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Externs
extern TCHAR szRegKeyCompany[];
extern TCHAR szRegKeyApp[];
extern TCHAR szRegKeyVer[];

#define ____max(a,b)            (((a) > (b)) ? (a) : (b))

////////////////////////////////////////////////////////////////////////////
// Locator
////////////////////////////////////////////////////////////////////////////


CLocator::CLocator()
{
	Center();
	m_nDistance = 75.0f;
	UpdateLocation();
}

void CLocator::UpdateLocation()
{
	CMatrix mtx, mtx2;

	// Add the rotation
	mtx.SetupRot(m_Right, m_nYAngle * -0.01745329f);
	// Note : Rotating about the world Y axis instead of the up vector makes this MUCH easier to understand
	mtx2.SetupRot(CVector(0.0f, 1.0f, 0.0f), m_nXZAngle * -0.01745329f);
	mtx2.Apply(mtx);
	mtx.Apply3x3(m_Location);
	mtx.Apply3x3(m_Orientation);
	mtx.Apply3x3(m_Right);
	mtx.Apply3x3(m_Up);

	// Move backward by the distance
	m_Location += m_Orientation * -m_nDistance;

	// Add the offset
	m_Location += m_Right * m_Offset.x;
	m_Location += m_Up * m_Offset.y;
	m_Location += m_Orientation * m_Offset.z;

	// Reset the adjusting members
	m_nDistance = 0.0f;
	m_nXZAngle = 0.0f;
	m_nYAngle = 0.0f;
	m_Offset.Init(0.0f, 0.0f, 0.0f);
}

void CLocator::Center()
{
	m_nDistance = 64.0f;
	m_nXZAngle = 0.0f;
	m_nYAngle = 0.0f;
	m_Offset.Init(0.0f, 0.0f, 0.0f);

	m_Location.Init(0.0f, 0.0f, 0.0f);
	m_Orientation.Init(0.0f, 0.0f, 1.0f);
	m_Up.Init(0.0f, 1.0f, 0.0f);
	m_Right.Init(1.0f, 0.0f, 0.0f);
}


////////////////////////////////////////////////////////////////////////////
// CCamera

CCamera::CCamera()
{
	Reset();
}

void CCamera::Update( void )
{
	float curDist = VEC_DIST( m_Position, m_LookAt );
//	curDist += m_fDistance;

	LTMatrix mtx, mtx2;

	mtx.SetupRot( m_Right, m_fYAngle * -0.01745329f );
	mtx2.SetupRot( LTVector(0.0f,1.0f,0.0f), m_fXZAngle * -0.01745329f );
	mtx2.Apply( mtx );
	mtx.Apply3x3( m_Direction );
	mtx.Apply3x3( m_Right );
	mtx.Apply3x3( m_Up );

	m_LookAt += m_Right * m_Offset.x;
	m_LookAt += m_Up * m_Offset.y;
	m_Position = m_LookAt - (curDist * m_Direction);

	m_fYAngle = 0.0f;
	m_fXZAngle = 0.0f;
	m_Offset.Init( 0.0f, 0.0f, 0.0f );
}

void CCamera::Reset( void )
{
	m_fXZAngle = 0.0f;
	m_fYAngle = 0.0f;
	m_Offset.Init( 0.0f, 0.0f, 0.0f );

	m_Position.Init( 0.0f, 0.0f, 64.0f );
	m_LookAt.Init( 0.0f, 0.0f, 0.0f );
	m_Direction.Init( 0.0f, 0.0f, 1.0f );
	m_Right.Init( 1.0f, 0.0f, 0.0f );
	m_Up.Init( 0.0f, 1.0f, 0.0f );

	Update();
}

void CCamera::LookAtPoint( const LTVector& lookAt )
{
	m_LookAt = lookAt;
	Update();
}



/////////////////////////////////////////////////////////////////////////////
// CRenderWnd

CRenderWnd::CRenderWnd()
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return;
	}

	m_pModelEditDlg = NULL;

	m_DrawStruct.m_bDrawSkeleton = FALSE;
	m_DrawStruct.m_bDrawBright = FALSE;
	m_hContext = NULL;

	m_DrawStruct.m_iLOD = PIECELOD_BASE;
	m_DrawStruct.m_CurrentLODDist = 0.0f ;

	m_DrawStruct.m_bWireframe = FALSE;
	m_bCameraFollow = TRUE;

	m_DrawStruct.m_bDims = FALSE;
	m_DrawStruct.m_bAnimBox = FALSE;

	m_DrawStruct.m_bShowNormalRef = FALSE;
	m_DrawStruct.m_bProfile = FALSE;

	m_DrawStruct.m_DimsColor.x = m_DrawStruct.m_DimsColor.y = m_DrawStruct.m_DimsColor.z = 255.0f;
	m_DrawStruct.m_ModelBoxColor.x = m_DrawStruct.m_ModelBoxColor.y = m_DrawStruct.m_ModelBoxColor.z = 1.0f;
	m_DrawStruct.m_AnimBoxColor.x = m_DrawStruct.m_AnimBoxColor.y = m_DrawStruct.m_AnimBoxColor.z = 1.0f;

	m_bTracking = FALSE;
	m_ptTracking.x = m_ptTracking.y = 0;
	m_ptTrackingScreen.x = m_ptTrackingScreen.y = 0;

	m_bCalcRadius = false;
	m_bCalcAndDraw = false;

	m_Scale = 1.0f;
	memset(m_pTrackers, 0, sizeof(m_pTrackers));

	m_pSetFOVDlg = NULL ;

	ResetLights();
}



CRenderWnd::~CRenderWnd()
{
}


BEGIN_MESSAGE_MAP(CRenderWnd, CWnd)
	//{{AFX_MSG_MAP(CRenderWnd)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_CHAR()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CRenderWnd::InitAnims(LTAnimTracker *pTrackers[NUM_ANIM_INFOS])
{
	DWORD i;

	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		m_DrawStruct.m_Times[i] = pTrackers[i];
	}
}

void CRenderWnd::ReleaseAllTextures()
{
	::ReleaseAllTextures( GetContext() );
}

BOOL CRenderWnd::SetTexture( TextureData *pTexture, DWORD nTextures )
{
	return  SetGLMTexture(GetContext(), pTexture, nTextures);
}

void CRenderWnd::SetBackgroundColor( COLORREF  ms_color )
{
	SetGLMBackgroundColor(GetContext(),ms_color);
}
	
// ------------------------------------------------------------------------
// Draw
// use gl to draw the model
// ------------------------------------------------------------------------
void CRenderWnd::Draw()
{
	static LTMatrix IdMat;
	IdMat.Identity();

	DrawStruct *pStruct;

	pStruct = &m_DrawStruct;

	// Setup with 2 lights.
	pStruct->m_ViewerPos = m_Camera.m_Position;
	pStruct->m_LookAt = m_Camera.m_LookAt;
	pStruct->m_LightPositions[0] = m_LightLocators[0].m_Location;
	pStruct->m_LightPositions[1] = m_LightLocators[1].m_Location;
	pStruct->m_LightColors[0].Init(255.0f, 255.0f, 255.0f);
	pStruct->m_LightColors[1].Init(128.0f, 128.0f, 128.0f);
	pStruct->m_nLights = 2;

	SetupViewingParameters((GLMContext*)m_hContext, pStruct );

 	DrawWorldCoordSys();
	
	// if the model exists 
	if (GetModel()  )
	{
		pStruct->m_SelectedPieces = m_SelectedPieces.GetArray();
		pStruct->m_SelectedNodes = m_SelectedNodes.GetArray();

		pStruct->m_bCalcRadius = m_bCalcRadius;
		pStruct->m_fModelRadius = 0.0f;
		pStruct->m_bCalcAndDraw = m_bCalcAndDraw;

		DrawModel (m_hContext, pStruct);

		m_fCurRadius = pStruct->m_fModelRadius;
	}
		
	SwapBuffers( (( GLMContext*)m_hContext)->m_hDC);

}

BOOL CRenderWnd::SetupTransformMaker(TransformMaker *pMaker)
{
	return m_DrawStruct.SetupTransformMaker(pMaker);
}

void CRenderWnd::ResetLocator()
{
	m_Camera.Reset();
	m_Camera.Update();
}

void CRenderWnd::ResetLights()
{
	m_LightLocators[0].Center();
	m_LightLocators[0].m_nDistance = 75.0f * m_Scale * 3.0f;
	m_LightLocators[0].m_nXZAngle = 30.0f;
	m_LightLocators[0].UpdateLocation();
	m_LightLocators[0].m_nYAngle = 30.0f;
	m_LightLocators[0].UpdateLocation();

	m_LightLocators[1].Center();
	m_LightLocators[1].m_nDistance = 75.0f * m_Scale * 3.0f;
	m_LightLocators[1].m_nXZAngle = -30.0f;
	m_LightLocators[1].UpdateLocation();
	m_LightLocators[1].m_nYAngle = -30.0f;
	m_LightLocators[1].UpdateLocation();
}


/////////////////////////////////////////////////////////////////////////////
// CRenderWnd message handlers

int CRenderWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_hContext = CreateGLMContext ((void*) GetSafeHwnd());
	if (!m_hContext)
	{
		MessageBox ("Error", "Could not create OpenGL context", MB_OK);
		return -1;
	}

	// set the context's permanent attribs.
	CRegMgr regMgr2;
	if (regMgr2.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "Render", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = sizeof(float)*3;
		float bgColor[3];
	
		regMgr2.Get("bgColor",bgColor , dwSize);

		SetGLMBackgroundColor( GetContext() , bgColor );	
	}

	return 0;
}

void CRenderWnd::OnDestroy() 
{
	DeleteGLMContext (m_hContext);

	CWnd::OnDestroy();
}

BOOL CRenderWnd::OnEraseBkgnd(CDC* pDC) 
{
	CRect rcClient;
	GetClientRect (rcClient);
	pDC->PatBlt (0, 0, rcClient.Width(), rcClient.Height(), BLACKNESS);
	return TRUE;
}

void CRenderWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// Delegate..
	if(m_pModelEditDlg && m_pModelEditDlg->HandleButtonDown(0, point))
		return;

	if (nFlags & MK_RBUTTON)
	{
		return;
	}

	if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && (!(GetAsyncKeyState(VK_CONTROL) & 0x8000)))
	{
		ResetLocator();
		ResetLights();
	}
	
	m_bTracking = TRUE;
	m_ptTracking = point;
	m_ptTrackingScreen = point;
	ClientToScreen (&m_ptTrackingScreen);
	SetCapture();
	ShowCursor (FALSE);

	CWnd::OnLButtonDown(nFlags, point);
}

void CRenderWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Delegate..
	if(m_pModelEditDlg && m_pModelEditDlg->HandleButtonUp(0))
		return;

	if (nFlags & MK_RBUTTON)
	{
		return;
	}
	
	if (m_bTracking)
	{
		m_bTracking = FALSE;
		ReleaseCapture();
		ShowCursor (TRUE);
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CRenderWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// Delegate..
	if(m_pModelEditDlg && m_pModelEditDlg->HandleButtonDown(2, point))
		return;

	if (nFlags & MK_LBUTTON)
	{
		return;
	}

	m_bTracking = TRUE;
	m_ptTracking = point;
	m_ptTrackingScreen = point;
	ClientToScreen (&m_ptTrackingScreen);
	SetCapture();
	ShowCursor (FALSE);

	CWnd::OnRButtonDown(nFlags, point);
}

void CRenderWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// Delegate..
	if(m_pModelEditDlg && m_pModelEditDlg->HandleButtonUp(2))
		return;

	if (nFlags & MK_LBUTTON)
	{
		return;
	}

	if (m_bTracking)
	{
		m_bTracking = FALSE;
		ReleaseCapture();
		ShowCursor (TRUE);
	}

	CWnd::OnRButtonUp(nFlags, point);
}

void CRenderWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	float xzAdd, yAdd, distanceAdd;
	static float turnSpeed = 0.3f;
	static float distanceSpeed = 0.3f;

	static DWORD lastTick = 0;
	DWORD holdTime;
	float timeAdjust;

	if ((point != m_ptTracking) && (m_bTracking))
	{
		if (!lastTick)
			lastTick = timeGetTime();
		holdTime = timeGetTime();
		timeAdjust = ____max((float)(holdTime - lastTick) / 10.0f, 1.0f); // t.f (don'task)
		lastTick = holdTime;
	}

	bool alterCamera = false;
	bool alterLight[2] = {false,false};

	if( m_bCameraFollow )
	{
		alterCamera = true;
		alterLight[0] = alterLight[1] = true;
	}
	else if( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
	{
		if( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
			alterLight[1] = true;
		else
			alterLight[0] = true;
	}
	else
	{
		alterCamera = true;
	}

	if (m_bTracking && (nFlags & MK_LBUTTON) && (nFlags & MK_RBUTTON) && point != m_ptTracking)
	{
		xzAdd = m_Scale * (float)(m_ptTracking.x - point.x) / timeAdjust;
		yAdd = m_Scale * (float)(m_ptTracking.y - point.y) / timeAdjust;

		m_Camera.m_Offset.x += xzAdd;
		m_Camera.m_Offset.y += yAdd;
		m_Camera.Update();
		
		SetCursorPos (m_ptTrackingScreen.x, m_ptTrackingScreen.y);
	}
	else if (m_bTracking && (nFlags & MK_LBUTTON) && point != m_ptTracking)
	{
		xzAdd = turnSpeed * (float)(m_ptTracking.x - point.x);
		yAdd = turnSpeed * (float)(point.y - m_ptTracking.y);

		if( alterCamera )
		{
			m_Camera.m_fXZAngle += xzAdd;
			m_Camera.m_fYAngle += yAdd;
			m_Camera.Update();
		}

		if( alterLight[0] )
		{
			m_LightLocators[0].m_nXZAngle += xzAdd;
			m_LightLocators[0].m_nYAngle += yAdd;
			m_LightLocators[0].UpdateLocation();
		}

		if( alterLight[1] )
		{
			m_LightLocators[1].m_nXZAngle += xzAdd;
			m_LightLocators[1].m_nYAngle += yAdd;
			m_LightLocators[1].UpdateLocation();
		}
		
		SetCursorPos (m_ptTrackingScreen.x, m_ptTrackingScreen.y);
	}
	else if (m_bTracking && (nFlags & MK_RBUTTON) && point.y != m_ptTracking.y)
	{
		distanceAdd = distanceSpeed * (float)(point.y - m_ptTracking.y) / timeAdjust;
		distanceAdd *= m_Scale;

		if( alterCamera )
		{
			m_Camera.m_Position.z += distanceAdd;
			m_Camera.Update();
		}

		if( alterLight[0] )
		{
			m_LightLocators[0].m_nDistance += distanceAdd;
			m_LightLocators[0].UpdateLocation();
		}

		if( alterLight[1] )
		{
			m_LightLocators[1].m_nDistance += distanceAdd;
			m_LightLocators[1].UpdateLocation();
		}

		SetCursorPos (m_ptTrackingScreen.x, m_ptTrackingScreen.y);
	}

	CWnd::OnMouseMove(nFlags, point);
}


void CRenderWnd::OnMButtonDown(UINT nFlags, CPoint point) 
{
	// Delegate..
	if(m_pModelEditDlg && m_pModelEditDlg->HandleButtonDown(1, point))
		return;

	CWnd::OnMButtonDown(nFlags, point);
}

void CRenderWnd::OnMButtonUp(UINT nFlags, CPoint point) 
{
	// Delegate..
	if(m_pModelEditDlg && m_pModelEditDlg->HandleButtonUp(1))
		return;
	
	CWnd::OnMButtonUp(nFlags, point);
}

void CRenderWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDblClk(nFlags, point);
}

// ------------------------------------------------------------------------
// Create/Open the FOV dialog window.
// ------------------------------------------------------------------------
void CRenderWnd::OpenFOVDlg()
{
	if( m_pSetFOVDlg == NULL )
	{
		m_pSetFOVDlg = new CSetFOVDlg( this );
		m_pSetFOVDlg->Create(CSetFOVDlg::IDD,this);
		m_pSetFOVDlg->SetBaseFOV((int) CModelEditDlg::CalcFOV() );
		m_pSetFOVDlg->ShowWindow(SW_SHOW);
	}else
	{
		m_pSetFOVDlg->SetBaseFOV((int) CModelEditDlg::CalcFOV() );
		m_pSetFOVDlg->ShowWindow(SW_SHOW);
	}
}

// ------------------------------------------------------------------------
// this is called by my SetFOVDlg 
// ------------------------------------------------------------------------
void CRenderWnd::SetFOV( int val )
{
	CModelEditDlg::SetFOV((float)val);
	m_DrawStruct.m_FOV = (float)val;
}
