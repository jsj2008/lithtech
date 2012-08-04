// SequencerWnd.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "SequencerWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSequencerWnd

//------------------------------------------------------------------
//
//   FUNCTION : CSequencerWnd()
//
//   PURPOSE  : Standard constructor
//
//------------------------------------------------------------------

CSequencerWnd::CSequencerWnd()
{
	m_pMemDC	 = NULL;
	m_pBitmap	 = NULL;
	m_pOldBitmap = NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CSequencerWnd()
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CSequencerWnd::~CSequencerWnd()
{
	if (m_pMemDC)
	{
		m_pMemDC->SelectObject(m_pOldBitmap);

		delete m_pMemDC;
		delete m_pBitmap;
	}

	m_pMemDC     = NULL;
	m_pOldBitmap = NULL;
}


BEGIN_MESSAGE_MAP(CSequencerWnd, CWnd)
	//{{AFX_MSG_MAP(CSequencerWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSequencerWnd message handlers

//------------------------------------------------------------------
//
//   FUNCTION : PreCreateWindow()
//
//   PURPOSE  : Called before window is created
//
//------------------------------------------------------------------

BOOL CSequencerWnd::PreCreateWindow(CREATESTRUCT& cs) 
{	
	return CWnd::PreCreateWindow(cs);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnCreate()
//
//   PURPOSE  : Called when window is created
//
//------------------------------------------------------------------

int CSequencerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSize()
//
//   PURPOSE  : Handles WM_SIZE
//
//------------------------------------------------------------------

void CSequencerWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	// Recreate memory dc and bitmap

	m_cx = cx;
	m_cy = cy;
	
	if (cx && cy)
	{
		if (m_pMemDC)
		{
			// Destroy the old memory DC

			m_pMemDC->SelectObject(m_pOldBitmap);
			
			delete m_pMemDC;
			delete m_pBitmap;
		}

		// Create the new memory DC

		m_pMemDC = new CDC;
		m_pMemDC->CreateCompatibleDC(NULL);

		// Get a DC to the window
		
		CDC *pDC = GetDC();

		m_pBitmap = new CBitmap;
		m_pBitmap->CreateCompatibleBitmap(pDC, cx, cy);

		// Select the new bitmap into the DC
		
		m_pOldBitmap = m_pMemDC->SelectObject(m_pBitmap);

		// Release the DC

		ReleaseDC(pDC);
	}		
}

//------------------------------------------------------------------
//
//   FUNCTION : OnEraseBkgnd()
//
//   PURPOSE  : Handles WM_ERASEBKGND
//
//------------------------------------------------------------------

BOOL CSequencerWnd::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnPaint()
//
//   PURPOSE  : Handles WM_PAINT
//
//------------------------------------------------------------------

void CSequencerWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	DrawSequencerWnd(&dc);	
}

//------------------------------------------------------------------
//
//   FUNCTION : DrawSequencerWnd()
//
//   PURPOSE  : Draws the tracks, keys and buttons
//
//------------------------------------------------------------------

void CSequencerWnd::DrawSequencerWnd(CDC *pDC)
{
	// Clear the background

	m_pMemDC->FillSolidRect(0, 0, m_cx, m_cy, RGB(80, 80, 80));

	// Blit the memory DC

	pDC->BitBlt(0, 0, m_cx, m_cy, m_pMemDC, 0, 0, SRCCOPY);
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Handles post WM_CREATE initialisation
//
//------------------------------------------------------------------

BOOL CSequencerWnd::Init(CTimeBarDlg *pTimeBarDlg, CPhaseInfoDlg *pPhaseInfoDlg)
{
	m_pTimeBarDlg   = pTimeBarDlg;
	m_pPhaseInfoDlg = pPhaseInfoDlg;

	// Success !!

	return TRUE;
}
