#if !defined(AFX_KEYFRAMEWND_H__59C0D230_2C80_11D1_9462_0020AFF7CDC1__INCLUDED_)
#define AFX_KEYFRAMEWND_H__59C0D230_2C80_11D1_9462_0020AFF7CDC1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// KeyframeWnd.h : header file
//


class CModelEditDlg;

/////////////////////////////////////////////////////////////////////////////
// CKeyframeWnd window

class CKeyframeWnd : public CWnd
{
// Construction
public:
	CKeyframeWnd();

// Attributes
public:

	CMoArray<BOOL>	m_Tagged;	// Track what keyframes are tagged.
	DWORD			m_iWnd;		// Which keyframe window are we?

protected:

	CDC			m_dcOffscreen;
	CBitmap		m_bmpOffscreen;
	CBitmap*	m_pOldBmpOffscreen;

	CDC			m_dcTopMarker;
	CBitmap		m_bmpTopMarker;
	CBitmap*	m_pOldBmpTopMarker;

	CDC			m_dcBottomMarker;
	CBitmap		m_bmpBottomMarker;
	CBitmap*	m_pOldBmpBottomMarker;

	CRect		m_rcTopMarker;
	CRect		m_rcBottomMarker;
	CRect		m_rcKeyframes;
	
	CBrush		m_brBackground;

	ModelAnim*	m_pModelAnim;
	DWORD		m_nCurrentTime;
	CMoArray<int>	m_XVal;

	BOOL		m_bTracking;
	BOOL		m_bSelectionBox;
	CPoint		m_ptOld;

	CPoint		m_ptStartBox;
	CPoint		m_ptEndBox;

	COLORREF	m_crKeyframe;
	COLORREF	m_crKeyframeString;
	COLORREF	m_crTaggedKeyframe;
	COLORREF	m_crTaggedKeyframeString;

	// Tracks whether the window has been selected.
	// Basically chooses whether the animation controls affect this window.
	BOOL		m_bActive;

	//this function will verify that this operation can be performed given the current animation
	//settings, and will inform the user and return false if it cannot
	bool		CanPerformOperation(const char* pszDescription);


// Operations
public:

	inline CModelEditDlg* GetModelEditDlg()	{return (CModelEditDlg*)GetParent();}

	// Change active status.
	BOOL			IsActive()				{return m_bActive;}
	void			SetActive(BOOL bActive)	{m_bActive = bActive;}

	// Initialize tagged array to the size of our animation.
	BOOL			InitTaggedArray();

	void			SetAnim (ModelAnim* pAnim);
	ModelAnim*		GetAnim() {return m_pModelAnim;}

	void			SetTime (DWORD nCurrentTime);
	DWORD			ForceNearestKeyframe();
	void			RemoveKeyframe (DWORD nKeyframe);
	void			MoveKeyframe (DWORD nDst, DWORD nSrc);
	void			DoEditKeyframeTime( DWORD nKeyframe, DWORD dwNewKeyframeTime );
	void			DoEditKeyframeString( DWORD nKeyframe, CString &sFrameString );
	DWORD			GetNearestKeyframe( int x );
	BOOL			DoTagKeyframes( DWORD nStartKeyframe, DWORD nEndKeyframe, BOOL bForce = FALSE, BOOL bTag = TRUE );
	void			Redraw( );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyframeWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CKeyframeWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CKeyframeWnd)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnClipAfter();
	afx_msg void OnClipBefore();
	afx_msg void OnTagKeyframe();
	afx_msg void OnTagAll();
	afx_msg void OnUnTagAll();
	afx_msg void OnInsertTimeDelay();
	afx_msg void OnUntagKeyframe();
	afx_msg void OnEditKeyframeTime();
	afx_msg void OnDeleteKeyframe();
	afx_msg void OnDeleteTaggedKeyframes();
	afx_msg void OnEditTaggedKeyframeString();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYFRAMEWND_H__59C0D230_2C80_11D1_9462_0020AFF7CDC1__INCLUDED_)
