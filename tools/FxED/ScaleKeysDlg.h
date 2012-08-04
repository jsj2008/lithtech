#if !defined(AFX_SCALEKEYSDLG_H__0C360701_8F62_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_SCALEKEYSDLG_H__0C360701_8F62_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScaleKeysDlg.h : header file
//

// Includes....

#include "ScaleKeyCtrl.h"
#include "Key.h"
#include "FloatButton.h"

/////////////////////////////////////////////////////////////////////////////
// CScaleKeysDlg dialog

class CScaleKeysDlg : public CDialog
{
// Construction
public:
	CScaleKeysDlg(CKey *pKey, CWnd* pParent = NULL);   // standard constructor

	// Member Functions
	
		void							Refresh();

	// Member Variables

		CKey							*m_pKey;
		float							m_fLastScale;

// Dialog Data
	//{{AFX_DATA(CScaleKeysDlg)
	enum { IDD = IDD_SCALEKEYS };
	CFloatButton	m_minScale;
	CFloatButton	m_maxScale;
	CScaleKeyCtrl	m_scaleKeyCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScaleKeysDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScaleKeysDlg)
	afx_msg void OnAddToFavourites();
	afx_msg void OnChooseFavourite();
	afx_msg void OnReset();
	afx_msg void OnSetAllScale();
	afx_msg void OnValueUpdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCALEKEYSDLG_H__0C360701_8F62_11D2_9B4A_0060971BDAD8__INCLUDED_)
