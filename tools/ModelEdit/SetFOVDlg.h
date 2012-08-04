#if !defined(AFX_SETFOVDLG_H__1BD6E2B0_B29F_4ED6_A6BC_19E83FB1A128__INCLUDED_)
#define AFX_SETFOVDLG_H__1BD6E2B0_B29F_4ED6_A6BC_19E83FB1A128__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetFOVDlg.h : header file
//
class CRenderWnd ;
/////////////////////////////////////////////////////////////////////////////
// CSetFOVDlg dialog

class CSetFOVDlg : public CDialog
{
	CRenderWnd *m_Host ;
	int m_baseFOV;		// default = 45
	int m_startVal, m_endVal ; // default = 0 - 90 

// Construction
public:

	enum	{	MIN_FOV		= 1,
				MAX_FOV		= 90
			};

	// pass the render window to this dialog
	CSetFOVDlg(CRenderWnd *host, CWnd* pParent = NULL);   // standard constructor
	
	void SetBaseFOV( int val )  { m_baseFOV = val ; }
	void SetFOVRange( int bot, int top ) { m_startVal = bot ; m_endVal = top ; }


// Dialog Data
	//{{AFX_DATA(CSetFOVDlg)
	enum { IDD = IDD_SETFOV };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetFOVDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetFOVDlg)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	afx_msg void OnEditChange();

	CEdit*			GetEditFOV();
	CSliderCtrl*	GetSliderFOV();	

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETFOVDLG_H__1BD6E2B0_B29F_4ED6_A6BC_19E83FB1A128__INCLUDED_)
