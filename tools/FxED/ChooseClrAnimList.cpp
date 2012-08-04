// ChooseClrAnimList.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "ChooseClrAnimList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseClrAnimList

CChooseClrAnimList::CChooseClrAnimList()
{
}

CChooseClrAnimList::~CChooseClrAnimList()
{
}


BEGIN_MESSAGE_MAP(CChooseClrAnimList, CListBox)
	//{{AFX_MSG_MAP(CChooseClrAnimList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseClrAnimList message handlers

//------------------------------------------------------------------
//
//   FUNCTION : MeasureItem()
//
//   PURPOSE  : Performs owner draw item measurement
//
//------------------------------------------------------------------

void CChooseClrAnimList::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{		
	lpMeasureItemStruct->itemHeight = 30;
}

//------------------------------------------------------------------
//
//   FUNCTION : DrawItem()
//
//   PURPOSE  : Draws an owner draw item
//
//------------------------------------------------------------------

void CChooseClrAnimList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC *pDC;

	pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
	//dc.Attach(lpDrawItemStruct->hDC);

	CRect rcDraw = lpDrawItemStruct->rcItem;

	if (lpDrawItemStruct->itemID >= 0)
	{
		if (lpDrawItemStruct->itemAction & ODA_SELECT)
		{
			CBrush brRect;
			if (lpDrawItemStruct->itemState & ODS_SELECTED)
			{
				// Draw selection
				brRect.CreateSolidBrush( RGB(0, 0, 128));
			}
			else
			{
				brRect.CreateSolidBrush( RGB(255, 255, 255) );
			}

			pDC->FillRect( rcDraw, &brRect );
		}
		
		CString sTxt;
		GetText(lpDrawItemStruct->itemID, sTxt);

		CK_FAVOURITE *pFavourite = (CK_FAVOURITE *)GetItemData(lpDrawItemStruct->itemID);

		CSize szText = pDC->GetTextExtent( sTxt );

		pDC->TextOut(rcDraw.left + 2, rcDraw.top + (rcDraw.Height() / 2) - (szText.cy / 2), sTxt);

		// Draw the keyframes

		CRect rcFrame;
		rcFrame.left   = rcDraw.left + szText.cx + 10;
		rcFrame.top	   = rcDraw.top + 2;
		rcFrame.right  = rcDraw.right;
		rcFrame.bottom = rcDraw.bottom - 2;

		CBrush brBlack(RGB(0, 0, 0));
		pDC->FrameRect(&rcFrame, &brBlack);

		// Fill in the frame with the colour keys

		rcFrame.left ++;
		rcFrame.top ++;
		rcFrame.bottom --;
		rcFrame.right --;

		m_posColRatio = 1.0f / rcFrame.Width();

		for (int i = 0; i < rcFrame.Width(); i ++)
		{
			int r, g, b;

			PosToCol(pFavourite, i, &r, &g, &b);
				
			CBrush brCol(RGB(r, g, b));
			pDC->FillRect(CRect(i + rcFrame.left, rcFrame.top, rcFrame.left + i + 1, rcFrame.Height() + rcFrame.top), &brCol);
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : PosToCol()
//
//   PURPOSE  : Converts an x position to a colour
//
//------------------------------------------------------------------

void CChooseClrAnimList::PosToCol(CK_FAVOURITE *pFavourite, int xPos, int *pRed, int *pGreen, int *pBlue)
{
	*pRed   = 0;
	*pGreen = 0;
	*pBlue  = 0;

	CLinkListNode<COLOURKEY> *pNode = pFavourite->m_collKeys.GetHead();

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
