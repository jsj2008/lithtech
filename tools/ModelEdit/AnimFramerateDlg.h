#if !defined(AFX_ANIMFRAMERATEDLG_H__298F2841_9438_11D1_A7D8_006097726515__INCLUDED_)
#define AFX_ANIMFRAMERATEDLG_H__298F2841_9438_11D1_A7D8_006097726515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AnimFramerateDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimFramerateDlg dialog

class CAnimFramerateDlg : public CDialog
{
// Construction
public:
	CAnimFramerateDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAnimFramerateDlg)
	enum { IDD = IDD_ANIMATIONFRAMERATE };
	BOOL	m_bAllAnimations;
	CString	m_FramerateString;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimFramerateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimFramerateDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMFRAMERATEDLG_H__298F2841_9438_11D1_A7D8_006097726515__INCLUDED_)
