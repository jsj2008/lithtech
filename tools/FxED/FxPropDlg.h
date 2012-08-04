#if !defined(AFX_FXPROPDLG_H__99D9CF21_7A26_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_FXPROPDLG_H__99D9CF21_7A26_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FxPropDlg.h : header file
//

// Includes....

#include "basefx.h"
#include "linklist.h"
#include "FastList.h"

// Structures....

struct FX_DLGPROP
{
						FX_DLGPROP()
						{
							m_pStatic = NULL;
							m_pWnd    = NULL;
						}
	
	FX_PROP				m_fxProp;
	CStatic			   *m_pStatic;
	CWnd			   *m_pWnd;
	CLinkList<CWnd *>	m_collWnds;
};

/////////////////////////////////////////////////////////////////////////////
// CFxPropDlg dialog

class CFxPropDlg : public CDialog
{
// Construction
public:
	CFxPropDlg(CKey *pKey, CFastList<FX_PROP> *pList, CWnd* pParent = NULL);   // standard constructor

	public :

	private :

		CKey						   *m_pKey;
		CFastList<FX_PROP>			   *m_pCollProps;
		CLinkList<FX_DLGPROP *>			m_collDlgProps;
		int								m_nCurScrollPos;














// Dialog Data
	//{{AFX_DATA(CFxPropDlg)
	enum { IDD = IDD_FXPROP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFxPropDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFxPropDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FXPROPDLG_H__99D9CF21_7A26_11D2_9B4A_0060971BDAD8__INCLUDED_)
