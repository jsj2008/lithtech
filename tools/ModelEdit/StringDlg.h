#ifndef __STRINGDLG_H__
#define __STRINGDLG_H__

	// StringDlg.h : header file
	//

	/////////////////////////////////////////////////////////////////////////////
	// CStringDlg dialog

	#include "resource.h"

	class CStringDlg : public CDialog
	{
	// Construction
	public:
		CStringDlg(UINT id = IDD_STRINGDLG, CWnd* pParent = NULL);   // standard constructor

	// Dialog Data
		//{{AFX_DATA(CStringDlg)
		enum { IDD = IDD_STRINGDLG };
		CString	m_EnteredText;
		CString	m_MsgText;
		//}}AFX_DATA

		UINT		m_idCaption, m_idText;

		BOOL		m_bAllowLetters;
		BOOL		m_bAllowNumbers;
		BOOL		m_bAllowOthers;
		BOOL		m_bBeeping;

		int			m_MaxStringLen;

		int			DoModal( UINT idCaption, UINT idText );


	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CStringDlg)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:

		// Generated message map functions
		//{{AFX_MSG(CStringDlg)
		virtual BOOL OnInitDialog();
		afx_msg void OnChangeEnteredtext();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};

#endif



