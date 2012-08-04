#if !defined(AFX_TRACKWND_H__F2D178A5_73FD_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_TRACKWND_H__F2D178A5_73FD_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TrackWnd.h : header file
//

// Includes....

#include "Phase.h"
#include "Key.h"
#include "PhaseInfoDlg.h"
#include "TimeBarDlg.h"
#include "TrackScroll.h"

// Defines....

#define HT_OUTSIDE						0
#define HT_INSIDE						1
#define HT_LEFT							2
#define HT_RIGHT						3

/////////////////////////////////////////////////////////////////////////////
// CTrackWnd window

class CTrackWnd : public CWnd
{
// Construction
public:
	CTrackWnd();

	public :

		// Member Functions

		void							DrawKey(CDC *pDC, CKey *pKey, CRect rcKey);
		void							DrawTracks(CDC *pDC);

		CTrack*							GetTrackByPos(int pos);
		CKey*							GetKeyByPos(CTrack *pTrack, int pos);
		CRect							GetKeyRect(CTrack *pTrack, CKey *pKey);
		CKey*							GetKeyByPt(CPoint ptTest);
		int								HitTestKey(CTrack *pTrack, CKey *pKey, CPoint ptTest);

		int								NumSelected();

		void							TrackMoveKey();
		void							TrackMoveMotionLinkedKeys();
		void							TrackMoveLeftSideKey();
		void							TrackMoveRightSideKey();
		void							TrackCopyKey(BOOL bRemoveOriginal);

		BOOL							IsLinkedTo(CKey *pKey);
		void							UnlinkKey(CKey *pKey);

		void							SelectKey(CKey *pKey);
		void							DeselectAllKeys();

		// Accessors

		CSpell*							GetSpell() { return m_pSpell; }
		CPhase*							GetPhase() { return m_pPhase; }
		CPhaseInfoDlg*					GetPhaseInfoDlg() { return &m_phaseInfoDlg; }
		CTimeBarDlg*					GetTimeBarDlg() { return &m_timeBarDlg; }
		CRect*							GetTrackRect() { return &m_rcTrack; }
		CKey*							GetSelKey() { return m_pSelKey; }

		void							SetPhase(CPhase *pPhase) { m_pPhase = pPhase; }
		void							SetSpell(CSpell *pSpell) { m_pSpell = pSpell; }

	private :

		// Member Variables

		CSpell						   *m_pSpell;
		CPhase						   *m_pPhase;
		CDC							   *m_pMemDC;
		CBitmap						   *m_pBitmap;
		CBitmap						   *m_pOldBitmap;
		CPhaseInfoDlg					m_phaseInfoDlg;
		CTimeBarDlg						m_timeBarDlg;
		CScrollBar						m_scrollBar;
		CRect							m_rcTrack;
		CRect							m_rcTrackClient;
		int								m_nTrackHeight;
		int								m_nScrollWidth;
		int								m_yOffset;

		CTrack						   *m_pSelTrack;
		CKey						   *m_pSelKey;
		CPoint							m_ptRbClick;

		CKey						   *m_pCopyKey;
		CRect							m_rcCopyKey;
		CPoint							m_ptAnchor;
		CPoint							m_ptLast;
		BOOL							m_bTracking;

		BOOL							m_bTrackingLinkKey;
		CPoint							m_ptAnchorLink;
		CPoint							m_ptAnchorCur;













// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrackWnd)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTrackWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTrackWnd)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTrackAddTrack();
	afx_msg void OnTrackInsertTrack();
	afx_msg void OnTrackAddKey();
	afx_msg void OnTrackNameKey();
	afx_msg void OnTrackDeleteKey();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTrackExpandKey();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTrackEditKey();
	afx_msg void OnTrackChangeKey();
	afx_msg void OnTrackEditColourKeys();
	afx_msg void OnTrackEditMotionKeys();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTrackEditScaleKeys();
	afx_msg void OnTrackAddFavouriteKey();
	afx_msg void OnTrackAddKeyToFavourites();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTrackDeleteTrack();
	afx_msg void OnTrackCopyMultiKeys();
	afx_msg void OnTrackMakeSameLength();
	afx_msg void OnTrackLinkKey();
	afx_msg void OnTrackUnlinkkey();
	afx_msg void OnTrackSetKeyLength();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRACKWND_H__F2D178A5_73FD_11D2_9B4A_0060971BDAD8__INCLUDED_)
