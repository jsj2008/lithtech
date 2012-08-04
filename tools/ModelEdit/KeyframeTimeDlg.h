#if !defined(AFX_KEYFRAMETIMEDLG_H__1B1481C1_2F46_11D1_9462_0020AFF7CDC1__INCLUDED_)
#define AFX_KEYFRAMETIMEDLG_H__1B1481C1_2F46_11D1_9462_0020AFF7CDC1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// KeyframeTimeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeyframeTimeDlg dialog

class CKeyframeTimeDlg : public CDialog
{
// Construction
public:
	CKeyframeTimeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeyframeTimeDlg)
	enum { IDD = IDD_EDIT_KEYFRAME_TIME };
	DWORD	m_nNewTime;
	DWORD	m_nCurrentTime;
	CString	m_sCaption;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyframeTimeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKeyframeTimeDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYFRAMETIMEDLG_H__1B1481C1_2F46_11D1_9462_0020AFF7CDC1__INCLUDED_)
