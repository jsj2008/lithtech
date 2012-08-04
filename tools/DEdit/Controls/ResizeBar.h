//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ResizableDlgBar.h : header file
//
#ifndef __RESIZEBAR__
#define __RESIZEBAR__


	class CMainFrame;

 
	class CResizeBar : public CDialogBar
	{
	// Construction
	public:

		CResizeBar();

		BOOL Create( CWnd* pParentWnd, UINT nIDTemplate, UINT nStyle,
			UINT nID, BOOL = TRUE);
		BOOL Create( CWnd* pParentWnd, LPCTSTR lpszTemplateName,
			UINT nStyle, UINT nID, BOOL = TRUE);


#if 0 // REMOVED1
		void SizeFixup();
#endif // REMOVED1

	// Attributes
	public:
		 CSize m_sizeDocked;
		 CSize m_sizeFloating;

#if 0 // REMOVED1
		 CMainFrame		*m_pFrame;
#endif // REMOVED1
     
		 // 0 = Stretch vertically, 1 = Stretch horizontally.
		 int	m_DockStretchMode;
		 		 
		 // Should it even stretch to fit its frame?
		 BOOL	m_bFitToFrame;
		 
		 BOOL	m_bChangeDockedSize;   // Indicates whether to keep
									 // a default size for docking


#if 0 // REMOVED1
		int				GetFidgetWidth( CMainFrame *pFrame );
		int				GetFidgetHeight( CMainFrame *pFrame );
		int				GetToolBarDocking( CMainFrame *pFrame );
#endif // REMOVED1

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CResizeBar)
		//}}AFX_VIRTUAL

		virtual CSize CalcDynamicLayout( int nLength, DWORD dwMode );

#if 0 // REMOVED1
	private:
		BOOL m_fInit;
		int m_cyTabFudge;
#endif // REMOVED1
 
	// Generated message map functions
	protected:
		//{{AFX_MSG(CResizeBar)
		afx_msg void OnSize(UINT nType, int cx, int cy);
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};


#endif//__RESIZEBAR__
