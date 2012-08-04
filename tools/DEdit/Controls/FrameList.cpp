//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// FrameList.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "framelist.h"
#include "texture.h"
#include "edithelpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define INDENT					4


/////////////////////////////////////////////////////////////////////////////
// CFrameList

CFrameList::CFrameList() :
	m_bDrawNumbers(FALSE),
	m_pNotifier(NULL),
	m_fZoom(1.0f),
	m_uContextMenu(CG_IDR_POPUP_FRAMELIST)
{
	m_bDrawNumbers = FALSE;
	m_pNotifier = NULL;
	m_fZoom = 1.0f;
}

CFrameList::~CFrameList()
{
}


BEGIN_MESSAGE_MAP(CFrameList, CListBox)
	//{{AFX_MSG_MAP(CFrameList)
	ON_CONTROL_REFLECT(LBN_DBLCLK, OnDblclk)
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelchange)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_POPUP_FRAMELIST_ZOOM_100, OnPopupFramelistZoom100)
	ON_COMMAND(ID_POPUP_FRAMELIST_ZOOM_200, OnPopupFramelistZoom200)
	ON_COMMAND(ID_POPUP_FRAMELIST_ZOOM_400, OnPopupFramelistZoom400)
	ON_COMMAND(ID_POPUP_FRAMELIST_ZOOM_50, OnPopupFramelistZoom50)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFrameList message handlers

void CFrameList::MeasureItem(LPMEASUREITEMSTRUCT lpItem) 
{
	lpItem->itemWidth  = (int)(64.0f * m_fZoom);
	lpItem->itemHeight = lpItem->itemWidth;	
}

void CFrameList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	HDC				hDC = lpDrawItemStruct->hDC;
	CRect			rcItem = lpDrawItemStruct->rcItem;
	CDC				dc;
	CPen			pen, *pOldPen;

	CTexture		*pTexture;
	DFileIdent *pRez;

	
	
	CRect rcFrame = rcItem;

	rcFrame.top += 1;
	rcFrame.left += 1;
	
	if(lpDrawItemStruct->itemID == -1)
		return;

	pRez = (DFileIdent*)GetItemDataPtr( lpDrawItemStruct->itemID );
	if(!pRez)
		return;

	pTexture = (CTexture*)dib_GetDibTexture(pRez);
	if(!pTexture)
		return;

	dc.Attach( hDC );

	// Adjust the aspect ratio of the output rectangle to match the texture
	CRect rcDib(0,0, pTexture->m_pDib->GetWidth(), pTexture->m_pDib->GetHeight());
	if (rcDib.Width() && rcDib.Height())
	{
		float fAspect = (float)rcDib.Width() / (float)rcDib.Height();
		rcDib.right = (int)((float)rcFrame.Width() * LTMIN(fAspect, 1.0f));
		rcDib.bottom = (int)((float)rcFrame.Height() * LTMIN(1/fAspect, 1.0f));
		rcDib.left = (rcFrame.Width() - rcDib.right) / 2;
		rcDib.top = (rcFrame.Height() - rcDib.bottom) / 2;
		rcFrame.left += rcDib.left;
		rcFrame.right -= rcDib.left;
		rcFrame.top += rcDib.top;
		rcFrame.bottom -= rcDib.top;
	}

	// Blit the dib into the rectangle
	dc.SetStretchBltMode( COLORONCOLOR );
	pTexture->m_pDib->Blt(dc.GetSafeHdc(), rcFrame.left, rcFrame.top, rcFrame.Width(), rcFrame.Height());

	if( lpDrawItemStruct->itemState & ODS_SELECTED )
	{
		pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	}
	else
	{
		pen.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	}

	if( m_bDrawNumbers )
	{
		CString s;
		s.Format("%d", lpDrawItemStruct->itemID);

		CSize szText = dc.GetTextExtent(s);
	
		dc.TextOut(rcItem.left + (rcItem.Width() / 2) - (szText.cx / 2), rcItem.top, s);
	}

	pOldPen = dc.SelectObject(&pen);

	dc.MoveTo(rcItem.left, rcItem.top);
	dc.LineTo(rcItem.right - 1, rcItem.top);
	dc.LineTo(rcItem.right - 1, rcItem.bottom - 1);
	dc.LineTo(rcItem.left, rcItem.bottom - 1);
	dc.LineTo(rcItem.left, rcItem.top);

	/*
	dc.MoveTo(rcItem.left + 1, rcItem.top + 1);
	dc.LineTo(rcItem.right - 2, rcItem.top + 1);
	dc.LineTo(rcItem.right - 2, rcItem.bottom - 2);
	dc.LineTo(rcItem.left + 1, rcItem.bottom - 2);
	dc.LineTo(rcItem.left + 1, rcItem.top + 1);
	*/

	dc.SelectObject(pOldPen);
	dc.Detach();
}

void CFrameList::OnDblclk()
{
	if( m_pNotifier && (GetCurSel() != LB_ERR) )
		m_pNotifier->NotifyDblClk( this, GetCurSel() );
}

void CFrameList::OnSelchange() 
{
	if( m_pNotifier && (GetCurSel() != LB_ERR) )
		m_pNotifier->NotifySelChange( this, GetCurSel() );
}

void CFrameList::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu menu;

	VERIFY(menu.LoadMenu(GetContextMenu()));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWnd);		
	
}

void CFrameList::OnPopupFramelistZoom100() 
{
	m_fZoom = 1.0f;	
	if (m_pNotifier)
		m_pNotifier->NotifyReCreate(this);
}

void CFrameList::OnPopupFramelistZoom200() 
{
	m_fZoom = 2.0f;	
	if (m_pNotifier)
		m_pNotifier->NotifyReCreate(this);
}

void CFrameList::OnPopupFramelistZoom400() 
{
	m_fZoom = 4.0f;	
	if (m_pNotifier)
		m_pNotifier->NotifyReCreate(this);
}

void CFrameList::OnPopupFramelistZoom50() 
{
	m_fZoom = 0.5f;	
	if (m_pNotifier)
		m_pNotifier->NotifyReCreate(this);
}

BOOL CFrameList::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// Pass non-zoom messages to our parent
	if ((wParam != ID_POPUP_FRAMELIST_ZOOM_50) &&
		(wParam != ID_POPUP_FRAMELIST_ZOOM_100) &&
		(wParam != ID_POPUP_FRAMELIST_ZOOM_200) &&
		(wParam != ID_POPUP_FRAMELIST_ZOOM_400))
		return GetParent()->SendMessage(WM_COMMAND, wParam, lParam);
	
	return CListBox::OnCommand(wParam, lParam);
}

void CFrameList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Echo the key press to the parent
	GetParent()->SendMessage(WM_KEYDOWN, nChar, nRepCnt + (nFlags << 16));
	CListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}
