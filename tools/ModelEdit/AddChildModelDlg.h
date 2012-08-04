#if !defined(AFX_ADDCHILDMODELDLG_H__FAF12B10_D371_4304_AF4D_57AC5058345B__INCLUDED_)
#define AFX_ADDCHILDMODELDLG_H__FAF12B10_D371_4304_AF4D_57AC5058345B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddChildModelDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddChildModelDlg dialog

class CAddChildModelDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CAddChildModelDlg)

public:
	CAddChildModelDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

	virtual void OnInitDone( );	

	BOOL	m_bScaleSkeleton;

protected:
	//{{AFX_MSG(CAddChildModelDlg)
		afx_msg void OnCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDCHILDMODELDLG_H__FAF12B10_D371_4304_AF4D_57AC5058345B__INCLUDED_)
