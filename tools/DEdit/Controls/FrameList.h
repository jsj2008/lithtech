//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __FRAMELIST_H__
#define __FRAMELIST_H__


	#include "editprojectmgr.h"


	class CFrameList;


	class CFrameListNotifier
	{
		public:

			virtual void	NotifySelChange( CFrameList *pList, int curSel ) {}
			virtual void	NotifyDblClk( CFrameList *pList, int curSel ) {}
			virtual void	NotifyReCreate( CFrameList *pList) {}

	};


	// FrameList.h : header file
	//

	/////////////////////////////////////////////////////////////////////////////
	// CFrameList window

	class CFrameList : public CListBox
	{
	// Construction
	public:
		CFrameList();

	// Attributes
	protected:
		UINT m_uContextMenu;
	public:
		virtual UINT GetContextMenu() const { return m_uContextMenu; };
		virtual void SetContextMenu(UINT uContextMenu) { m_uContextMenu = uContextMenu; };
		

	// Operations
	public:

		CFrameListNotifier	*m_pNotifier;
		BOOL				m_bDrawNumbers;
		float				m_fZoom;

	
	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CFrameList)
	public:
		virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
		virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CFrameList();

		// Generated message map functions
	protected:
		//{{AFX_MSG(CFrameList)
		afx_msg void OnDblclk();
	afx_msg void OnSelchange();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPopupFramelistZoom100();
	afx_msg void OnPopupFramelistZoom200();
	afx_msg void OnPopupFramelistZoom400();
	afx_msg void OnPopupFramelistZoom50();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
	};

	/////////////////////////////////////////////////////////////////////////////

#endif
