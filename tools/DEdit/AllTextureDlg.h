//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_ALLTEXTUREDLG_H__B7A8FE22_766A_11D1_A428_006097098780__INCLUDED_)
#define AFX_ALLTEXTUREDLG_H__B7A8FE22_766A_11D1_A428_006097098780__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AllTextureDlg.h : header file
//

#include "framelist.h"

#include "resizedlg.h"

#include "filepalette.h"

class CEditRegion;

/////////////////////////////////////////////////////////////////////////////
// CAllTextureDlg dialog

class CAllTextureDlg : public CDialog, public CFrameListNotifier
{
// Construction
public:
	CAllTextureDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAllTextureDlg)
	enum { IDD = IDD_ALLTEXTUREDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Data types
public:
	// Current viewing mode
	enum EViewMode {
		MODE_PALETTE,	// Palette editing mode
		MODE_VIEWALL	// All texture viewing mode
	};

// Members
protected:
	// Status Bar
	CStatusBar m_wndStatusBar;

	// Palette members
	CFilePalette m_cPalette;
	CString m_csPaletteFileName;

	// Palette change status
	BOOL m_bPaletteChanged;

	EViewMode m_eCurMode;
	void SetViewMode(EViewMode eMode) { m_eCurMode = eMode; };

// Internal functions
protected:

	// Confirm an action & prompt the user to save
	int ConfirmSave(char *pMessage = NULL);

public:
	EViewMode GetViewMode() const { return m_eCurMode; };

	CFrameList	m_TextureList;

	int32		m_nTotalTextureSize; // Size of all textures together

	// Palette access
	void	AddTextureToPalette(LPCTSTR pTexture);
	LPCTSTR GetPaletteName() const { return m_cPalette.GetName(); };

	// Palette change status
	void SetPaletteChanged(BOOL bChanged);
	BOOL GetPaletteChanged() const { return m_bPaletteChanged; };

	// Directory change notification
	void NotifyDirChange();

	// Reload the texture list  (Note : This really should be protected or private...)
	void	FillTextureList();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAllTextureDlg)
	public:
	virtual BOOL DestroyWindow(bool bAcceptCancel=true);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	void	NotifySelChange( CFrameList *pList, int curSel );
	void	NotifyDblClk( CFrameList *pList, int curSel );
	void	NotifyReCreate( CFrameList *pList );

// Implementation
protected:

	CDlgResizer m_cResizer;

	//adds all the textures in the specified world, will recurse into prefabs if specified
	void	AddWorldTextures(const CWorldNode* pRegion, bool bAddPrefabs, bool& bChanged);

	void	DeleteSelectedTexture();

	// Generated message map functions
	//{{AFX_MSG(CAllTextureDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPopupTexturePaletteLoad();
	afx_msg void OnPopupTexturePaletteSave();
	afx_msg void OnPopupTexturePaletteViewAll();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPopupTexturePaletteNew();
	afx_msg void OnPopupTexturePaletteDelete();
	afx_msg void OnPopupTexturePaletteRemove();
	afx_msg void OnPopupTexturePaletteImport();
	afx_msg void OnPopupTexturePaletteSort();
	afx_msg void OnPopupTexturePaletteRenameTextures();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALLTEXTUREDLG_H__B7A8FE22_766A_11D1_A428_006097098780__INCLUDED_)
