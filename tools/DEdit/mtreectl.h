//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// MyTreeCtrl.h : header file
//

// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


#ifndef INC_MYTREECTRL_H
#define INC_MYTREECTRL_H

#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// CMyTreeCtrl window

class CMyTreeCtrl : public CTreeCtrl
{
// Constants

	enum { DRAG_DELAY = 80, TIMER_ID_SCROLL = 101, TIMER_ID_HOVER = 102 };

// Construction
public:
	CMyTreeCtrl();

// Attributes
public:
	BOOL		m_bDragging;
	HTREEITEM	m_hitemDrag;
	HTREEITEM	m_hitemDrop;
	CImageList	*m_pimagelist;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyTreeCtrl)
	//}}AFX_VIRTUAL


	BOOL PreTranslateMessage( MSG *pMsg );

// Implementation
public:
	virtual				~CMyTreeCtrl();
	virtual void		SetNewStyle(long lStyleMask, BOOL bSetBits);
	virtual	HTREEITEM	TransferItem(HTREEITEM hitem, HTREEITEM hNewParent);
	virtual void		OnButtonUp(void);
	BOOL				IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent);

	// Recursively sorts a node and its children
	void				SortNodeAndChildren(HTREEITEM hItem);
	void				SortNodeAndChildren(HTREEITEM hItem, PFNTVCOMPARE lpfnCompare);

	// Set/Get an items font
	void				SetItemFont(HTREEITEM hItem, LOGFONT& logfont);
	BOOL				GetItemFont(HTREEITEM hItem, LOGFONT * plogfont);

	// Set/Get an items bold state
	void				SetItemBold(HTREEITEM hItem, BOOL bBold);
	BOOL				GetItemBold(HTREEITEM hItem);

	// Set/Get an items color
	void				SetItemColor(HTREEITEM hItem, COLORREF color);
	COLORREF			GetItemColor(HTREEITEM hItem);

protected:
	void		HandleScrollTimer();
	void		HandleHoverTimer();

protected:
	DWORD	m_dwDragStart;
	UINT	m_nTimerIDScroll;
	UINT	m_nTimerIDHover;
	UINT	m_timerticks;

	CPoint	m_hoverPoint;
	
	
	struct Color_Font
	{
		COLORREF color;
		LOGFONT  logfont;
	};
	CMap<void*, void*, Color_Font, Color_Font&> m_mapColorFont;

	// Generated message map functions
protected:
	//{{AFX_MSG(CMyTreeCtrl)
	afx_msg void OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult);
	afx_msg void OnBeginDrag(LPNMHDR pnmhdr, LRESULT *pLResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
