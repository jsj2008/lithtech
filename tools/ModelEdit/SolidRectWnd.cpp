// SolidRectWnd.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "SolidRectWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SolidRectWnd

SolidRectWnd::SolidRectWnd()
{
	m_Color = RGB(0,0,0);
	m_hParentWnd = NULL;
}

SolidRectWnd::~SolidRectWnd()
{
}


BEGIN_MESSAGE_MAP(SolidRectWnd, CWnd)
	//{{AFX_MSG_MAP(SolidRectWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void SolidRectWnd::SetColor(COLORREF color)
{
	if(color != m_Color)
	{
		m_Color = color;
		InvalidateRect(NULL, FALSE);
	}
}

void SolidRectWnd::SetForward(BOOL bForward)
{
	m_hParentWnd = NULL;
	if(bForward)
	{
		if(GetParent())
		{
			m_hParentWnd = GetParent()->m_hWnd;
		}
	}
}

void SolidRectWnd::SendMouseMessageToParent(UINT msgID, WPARAM nFlags, CPoint point)
{
	if(m_hParentWnd)
	{
		ClientToScreen(&point);
		::ScreenToClient(m_hParentWnd, &point);

		::SendMessage(m_hParentWnd, msgID, nFlags, MAKELPARAM(point.x, point.y));
	}
}



/////////////////////////////////////////////////////////////////////////////
// SolidRectWnd message handlers

void SolidRectWnd::OnPaint() 
{
	CRect clientRect;
	CBrush brush, *pOldBrush;

	CPaintDC dc(this); // device context for painting
	
	if(brush.CreateSolidBrush(m_Color))
	{
		if(pOldBrush  = dc.SelectObject(&brush))
		{
			GetClientRect(&clientRect);
			dc.Rectangle(&clientRect);
			dc.SelectObject(pOldBrush);
		}
	
		brush.DeleteObject();
	}
}

void SolidRectWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SendMouseMessageToParent(WM_LBUTTONDOWN, nFlags, point);
	CWnd::OnLButtonDown(nFlags, point);
}

void SolidRectWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	SendMouseMessageToParent(WM_LBUTTONUP, nFlags, point);
	CWnd::OnLButtonUp(nFlags, point);
}

void SolidRectWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	SendMouseMessageToParent(WM_RBUTTONDOWN, nFlags, point);
	CWnd::OnRButtonDown(nFlags, point);
}

void SolidRectWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{
	SendMouseMessageToParent(WM_RBUTTONUP, nFlags, point);
	CWnd::OnRButtonUp(nFlags, point);
}

void SolidRectWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	SendMouseMessageToParent(WM_MOUSEMOVE, nFlags, point);
	CWnd::OnMouseMove(nFlags, point);
}
