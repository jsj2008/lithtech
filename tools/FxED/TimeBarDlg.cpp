// TimeBarDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "TimeBarDlg.h"
#include "TrackWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TEXT_MIN_SPACING		5

/////////////////////////////////////////////////////////////////////////////
// CTimeBarDlg dialog


CTimeBarDlg::CTimeBarDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTimeBarDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTimeBarDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pMemDC	 = NULL;
	m_pBitmap	 = NULL;
	m_pOldBitmap = NULL;

	m_nTimeAnchor = 0;
	m_nTotalTime  = 10000;
	m_fScale	  = 1.0f;
}


void CTimeBarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimeBarDlg)
	DDX_Control(pDX, IDC_TIMESCROLL, m_timeScroll);
	DDX_Control(pDX, IDC_TIMESCALE, m_timeScale);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		// Initialise the time scroll and time 

		CSliderCtrl *pTimeScale  = (CSliderCtrl *)GetDlgItem(IDC_TIMESCALE);
		CSliderCtrl *pTimeScroll = (CSliderCtrl *)GetDlgItem(IDC_TIMESCROLL);

		pTimeScale->SetRangeMin(0);
		pTimeScale->SetRangeMax(50);

		pTimeScroll->SetRangeMin(0);
		pTimeScroll->SetRangeMax(100);

		SetTotalTime(m_nTotalTime);

		CTrackWnd *pWnd = (CTrackWnd *)GetParent();
		if (pWnd) SetTotalTime(pWnd->GetPhase()->GetPhaseLength());
	}
}


BEGIN_MESSAGE_MAP(CTimeBarDlg, CDialog)
	//{{AFX_MSG_MAP(CTimeBarDlg)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeBarDlg message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnSize()
//
//   PURPOSE  : Handles WM_SIZE
//
//------------------------------------------------------------------

void CTimeBarDlg::OnSize(UINT nType, int cx, int cy) 
{
	CRect rcClient;
	CRect rcWnd;

	CDialog::OnSize(nType, cx, cy);

	GetClientRect(&rcClient);
	
	// Resize the scroll bar
	
	CSliderCtrl *pSliderCtrl = (CSliderCtrl *)GetDlgItem(IDC_TIMESCROLL);
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_TIMEPROP);
	CStatic *pTimeBar = (CStatic *)GetDlgItem(IDC_TIMEBAR);

	if (pSliderCtrl)
	{
		pSliderCtrl->GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		pSliderCtrl->MoveWindow(0, rcWnd.top, rcClient.Width(), rcWnd.Height());
	}

	if (pStatic)
	{
		pStatic->GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		pStatic->MoveWindow(0, rcWnd.top, rcClient.Width(), rcWnd.Height());
	}

	if (pTimeBar)
	{
		pTimeBar->GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		pTimeBar->MoveWindow(0, rcWnd.top, rcClient.Width(), rcWnd.Height());

		CRect rcTimeBar;
		pTimeBar->GetWindowRect(&rcTimeBar);

		if (m_pMemDC)
		{
			m_pMemDC->SelectObject(m_pOldBitmap);
			delete m_pMemDC;
			delete m_pBitmap;
		}

		m_pMemDC = new CDC;
		m_pMemDC->CreateCompatibleDC(NULL);

		CDC *pDC = GetDC();

		m_pBitmap = new CBitmap;
		m_pBitmap->CreateCompatibleBitmap(pDC, rcTimeBar.Width(), rcTimeBar.Height());

		m_pOldBitmap = m_pMemDC->SelectObject(m_pBitmap);

		m_pMemDC->SetBkMode(TRANSPARENT);

		ReleaseDC(pDC);
	}
	
	if (pStatic) SetTimePixelRatio();
}

//------------------------------------------------------------------
//
//   FUNCTION : DrawTimeBar()
//
//   PURPOSE  : Draws the time bar out
//
//------------------------------------------------------------------

void CTimeBarDlg::DrawTimeBar(CDC *pDC)
{
	if (!m_pMemDC) return;

	CStatic *pTimeBar = (CStatic *)GetDlgItem(IDC_TIMEBAR);
	if (!pTimeBar) return;

	CRect rcTimeBar;
	pTimeBar->GetWindowRect(&rcTimeBar);
	ScreenToClient(&rcTimeBar);

	CRect rcClient;
	pTimeBar->GetClientRect(&rcClient);

	// Draw the background
	
	CBrush brGray(RGB(220, 220, 220));
	m_pMemDC->FillRect(&rcClient, &brGray);

	// Draw the tick marks, one every 100 milliseconds

	int nTicks = (m_nTotalTime / 100) + 1;
	int nCurTime = (m_nTimeAnchor / 100) * 100;

	int nLastTextPos = -50;
	
	for (int i = 0; i < nTicks; i ++)
	{
		BOOL bBigCheck = FALSE;
		if (!(nCurTime % 1000)) bBigCheck = TRUE;

		int xPos = TimeToPos(nCurTime) - TimeToPos(m_nTimeAnchor);

		m_pMemDC->MoveTo(xPos, 0);
		m_pMemDC->LineTo(xPos, bBigCheck ? 10 : 5);

		if (bBigCheck)
		{
			// Draw the time

			char sTmp[256];
			sprintf(sTmp, "%d", nCurTime / 1000);

			CSize szText = m_pMemDC->GetTextExtent(sTmp);

			//make sure to offset our beginning time position
			if(xPos - (szText.cx / 2) > nLastTextPos)
			{
				m_pMemDC->TextOut(xPos - (szText.cx / 2), 10, sTmp, strlen(sTmp));

				nLastTextPos = xPos + (szText.cx / 2) + TEXT_MIN_SPACING;
			}
		}

		nCurTime += 100;
	}
		
	// Blit the time bar

	pDC->BitBlt(0, 0, rcTimeBar.Width(), rcTimeBar.Height(), m_pMemDC, 0, 0, SRCCOPY);

	// And redraw the parent

	RedrawParent();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnPaint()
//
//   PURPOSE  : Handles WM_PAINT
//
//------------------------------------------------------------------

void CTimeBarDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawTimeBar(&dc);
}

//------------------------------------------------------------------
//
//   FUNCTION : SetTimePixelRatio()
//
//   PURPOSE  : Sets the ratio for time per pixel
//
//------------------------------------------------------------------

void CTimeBarDlg::SetTimePixelRatio()
{
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_TIMEBAR);
	if (!pStatic) return;

	CRect rcWnd;
	pStatic->GetClientRect(&rcWnd);

	m_fTmPxRatio = ((float)rcWnd.Width() / (float)m_nTotalTime) * m_fScale;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnHScroll()
//
//   PURPOSE  : Handles WM_HSCROLL
//
//------------------------------------------------------------------

void CTimeBarDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl *pTimeScale  = (CSliderCtrl *)GetDlgItem(IDC_TIMESCALE);
	CSliderCtrl *pTimeScroll = (CSliderCtrl *)GetDlgItem(IDC_TIMESCROLL);

	if (pScrollBar == (CScrollBar *)pTimeScale)
	{
		// Update the scale....

		int nScale = (int)(float)(50.0f * ((float)pTimeScale->GetPos() / 50.0f));
		m_fScale = 1.0f + (0.2f * (float)nScale);

		SetTimePixelRatio();

		CDC *pDC = GetDC();
		DrawTimeBar(pDC);
		ReleaseDC(pDC);
	}

	if (pScrollBar == (CScrollBar *)pTimeScroll)
	{
		CRect rcWnd;
		GetWindowRect(&rcWnd);
		
		// Update the time anchor

		int nUnseenDist = m_nTotalTime - PosToTime(rcWnd.Width());

		int nPos = pTimeScroll->GetPos();

		float fTime = ((float)nUnseenDist / (float)pTimeScroll->GetRangeMax()) * (float)nPos;
		m_nTimeAnchor = (int)fTime;

		SetTimePixelRatio();

		CDC *pDC = GetDC();
		DrawTimeBar(pDC);
		ReleaseDC(pDC);
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

//------------------------------------------------------------------
//
//   FUNCTION : RedrawParent()
//
//   PURPOSE  : Redraws parent window
//
//------------------------------------------------------------------

void CTimeBarDlg::RedrawParent()
{
	GetParent()->InvalidateRect(NULL, TRUE);
}

//------------------------------------------------------------------
//
//   FUNCTION : DestroyWindow()
//
//   PURPOSE  : Destroys the time bar dialog window
//
//------------------------------------------------------------------

BOOL CTimeBarDlg::DestroyWindow() 
{
	return CDialog::DestroyWindow();
}

//------------------------------------------------------------------
//
//   FUNCTION : PostNCDestroy()
//
//   PURPOSE  : Called after the window is destroyed
//
//------------------------------------------------------------------

void CTimeBarDlg::PostNcDestroy() 
{
	if (m_pMemDC)
	{
		m_pMemDC->SelectObject(m_pOldBitmap);
		delete m_pMemDC;
		delete m_pBitmap;
	}
	
	CDialog::PostNcDestroy();
}

//------------------------------------------------------------------
//
//   FUNCTION : SetTotalTime()
//
//   PURPOSE  : Sets the total time of the current phase
//
//------------------------------------------------------------------

void CTimeBarDlg::SetTotalTime(int nTotalTime)
{
	m_nTotalTime = nTotalTime;
	SetTimePixelRatio();

	CDC *pDC = GetDC();
	DrawTimeBar(pDC);
	ReleaseDC(pDC);

	GetParent()->Invalidate();
}