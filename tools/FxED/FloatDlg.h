#if !defined(AFX_FLOATDLG_H__B1D95DA6_9058_11D2_9B4B_0060971BDAD8__INCLUDED_)
#define AFX_FLOATDLG_H__B1D95DA6_9058_11D2_9B4B_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FloatDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFloatDlg dialog

class CFloatDlg : public CDialog
{
// Construction
public:
	CFloatDlg(float fInitial, CString sName, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFloatDlg)
	enum { IDD = IDD_FLOATDLG };
	float	m_float;
	//}}AFX_DATA

	CString						m_sName;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFloatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFloatDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLOATDLG_H__B1D95DA6_9058_11D2_9B4B_0060971BDAD8__INCLUDED_)
