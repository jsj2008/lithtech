//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __SPRITEEDITDLG_H__
#define __SPRITEEDITDLG_H__


	// Includes....
	#include "spriteeditdlg.h"
	#include "editprojectmgr.h"
	#include "framelist.h"

	#include "resizedlg.h"

	

	// SpriteEditDlg.h : header file
	//

	/////////////////////////////////////////////////////////////////////////////
	// CSpriteEditDlg dialog

	class CSpriteEditDlg : public CDialog, public CFrameListNotifier
	{
	// Construction
	public:
		CSpriteEditDlg(CWnd* pParent = NULL);   // standard constructor
		~CSpriteEditDlg();

		void Term( );

	// Dialog Data
		//{{AFX_DATA(CSpriteEditDlg)
	enum { IDD = IDD_SPRITEEDIT_DLG };
	CSpinButtonCtrl	m_FrameRate;
	BOOL	m_bTransparent;
	BOOL	m_bTranslucent;
	//}}AFX_DATA

				
//		CFrameList			m_TextureList;
		CFrameList			m_SpriteList;
		void				InitSpriteList();

		DFileIdent			*m_pFile;


		void				LoadSpriteFile();
		void				SaveSpriteFile();

//		void				FillTextureList();
		void				FillSpriteList();

		void				UpdateControls();


		CMoArray<char*>		m_Frames;
		CStringHolder		m_StringHolder;

		DWORD				m_Key;

//		DDirIdent			*m_pTextureDir;

		void				NotifyDblClk( CFrameList *pList, int curSel );
		void				NotifyReCreate( CFrameList *pList);
		void				AddTexture( DFileIdent *pIdent );

		BOOL				Create( CWnd *pParent = NULL );

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CSpriteEditDlg)
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// Implementation
	protected:
		CDlgResizer			m_cResizer;

		// Generated message map functions
		//{{AFX_MSG(CSpriteEditDlg)
		virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSaveButton();
	afx_msg void OnClickFramerateSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};


#endif

