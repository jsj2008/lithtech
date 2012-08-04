#if !defined(AFX_RESOURCELOCATOR_H__8E4BF841_C6B5_11D3_9B58_0060971BDAD8__INCLUDED_)
#define AFX_RESOURCELOCATOR_H__8E4BF841_C6B5_11D3_9B58_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourceLocator.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResourceLocator dialog

class CResourceLocator : public CDialog
{
// Construction
public:
	CResourceLocator(CString sName, CLinkList<SPELLNAME> *pList, CWnd* pParent = NULL);   // standard constructor

	CLinkList<SPELLNAME> *m_pList;
	CString				  m_sName;

// Dialog Data
	//{{AFX_DATA(CResourceLocator)
	enum { IDD = IDD_SHOWRESOURCES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourceLocator)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CResourceLocator)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCELOCATOR_H__8E4BF841_C6B5_11D3_9B58_0060971BDAD8__INCLUDED_)
