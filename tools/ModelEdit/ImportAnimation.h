#if !defined(AFX_IMPORTANIMATION_H__FDFC39C2_8E6D_11D1_99E4_0060970987C3__INCLUDED_)
#define AFX_IMPORTANIMATION_H__FDFC39C2_8E6D_11D1_99E4_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ImportAnimation.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImportAnimation dialog

class CImportAnimation : public CFileDialog
{
	DECLARE_DYNAMIC(CImportAnimation)

public:
	CImportAnimation(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);


	BOOL	m_bUseUVCoords;
	BOOL	m_bImportAnimations;
	BOOL	m_bImportUserDims;
	BOOL	m_bImportTranslations;
	BOOL	m_bImportSockets;
	BOOL	m_bImportWeightSets;
	
	virtual void OnInitDone( );	

protected:
	//{{AFX_MSG(CImportAnimation)
	afx_msg void OnCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTANIMATION_H__FDFC39C2_8E6D_11D1_99E4_0060970987C3__INCLUDED_)
