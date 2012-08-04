#if !defined(AFX_PROJECTTABCTRL_H__C6BB81D4_1D25_11D3_BE2F_0060971BDC6D__INCLUDED_)
#define AFX_PROJECTTABCTRL_H__C6BB81D4_1D25_11D3_BE2F_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProjectTabCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProjectTabCtrl window

class CProjectBar;
class CProjectTabCtrl : public CTabCtrl
{
// Construction
public:
	CProjectTabCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectTabCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProjectTabCtrl();

	// Set the project bar pointer
	void	SetProjectBar(CProjectBar *pProjectBar)		{ m_pProjectBar=pProjectBar; }

	// This sets the image for the currently selected tab

	void	SetCurrentTabImage();				// no image specified
	void	SetCurrentTabImage(int nIndex);		// image specified

	// Sets all tab images to the appropriate icon
	void	SetTabImages();

	// Generated message map functions
protected:
	//{{AFX_MSG(CProjectTabCtrl)
	afx_msg void OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
	CProjectBar		*m_pProjectBar;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTTABCTRL_H__C6BB81D4_1D25_11D3_BE2F_0060971BDC6D__INCLUDED_)
