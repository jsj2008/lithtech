// TrackWnd.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "TrackWnd.h"
#include "FxDlg.h"
#include "FxPropDlg.h"
#include "ColourKeyDlg.h"
#include "MotionKeysDlg.h"
#include "ScaleKeysDlg.h"
#include "ChooseKeyDlg.h"
#include "StringDlg.h"
#include "CopyDataDlg.h"
#include "IntDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//helper function that determines if a key is pressed or not
static bool IsKeyDown(DWORD nKey)
{
	return (GetAsyncKeyState(nKey) & 0x8000) ? true : false;
}

/////////////////////////////////////////////////////////////////////////////
// CTrackWnd

CTrackWnd::CTrackWnd()
{
	m_pMemDC	       = NULL;
	m_pBitmap	       = NULL;
	m_pOldBitmap       = NULL;
	m_nTrackHeight     = 36;
	m_pSelTrack	       = NULL;
	m_pSelKey	       = NULL;
	m_bTracking	       = FALSE;
	m_pCopyKey	       = NULL;
	m_yOffset	       = 0;
	m_bTrackingLinkKey = FALSE;
}

CTrackWnd::~CTrackWnd()
{
}


BEGIN_MESSAGE_MAP(CTrackWnd, CWnd)
	//{{AFX_MSG_MAP(CTrackWnd)
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_TRACK_ADDTRACK, OnTrackAddTrack)
	ON_COMMAND(ID_TRACK_INSERTTRACK, OnTrackInsertTrack)
	ON_COMMAND(ID_TRACK_ADDKEY, OnTrackAddKey)
	ON_COMMAND(ID_TRACK_NAMEKEY, OnTrackNameKey)
	ON_COMMAND(ID_TRACK_DELETEKEY, OnTrackDeleteKey)
	ON_COMMAND(ID_TRACK_CHANGEKEY, OnTrackChangeKey)
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_TRACK_EXPANDKEY, OnTrackExpandKey)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_TRACK_EDITKEY, OnTrackEditKey)
	ON_COMMAND(ID_TRACK_EDITCOLOURKEYS, OnTrackEditColourKeys)
	ON_COMMAND(ID_TRACK_EDITMOTIONKEYS, OnTrackEditMotionKeys)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_TRACK_EDITSCALEKEYS, OnTrackEditScaleKeys)
	ON_COMMAND(ID_TRACK_ADDFAVOURITEKEY, OnTrackAddFavouriteKey)
	ON_COMMAND(ID_TRACK_ADDKEYTOFAVOURITES, OnTrackAddKeyToFavourites)
	ON_WM_VSCROLL()
	ON_COMMAND(ID_TRACK_DELETETRACK, OnTrackDeleteTrack)
	ON_COMMAND(ID_TRACK_COPYMULTIKEYS, OnTrackCopyMultiKeys)
	ON_COMMAND(ID_TRACK_MAKESAMELENGTH, OnTrackMakeSameLength)
	ON_COMMAND(ID_TRACK_LINKKEY, OnTrackLinkKey)
	ON_COMMAND(ID_TRACK_UNLINKKEY, OnTrackUnlinkkey)
	ON_COMMAND(ID_TRACK_SETKEYLENGTH, OnTrackSetKeyLength)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTrackWnd message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnShowWindow()
//
//   PURPOSE  : Handles WM_SHOWWINDOW
//
//------------------------------------------------------------------

void CTrackWnd::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CWnd::OnShowWindow(bShow, nStatus);	
}

//------------------------------------------------------------------
//
//   FUNCTION : DrawKey()
//
//   PURPOSE  : Draws a key
//
//------------------------------------------------------------------

void CTrackWnd::DrawKey(CDC *pDC, CKey *pKey, CRect rcKey)
{
	m_pMemDC->DrawFrameControl(&rcKey, DFC_BUTTON, DFCS_BUTTONPUSH);

	// Draw the name of the FX

	CString sKey;

	if (pKey->GetFxRef())
	{
		sKey = pKey->GetFxRef()->m_sName;

		if( pKey->GetCustomName()[0] )
		{
			sKey += " - ";
			sKey += pKey->GetCustomName();
		}

		// Draw the icon...

//		if (rcKey.Width() > 50) m_pMemDC->DrawIcon(rcKey.left + 10, rcKey.top + (rcKey.Height() / 2) - 16, pKey->GetFxRef()->m_hIcon);
	}
	else
	{
		// No FX...

		sKey = "NULL FX";
	}

	CSize szText = m_pMemDC->GetTextExtent(sKey);
	if (szText.cx < rcKey.Width() - 5)
	{
		CRect rcTxt = rcKey;
//		if (pKey->GetFxRef()) rcTxt.left += 42;
		m_pMemDC->DrawText(sKey, rcTxt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	if (pKey->IsSelected())
	{
		// Draw focus rect

		int xRad = 4;
		int yRad = 4;

		CPen pnDash(PS_DOT, 1, RGB(0, 0, 0));
		CPen *pOldPen = m_pMemDC->SelectObject(&pnDash);

		CRect rcFocus = rcKey;

		rcFocus.left += xRad;
		rcFocus.right -= xRad;
		rcFocus.top += yRad;
		rcFocus.bottom -= yRad;

		if ((rcFocus.left < rcFocus.right) && (rcFocus.top < rcFocus.bottom))
		{
			m_pMemDC->MoveTo(rcFocus.left, rcFocus.top);
			m_pMemDC->LineTo(rcFocus.right, rcFocus.top);
			m_pMemDC->LineTo(rcFocus.right, rcFocus.bottom);
			m_pMemDC->LineTo(rcFocus.left, rcFocus.bottom);
			m_pMemDC->LineTo(rcFocus.left, rcFocus.top);
		}
		m_pMemDC->SelectObject(pOldPen);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : DrawTracks()
//
//   PURPOSE  : Draws the tracks
//
//------------------------------------------------------------------

void CTrackWnd::DrawTracks(CDC *pDC)
{
	CRect rcClient;
	CRect rcTrack;
	CRect rcSel;

	int i, yStart = -10, yEnd = 0;

	// Draw the tracks....

	CLinkListNode<CTrack *> *pNode = m_pPhase->GetTracks()->GetHead();

	CBrush brBlack(RGB(0, 0, 0));

	m_pMemDC->FillSolidRect(0, 0, 1000, 1000, RGB(100, 100, 100));

	i = 0;
	
	while (pNode)
	{
		yStart = m_nTrackHeight * i;
		yEnd   = yStart + m_nTrackHeight;

		rcTrack = CRect(-100, yStart - m_yOffset, m_rcTrackClient.Width() + 100, yEnd - m_yOffset);
		
		m_pMemDC->DrawFrameControl(&rcTrack, DFC_BUTTON, DFCS_PUSHED | DFCS_BUTTONPUSH);

		// Draw the keys....

		CLinkListNode<CKey *> *pKeyNode = pNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			CKey *pKey = pKeyNode->m_Data;

			int nStartPos = m_timeBarDlg.TimeToPos(pKey->GetStartTime() - m_timeBarDlg.GetTimeAnchor());
			int nEndPos   = m_timeBarDlg.TimeToPos(pKey->GetEndTime() - m_timeBarDlg.GetTimeAnchor());

			CRect rcKey(nStartPos, rcTrack.top, nEndPos, rcTrack.bottom);

			// Draw the key

			DrawKey(m_pMemDC, pKey, rcKey);

			if (pKey->IsSelected()) rcSel = rcKey;

			pKeyNode = pKeyNode->m_pNext;
		}

		pNode = pNode->m_pNext;
		
		i ++;
	}

	rcTrack = CRect(-100, yEnd, m_rcTrackClient.Width() + 100, yEnd + 1000);

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{	
			if (pKeyNode->m_Data->IsSelected())
			{
				CBrush brGreen(RGB(255, 0, 0));
				CBrush *pOldBrush = m_pMemDC->SelectObject(&brGreen);

				// Draw the time lines down to the bottom of the screen

				rcSel = GetKeyRect(m_pPhase->GetTracks()->Get(pKeyNode->m_Data->GetTrack()), pKeyNode->m_Data);

				m_pMemDC->MoveTo(rcSel.left, rcSel.top - m_rcTrack.top);
				m_pMemDC->LineTo(rcSel.left, m_rcTrack.Height());

				m_pMemDC->MoveTo(rcSel.right, rcSel.top - m_rcTrack.top);
				m_pMemDC->LineTo(rcSel.right, m_rcTrack.Height());

				int nTriSize = 5;
				
				POINT ptTri[3];
				ptTri[0].x = rcSel.left - nTriSize;
				ptTri[0].y = m_rcTrack.Height() - nTriSize;

				ptTri[1].x = rcSel.left + nTriSize;
				ptTri[1].y = m_rcTrack.Height() - nTriSize;

				ptTri[2].x = rcSel.left;
				ptTri[2].y = m_rcTrack.Height();

				m_pMemDC->Polygon(ptTri, 3);

				ptTri[0].x = rcSel.right - nTriSize;
				ptTri[0].y = m_rcTrack.Height() - nTriSize;

				ptTri[1].x = rcSel.right + nTriSize;
				ptTri[1].y = m_rcTrack.Height() - nTriSize;

				ptTri[2].x = rcSel.right;
				ptTri[2].y = m_rcTrack.Height();

				m_pMemDC->Polygon(ptTri, 3);

				m_pMemDC->SelectObject(pOldBrush);
			}

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	// Draw the copy key if neccessary

	if (m_pCopyKey)
	{
		CRect rcKey = m_rcCopyKey;

		rcKey.left   += m_ptLast.x;
		rcKey.right  += m_ptLast.x;
		rcKey.top    += m_ptLast.y - m_rcTrack.top;
		rcKey.bottom += m_ptLast.y - m_rcTrack.top;

		DrawKey(m_pMemDC, m_pCopyKey, rcKey);
	}

	// Draw the cast point if neccessary

	if (m_pSpell->GetPhase(0) == m_pPhase)
	{
		CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
		
		int xPos = m_timeBarDlg.TimeToPos(pApp->GetCastAnimTime(m_pSpell->GetCastSpeed()) - m_timeBarDlg.GetTimeAnchor());

		m_pMemDC->MoveTo(xPos, 0);
		m_pMemDC->LineTo(xPos, 1000);

		int nTriSize = 5;

		CBrush brBlack(RGB(0, 0, 0));
		CBrush *pOldBrush = m_pMemDC->SelectObject(&brBlack);

		POINT ptTri[3];
		ptTri[0].x = xPos - nTriSize;
		ptTri[0].y = 0;

		ptTri[1].x = xPos + nTriSize;
		ptTri[1].y = 0;

		ptTri[2].x = xPos;
		ptTri[2].y = nTriSize;

		m_pMemDC->Polygon(ptTri, 3);

		ptTri[0].x = xPos - nTriSize;
		ptTri[0].y = m_rcTrack.Height() - 1;

		ptTri[1].x = xPos + nTriSize;
		ptTri[1].y = m_rcTrack.Height() - 1;

		ptTri[2].x = xPos;
		ptTri[2].y = (m_rcTrack.Height() - 1) - nTriSize;

		m_pMemDC->Polygon(ptTri, 3);

		m_pMemDC->SetTextColor(RGB(255, 0, 0));
		m_pMemDC->TextOut(xPos + 5, nTriSize + 5, "Active");
		m_pMemDC->SetTextColor(RGB(50, 50, 50));

		m_pMemDC->SelectObject(pOldBrush);
	}

	// Draw any links....

	pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			if (pKeyNode->m_Data->IsLinked())
			{
				// Draw the link

				CKey *pLinkedKey = m_pPhase->GetKeyByID(pKeyNode->m_Data->GetLinkedID());

				if (pLinkedKey)
				{
					CRect rcSrc = GetKeyRect(pTrackNode->m_Data, pKeyNode->m_Data);
					CRect rcDst = GetKeyRect(m_pPhase->GetTracks()->Get(pLinkedKey->GetTrack()), pLinkedKey);

					rcSrc.top -= m_rcTrack.top;
					rcSrc.bottom -= m_rcTrack.top;

					rcDst.top -= m_rcTrack.top;
					rcDst.bottom -= m_rcTrack.top;
					
					m_pMemDC->MoveTo((rcSrc.left + rcSrc.right) >> 1,
									 (rcSrc.top + rcSrc.bottom) >> 1);

					m_pMemDC->LineTo((rcDst.left + rcDst.right) >> 1,
									 (rcDst.top + rcDst.bottom) >> 1);

					int nSize = 5;
					CPoint ptDst = CPoint((rcDst.left + rcDst.right) >> 1,
										  (rcDst.top + rcDst.bottom) >> 1);
					m_pMemDC->Ellipse(CRect(ptDst.x - nSize, ptDst.y - nSize, ptDst.x + nSize + 1, ptDst.y + nSize + 1));
				}
			}

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	// Draw the linked tracking line

	if (m_bTrackingLinkKey)
	{
		m_pMemDC->MoveTo(m_ptAnchorLink);
		m_pMemDC->LineTo(m_ptAnchorCur);
		int nSize = 5;
		m_pMemDC->Ellipse(CRect(m_ptAnchorCur.x - nSize, m_ptAnchorCur.y - nSize, m_ptAnchorCur.x + nSize + 1, m_ptAnchorCur.y + nSize + 1));
	}

	// Blit the offscreen.....

	pDC->BitBlt(0, m_rcTrack.top, m_rcTrack.Width(), m_rcTrack.Height(), m_pMemDC, 0, 0, SRCCOPY);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnPaint()
//
//   PURPOSE  : Handles WM_PAINT
//
//------------------------------------------------------------------

void CTrackWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	DrawTracks(&dc);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnCreate()
//
//   PURPOSE  : Handles WM_CREATE
//
//------------------------------------------------------------------

int CTrackWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Create the time bar dialog

	m_timeBarDlg.Create(IDD_TIMEBAR, this);
	m_timeBarDlg.ShowWindow(SW_SHOW);

	// Create the phase info dialog

	m_phaseInfoDlg.SetTimeBar(&m_timeBarDlg);
	m_phaseInfoDlg.Create(IDD_PHASEINFO, this);
	m_phaseInfoDlg.ShowWindow(SW_SHOW);

	// Get the width of a scroll bar
	
	m_nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);

	// Create the scroll bar

	m_scrollBar.Create(WS_VISIBLE | SBS_VERT, CRect(0, 0, 0, 0), this, 5678);
	m_scrollBar.SetScrollRange(0, 200);
	
	return 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : PreCreateWindow()
//
//   PURPOSE  : Called before window creation
//
//------------------------------------------------------------------

BOOL CTrackWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= WS_CLIPCHILDREN;

	return CWnd::PreCreateWindow(cs);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSize()
//
//   PURPOSE  : Handles WM_SIZE
//
//------------------------------------------------------------------

void CTrackWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rcWnd;
	CRect rcClient;
	CRect rcPhaseInfo;
	CRect rcTimeBar;

	CWnd::OnSize(nType, cx, cy);

	if ((!cx) && (!cy)) return;
	
	GetClientRect(&rcClient);

	m_phaseInfoDlg.GetWindowRect(&rcPhaseInfo);
	m_timeBarDlg.GetWindowRect(&rcTimeBar);

	// Resize the phase info dialog

	m_phaseInfoDlg.MoveWindow(0, 0, rcClient.Width(), rcPhaseInfo.Height());

	// Resize the time bar dialog

	m_timeBarDlg.MoveWindow(0, rcClient.Height() - rcTimeBar.Height(), rcClient.Width() - m_nScrollWidth, rcTimeBar.Height());

	// Resize the scroll bar

	m_scrollBar.MoveWindow(rcClient.Width() - m_nScrollWidth, rcPhaseInfo.Height(), m_nScrollWidth, rcClient.Height() - rcPhaseInfo.Height());

	// Compute the track window rect

	m_phaseInfoDlg.GetWindowRect(&rcPhaseInfo);
	m_timeBarDlg.GetWindowRect(&rcTimeBar);

	ScreenToClient(&rcPhaseInfo);
	ScreenToClient(&rcTimeBar);

	m_rcTrack.left   = 0;
	m_rcTrack.top    = rcPhaseInfo.bottom;
	m_rcTrack.right  = rcClient.right;
	m_rcTrack.bottom = rcClient.Height() - rcTimeBar.Height();

	m_rcTrackClient = m_rcTrack;

	m_rcTrackClient.top = 0;
	m_rcTrackClient.bottom -= m_rcTrack.Height();

	// Setup the memory dc

	if (m_pMemDC)
	{
		m_pMemDC->SelectObject(m_pOldBitmap);
		delete m_pMemDC;
		delete m_pBitmap;
	}

	m_pMemDC = new CDC;
	m_pMemDC->CreateCompatibleDC(NULL);

	m_pMemDC->SetBkMode(TRANSPARENT);

	// Set up the initial coordinate mappings of the DC

	m_pMemDC->SetMapMode(MM_TEXT);
	m_pMemDC->SetViewportExt(m_rcTrack.Width(), m_rcTrack.Height());

	CDC *pDC = GetDC();

	m_pBitmap = new CBitmap;
	m_pBitmap->CreateCompatibleBitmap(pDC, m_rcTrack.Width(), m_rcTrack.Height());

	m_pOldBitmap = m_pMemDC->SelectObject(m_pBitmap);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnEraseBkgnd()
//
//   PURPOSE  : Handles WM_ERASEBKGND
//
//------------------------------------------------------------------

BOOL CTrackWnd::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRButtonUp()
//
//   PURPOSE  : Handles right mouse clicking
//
//------------------------------------------------------------------

void CTrackWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{		
	CMenu rbMenu;
	rbMenu.CreatePopupMenu();

	CMenu rbKeyMenu;
	rbKeyMenu.CreatePopupMenu();

	CMenu rbMultipleKeyMenu;
	rbMultipleKeyMenu.CreatePopupMenu();

	CMenu rbTrackMenu;
	rbTrackMenu.CreatePopupMenu();

	m_pSelTrack = GetTrackByPos(point.y);
	if (m_pSelTrack) SelectKey(GetKeyByPos(m_pSelTrack, point.x));

	CDC *pDC = GetDC();
	DrawTracks(pDC);
	ReleaseDC(pDC);

	if (!IsKeyDown(VK_CONTROL)) DeselectAllKeys();

	m_ptRbClick	= point;

	// Key menu options

	int nCount = 0;
	
	CKey *pKey = NULL;
	if (m_pSelTrack)
	{
		pKey = GetKeyByPos(m_pSelTrack, point.x);
		if (pKey)
		{		
			SelectKey(pKey);

			// Redraw

			InvalidateRect(NULL, TRUE);
		}

		if (pKey)
		{
			if (pKey->GetFxRef())
			{
				rbMenu.AppendMenu(MF_POPUP, (DWORD)rbKeyMenu.GetSafeHmenu(), "&Key");
				rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_EDITKEY, "Edit &Key");
				if (pKey->GetFxRef()->m_dwType & FX_NEEDCOLOURKEY) rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_EDITCOLOURKEYS, "Edit &Colour Keys");
				if (pKey->GetFxRef()->m_dwType & FX_NEEDSCALEKEY) rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_EDITSCALEKEYS, "Edit &Scale Keys");
			}
			
			rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_CHANGEKEY, "&Change Key");
			rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_NAMEKEY, "&Name Key");
			if (!pKey->IsLinked()) rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_EXPANDKEY, "E&xpand Key");
			if (!pKey->IsLinked()) rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_SETKEYLENGTH, "&Set Key Length");
			rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_LINKKEY, "&Motion Link To Key");
			if ((pKey->IsLinked()) || (IsLinkedTo(pKey))) rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_UNLINKKEY, "&Unlink Key");
			rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_ADDKEYTOFAVOURITES, "&Add To Favourites");

			if (NumSelected() > 1) 
			{
				if (rbMenu.AppendMenu(MF_POPUP, (DWORD)rbMultipleKeyMenu.GetSafeHmenu(), "&Multiple Key"))
				{
					rbMultipleKeyMenu.AppendMenu(MF_STRING, ID_TRACK_COPYMULTIKEYS, "&Copy Multiple Keys To New Tracks");
					rbMultipleKeyMenu.AppendMenu(MF_STRING, ID_TRACK_MAKESAMELENGTH, "&Make Same Length");
				}
			}

			rbKeyMenu.AppendMenu(MF_SEPARATOR, 0, "");
			rbKeyMenu.AppendMenu(MF_STRING, ID_TRACK_DELETEKEY, "&Delete Key");
		}
		else
		{
			rbTrackMenu.AppendMenu(MF_STRING, ID_TRACK_ADDKEY, "&Add Key");
			rbTrackMenu.AppendMenu(MF_STRING, ID_TRACK_ADDFAVOURITEKEY, "Add &Favourite Key");
		}
	}

	// Track menu options

	rbMenu.AppendMenu(MF_POPUP, (DWORD)rbTrackMenu.GetSafeHmenu(), "&Track");

	if (pKey) rbTrackMenu.AppendMenu(MF_SEPARATOR, 0, "");
	if (m_pSelTrack) 
	{
		rbTrackMenu.AppendMenu(MF_STRING, ID_TRACK_INSERTTRACK, "&Insert Track");
		rbTrackMenu.AppendMenu(MF_STRING, ID_TRACK_DELETETRACK, "&Delete Track");
	}

	rbTrackMenu.AppendMenu(MF_STRING, ID_TRACK_ADDTRACK, "&Add Track");

	CPoint ptCursor = point;
	ClientToScreen(&ptCursor);
	rbMenu.TrackPopupMenu(TPM_CENTERALIGN | TPM_RIGHTBUTTON, ptCursor.x, ptCursor.y, this, NULL);		

	pDC = GetDC();
	DrawTracks(pDC);
	ReleaseDC(pDC);
	
	CWnd::OnRButtonUp(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackAddTrack()
//
//   PURPOSE  : Adds a new track to this phase
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackAddTrack() 
{
	// Create a new track
	
	m_pPhase->AddTrack();

	// Redraw
	
	InvalidateRect(NULL, TRUE);

	// Refresh the phase info dialog

	m_phaseInfoDlg.Refresh();
}

//------------------------------------------------------------------
//
//   FUNCTION : GetTrackByPos()
//
//   PURPOSE  : Returns the track for a given pos
//
//------------------------------------------------------------------

CTrack* CTrackWnd::GetTrackByPos(int pos)
{
	pos -= m_rcTrack.top;
	pos += m_yOffset;

	int nTracks = m_pPhase->GetTracks()->GetSize();

	if (nTracks)
	{
		pos /= m_nTrackHeight;

		if (pos < (int)m_pPhase->GetTracks()->GetSize())
		{
			return m_pPhase->GetTracks()->Get(pos);
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : GetKeyByPos()
//
//   PURPOSE  : Returns a key for a given position
//
//------------------------------------------------------------------

CKey* CTrackWnd::GetKeyByPos(CTrack *pTrack, int pos)
{
	int nAnchor = m_timeBarDlg.GetTimeAnchor();
	int nTime   = m_timeBarDlg.PosToTime(pos) + nAnchor;

	CLinkListNode<CKey *> *pNode = pTrack->GetKeys()->GetHead();

	while (pNode)
	{
		int tmStart = pNode->m_Data->GetStartTime();
		int tmEnd   = pNode->m_Data->GetEndTime();

		if (nTime >= tmStart && nTime < tmEnd) return pNode->m_Data;

		pNode = pNode->m_pNext;
	}

	// Failure

	return NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackInsertTrack()
//
//   PURPOSE  : Inserts a track
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackInsertTrack() 
{
	if (!m_pSelTrack) return;
	
	CTrack *pTrack = new CTrack;
	
	CLinkListNode<CTrack *> *pNode = m_pPhase->GetTracks()->GetHead();
	
	while (pNode)
	{
		if (pNode->m_Data == m_pSelTrack)
		{
			m_pPhase->GetTracks()->InsertAfter(pNode, pTrack);


			// Get the track after the new track, if it exists...

			pNode = pNode->m_pNext->m_pNext;
			while( pNode )
			{
				CLinkListNode<CKey *> *pKeyNode = pNode->GetData()->GetKeys()->GetHead();

				while( pKeyNode )
				{
					CKey *pKey = pKeyNode->m_Data;
					pKey->m_nTrack++;

					pKeyNode = pKeyNode->m_pNext;
				}

				pNode = pNode->m_pNext;
			}

			// Redraw
	
			InvalidateRect(NULL, TRUE);

			// Refresh the phase info dialog

			m_phaseInfoDlg.Refresh();

			CDC *pDC = GetDC();
			DrawTracks(pDC);
			ReleaseDC(pDC);
			
			return;
		}
		
		pNode = pNode->m_pNext;
	}	
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackAddKey()
//
//   PURPOSE  : Adds a key
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackAddKey() 
{
	if (!m_pSelTrack) return;

	CFxDlg dlg;

	if (dlg.DoModal() != IDOK) return;
	if (!dlg.m_pFxRef) return;

	CKey *pKey = new CKey;
	if (!pKey) return;

	pKey->Init();

	// Setup the parameters

	int nTrack	   = m_pPhase->GetTracks()->GetIndex(m_pSelTrack);
	int nTimeClick = m_timeBarDlg.GetTimeAnchor() + m_timeBarDlg.PosToTime(m_ptRbClick.x);
	
	pKey->SetTrack(nTrack);
	pKey->SetStartTime(nTimeClick - 1000);
	pKey->SetEndTime(nTimeClick + 1000);

	// Setup new reference
	
	pKey->SetFxRef(dlg.m_pFxRef);

	// Get a unique ID for the key

	pKey->SetID(m_pPhase->GetUniqueID());

	// Add it to the list

	m_pSelTrack->GetKeys()->AddTail(pKey);

	// And arrange correctly

	m_pSelTrack->ArrangeKeys(pKey);

	// Redraw

	InvalidateRect(NULL, TRUE);

	// Refresh the phase info dialog

	m_phaseInfoDlg.Refresh();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTrackWnd::OnTrackNameKey
//
//  PURPOSE:	Adds a custom name to the key so we know what it is.
//
// ----------------------------------------------------------------------- //

void CTrackWnd::OnTrackNameKey( )
{
	if (!m_pSelKey) return;

	CStringDlg dlg("Enter Custom Name");

	dlg.m_sText = m_pSelKey->GetCustomName();
	if (dlg.DoModal() == IDOK)
	{
		if( strlen( dlg.m_sText ) < 31 )
		{
			char szName[32] = {0};
			strcpy( szName, dlg.m_sText );
			m_pSelKey->SetCustomName( strtok(szName, " " ));
		}
	}

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackDeleteKey()
//
//   PURPOSE  : Deletes a key
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackDeleteKey() 
{
	if (!m_pSelTrack) return;
	if (!m_pSelKey) return;

	// Delete the key

	m_pSelTrack->GetKeys()->Remove(m_pSelKey);
	delete m_pSelKey;
	SelectKey(NULL);

	// Break any links

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->GetData()->GetKeys()->GetHead();

		while (pKeyNode)
		{
			if ((pKeyNode->m_Data->IsLinked()) && (pKeyNode->m_Data->GetLinkedID() == pKeyNode->m_Data->GetID()))
			{
				pKeyNode->m_Data->SetLink(FALSE, 0, "");
			}
			
			pKeyNode = pKeyNode->m_pNext;
		}
		
		pTrackNode = pTrackNode->m_pNext;
	}

	// Redraw

	InvalidateRect(NULL, TRUE);

	// Refresh the phase info dialog

	m_phaseInfoDlg.Refresh();
}


//------------------------------------------------------------------
//
//   FUNCTION : OnTrackDeleteKey()
//
//   PURPOSE  : Handles changing a key from one effect type to another
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackChangeKey()
{
	// See if we have a key
	CKey *pKey = m_pSelKey;
	
	if (pKey)
	{
		if (pKey->GetFxRef())
		{
			int ret = AfxMessageBox("Change FX ?", MB_YESNO | MB_ICONEXCLAMATION);
			if (ret == IDYES)
			{
				CFxDlg dlg;

				if (dlg.DoModal() == IDOK)
				{
					// Setup new reference
					
					pKey->SetFxRef(dlg.m_pFxRef);

					// Redraw

					InvalidateRect(NULL, TRUE);
				}
			}
		}
		else
		{
			// No key yet, so call up the fx dialog

			CFxDlg dlg;

			if (dlg.DoModal() == IDOK)
			{
				// Setup the new reference

				pKey->SetFxRef(dlg.m_pFxRef);

				// Redraw

				InvalidateRect(NULL, TRUE);
			}
		}
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : OnLButtonDown()
//
//   PURPOSE  : Handles WM_LBUTTONDOWN
//
//------------------------------------------------------------------

void CTrackWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();

	BOOL bCtrl = IsKeyDown(VK_CONTROL) ? TRUE : FALSE;
	BOOL bShift = IsKeyDown(VK_SHIFT) ? TRUE : FALSE;
	
	if (!bCtrl)
	{
		// Deselect all tracks

		CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

		while (pTrackNode)
		{
			CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

			// Deselect all keys

			while (pKeyNode)
			{
				pKeyNode->m_Data->Select(FALSE);

				pKeyNode = pKeyNode->m_pNext;
			}

			pTrackNode = pTrackNode->m_pNext;
		}
	}

	m_pSelTrack = GetTrackByPos(point.y);
	if (!m_pSelTrack) return;

	// Select the track
	
	m_pSelTrack->Select();
	
	SelectKey(GetKeyByPos(m_pSelTrack, point.x));
	if (!m_pSelKey) return;

	// Redraw !!

	InvalidateRect(NULL, TRUE);

	// Save the anchor point

	m_ptAnchor = point;

	// Check the cursor before we track....

	HCURSOR hCursor = ::GetCursor();
	HCURSOR hSizeWE = ::LoadCursor(NULL, IDC_SIZEWE);

	if ((m_pSelKey) && (m_pSelTrack))
	{
		if (!bShift)
		{
			int nRet = HitTestKey(m_pSelTrack, m_pSelKey, point);

			switch (nRet)
			{
				case HT_INSIDE :
				{
					TrackMoveKey();
				}
				break;

				case HT_LEFT :
				{
					TrackMoveLeftSideKey();
				}
				break;

				case HT_RIGHT :
				{
					TrackMoveRightSideKey();
				}
				break;
			}
		}

		if ((bShift) && (NumSelected() == 1))
		{
			TrackCopyKey(bCtrl);
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : TrackMoveKey()
//
//   PURPOSE  : Moves a key
//
//------------------------------------------------------------------

void CTrackWnd::TrackMoveKey()
{
	if (!m_pSelKey) return;

	// Check all keys and see if we have any motion linking....

	{
		BOOL bMotionLinked = FALSE;
		
		CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

		while (pTrackNode)
		{
			CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();
			
			while (pKeyNode)
			{
				if ((pKeyNode->m_Data->IsSelected()) && (pKeyNode->m_Data->IsLinked())) bMotionLinked = TRUE;
				
				pKeyNode = pKeyNode->m_pNext;
			}		

			pTrackNode = pTrackNode->m_pNext;
		}

		if (bMotionLinked)
		{
			TrackMoveMotionLinkedKeys();
			return;
		}
	}
	
	if (IsLinkedTo(m_pSelKey))
	{
		int ret = AfxMessageBox("This key has been linked to, you cannot move it, Unlink ?", MB_YESNO);

		if (ret == IDYES) UnlinkKey(m_pSelKey);

		return;
	}

	HCURSOR hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
	::SetCursor(hCursor);

	ShowCursor(TRUE);

	CPoint ptCursor;
	GetCursorPos(&ptCursor);

	m_ptAnchor = ptCursor;
	ScreenToClient(&m_ptAnchor);

	m_ptLast = m_ptAnchor;

	int tmStart  = m_pSelKey->GetStartTime();
	int tmEnd    = m_pSelKey->GetEndTime();
	int tmLen    = tmEnd - tmStart;
	int tmAnchor = m_timeBarDlg.PosToTime(m_ptAnchor.x) + m_timeBarDlg.GetTimeAnchor();
	int tmTotal  = m_timeBarDlg.GetTotalTime();

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	// Setup the anchors
	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			CKey *pKey = pKeyNode->m_Data;

			if (pKey->IsSelected())
			{
				pKey->SetAnchorOffset(tmAnchor - pKey->GetStartTime());
				pKey->SetAnchorLength(pKey->GetEndTime() - pKey->GetStartTime());
			}

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}
	CDC *pDC = GetDC();

	m_bTracking = TRUE;

	while (IsKeyDown(VK_LBUTTON))
	{
		// Pump messages

		MSG msg;

		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		CPoint ptCursor;
		GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		int nTime = m_timeBarDlg.PosToTime(ptCursor.x) + m_timeBarDlg.GetTimeAnchor();
		if (ptCursor.x != m_ptLast.x)
		{
			CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

			while (pTrackNode)
			{
				CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

				while (pKeyNode)
				{
					CKey *pKey = pKeyNode->m_Data;

					if (pKey->IsSelected())
					{
						CKey *pPrev = NULL;
						CKey *pNext = NULL;
						
						CLinkListNode<CKey *> *pPrevNode = pKeyNode->m_pPrev;
						CLinkListNode<CKey *> *pNextNode = pKeyNode->m_pNext;
						
						if (pPrevNode) pPrev = pPrevNode->m_Data;
						if (pNextNode) pNext = pNextNode->m_Data;
					
						int tmNewStart = nTime - pKey->GetAnchorOffset();
						int tmNewEnd   = tmNewStart + pKey->GetAnchorLength();
						tmLen   = tmNewEnd - tmNewStart;
												
						BOOL bOkay = TRUE;
						
						// Clamp checks

						if ((!pPrev) && (tmNewStart < 0))
						{			
							pKey->SetStartTime(0);
							pKey->SetEndTime(tmLen);
							bOkay = FALSE;
						}

						if (tmNewEnd > tmTotal)
						{
							pKey->SetStartTime(tmTotal - tmLen);
							pKey->SetEndTime(tmTotal);
							bOkay = FALSE;
						}

						if ((pPrev) && (tmNewStart < pPrev->GetEndTime()))
						{
							pKey->SetStartTime(pPrev->GetEndTime());
							pKey->SetEndTime(pPrev->GetEndTime() + tmLen);
							bOkay = FALSE;
						}

						if ((pNext) && (tmNewEnd >= pNext->GetStartTime()))
						{
							pKey->SetStartTime(pNext->GetStartTime() - tmLen);
							pKey->SetEndTime(pNext->GetStartTime());
							bOkay = FALSE;
						}

						// Check for links

						CKey *pLinkedKey = NULL;
						
						if (pKey->IsLinked()) pLinkedKey = m_pPhase->GetKeyByID(pKey->GetLinkedID());

						if (pLinkedKey)
						{
							if (tmNewStart < pLinkedKey->GetStartTime() + 50)
							{
								pKey->SetStartTime(pLinkedKey->GetStartTime() + 50);
								pKey->SetEndTime(pLinkedKey->GetStartTime() + 50 + pKey->GetAnchorLength());

								bOkay = FALSE;
							}

							if (tmNewEnd >= pLinkedKey->GetEndTime() - 50)
							{
								pKey->SetStartTime(pLinkedKey->GetEndTime() - 50 - pKey->GetAnchorLength());
								pKey->SetEndTime(pLinkedKey->GetEndTime() - 50);

								bOkay = FALSE;
							}
						}

						if (bOkay)
						{
							pKey->SetStartTime(tmNewStart);
							pKey->SetEndTime(tmNewEnd);
						}
					}

					pKeyNode = pKeyNode->m_pNext;
				}

				pTrackNode = pTrackNode->m_pNext;
			}

			// Redraw !!

			DrawTracks(pDC);
			m_ptLast = ptCursor;

			// Refresh the phase info dialog

			m_phaseInfoDlg.Refresh();
		}
	}

	m_bTracking = FALSE;

	ReleaseDC(pDC);

	hCursor = ::LoadCursor(NULL, IDC_ARROW);
	::SetCursor(hCursor);
}

//------------------------------------------------------------------
//
//   FUNCTION : TrackMoveLeftSideKey()
//
//   PURPOSE  : Moves a key
//
//------------------------------------------------------------------

void CTrackWnd::TrackMoveLeftSideKey()
{
	if (!m_pSelKey) return;

	if (IsLinkedTo(m_pSelKey))
	{
		int ret = AfxMessageBox("This key has been linked to, you cannot move it, Unlink ?", MB_YESNO);

		if (ret == IDYES) UnlinkKey(m_pSelKey);

		return;
	}

	HCURSOR hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
	::SetCursor(hCursor);

	ShowCursor(TRUE);

	CPoint ptCursor;
	GetCursorPos(&ptCursor);

	m_ptAnchor = ptCursor;
	ScreenToClient(&m_ptAnchor);

	m_ptLast = m_ptAnchor;

	int tmStart  = m_pSelKey->GetStartTime();
	int tmEnd    = m_pSelKey->GetEndTime();
	int tmLen    = tmEnd - tmStart;
	int tmAnchor = m_timeBarDlg.PosToTime(m_ptAnchor.x) + m_timeBarDlg.GetTimeAnchor();

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	// Setup the anchors
	
	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			CKey *pKey = pKeyNode->m_Data;

			if (pKey->IsSelected())
			{
				pKey->SetAnchorOffset(tmAnchor - pKey->GetStartTime());
				pKey->SetAnchorLength(pKey->GetEndTime() - pKey->GetStartTime());
			}

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}
	
	CKey *pPrev = NULL;
	CKey *pNext = NULL;
	
	CLinkListNode<CKey *> *pPrevNode = m_pSelTrack->GetKeys()->Find(m_pSelKey)->m_pPrev;
	CLinkListNode<CKey *> *pNextNode = m_pSelTrack->GetKeys()->Find(m_pSelKey)->m_pNext;
	
	if (pPrevNode) pPrev = pPrevNode->m_Data;
	if (pNextNode) pNext = pNextNode->m_Data;

	CDC *pDC = GetDC();

	m_bTracking = TRUE;

	while (IsKeyDown(VK_LBUTTON))
	{
		// Pump messages

		MSG msg;

		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		CPoint ptCursor;
		GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (ptCursor.x != m_ptLast.x)
		{
			CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

			while (pTrackNode)
			{
				CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

				while (pKeyNode)
				{
					CKey *pKey = pKeyNode->m_Data;

					if (pKey->IsSelected())
					{
						CKey *pPrev = NULL;
						CKey *pNext = NULL;
						
						CLinkListNode<CKey *> *pPrevNode = pKeyNode->m_pPrev;
						CLinkListNode<CKey *> *pNextNode = pKeyNode->m_pNext;
						
						if (pPrevNode) pPrev = pPrevNode->m_Data;
						if (pNextNode) pNext = pNextNode->m_Data;

						int nTime = m_timeBarDlg.PosToTime(ptCursor.x) + m_timeBarDlg.GetTimeAnchor();

						int tmNewStart = nTime - pKey->GetAnchorOffset();
						int tmNewEnd   = tmNewStart + pKey->GetAnchorLength();
						tmLen   = tmNewEnd - tmNewStart;

						BOOL bOkay = TRUE;
						
						// Clamp checks
									
						if ((!pPrev) && (tmNewStart < 0))
						{			
							pKey->SetStartTime(0);
							bOkay = FALSE;
						}

						if ((pPrev) && (tmNewStart < pPrev->GetEndTime()))
						{
							pKey->SetStartTime(pPrev->GetEndTime());
							bOkay = FALSE;
						}

						if (tmNewStart >= pKey->GetEndTime() - 100)
						{
							tmNewStart = pKey->GetEndTime() - 100;
						}

						// Check for links

						CKey *pLinkedKey = NULL;
						
						if (pKey->IsLinked()) pLinkedKey = m_pPhase->GetKeyByID(pKey->GetLinkedID());

						if (pLinkedKey)
						{
							if (tmNewStart < pLinkedKey->GetStartTime() + 50)
							{
								pKey->SetStartTime(pLinkedKey->GetStartTime() + 50);

								bOkay = FALSE;
							}
						}

						if (bOkay)
						{
							pKey->SetStartTime(tmNewStart);
						}
					}

					pKeyNode = pKeyNode->m_pNext;
				}

				pTrackNode = pTrackNode->m_pNext;
			}

			// Refresh the phase info dialog

			m_phaseInfoDlg.Refresh();

			// Redraw !!

			DrawTracks(pDC);
			m_ptLast = ptCursor;
		}
	}

	m_bTracking = FALSE;

	ReleaseDC(pDC);

	hCursor = ::LoadCursor(NULL, IDC_ARROW);
	::SetCursor(hCursor);
}

//------------------------------------------------------------------
//
//   FUNCTION : TrackMoveRightSideKey()
//
//   PURPOSE  : Moves a key
//
//------------------------------------------------------------------

void CTrackWnd::TrackMoveRightSideKey()
{
	if (!m_pSelKey) return;

	if (IsLinkedTo(m_pSelKey))
	{
		int ret = AfxMessageBox("This key has been linked to, you cannot move it, Unlink ?", MB_YESNO);

		if (ret == IDYES) UnlinkKey(m_pSelKey);

		return;
	}

	HCURSOR hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
	::SetCursor(hCursor);

	ShowCursor(TRUE);

	CPoint ptCursor;
	GetCursorPos(&ptCursor);

	m_ptAnchor = ptCursor;
	ScreenToClient(&m_ptAnchor);

	m_ptLast = m_ptAnchor;

	int tmStart  = m_pSelKey->GetStartTime();
	int tmEnd    = m_pSelKey->GetEndTime();
	int tmLen    = tmEnd - tmStart;
	int tmAnchor = m_timeBarDlg.PosToTime(m_ptAnchor.x) + m_timeBarDlg.GetTimeAnchor();
	int tmTotal	 = m_timeBarDlg.GetTotalTime();
	
	CKey *pPrev = NULL;
	CKey *pNext = NULL;
	
	CLinkListNode<CKey *> *pPrevNode = m_pSelTrack->GetKeys()->Find(m_pSelKey)->m_pPrev;
	CLinkListNode<CKey *> *pNextNode = m_pSelTrack->GetKeys()->Find(m_pSelKey)->m_pNext;
	
	if (pPrevNode) pPrev = pPrevNode->m_Data;
	if (pNextNode) pNext = pNextNode->m_Data;

	CDC *pDC = GetDC();

	m_bTracking = TRUE;

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	// Setup the anchors
	
	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			CKey *pKey = pKeyNode->m_Data;

			if (pKey->IsSelected())
			{
				pKey->SetAnchorOffset(tmAnchor - pKey->GetStartTime());
				pKey->SetAnchorLength(pKey->GetEndTime() - pKey->GetStartTime());
			}

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	while (IsKeyDown(VK_LBUTTON))
	{
		// Pump messages

		MSG msg;

		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		CPoint ptCursor;
		GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (ptCursor.x != m_ptLast.x)
		{
			CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

			while (pTrackNode)
			{
				CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

				while (pKeyNode)
				{
					CKey *pKey = pKeyNode->m_Data;

					if (pKey->IsSelected())
					{
						CKey *pPrev = NULL;
						CKey *pNext = NULL;
						
						CLinkListNode<CKey *> *pPrevNode = pKeyNode->m_pPrev;
						CLinkListNode<CKey *> *pNextNode = pKeyNode->m_pNext;
						
						if (pPrevNode) pPrev = pPrevNode->m_Data;
						if (pNextNode) pNext = pNextNode->m_Data;

						int nTime = m_timeBarDlg.PosToTime(ptCursor.x) + m_timeBarDlg.GetTimeAnchor();
					
						int tmNewStart = nTime - pKey->GetAnchorOffset();
						int tmNewEnd   = tmNewStart + pKey->GetAnchorLength();
						tmLen   = tmNewEnd - tmNewStart;

						BOOL bOkay = TRUE;
						
						// Clamp checks
									
						if ((pNext) && (tmNewEnd >= pNext->GetStartTime()))
						{
							pKey->SetEndTime(pNext->GetStartTime());
							bOkay = FALSE;
						}

						if (tmNewEnd > tmTotal)
						{
							pKey->SetEndTime(tmTotal);
							bOkay = FALSE;
						}

						if (tmNewEnd <= pKey->GetStartTime() + 100)
						{
							tmNewEnd = pKey->GetStartTime() +  100;
						}

						// Check for links

						CKey *pLinkedKey = NULL;
						
						if (pKey->IsLinked()) pLinkedKey = m_pPhase->GetKeyByID(pKey->GetLinkedID());

						if (pLinkedKey)
						{
							if (tmNewEnd >= pLinkedKey->GetEndTime() - 50)
							{
								pKey->SetEndTime(pLinkedKey->GetEndTime() - 50);

								bOkay = FALSE;
							}
						}
						
						if (bOkay)
						{
							pKey->SetEndTime(tmNewEnd);
						}
					}

					pKeyNode = pKeyNode->m_pNext;
				}

				pTrackNode = pTrackNode->m_pNext;
			}

			// Redraw !!

			DrawTracks(pDC);
			m_ptLast = ptCursor;

			// Refresh the phase info dialog

			m_phaseInfoDlg.Refresh();
		}
	}

	m_bTracking = FALSE;

	ReleaseDC(pDC);

	hCursor = ::LoadCursor(NULL, IDC_ARROW);
	::SetCursor(hCursor);
}

//------------------------------------------------------------------
//
//   FUNCTION : TrackCopyKey()
//
//   PURPOSE  : Copies a key
//
//------------------------------------------------------------------

void CTrackWnd::TrackCopyKey(BOOL bRemoveOriginal)
{
	if (!m_pSelKey) return;

	m_bTracking = TRUE;

	// Clone the selected key

	if (bRemoveOriginal)
	{
		if ((IsLinkedTo(m_pSelKey)) || (m_pSelKey->IsLinked()))
		{
			int ret = AfxMessageBox("You must unlink this key first, Unlink ?", MB_YESNO);

			if (ret == IDYES)
			{
				UnlinkKey(m_pSelKey);
			}
			else if (ret == IDNO)
			{
				return;
			}
		}

		m_pCopyKey = m_pSelKey;
		m_pSelTrack->GetKeys()->Remove(m_pSelKey);
	}
	else
	{
		m_pCopyKey = m_pSelKey->Clone();
		m_pCopyKey->SetID(m_pPhase->GetUniqueID());
	}

	// Save the key rect

	m_rcCopyKey = GetKeyRect(m_pSelTrack, m_pSelKey);
	m_rcCopyKey.left   -= m_ptAnchor.x;
	m_rcCopyKey.top    -= m_ptAnchor.y;
	m_rcCopyKey.right  -= m_ptAnchor.x;
	m_rcCopyKey.bottom -= m_ptAnchor.y;

	m_ptLast = m_ptAnchor;

	while (IsKeyDown(VK_LBUTTON))
	{
		// Pump messages

		MSG msg;

		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		CPoint ptCursor;
		GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (ptCursor != m_ptLast)
		{
			m_ptLast = ptCursor;

			// Redraw !!

			InvalidateRect(NULL, TRUE);
		}
	}

	// Lets see if we can add it....

	CTrack *pTrack = GetTrackByPos(m_ptLast.y);
	if( !pTrack )
		pTrack = m_pPhase->AddTrack();

	if (pTrack)
	{
		// Valid track, calculate the new times for the key

		int nStart = m_timeBarDlg.PosToTime(m_ptLast.x + m_rcCopyKey.left) + m_timeBarDlg.GetTimeAnchor();
		int nEnd   = m_timeBarDlg.PosToTime(m_ptLast.x + m_rcCopyKey.right) + m_timeBarDlg.GetTimeAnchor();
	
		CKey *pKey = GetKeyByPos(pTrack, m_ptLast.x);
		
		if (pKey)
		{
			BOOL bSameKey = ((pKey->GetTrack() == m_pCopyKey->GetTrack()) &&
					         (pKey->GetStartTime() == m_pCopyKey->GetStartTime()) &&
					         (pKey->GetEndTime() == m_pCopyKey->GetEndTime())) ? TRUE : FALSE;

			if (!bSameKey)
			{
				m_pCopyKey->SetStartTime(nStart);
				m_pCopyKey->SetEndTime(nEnd);

				MessageBeep(0);

				BOOL bCopyColour = TRUE;
				BOOL bCopyMotion = TRUE;
				BOOL bCopyScale  = TRUE;

				DWORD dwSrcType = m_pCopyKey->GetFxRef()->m_dwType;
				DWORD dwDstType = pKey->GetFxRef()->m_dwType;

				if ((!(dwSrcType & FX_NEEDCOLOURKEY)) || (!(dwDstType & FX_NEEDCOLOURKEY))) bCopyColour = FALSE;
				if ((!(dwSrcType & FX_NEEDMOTIONKEY)) || (!(dwDstType & FX_NEEDMOTIONKEY))) bCopyMotion = FALSE;
				if ((!(dwSrcType & FX_NEEDSCALEKEY)) || (!(dwDstType & FX_NEEDSCALEKEY))) bCopyScale    = FALSE;
				
				CCopyDataDlg dlg(bCopyColour, bCopyMotion, bCopyScale);

				if (dlg.DoModal() == IDOK)
				{
					// Copy the colour key frames

					if (CCopyDataDlg::s_bCopyColour)
					{
						pKey->GetColourKeys()->RemoveAll();

						CLinkListNode<COLOURKEY> *pNode = m_pCopyKey->GetColourKeys()->GetHead();

						while (pNode)
						{
							pKey->GetColourKeys()->AddTail(pNode->m_Data);
							
							pNode = pNode->m_pNext;
						}
					}

					// Copy the scale key frames

					if (CCopyDataDlg::s_bCopyScale)
					{
						pKey->GetScaleKeys()->RemoveAll();

						CLinkListNode<SCALEKEY> *pNode = m_pCopyKey->GetScaleKeys()->GetHead();

						while (pNode)
						{
							pKey->GetScaleKeys()->AddTail(pNode->m_Data);
							
							pNode = pNode->m_pNext;
						}
					}

					// Copy the motion key frames

					if (CCopyDataDlg::s_bCopyMotion)
					{
						pKey->GetMoveKeys()->RemoveAll();

						CLinkListNode<MOVEKEY> *pNode = m_pCopyKey->GetMoveKeys()->GetHead();

						while (pNode)
						{
							pKey->GetMoveKeys()->AddTail(pNode->m_Data);
							
							pNode = pNode->m_pNext;
						}
					}
				}
			}

			// Delete the copy key

			delete m_pCopyKey;
		}
		
		if (!pKey)
		{
			m_pCopyKey->SetStartTime(nStart);
			m_pCopyKey->SetEndTime(nEnd);

			// No key here so go ahead and add it

			m_pCopyKey->SetTrack(m_pPhase->GetTracks()->GetIndex(pTrack));
			pTrack->GetKeys()->AddTail(m_pCopyKey);
			pTrack->ArrangeKeys(m_pCopyKey);
		}
	}

	// Redraw

	InvalidateRect(NULL, TRUE);

	// Reset copy key

	m_pCopyKey = NULL;

	// Stop tracking

	m_bTracking = FALSE;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSetCursor()
//
//   PURPOSE  : Handles WM_SETCURSOR
//
//------------------------------------------------------------------

BOOL CTrackWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	return FALSE;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnMouseMove()
//
//   PURPOSE  : Handles WM_MOUSEMOVE
//
//------------------------------------------------------------------

void CTrackWnd::OnMouseMove(UINT nFlags, CPoint point) 	
{
	if (m_bTracking) return;

	// Check to see if we need to change the cursor....

	BOOL bSetLeftRight = FALSE;

	CRect rcKey;

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		// Deselect all keys

		while (pKeyNode)
		{
			CRect rcKey = GetKeyRect(pTrackNode->m_Data, pKeyNode->m_Data);

			if (rcKey.PtInRect(point))
			{
				// We have a winner

				int left  = abs(point.x - rcKey.left);
				int right = abs(rcKey.right - point.x);

				if ((left < 4) || (right < 4))
				{
					bSetLeftRight = TRUE;
				}
			}

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	if (bSetLeftRight)
	{
		HCURSOR hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
		::SetCursor(hCursor);
	}
	else
	{
		HCURSOR hCursor = ::LoadCursor(NULL, IDC_ARROW);
		::SetCursor(hCursor);
	}

	CWnd::OnMouseMove(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : GetKeyRect()
//
//   PURPOSE  : Returns rectangle for a key
//
//------------------------------------------------------------------

CRect CTrackWnd::GetKeyRect(CTrack *pTrack, CKey *pKey)
{
	int nPos = m_pPhase->GetTracks()->GetIndex(pTrack) * m_nTrackHeight;

	int nStartPos = m_timeBarDlg.TimeToPos(pKey->GetStartTime() - m_timeBarDlg.GetTimeAnchor());
	int nEndPos   = m_timeBarDlg.TimeToPos(pKey->GetEndTime() - m_timeBarDlg.GetTimeAnchor());

	CRect rcKey;
	rcKey.left   = nStartPos;
	rcKey.right  = nEndPos;
	rcKey.top	 = nPos + m_rcTrack.top - m_yOffset;
	rcKey.bottom = nPos + m_nTrackHeight + m_rcTrack.top - m_yOffset;

	return rcKey;
}

//------------------------------------------------------------------
//
//   FUNCTION : HitTestKey()
//
//   PURPOSE  : Performs hit testing on a specific key
//
//------------------------------------------------------------------

int CTrackWnd::HitTestKey(CTrack *pTrack, CKey *pKey, CPoint ptTest)
{
	CRect rcKey = GetKeyRect(pTrack, pKey);

	if (rcKey.PtInRect(ptTest))
	{
		int left  = abs(ptTest.x - rcKey.left);
		int right = abs(rcKey.right - ptTest.x);

		if (left < 4) return HT_LEFT;
		if (right < 4) return HT_RIGHT;
		
		return HT_INSIDE;
	}
	else
	{
		return HT_OUTSIDE;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackExpandKey()
//
//   PURPOSE  : Expands a key
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackExpandKey() 
{
	if (!m_pSelTrack) return;
	if (!m_pSelKey) return;
	
	CLinkListNode<CKey *> *pNode = m_pSelTrack->GetKeys()->Find(m_pSelKey);
	if (!pNode) return;
	
	if (pNode->m_pPrev)
	{
		m_pSelKey->SetStartTime(pNode->m_pPrev->m_Data->GetEndTime());
	}
	else
	{
		m_pSelKey->SetStartTime(0);
	}

	if (pNode->m_pNext)
	{
		m_pSelKey->SetEndTime(pNode->m_pNext->m_Data->GetStartTime());
	}
	else
	{
		m_pSelKey->SetEndTime(m_timeBarDlg.GetTotalTime());
	}

	// Redraw....

	InvalidateRect(NULL, TRUE);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnKeyDown()
//
//   PURPOSE  : Handles WM_KEYDOWN
//
//------------------------------------------------------------------

void CTrackWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar)
	{
	case VK_DELETE:
		if ((m_pSelTrack) && (m_pSelKey)) OnTrackDeleteKey();
		break;
	case VK_F2:
		OnTrackNameKey();
		break;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

//------------------------------------------------------------------
//
//   FUNCTION : GetKeyByPt()
//
//   PURPOSE  : Returns a given key under a point or NULL
//
//------------------------------------------------------------------

CKey* CTrackWnd::GetKeyByPt(CPoint ptTest)
{
	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		// Deselect all keys

		while (pKeyNode)
		{
			CRect rcKey = GetKeyRect(pTrackNode->m_Data, pKeyNode->m_Data);

			if (rcKey.PtInRect(ptTest))
			{
				return pKeyNode->m_Data;
			}

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}	
	
	// Failure !!

	return NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnLButtonDblClk()
//
//   PURPOSE  : Handles WM_LBUTTONDBLCLK
//
//------------------------------------------------------------------

void CTrackWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	//see if they have any modifiers held down to quickly access different components
	OnTrackEditKey();	

	CWnd::OnLButtonDblClk(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackEditKey()
//
//   PURPOSE  : Edits a keys properties (FX or otherwise)
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackEditKey() 
{
	// Return if we don't have a selected key

	if (!m_pSelKey) return;	
	
	CFxPropDlg dlg(m_pSelKey, m_pSelKey->GetCollProps());

	// Display the dialog

	if (dlg.DoModal() == IDOK)
	{
		// Retrieve the dialogs properties and update the FX
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackEditColourKeys()
//
//   PURPOSE  : Edits the colour key frames for this key
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackEditColourKeys() 
{
	CColourKeyDlg dlg(m_pSelKey->GetColourKeys(), m_pSelKey);

	if (dlg.DoModal() == IDOK)
	{
		// Copy the keys

		m_pSelKey->GetColourKeys()->RemoveAll();

		CLinkListNode<COLOURKEY> *pNode = dlg.GetKeys()->GetHead();

		while (pNode)
		{
			m_pSelKey->GetColourKeys()->AddTail(pNode->m_Data);
			
			pNode = pNode->m_pNext;
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRButtonDown()
//
//   PURPOSE  : 
//
//------------------------------------------------------------------

void CTrackWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnRButtonDown(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : DestroyWindow()
//
//   PURPOSE  : Destroys the track window
//
//------------------------------------------------------------------

BOOL CTrackWnd::DestroyWindow() 
{
	return CWnd::DestroyWindow();
}

//------------------------------------------------------------------
//
//   FUNCTION : PostNcDestroy()
//
//   PURPOSE  : Called after the window is destroyed
//
//------------------------------------------------------------------

void CTrackWnd::PostNcDestroy() 
{
	if (m_pMemDC)
	{
		m_pMemDC->SelectObject(m_pOldBitmap);

		delete m_pMemDC;
		delete m_pBitmap;
	}
	
	CWnd::PostNcDestroy();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackEditMotionKeys()
//
//   PURPOSE  : Called to bring up the motion keys dialog
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackEditMotionKeys()
{
	if (!m_pSelKey) return;

	CMotionKeysDlg dlg(m_pSelKey);

	dlg.DoModal();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackEditScaleKeys()
//
//   PURPOSE  : Called to bring up the scale keys dialog
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackEditScaleKeys() 
{
	CScaleKeysDlg dlg(m_pSelKey);

	dlg.DoModal();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackAddFavouriteKey()
//
//   PURPOSE  : Adds a favourite key to this track
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackAddFavouriteKey() 
{
	CChooseKeyDlg dlg;

	if (dlg.DoModal() == IDOK)
	{
		CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

		// Add a new key and set it up

		FK_FAVOURITE *pFav = dlg.m_pFav;

		if (pFav)
		{
			CKey *pKey = new CKey;
			if (!pKey) return;

			// Setup the parameters

			int nTrack	   = m_pPhase->GetTracks()->GetIndex(m_pSelTrack);
			int nTimeClick = m_timeBarDlg.GetTimeAnchor() + m_timeBarDlg.PosToTime(m_ptRbClick.x);
			
			pKey->SetTrack(nTrack);
			pKey->SetStartTime(nTimeClick - (pFav->m_tmLength / 2));
			pKey->SetEndTime(pKey->GetStartTime() + pFav->m_tmLength);
			pKey->SetMinScale(pFav->m_minScale);
			pKey->SetMaxScale(pFav->m_maxScale);

			pKey->SetFxRef(pApp->GetFxMgr()->FindFX(pFav->m_ref.m_sName));

			// Copy the properties

			pKey->GetCollProps()->RemoveAll();

			CFastListNode<FX_PROP> *pPropNode = pFav->m_collProps.GetHead();

			while (pPropNode)
			{
				pKey->GetCollProps()->AddTail(pPropNode->m_Data);

				pPropNode = pPropNode->m_pNext;
			}

			// Copy the colour keys

			pKey->GetColourKeys()->CopyList(&pFav->m_collColourKeys);

			// Copy the scale keys

			pKey->GetScaleKeys()->CopyList(&pFav->m_collScaleKeys);

			// Copy the motion keys

			pKey->GetMoveKeys()->CopyList(&pFav->m_collMoveKeys);

			// Get an ID for the key

			pKey->SetID(m_pPhase->GetUniqueID());

			// Add it to the list

			m_pSelTrack->GetKeys()->AddTail(pKey);

			// And arrange correctly

			m_pSelTrack->ArrangeKeys(pKey);

			// Redraw

			InvalidateRect(NULL, TRUE);
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackAddToFavourites()
//
//   PURPOSE  : Adds the current key to the favourites list
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackAddKeyToFavourites() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	if (!m_pSelKey) return;

	CStringDlg dlg("Choose favourite key name");

	if (dlg.DoModal() == IDOK)
	{		
		FK_FAVOURITE *pNewFav = new FK_FAVOURITE;

		pNewFav->m_sName = dlg.m_sText;
		pNewFav->m_tmLength = m_pSelKey->GetTotalTime();

		// Copy properties

		CFastListNode<FX_PROP> *pPropNode = m_pSelKey->GetCollProps()->GetHead();

		while (pPropNode)
		{
			pNewFav->m_collProps.AddTail(pPropNode->m_Data);
			
			pPropNode = pPropNode->m_pNext;
		}

		// Copy colour keys

		CLinkListNode<COLOURKEY> *pColourNode = m_pSelKey->GetColourKeys()->GetHead();

		while (pColourNode)
		{
			pNewFav->m_collColourKeys.AddTail(pColourNode->m_Data);
			
			pColourNode = pColourNode->m_pNext;
		}

		// Copy scale keys

		CLinkListNode<SCALEKEY> *pScaleNode = m_pSelKey->GetScaleKeys()->GetHead();

		while (pScaleNode)
		{
			pNewFav->m_collScaleKeys.AddTail(pScaleNode->m_Data);
			
			pScaleNode = pScaleNode->m_pNext;
		}

		// Copy motion keys

		CLinkListNode<MOVEKEY> *pMotionNode = m_pSelKey->GetMoveKeys()->GetHead();

		while (pMotionNode)
		{
			pNewFav->m_collMoveKeys.AddTail(pMotionNode->m_Data);
			
			pMotionNode = pMotionNode->m_pNext;
		}

		// Copy the reference

		pNewFav->m_ref = *m_pSelKey->GetFxRef();

		// Copy min and max scales

		pNewFav->m_minScale = m_pSelKey->GetMinScale();
		pNewFav->m_maxScale = m_pSelKey->GetMaxScale();
	
		// Add the favourite key

		pApp->GetKeyFavourites()->AddTail(pNewFav);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : SelectKey()
//
//   PURPOSE  : Selects a key
//
//------------------------------------------------------------------

void CTrackWnd::SelectKey(CKey *pKey)
{
	// Select the key

	if (pKey)
	{
		pKey->Select(TRUE);
		m_pSelKey = pKey;
	}
	else
	{
		m_pSelKey = NULL;
	}

	// Update the phase info dialog

	m_phaseInfoDlg.Refresh();
}

//------------------------------------------------------------------
//
//   FUNCTION : DeselectAllKeys()
//
//   PURPOSE  : Deselects any selected keys
//
//------------------------------------------------------------------

void CTrackWnd::DeselectAllKeys()
{
	// Deselect all tracks

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		// Deselect all keys

		while (pKeyNode)
		{
			pKeyNode->m_Data->Select(FALSE);

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnVScroll()
//
//   PURPOSE  : Handles WM_VSCROLL
//
//------------------------------------------------------------------

void CTrackWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar == &m_scrollBar)
	{
		int nCurPos = m_scrollBar.GetScrollPos();

		switch (nSBCode)
		{
			case SB_LINEUP :
			{
				nCurPos -= 5;
			}
			break;

			case SB_LINEDOWN :
			{
				nCurPos += 5;
			}
			break;

			case SB_PAGEUP :
			{
				nCurPos -= 20;
			}
			break;

			case SB_PAGEDOWN :
			{
				nCurPos += 20;
			}
			break;

			case SB_THUMBTRACK :
			{
				nCurPos = (int)nPos;
			}
			break;
		}

		SCROLLINFO si;

		si.cbSize = sizeof(SCROLLINFO);
		si.fMask  = SIF_ALL;
		si.nMax   = 200;
		si.nMin	  = 0;
		si.nPage  = 10;
		si.nPos	  = nCurPos;

		m_scrollBar.SetScrollInfo(&si);

		m_yOffset = nCurPos * 4;

		CDC *pDC = GetDC();
		DrawTracks(pDC);
		ReleaseDC(pDC);
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

//------------------------------------------------------------------
//
//   FUNCTION : SingleKeySelected()
//
//   PURPOSE  : Returns whether or not only one key is selected
//
//------------------------------------------------------------------

int CTrackWnd::NumSelected()
{
	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	int nSelCount = 0;
	
	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			if (pKeyNode->m_Data->IsSelected())
			{
				nSelCount ++;
			}
			
			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	return nSelCount;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackDeleteTrack()
//
//   PURPOSE  : Deletes the currently selected track altogether
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackDeleteTrack() 
{
	if (!m_pSelTrack) return;
	
	if (AfxMessageBox("Are you sure ?", MB_YESNO) == IDNO) return;
	
	int nTrackIndex = m_pPhase->GetTracks()->GetIndex(m_pSelTrack);
	
	if ((nTrackIndex < 0) || (nTrackIndex > (int)m_pPhase->GetTracks()->GetSize())) return;

	// Delete the track

	m_pPhase->GetTracks()->Remove(m_pSelTrack);
	delete m_pSelTrack;

	// Renumber all the keys

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			if (pKeyNode->m_Data->GetTrack() > nTrackIndex) pKeyNode->m_Data->SetTrack(pKeyNode->m_Data->GetTrack() - 1);
			
			pKeyNode = pKeyNode->m_pNext;
		}
		
		pTrackNode = pTrackNode->m_pNext;
	}

	m_pSelTrack = NULL;
	m_pSelKey = NULL;

	// Redraw
	
	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackCopyMultiKeys()
//
//   PURPOSE  : 
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackCopyMultiKeys() 
{
	int nBaseTrack = m_pPhase->GetTracks()->GetSize();
	int nNumNewTracks = 0;

	// Figure out how many extra tracks we need

	int nDeepestTrack = -1;

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			if (pKeyNode->m_Data->IsSelected())
			{
				if (pKeyNode->m_Data->GetTrack() > nDeepestTrack) 
				{
					nDeepestTrack = pKeyNode->m_Data->GetTrack();
					nNumNewTracks++;
				}
			}
			
			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	// Create the new tracks

	for (int i = 0; i < nNumNewTracks; i ++)
	{
		// Create a new track
		
		m_pPhase->AddTrack();
	}

	int test = m_pPhase->GetTracks()->GetSize();

	// Create the new keys

	pTrackNode = m_pPhase->GetTracks()->GetHead();
	int nLastTrack = -1;
	int nNewTrack = -1;

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			if (pKeyNode->m_Data->IsSelected())
			{
				nNewTrack = (pKeyNode->m_Data->GetTrack() == nLastTrack ? nNewTrack : nBaseTrack++);
				nLastTrack = pKeyNode->m_Data->GetTrack();
				
				CKey *pNewKey = pKeyNode->m_Data->Clone();

				pNewKey->m_nTrack = nNewTrack;
				m_pPhase->GetTracks()->Get(nNewTrack)->GetKeys()->AddTail(pNewKey);
				m_pPhase->GetTracks()->Get(nNewTrack)->ArrangeKeys(pNewKey);
			}
			
			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	// Redraw

	InvalidateRect(NULL, TRUE);

	// Refresh the phase info dialog

	m_phaseInfoDlg.Refresh();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackMakeSameLength()
//
//   PURPOSE  : Makes all selected keys the same length
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackMakeSameLength() 
{
	// Locate first selected key

	int msLen = 0;

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			if (pKeyNode->m_Data->IsSelected())
			{
				msLen = pKeyNode->m_Data->GetEndTime() - pKeyNode->m_Data->GetStartTime();
			}
			
			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			if (pKeyNode->m_Data->IsSelected())
			{
				CKey *pKey = pKeyNode->m_Data;

				if (pKeyNode->m_pNext)
				{
					if (pKey->GetStartTime() + msLen < pKeyNode->m_pNext->m_Data->GetStartTime())
					{
						pKey->SetEndTime(pKey->GetStartTime() + msLen);
					}
					else
					{
						pKey->SetEndTime(pKeyNode->m_pNext->m_Data->GetStartTime());
					}
				}
				else
				{
					if (pKey->GetStartTime() + msLen > m_timeBarDlg.GetTotalTime())
					{
						pKey->SetEndTime(m_timeBarDlg.GetTotalTime());
					}
					else
					{
						pKey->SetEndTime(pKey->GetStartTime() + msLen);
					}
				}

			}
			
			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackLinkKey()
//
//   PURPOSE  : Tries to link one key to another
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackLinkKey() 
{
	if (!m_pSelKey) return;
	if (!m_pSelTrack) return;

	// Track....

	CRect rcSrc = GetKeyRect(m_pSelTrack, m_pSelKey);

	m_ptAnchorLink.x = (rcSrc.left + rcSrc.right) >> 1;
	m_ptAnchorLink.y = (rcSrc.top + rcSrc.bottom) >> 1;
	m_ptAnchorLink.y -= m_rcTrack.top;
	
	m_bTrackingLinkKey = TRUE;

	CDC *pDC = GetDC();

	CPoint ptCursor;
	GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);
	ptCursor.y -= m_yOffset;

	m_ptLast	  = ptCursor;
	m_ptAnchorCur = ptCursor;

	DrawTracks(pDC);

	while (!IsKeyDown(VK_LBUTTON))
	{
		// Track the point

		GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);
		ptCursor.y -= m_rcTrack.top;
		m_ptAnchorCur = ptCursor;

		if (m_ptLast != ptCursor)
		{
			DrawTracks(pDC);
			m_ptLast = ptCursor;
		}
	}

	m_ptLast.y += m_rcTrack.top;
	CKey *pKey = GetKeyByPt(m_ptLast);

	// Link the key
	
	if ((pKey) && (pKey != m_pSelKey))
	{
		// Verify that we can link to this key

		if ((m_pSelKey->GetStartTime() > pKey->GetStartTime()) && 
			(m_pSelKey->GetEndTime() < pKey->GetEndTime()))
		{	
			m_pSelKey->SetLink(TRUE, pKey->GetID(), "");
		}
		else
		{
			AfxMessageBox("You cannot link this key, its boundaries are outside of the target key", MB_ICONEXCLAMATION);
		}
	}

	m_bTrackingLinkKey = FALSE;
	DrawTracks(pDC);

	ReleaseDC(pDC);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnTrackUnlinkkey()
//
//   PURPOSE  : Unlinks a key
//
//------------------------------------------------------------------

void CTrackWnd::OnTrackUnlinkkey() 
{
	if (!m_pSelKey) return;

	// Unlink the key

	UnlinkKey(m_pSelKey);

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : IsLinkedTo()
//
//   PURPOSE  : Returns whether this key has links
//
//------------------------------------------------------------------

BOOL CTrackWnd::IsLinkedTo(CKey *pKey)
{
	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			if ((pKey != pKeyNode->m_Data) && (pKeyNode->m_Data->IsLinked()) && (pKeyNode->m_Data->GetLinkedID() == pKey->GetID()))
			{
				return TRUE;
			}
			
			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	// No links....

	return FALSE;
}

//------------------------------------------------------------------
//
//   FUNCTION : UnlinkKey()
//
//   PURPOSE  : Unlinks a key
//
//------------------------------------------------------------------

void CTrackWnd::UnlinkKey(CKey *pKey)
{
	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			if ((pKey != pKeyNode->m_Data) && (pKeyNode->m_Data->IsLinked()) && (pKeyNode->m_Data->GetLinkedID() == pKey->GetID()))
			{
				pKeyNode->m_Data->SetLink(FALSE, 0, "");
			}
			
			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	pKey->SetLink(FALSE, 0, "");

	// Redraw

	Invalidate();
}

void CTrackWnd::OnTrackSetKeyLength() 
{
	if (!m_pSelTrack) return;
	if (!m_pSelKey) return;

	CIntDlg dlg(m_pSelKey->GetEndTime() - m_pSelKey->GetStartTime(), "Enter Key Length in Milliseconds");

	if (dlg.DoModal() == IDOK)
	{
		CLinkListNode<CKey *> *pNode = m_pSelTrack->GetKeys()->Find(m_pSelKey);
		if (!pNode) return;

		m_pSelKey->SetEndTime(m_pSelKey->GetStartTime() + dlg.m_int);
		m_pSelTrack->ArrangeKeys(m_pSelKey);

		// Redraw....

		InvalidateRect(NULL, TRUE);
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : TrackMoveMotionLinkedKeys()
//
//   PURPOSE  : Moves keys that are motion linked
//
//------------------------------------------------------------------

void CTrackWnd::TrackMoveMotionLinkedKeys()
{
	HCURSOR hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
	::SetCursor(hCursor);

	ShowCursor(TRUE);

	CPoint ptCursor;
	GetCursorPos(&ptCursor);

	m_ptAnchor = ptCursor;
	ScreenToClient(&m_ptAnchor);

	m_ptLast = m_ptAnchor;

	int tmStart  = m_pSelKey->GetStartTime();
	int tmEnd    = m_pSelKey->GetEndTime();
	int tmLen    = tmEnd - tmStart;
	int tmAnchor = m_timeBarDlg.PosToTime(m_ptAnchor.x) + m_timeBarDlg.GetTimeAnchor();
	int tmTotal  = m_timeBarDlg.GetTotalTime();

	CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

	// Setup the anchors
	
	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			CKey *pKey = pKeyNode->m_Data;

			if (pKey->IsSelected())
			{
				pKey->SetAnchorOffset(tmAnchor - pKey->GetStartTime());
				pKey->SetAnchorLength(pKey->GetEndTime() - pKey->GetStartTime());
			}

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}
	
	CDC *pDC = GetDC();

	m_bTracking = TRUE;

	while (IsKeyDown(VK_LBUTTON))
	{
		// Pump messages

		MSG msg;

		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		CPoint ptCursor;
		GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		int nTime = m_timeBarDlg.PosToTime(ptCursor.x) + m_timeBarDlg.GetTimeAnchor();

		if (ptCursor.x != m_ptLast.x)
		{	
			BOOL bCanMove = TRUE;

			// Run everything WITHOUT moving anything to make sure we
			// can move....

			CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

			while (pTrackNode)
			{
				CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

				while (pKeyNode)
				{
					CKey *pKey = pKeyNode->m_Data;

					if (pKey->IsSelected())
					{
						CKey *pPrev = NULL;
						CKey *pNext = NULL;
						
						CLinkListNode<CKey *> *pPrevNode = pKeyNode->m_pPrev;
						CLinkListNode<CKey *> *pNextNode = pKeyNode->m_pNext;
						
						if (pPrevNode) pPrev = pPrevNode->m_Data;
						if (pNextNode) pNext = pNextNode->m_Data;
					
						int tmNewStart = nTime - pKey->GetAnchorOffset();
						int tmNewEnd   = tmNewStart + pKey->GetAnchorLength();
						tmLen   = tmNewEnd - tmNewStart;
												
					
						// Clamp checks

						if ((!pPrev) && (tmNewStart < 0))
						{			
							bCanMove = FALSE;
						}

						if (tmNewEnd > tmTotal)
						{
							bCanMove = FALSE;
						}

						if ((pPrev) && (tmNewStart < pPrev->GetEndTime()))
						{
							bCanMove = FALSE;
						}

						if ((pNext) && (tmNewEnd >= pNext->GetStartTime()))
						{
							bCanMove = FALSE;
						}
					}

					pKeyNode = pKeyNode->m_pNext;
				}

				pTrackNode = pTrackNode->m_pNext;
			}

			if (bCanMove)
			{
				CLinkListNode<CTrack *> *pTrackNode = m_pPhase->GetTracks()->GetHead();

				while (pTrackNode)
				{
					CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

					while (pKeyNode)
					{
						CKey *pKey = pKeyNode->m_Data;

						if (pKey->IsSelected())
						{
							CKey *pPrev = NULL;
							CKey *pNext = NULL;
							
							CLinkListNode<CKey *> *pPrevNode = pKeyNode->m_pPrev;
							CLinkListNode<CKey *> *pNextNode = pKeyNode->m_pNext;
							
							if (pPrevNode) pPrev = pPrevNode->m_Data;
							if (pNextNode) pNext = pNextNode->m_Data;
						
							int tmNewStart = nTime - pKey->GetAnchorOffset();
							int tmNewEnd   = tmNewStart + pKey->GetAnchorLength();
							tmLen   = tmNewEnd - tmNewStart;
													
							BOOL bOkay = TRUE;
							
							// Clamp checks

							if ((!pPrev) && (tmNewStart < 0))
							{			
								pKey->SetStartTime(0);
								pKey->SetEndTime(tmLen);
								bOkay = FALSE;
							}

							if (tmNewEnd > tmTotal)
							{
								pKey->SetStartTime(tmTotal - tmLen);
								pKey->SetEndTime(tmTotal);
								bOkay = FALSE;
							}

							if ((pPrev) && (tmNewStart < pPrev->GetEndTime()))
							{
								pKey->SetStartTime(pPrev->GetEndTime());
								pKey->SetEndTime(pPrev->GetEndTime() + tmLen);
								bOkay = FALSE;
							}

							if ((pNext) && (tmNewEnd >= pNext->GetStartTime()))
							{
								pKey->SetStartTime(pNext->GetStartTime() - tmLen);
								pKey->SetEndTime(pNext->GetStartTime());
								bOkay = FALSE;
							}

							// Check for links

							CKey *pLinkedKey = NULL;
							
							if (pKey->IsLinked()) pLinkedKey = m_pPhase->GetKeyByID(pKey->GetLinkedID());

							if (pLinkedKey)
							{
								if (tmNewStart < pLinkedKey->GetStartTime() + 50)
								{
									pKey->SetStartTime(pLinkedKey->GetStartTime() + 50);
									pKey->SetEndTime(pLinkedKey->GetStartTime() + 50 + pKey->GetAnchorLength());

									bOkay = FALSE;
								}

								if (tmNewEnd >= pLinkedKey->GetEndTime() - 50)
								{
									pKey->SetStartTime(pLinkedKey->GetEndTime() - 50 - pKey->GetAnchorLength());
									pKey->SetEndTime(pLinkedKey->GetEndTime() - 50);

									bOkay = FALSE;
								}
							}

							if (bOkay)
							{
								pKey->SetStartTime(tmNewStart);
								pKey->SetEndTime(tmNewEnd);
							}
						}

						pKeyNode = pKeyNode->m_pNext;
					}

					pTrackNode = pTrackNode->m_pNext;
				}
			}

			// Redraw !!

			DrawTracks(pDC);
			m_ptLast = ptCursor;

			// Refresh the phase info dialog

			m_phaseInfoDlg.Refresh();
		}
	}

	m_bTracking = FALSE;

	ReleaseDC(pDC);

	hCursor = ::LoadCursor(NULL, IDC_ARROW);
	::SetCursor(hCursor);
}
