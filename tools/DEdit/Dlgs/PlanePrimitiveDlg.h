#ifndef __PLANEPRIMITIVEDLG_H__
#define __PLANEPRIMITIVEDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlanePrimitiveDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlanePrimitiveDlg dialog

class CPlanePrimitiveDlg : public CDialog
{
// Construction
public:
	CPlanePrimitiveDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPlanePrimitiveDlg)
	enum { IDD = IDD_PRIMPLANE };
	float	m_fWidth;
	float	m_fHeight;
	int		m_nOrientation;
	int		m_nType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanePrimitiveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPlanePrimitiveDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif 
