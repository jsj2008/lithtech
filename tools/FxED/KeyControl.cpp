// KeyControl.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "KeyControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TEXT_MIN_SPACING		5

/////////////////////////////////////////////////////////////////////////////
// CKeyControl

CKeyControl::CKeyControl()
{
	m_pMenuKey = NULL;
}

CKeyControl::~CKeyControl()
{
	if (m_pMemDC)
	{
		m_pMemDC->SelectObject(m_pOldBitmap);
		delete m_pMemDC;
		delete m_pBitmap;
	}
}


BEGIN_MESSAGE_MAP(CKeyControl, CStatic)
	//{{AFX_MSG_MAP(CKeyControl)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyControl message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnSize()
//
//   PURPOSE  : Handles WM_SIZE
//
//------------------------------------------------------------------

void CKeyControl::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
}

//------------------------------------------------------------------
//
//   FUNCTION : PreSubclassWindow()
//
//   PURPOSE  : Called after window is subclassed
//
//------------------------------------------------------------------

void CKeyControl::PreSubclassWindow() 
{
	CRect rcClient;
	GetClientRect(&rcClient);

	m_cx = rcClient.Width();
	m_cy = rcClient.Height();

	m_pMemDC = new CDC;
	if (!m_pMemDC) return;

	// Create the DC
	
	m_pMemDC->CreateCompatibleDC(NULL);

	m_pBitmap = new CBitmap;
	if (!m_pBitmap) return;

	// Create the bitmap

	CDC *pDC = GetDC();
	m_pBitmap->CreateCompatibleBitmap(pDC, m_cx, m_cy);
	ReleaseDC(pDC);

	// Select the bitmap into the dc

	m_pOldBitmap = m_pMemDC->SelectObject(m_pBitmap);

	// Set the text mode

	m_pMemDC->SetBkMode(TRANSPARENT);

	// Call the base class
	
	CStatic::PreSubclassWindow();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnPaint()
//
//   PURPOSE  : Handles WM_PAINT
//
//------------------------------------------------------------------

void CKeyControl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Clear the background

	ClearMemDC();

	// Draw the time bar

	DrawTimeBar();

	// Draw the contents

	DrawKeys();
	
	// Blit the memory DC

	dc.BitBlt(0, 0, m_cx, m_cy, m_pMemDC, 0, 0, SRCCOPY);
}

//------------------------------------------------------------------
//
//   FUNCTION : ClearMemDC()
//
//   PURPOSE  : Clears the background
//
//------------------------------------------------------------------

void CKeyControl::ClearMemDC()
{
	m_pMemDC->FillSolidRect(0, 0, m_cx, m_cy, RGB(255, 255, 255));
}

//------------------------------------------------------------------
//
//   FUNCTION : DrawKeys()
//
//   PURPOSE  : Draws the keys
//
//------------------------------------------------------------------

void CKeyControl::DrawKeys(CLinkListNode<KEY> *pSelectNode)
{
	CLinkListNode<KEY> *pNode = m_collKeys.GetHead();

	// Draw the background lines
	
	if (pNode)
	{
		while (pNode->m_pNext)
		{
			m_pMemDC->MoveTo(KeyToPos(&pNode->m_Data));
			m_pMemDC->LineTo(KeyToPos(&pNode->m_pNext->m_Data));

			pNode = pNode->m_pNext;
		}
	}

	// Draw the keys
	
	pNode = m_collKeys.GetHead();

	CBrush brBlack;
	brBlack.CreateSolidBrush(RGB(0, 0, 0));

	CBrush *pOldBrush = m_pMemDC->SelectObject(&brBlack);
	
	while (pNode)
	{
		CPoint ptCenter = KeyToPos(&pNode->m_Data);
		
		if (pNode == pSelectNode)
		{
			m_pMemDC->MoveTo(ptCenter.x, ptCenter.y);
			m_pMemDC->LineTo(ptCenter.x, m_cy);

			CBrush brGreen(RGB(255, 0, 0));
			CBrush *pOldBrush = m_pMemDC->SelectObject(&brGreen);

			// Draw the time lines down to the bottom of the screen

			int nTriSize = 5;
			
			POINT ptTri[3];
			ptTri[0].x = ptCenter.x - nTriSize;
			ptTri[0].y = m_cy - nTriSize;

			ptTri[1].x = ptCenter.x + nTriSize;
			ptTri[1].y = m_cy - nTriSize;

			ptTri[2].x = ptCenter.x;
			ptTri[2].y = m_cy;

			m_pMemDC->Polygon(ptTri, 3);

			m_pMemDC->SelectObject(pOldBrush);
		}

		if (pNode->m_Data.m_bSelected)
		{
			m_pMemDC->FillSolidRect(CRect(ptCenter.x - 5, ptCenter.y - 5, ptCenter.x + 5, ptCenter.y + 5), RGB(255, 0, 0));
			m_pMemDC->FrameRect(CRect(ptCenter.x - 5, ptCenter.y - 5, ptCenter.x + 5, ptCenter.y + 5), &brBlack);
		}
		else
		{
			m_pMemDC->FillSolidRect(CRect(ptCenter.x - 5, ptCenter.y - 5, ptCenter.x + 5, ptCenter.y + 5), RGB(150, 150, 150));
			m_pMemDC->FrameRect(CRect(ptCenter.x - 5, ptCenter.y - 5, ptCenter.x + 5, ptCenter.y + 5), &brBlack);
		}

		if (pNode == pSelectNode)
		{
			CString sVal = GetTrackValue(pSelectNode);
			CSize szText = m_pMemDC->GetTextExtent(sVal);

			ptCenter.x += 10;
			if (ptCenter.x + szText.cx > m_cx)
			{
				ptCenter.x = m_cx - szText.cx;
			}

			ptCenter.y -= 5;
			ptCenter.y -= szText.cy;

			if (ptCenter.y < 0)
			{
				ptCenter.y = 0;
			}

			m_pMemDC->TextOut(ptCenter.x, ptCenter.y, GetTrackValue(pSelectNode));
		}

		pNode = pNode->m_pNext;
	}

	m_pMemDC->SelectObject(pOldBrush);
}

//------------------------------------------------------------------
//
//   FUNCTION : KeyToPos()
//
//   PURPOSE  : Converts a key to a position
//
//------------------------------------------------------------------

CPoint CKeyControl::KeyToPos(KEY *pKey)
{
	return CPoint((int)((pKey->m_tmAnchor + pKey->m_tmKey) * (float)m_cx),
				  (int)(m_cy - ((pKey->m_val + pKey->m_valAnchor) * (float)m_cy)));
}

//------------------------------------------------------------------
//
//   FUNCTION : OnLButtonDown()
//
//   PURPOSE  : Handles WM_LBUTTONDOWN
//
//------------------------------------------------------------------

void CKeyControl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CLinkListNode<KEY> *pNode = PtInKey(point);

	if (PtInKey(point))
	{
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			pNode->m_Data.m_bSelected = !pNode->m_Data.m_bSelected;

			Redraw();
		}
		else
		{
			TrackKey(pNode, KeyToPos(&pNode->m_Data), point);
		}

		return;
	}
	
	AddKey(point);

	InvalidateRect(NULL, TRUE);
	
	CStatic::OnLButtonDown(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : AddKey()
//
//   PURPOSE  : Adds a new key
//
//------------------------------------------------------------------

void CKeyControl::AddKey(CPoint ptKey)
{
	// Add in a new key in order

	KEY newKey;

	newKey.m_tmAnchor  = (float)ptKey.x / m_cx;
	newKey.m_valAnchor = ((float)(m_cy - ptKey.y) / m_cy);
	newKey.m_pData     = GetNewKeyData();

	CLinkListNode<KEY> *pNode = m_collKeys.GetHead();

	if (!pNode)
	{
		newKey.m_tmAnchor = 0.0f;
		m_collKeys.AddHead(newKey);
	}
	else if (!pNode->m_pNext)
	{
		newKey.m_tmAnchor = 1.0f;
		m_collKeys.AddTail(newKey);
	}
	else
	{
		while (pNode->m_pNext)
		{
			if ((newKey.m_tmAnchor >= pNode->m_Data.m_tmAnchor) && 
				(newKey.m_tmAnchor < pNode->m_pNext->m_Data.m_tmAnchor))
			{
				m_collKeys.InsertAfter(pNode, newKey);

				return;
			}

			pNode = pNode->m_pNext;
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : TrackKey()
//
//   PURPOSE  : Tracks and moves a key
//
//------------------------------------------------------------------

void CKeyControl::TrackKey(CLinkListNode<KEY> *pNode, CPoint ptAnchor, CPoint ptOffset)
{
	CPoint ptLast = ptAnchor;
	CPoint ptCur;

	int xOff = ptOffset.x - ptAnchor.x;
	int yOff = ptOffset.y - ptAnchor.y;

	pNode->m_Data.m_bSelected = TRUE;

	// Make a list of all the keys that are selected

	CLinkList<CLinkListNode<KEY> *> collSelKeys;

	CLinkListNode<KEY> *pKeyNode = m_collKeys.GetHead();

	while (pKeyNode)
	{
		if (pKeyNode->m_Data.m_bSelected) collSelKeys.AddTail(pKeyNode);
		
		pKeyNode = pKeyNode->m_pNext;
	}

	while (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
	{
		GetCursorPos(&ptCur);
		ScreenToClient(&ptCur);

		ptCur.x -= xOff;
		ptCur.y -= yOff;

		if (ptCur != ptLast)
		{
			float tmKeyBase = (float)ptCur.x / (float)m_cx;
			float valBase   = (float)(m_cy - ptCur.y) / (float)m_cy;

			if (valBase < 0.0f) valBase = 0.0f;
			if (valBase > 1.0f) valBase = 1.0f;

			CLinkListNode<CLinkListNode<KEY> *> *pSelNode = collSelKeys.GetHead();

			while (pSelNode)
			{
				CLinkListNode<KEY> *pCurNode = pSelNode->m_Data;

				float tmKey = tmKeyBase - pNode->m_Data.m_tmAnchor;
				float val   = valBase - pNode->m_Data.m_valAnchor;

				float tmActual = tmKey + pCurNode->m_Data.m_tmAnchor;
				
				if (pCurNode->m_pPrev)
				{
					float tmPrevActual = pCurNode->m_pPrev->m_Data.m_tmAnchor + pCurNode->m_pPrev->m_Data.m_tmKey;

					if (tmPrevActual > tmActual)
					{
						tmKey = tmPrevActual - pCurNode->m_Data.m_tmAnchor;
					}
				}
				else
				{
					tmKey = 0.0f;
				}

				if (pCurNode->m_pNext)
				{
					float tmNextActual = pCurNode->m_pNext->m_Data.m_tmAnchor + pCurNode->m_pNext->m_Data.m_tmKey;

					if (tmNextActual < tmActual)
					{
						tmKey = tmNextActual - pCurNode->m_Data.m_tmAnchor;
					}
				}
				else
				{
					tmKey = 1.0f - pCurNode->m_Data.m_tmAnchor;
				}

				if (val + pCurNode->m_Data.m_valAnchor < 0.0f) val = -pCurNode->m_Data.m_valAnchor;
				if (val + pCurNode->m_Data.m_valAnchor > 1.0f) val = 1.0f - pCurNode->m_Data.m_valAnchor;

				pCurNode->m_Data.m_tmKey = tmKey;
				pCurNode->m_Data.m_val = val;

				pSelNode = pSelNode->m_pNext;
			}

			ptLast = ptCur;
		}
		
		Redraw(pNode);
	}

	// And update the new positions

	pNode = m_collKeys.GetHead();

	while (pNode)
	{
		pNode->m_Data.m_tmAnchor  += pNode->m_Data.m_tmKey;
		pNode->m_Data.m_valAnchor += pNode->m_Data.m_val;
		pNode->m_Data.m_tmKey = 0.0f;
		pNode->m_Data.m_val   = 0.0f;
		pNode->m_Data.m_bSelected = FALSE;
		
		pNode = pNode->m_pNext;
	}

	// And one final redraw to remove the selected stuff
	
	Redraw(NULL);
}

//------------------------------------------------------------------
//
//   FUNCTION : Redraw()
//
//   PURPOSE  : Redraws everything
//
//------------------------------------------------------------------

void CKeyControl::Redraw(CLinkListNode<KEY> *pSelectNode)
{
	CDC *pDC = GetDC();

	// Clear the background

	ClearMemDC();

	// Draw the time bar

	DrawTimeBar();

	// Draw the contents

	DrawKeys(pSelectNode);
	
	// Blit the memory DC

	pDC->BitBlt(0, 0, m_cx, m_cy, m_pMemDC, 0, 0, SRCCOPY);

	ReleaseDC(pDC);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRButtonDown()
//
//   PURPOSE  : Handles WM_RBUTTONDOWN
//
//------------------------------------------------------------------

void CKeyControl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CMenu rbMenu;
	rbMenu.CreatePopupMenu();

	CLinkListNode<KEY> *pNode = PtInKey(point);
	
	if (pNode)
	{
		rbMenu.AppendMenu(MF_STRING, 5000, "&Edit Key");
		if ((pNode->m_pPrev) && (pNode->m_pNext))
		{
			rbMenu.AppendMenu(MF_SEPARATOR, 5001, "");
			rbMenu.AppendMenu(MF_STRING, 5002, "&Delete Key");
		}
		
		m_pMenuKey = pNode;			

		ClientToScreen(&point);
		rbMenu.TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
	}

	
	CStatic::OnRButtonDown(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : PtInKey()
//
//   PURPOSE  : Tests a point to see if it's in a key
//
//------------------------------------------------------------------

CLinkListNode<KEY>* CKeyControl::PtInKey(CPoint ptTest)
{
	CLinkListNode<KEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		CPoint ptKey = KeyToPos(&pNode->m_Data);
		CRect rcKey(ptKey.x - 5, ptKey.y - 5, ptKey.x + 5, ptKey.y + 5);

		if (rcKey.PtInRect(ptTest)) return pNode;

		pNode = pNode->m_pNext;
	}

	return NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnCommand()
//
//   PURPOSE  : Handles menu commands etc
//
//------------------------------------------------------------------

BOOL CKeyControl::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD wID = LOWORD(wParam);

	switch (wID)
	{
		case 5000 :
		{
			if (m_pMenuKey)
			{
				// Edit this key

				EditKey(m_pMenuKey);
			}
		}
		break;

		case 5002 :
		{
			if (m_pMenuKey)
			{		
				// Delete this key

				m_collKeys.Remove(m_pMenuKey);
				m_pMenuKey = NULL;

				// Redraw

				InvalidateRect(NULL, TRUE);
			}
		}
		break;
	}
	
	return CStatic::OnCommand(wParam, lParam);
}

//------------------------------------------------------------------
//
//   FUNCTION : GetTrackValue()
//
//   PURPOSE  : Returns a string containing the tracked key value
//
//------------------------------------------------------------------

CString CKeyControl::GetTrackValue(CLinkListNode<KEY> *pNode)
{
	CString sVal;
	sVal.Format(FLOAT_PRECISION, pNode->m_Data.m_valAnchor + pNode->m_Data.m_val);

	return sVal;
}

//------------------------------------------------------------------
//
//   FUNCTION : DrawTimeBar()
//
//   PURPOSE  : Draws the time bar and notches
//
//------------------------------------------------------------------

void CKeyControl::DrawTimeBar()
{
	if (!m_pKey) return;

	DWORD tmStart = m_pKey->GetStartTime();
	DWORD tmEnd   = m_pKey->GetEndTime();

	// Draw the notches every 100 Milliseconds

	DWORD dwNumTicks = (tmEnd - tmStart) / 100;

	float timeToPos = (float)m_cx / (float)(tmEnd - tmStart);

	int nLastTextPos = -50;

	for (DWORD i = 0; i < dwNumTicks + 2; i ++)
	{
		DWORD dwTime = ((tmStart / 100) * 100) + (i * 100);

		int xPos = (int)((float)(dwTime - tmStart) * timeToPos);

		if ((dwTime % 1000) == 0)
		{
			int nSec = (int)dwTime / 1000;

			char sTmp[256];
			sprintf(sTmp, "%d", nSec);

			CSize szText = m_pMemDC->GetTextExtent(sTmp);

			m_pMemDC->MoveTo(xPos, m_cy - 6);
			m_pMemDC->LineTo(xPos, m_cy);

			if(xPos - (szText.cx / 2) > nLastTextPos)
			{
				m_pMemDC->TextOut(xPos - (szText.cx / 2), m_cy - (6 + szText.cy), sTmp);
				nLastTextPos = xPos + (szText.cx / 2) + TEXT_MIN_SPACING;
			}
		}
		else
		{
			m_pMemDC->MoveTo(xPos, m_cy - 3);
			m_pMemDC->LineTo(xPos, m_cy);
		}
	}
}