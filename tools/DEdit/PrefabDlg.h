#ifndef __PREFABDLG_H__
#define __PREFABDLG_H__

#include "resourcemgr.h"
#include "baseimgdlg.h"
#include "oldtypes.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefabDlg dialog

class CPrefabDlg : public CBaseImgDlg
{
// Construction
public:
	CPrefabDlg();   // standard constructor
	virtual ~CPrefabDlg();

// Dialog Data
	//{{AFX_DATA(CPrefabDlg)
	enum { IDD = IDD_PREFAB_TABDLG };
	CTreeCtrl	m_PrefabTree;
	CListCtrl	m_PrefabList;
	//}}AFX_DATA

	// Overrides
	//{{AFX_VIRTUAL(CPrefabDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL OnInitDialogBar();

	//{{AFX_MSG(CPrefabDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkPrefabList(NMHDR* pNMHDR, LRESULT* pResult);	
	afx_msg void OnListSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void	OnPrefabOpen();
	afx_msg void	OnReplaceSelectedPrefabs();
	afx_msg void	OnRenamePrefab();

	void			RepositionControls();

	bool			SelectPrefab(const char* pszPrefabName);
	bool			RecursivelySelectPrefabDir(HTREEITEM hParent, const char* pszNextDir);

	virtual void	PopulateList();

	//this must be overridden by a derived class to render the icon for the appropriate
	//list item
	virtual bool	RenderIcon(HDC BlitTo, uint32 nXOff, uint32 nImgSize, uint32 nItem); 

	//this must be overridden by a derived class to render the large selected image
	virtual void	RenderLargeImage();


	CBitmap *m_pThumbnails;
};

//{{AFX_INSERT_LOCATION}}

#endif
