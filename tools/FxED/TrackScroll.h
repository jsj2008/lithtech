#if !defined(AFX_TRACKSCROLL_H__8F591984_95EB_11D2_9B4C_0060971BDAD8__INCLUDED_)
#define AFX_TRACKSCROLL_H__8F591984_95EB_11D2_9B4C_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TrackScroll.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTrackScroll window

class CTrackScroll : public CScrollBar
{
// Construction
public:
	CTrackScroll();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrackScroll)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTrackScroll();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTrackScroll)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRACKSCROLL_H__8F591984_95EB_11D2_9B4C_0060971BDAD8__INCLUDED_)
