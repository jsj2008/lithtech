#if !defined(AFX_SPHEREPRIMITIVEDLG_H__62828CD2_EDF7_11D2_BE12_0060971BDC6D__INCLUDED_)
#define AFX_SPHEREPRIMITIVEDLG_H__62828CD2_EDF7_11D2_BE12_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpherePrimitiveDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpherePrimitiveDlg dialog

class CSpherePrimitiveDlg : public CDialog
{
// Construction
public:
	CSpherePrimitiveDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSpherePrimitiveDlg)
	enum { IDD = IDD_PRIMSPHERE };
	float	m_fRadius;
	int		m_nVerticalSubdivisions;
	int		m_nSides;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpherePrimitiveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpherePrimitiveDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPHEREPRIMITIVEDLG_H__62828CD2_EDF7_11D2_BE12_0060971BDC6D__INCLUDED_)
