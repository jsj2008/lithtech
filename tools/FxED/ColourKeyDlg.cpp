// ColourKeyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "ColourKeyDlg.h"
#include "ChooseClrAnimDlg.h"
#include "StringDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TEXT_MIN_SPACING		5

/////////////////////////////////////////////////////////////////////////////
// CColourKeyDlg dialog


CColourKeyDlg::CColourKeyDlg(CLinkList<COLOURKEY> *pList, CKey *pKey, CWnd* pParent /*=NULL*/)
	: CDialog(CColourKeyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CColourKeyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Copy the keys over

	CLinkListNode<COLOURKEY> *pSrcNode = pList->GetHead();

	while (pSrcNode)
	{
		m_collKeys.AddTail(pSrcNode->m_Data);
		
		pSrcNode = pSrcNode->m_pNext;
	}

	m_pSelKey = NULL;
	m_pKey = pKey;
}


void CColourKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColourKeyDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		GetDlgItem(IDC_DELETEKEY)->EnableWindow(FALSE);
	}
	else
	{
	}
}


BEGIN_MESSAGE_MAP(CColourKeyDlg, CDialog)
	//{{AFX_MSG_MAP(CColourKeyDlg)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_FULLOPAC, OnFullOpacity)
	ON_BN_CLICKED(IDC_FULLTRANS, OnFullTranslucency)
	ON_BN_CLICKED(IDC_HALF, OnHalfAndHalf)
	ON_BN_CLICKED(IDC_CHOOSEFAVOURITE, OnChooseFavourite)
	ON_BN_CLICKED(IDC_ADDTOFAVOURITES, OnAddToFavourites)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(IDC_DELETEKEY, OnDeleteKey)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColourKeyDlg message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnInitDialog()
//
//   PURPOSE  : Handles WM_INITDIALOG
//
//------------------------------------------------------------------

BOOL CColourKeyDlg::OnInitDialog() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	CDialog::OnInitDialog();

	CDC *pDC = GetDC();

	CRect rcKeys;
	GetDlgItem(IDC_KEYCONTROL)->GetClientRect(&rcKeys);

	// Create the dc

	m_memDC.CreateCompatibleDC(NULL);

	// Create the bitmap

	m_bitmap.CreateCompatibleBitmap(pDC, rcKeys.Width(), rcKeys.Height());

	// Select the bitmap

	m_pOldBitmap = m_memDC.SelectObject(&m_bitmap);

	m_posColRatio = 1.0f / (float)rcKeys.Width();

	// Draw the keyframes
	
	DrawKeys(pDC);

	ReleaseDC(pDC);

	if (!pApp->GetColourFavourites()->GetSize()) GetDlgItem(IDC_CHOOSEFAVOURITE)->EnableWindow(FALSE);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//------------------------------------------------------------------
//
//   FUNCTION : DrawKeys()
//
//   PURPOSE  : Draws the keyframes
//
//------------------------------------------------------------------

void CColourKeyDlg::DrawTimeBar()
{
	if (!m_pKey) return;

	DWORD tmStart = m_pKey->GetStartTime();
	DWORD tmEnd   = m_pKey->GetEndTime();

	CRect rcClient;
	GetDlgItem(IDC_KEYCONTROL)->GetClientRect(&rcClient);

	int cx = rcClient.Width();
	int cy = rcClient.Height();
	
	// Draw the notches every 100 Milliseconds

	DWORD dwNumTicks = (tmEnd - tmStart) / 100;

	float timeToPos = (float)cx / (float)(tmEnd - tmStart);

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

			CSize szText = m_memDC.GetTextExtent(sTmp);

			m_memDC.MoveTo(xPos, cy - 6);
			m_memDC.LineTo(xPos, cy);

			m_memDC.SetBkMode(TRANSPARENT);

			if(xPos - (szText.cx / 2) > nLastTextPos)
			{
				m_memDC.TextOut(xPos - (szText.cx / 2), cy - (6 + szText.cy), sTmp);
				nLastTextPos = xPos + (szText.cx / 2) + TEXT_MIN_SPACING;
			}
		}
		else
		{
			m_memDC.MoveTo(xPos, cy - 3);
			m_memDC.LineTo(xPos, cy);
		}
	}
}

void CColourKeyDlg::DrawKeys(CDC *pDC, BOOL bShowVal)
{
	CWnd *pWnd = GetDlgItem(IDC_KEYCONTROL);

	CRect rcKeys;
	pWnd->GetClientRect(&rcKeys);

	if (m_collKeys.GetSize() < 2) return;

	for (int i = 0; i < rcKeys.Width(); i ++)
	{
		int r, g, b;

		PosToCol(i, &r, &g, &b);
	
		CBrush brCol(RGB(r, g, b));
		m_memDC.FillRect(CRect(i, 0, i + 1, rcKeys.Height()), &brCol);
	}

	// Run through the keys and draw the alpha circles

	CLinkListNode<COLOURKEY> *pNode = m_collKeys.GetHead();

	CPoint ptLast;

	while (pNode)
	{
		int xPos = (int)(pNode->m_Data.m_tmKey / m_posColRatio);
		int yPos = (int)((float)rcKeys.Height() * (1.0f - pNode->m_Data.m_alpha));

		if (pNode->m_pPrev)
		{
			m_memDC.MoveTo(ptLast);
			m_memDC.LineTo(CPoint(xPos, yPos));
		}

		ptLast.x = xPos;
		ptLast.y = yPos;
				
		pNode = pNode->m_pNext;
	}
	
	pNode = m_collKeys.GetHead();

	while (pNode)
	{
		int xPos = (int)(pNode->m_Data.m_tmKey / m_posColRatio);
		int yPos = (int)((float)rcKeys.Height() * (1.0f - pNode->m_Data.m_alpha));

		CBrush brBlack(RGB(0, 0, 0));
		CBrush brRed(RGB(255, 0, 0));

		m_memDC.FillSolidRect(CRect(xPos - 5, yPos - 5, xPos + 5, yPos + 5), (pNode == m_pSelKey) ? RGB(230, 0, 0) : RGB(128, 128, 128));
		m_memDC.FrameRect(CRect(xPos - 5, yPos - 5, xPos + 5, yPos + 5), (pNode == m_pSelKey) ? &brRed : &brBlack);

		if ((pNode == m_pSelKey) && (bShowVal))
		{
			char sTmp[256];
			
			if (pNode->m_Data.m_alpha <= 0.25f)
			{
				sprintf(sTmp, COLORKEY_PRECISION" [Opaque]", pNode->m_Data.m_alpha);
			}
			else if (pNode->m_Data.m_alpha <= 0.5f)
			{
				sprintf(sTmp, COLORKEY_PRECISION" [Semi-Opaque]", pNode->m_Data.m_alpha);
			}
			else if (pNode->m_Data.m_alpha <= 0.75f)
			{
				sprintf(sTmp, COLORKEY_PRECISION" [Semi-Translucent]", pNode->m_Data.m_alpha);
			}
			else
			{
				sprintf(sTmp, COLORKEY_PRECISION" [Translucent]", pNode->m_Data.m_alpha);
			}

			m_memDC.SetBkMode(TRANSPARENT);
			m_memDC.SetTextColor(RGB(128, 128, 128));

			m_memDC.TextOut(xPos + 10, yPos, sTmp);
		}

		ptLast.x = xPos;
		ptLast.y = yPos;
				
		pNode = pNode->m_pNext;
	}

	// Draw the time....

	DrawTimeBar();

	pWnd->GetWindowRect(&rcKeys);
	ScreenToClient(&rcKeys);

	pDC->BitBlt(rcKeys.left, rcKeys.top, rcKeys.Width(), rcKeys.Height(), &m_memDC, 0, 0, SRCCOPY);
}

//------------------------------------------------------------------
//
//   FUNCTION : PosToCol()
//
//   PURPOSE  : Converts an x position to a colour
//
//------------------------------------------------------------------

void CColourKeyDlg::PosToCol(int xPos, int *pRed, int *pGreen, int *pBlue)
{
	*pRed   = 0;
	*pGreen = 0;
	*pBlue  = 0;

	if (m_collKeys.GetSize() < 2) return;

	CLinkListNode<COLOURKEY> *pNode = m_collKeys.GetHead();

	while (pNode->m_pNext)
	{
		float tmPos = (float)xPos * m_posColRatio;
		
		if ((tmPos >= pNode->m_Data.m_tmKey) && (tmPos < pNode->m_pNext->m_Data.m_tmKey))
		{
			// Compute the colour

			float rs = pNode->m_Data.m_red;
			float gs = pNode->m_Data.m_green;
			float bs = pNode->m_Data.m_blue;

			float re = pNode->m_pNext->m_Data.m_red;
			float ge = pNode->m_pNext->m_Data.m_green;
			float be = pNode->m_pNext->m_Data.m_blue;

			int xKeyStart = (int)(pNode->m_Data.m_tmKey / m_posColRatio);
			int xKeyEnd   = (int)(pNode->m_pNext->m_Data.m_tmKey / m_posColRatio);
		
			if (xKeyStart == xPos)
			{
				*pRed   = (int)(rs * 255.0f);
				*pGreen = (int)(gs * 255.0f);
				*pBlue  = (int)(bs * 255.0f);
			}
			else
			{
				float tmTotalDist = pNode->m_pNext->m_Data.m_tmKey - pNode->m_Data.m_tmKey;
				float tmDist = tmPos - pNode->m_pNext->m_Data.m_tmKey;

				float rRatio = (re - rs) / (float)(xKeyEnd - xKeyStart);
				float r = 255.0f * (rs + (rRatio * (xPos - xKeyStart)));

				float gRatio = (ge - gs) / (float)(xKeyEnd - xKeyStart);
				float g = 255.0f * (gs + (gRatio * (xPos - xKeyStart)));

				float bRatio = (be - bs) / (float)(xKeyEnd - xKeyStart);
				float b = 255.0f * (bs + (bRatio * (xPos - xKeyStart)));

				*pRed   = (int)r;
				*pGreen = (int)g;
				*pBlue  = (int)b;
			}
		}

		pNode = pNode->m_pNext;
	}
}


void CColourKeyDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Draw the keyframes

	DrawKeys(&dc);
	
	// Do not call CDialog::OnPaint() for painting messages
}

//------------------------------------------------------------------
//
//   FUNCTION : OnLButtonDown()
//
//   PURPOSE  : Handles WM_LBUTTONDOWN
//
//------------------------------------------------------------------

void CColourKeyDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd *pWnd = GetDlgItem(IDC_KEYCONTROL);
	CRect rcWnd;
	pWnd->GetWindowRect(&rcWnd);
	ScreenToClient(&rcWnd);

	CPoint ptKeys = point;
	ptKeys.x -= rcWnd.left;
	ptKeys.y -= rcWnd.top;

	// Check to see if we have clicked on a key circle

	CLinkListNode<COLOURKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		int xPos = (int)(pNode->m_Data.m_tmKey / m_posColRatio);
		int yPos = (int)((float)rcWnd.Height() * (1.0f - pNode->m_Data.m_alpha));

		CRect rcCheck(xPos - 5, yPos - 5, xPos + 5, yPos + 5);

		if (rcCheck.PtInRect(ptKeys))
		{
			// Select the node

			SelectNode(pNode);

			// Check to see if we need to delete it

			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				OnDeleteKey();

				return;
			}
			
			// Track the key

			TrackKey(pNode);

			return;
		}
		
		pNode = pNode->m_pNext;
	}

	if (rcWnd.PtInRect(point))
	{
		COLORREF rgbInit;
		
		if (m_pSelKey)
		{
			rgbInit = m_pSelKey->m_Data.GetColorRef();
		}
		else
		{
			rgbInit = RGB(255, 255, 255);
		}

		CColorDialog dlg(RGB(255, 255, 255), CC_FULLOPEN);

		if (dlg.DoModal() == IDOK)
		{
			COLORREF cref = dlg.GetColor();

			int red   = cref & 0x000000FF;
			int green = (cref & 0x0000FF00) >> 8;
			int blue  = (cref & 0x00FF0000) >> 16;

			COLOURKEY newKey;
			newKey.m_tmKey = (float)ptKeys.x * m_posColRatio;

			newKey.m_red   = (float)red / 255.0f;
			newKey.m_green = (float)green / 255.0f;
			newKey.m_blue  = (float)blue / 255.0f;
			newKey.m_alpha = ((1.0f / (float)rcWnd.Height()) * (float)(rcWnd.Height() - (point.y - rcWnd.top)));

			CLinkListNode<COLOURKEY> *pNode = m_collKeys.GetHead();

			while (pNode->m_pNext)
			{
				if ((newKey.m_tmKey >= pNode->m_Data.m_tmKey) && (newKey.m_tmKey < pNode->m_pNext->m_Data.m_tmKey))
				{
					// Insert the new colour keyframe

					CLinkListNode<COLOURKEY> *pNewNode = m_collKeys.InsertAfter(pNode, newKey);

					SelectNode(pNewNode);

					break;
				}
				
				pNode = pNode->m_pNext;
			}

			InvalidateRect(&rcWnd, TRUE);
		}
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : TrackKey()
//
//   PURPOSE  : Tracks an individual key
//
//------------------------------------------------------------------

void CColourKeyDlg::TrackKey(CLinkListNode<COLOURKEY> *pNode)
{
	CWnd *pWnd = GetDlgItem(IDC_KEYCONTROL);
	CRect rcWnd;
	pWnd->GetWindowRect(&rcWnd);
	ScreenToClient(&rcWnd);
	
	BOOL bTracking = TRUE;

	CPoint ptAnchor;
	CPoint ptLast;

	GetCursorPos(&ptAnchor);
	pWnd->ScreenToClient(&ptAnchor);

	ptLast = ptAnchor;

	while (bTracking)
	{
		CPoint ptCur;
		GetCursorPos(&ptCur);
		pWnd->ScreenToClient(&ptCur);
		ptCur.y = rcWnd.Height() - ptCur.y;

		if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) bTracking = FALSE;

		if (ptCur != ptLast)
		{
			// Recalculate alpha

			float alpha = (1.0f / (float)rcWnd.Height()) * (float)ptCur.y;
			if (alpha < 0.0f) alpha = 0.0f;
			if (alpha > 1.0f) alpha = 1.0f;

			pNode->m_Data.m_alpha = alpha;

			// Recalculate position

			float tmKey = (1.0f / (float)rcWnd.Width()) * (float)ptCur.x;
			
			if (pNode->m_pPrev)
			{
				if (tmKey < pNode->m_pPrev->m_Data.m_tmKey)
				{
					tmKey = pNode->m_pPrev->m_Data.m_tmKey;
				}
			}
			else
			{
				tmKey = 0.0f;
			}


			if (pNode->m_pNext)
			{
				if (tmKey > pNode->m_pNext->m_Data.m_tmKey)
				{
					tmKey = pNode->m_pNext->m_Data.m_tmKey;
				}
			}
			else
			{
				tmKey = 1.0f;
			}

			pNode->m_Data.m_tmKey = tmKey;

			CDC *pDC = GetDC();
			DrawKeys(pDC, TRUE);
			ReleaseDC(pDC);

			ptLast = ptCur;
		}
	}

	CDC *pDC = GetDC();
	DrawKeys(pDC, FALSE);
	ReleaseDC(pDC);
}

//------------------------------------------------------------------
//
//   FUNCTION : SelectNode()
//
//   PURPOSE  : Selects a specific keyframe node
//
//------------------------------------------------------------------

void CColourKeyDlg::SelectNode(CLinkListNode<COLOURKEY> *pSelNode)
{
	m_pSelKey = pSelNode;

	if ((m_pSelKey->m_pNext) && (m_pSelKey->m_pPrev))
	{
		GetDlgItem(IDC_DELETEKEY)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_DELETEKEY)->EnableWindow(FALSE);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRButtonDown()
//
//   PURPOSE  : Handler for WM_RBUTTONDOWN
//
//------------------------------------------------------------------

void CColourKeyDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{		
	CDialog::OnRButtonDown(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRButtonUp()
//
//   PURPOSE  : Handles WM_RBUTTONUP
//
//------------------------------------------------------------------

void CColourKeyDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CWnd *pWnd = GetDlgItem(IDC_KEYCONTROL);
	CRect rcWnd;
	pWnd->GetWindowRect(&rcWnd);
	ScreenToClient(&rcWnd);

	CPoint ptKeys = point;
	ptKeys.x -= rcWnd.left;
	ptKeys.y -= rcWnd.top;

	// Check to see if we have clicked on a key circle

	CLinkListNode<COLOURKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		int xPos = (int)(pNode->m_Data.m_tmKey / m_posColRatio);
		int yPos = (int)((float)rcWnd.Height() * (1.0f - pNode->m_Data.m_alpha));

		CRect rcCheck(xPos - 5, yPos - 5, xPos + 5, yPos + 5);

		if (rcCheck.PtInRect(ptKeys))
		{
			m_pSelKey = pNode;
			
			// Change it's colour

			CColorDialog dlg(pNode->m_Data.GetColorRef(), CC_FULLOPEN);

			if (dlg.DoModal() == IDOK)
			{
				COLORREF cref = dlg.GetColor();

				int red   = cref & 0x000000FF;
				int green = (cref & 0x0000FF00) >> 8;
				int blue  = (cref & 0x00FF0000) >> 16;

				pNode->m_Data.m_red   = (float)red / 255.0f;
				pNode->m_Data.m_green = (float)green / 255.0f;
				pNode->m_Data.m_blue  = (float)blue / 255.0f;

				CDC *pDC = GetDC();
				DrawKeys(pDC);
				ReleaseDC(pDC);
			}

			return;
		}
		
		pNode = pNode->m_pNext;
	}
		
	CDialog::OnRButtonUp(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFullOpacity()
//
//   PURPOSE  : Called to make all keys fully opaque
//
//------------------------------------------------------------------

void CColourKeyDlg::OnFullOpacity() 
{
	int ret = AfxMessageBox("Are you sure ?", MB_YESNO);
	if (ret != IDYES) return;

	CLinkListNode<COLOURKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		pNode->m_Data.m_alpha = 0.0f;
		
		pNode = pNode->m_pNext;
	}

	CDC *pDC = GetDC();
	DrawKeys(pDC);
	ReleaseDC(pDC);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFullTranslucency()
//
//   PURPOSE  : Called to make all keys fully translucent
//
//------------------------------------------------------------------

void CColourKeyDlg::OnFullTranslucency() 
{
	int ret = AfxMessageBox("Are you sure ?", MB_YESNO);
	if (ret != IDYES) return;

	CLinkListNode<COLOURKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		pNode->m_Data.m_alpha = 1.0f;
		
		pNode = pNode->m_pNext;
	}

	CDC *pDC = GetDC();
	DrawKeys(pDC);
	ReleaseDC(pDC);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnHalfAndHalf()
//
//   PURPOSE  : Called to make all keys half translucent
//
//------------------------------------------------------------------

void CColourKeyDlg::OnHalfAndHalf() 
{
	int ret = AfxMessageBox("Are you sure ?", MB_YESNO);
	if (ret != IDYES) return;

	CLinkListNode<COLOURKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		pNode->m_Data.m_alpha = 0.5f;
		
		pNode = pNode->m_pNext;
	}

	CDC *pDC = GetDC();
	DrawKeys(pDC);
	ReleaseDC(pDC);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnChooseFavourite()
//
//   PURPOSE  : Picks a colour animation from the favourites list
//
//------------------------------------------------------------------

void CColourKeyDlg::OnChooseFavourite() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	CChooseClrAnimDlg dlg(pApp->GetColourFavourites());;

	if ((dlg.DoModal() == IDOK) && (dlg.m_pFavourite))
	{
		m_collKeys.RemoveAll();

		// Copy in the new colour keys

		CLinkListNode<COLOURKEY> *pNode = dlg.m_pFavourite->m_collKeys.GetHead();

		while (pNode)
		{
			m_collKeys.AddTail(pNode->m_Data);

			pNode = pNode->m_pNext;
		}

		CDC *pDC = GetDC();
		DrawKeys(pDC);
		ReleaseDC(pDC);		
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnAddToFavourites()
//
//   PURPOSE  : Adds this animation to the list of favourites
//
//------------------------------------------------------------------

void CColourKeyDlg::OnAddToFavourites() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	CStringDlg dlg("Choose Name");

	if (dlg.DoModal() == IDOK)
	{
		CK_FAVOURITE *pNewFavourite = new CK_FAVOURITE;

		pNewFavourite->m_sName = dlg.m_sText;

		// Copy the current keys

		CLinkListNode<COLOURKEY> *pNode = m_collKeys.GetHead();

		while (pNode)
		{
			pNewFavourite->m_collKeys.AddTail(pNode->m_Data);
			
			pNode = pNode->m_pNext;
		}

		// Add it

		pApp->GetColourFavourites()->AddTail(pNewFavourite);

		if (pApp->GetColourFavourites()->GetSize())
		{
			GetDlgItem(IDC_CHOOSEFAVOURITE)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_CHOOSEFAVOURITE)->EnableWindow(FALSE);
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnReset()
//
//   PURPOSE  : Resets all the colour keys
//
//------------------------------------------------------------------

void CColourKeyDlg::OnReset() 
{
	int ret = AfxMessageBox("Are you sure ?", MB_YESNO | MB_ICONEXCLAMATION);
	if (ret == IDYES)
	{
		m_collKeys.RemoveAll();

		COLOURKEY k;
		
		k.m_tmKey = 0.0f;
		k.m_red   = 1.0f;
		k.m_green = 1.0f;
		k.m_blue  = 1.0f;
		m_collKeys.AddTail(k);

		k.m_tmKey = 1.0f;
		k.m_red   = 1.0f;
		k.m_green = 1.0f;
		k.m_blue  = 1.0f;
		m_collKeys.AddTail(k);

		CDC *pDC = GetDC();
		DrawKeys(pDC);
		ReleaseDC(pDC);		
	}	
}

void CColourKeyDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_DELETE)
	{
		int a = 1;
	}
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CColourKeyDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	int a = 1;
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDeleteKey()
//
//   PURPOSE  : Deletes the currently selected key
//
//------------------------------------------------------------------

void CColourKeyDlg::OnDeleteKey() 
{
	if ((m_pSelKey) && (m_pSelKey->m_pPrev) && (m_pSelKey->m_pNext))
	{
		m_collKeys.Remove(m_pSelKey);

		m_pSelKey = NULL;

		CDC *pDC = GetDC();
		DrawKeys(pDC);
		ReleaseDC(pDC);
		
		GetDlgItem(IDC_DELETEKEY)->EnableWindow(FALSE);	
	}	
}
