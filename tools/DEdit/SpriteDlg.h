//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __SPRITEDLG_H__
#define __SPRITEDLG_H__


#include "resourcemgr.h"
#include "baseimgdlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSpriteDlg dialog

class CSpriteDlg : public CBaseImgDlg
{
// Construction
public:
	CSpriteDlg();   // standard constructor
	virtual ~CSpriteDlg();	// destructor

// Dialog Data
	//{{AFX_DATA(CSpriteDlg)
	enum { IDD = IDD_SPRITE_TABDLG };
	CTreeCtrl	m_SpriteTree;
	CListCtrl	m_SpriteList;
	//}}AFX_DATA

// Overrides
	//{{AFX_VIRTUAL(CSpriteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL OnInitDialogBar();

	void			RepositionControls();


	virtual void	PopulateList();

	//this must be overridden by a derived class to render the icon for the appropriate
	//list item
	virtual bool	RenderIcon(HDC BlitTo, uint32 nXOff, uint32 nImgSize, uint32 nItem); 

	//this must be overridden by a derived class to render the large selected image
	virtual void	RenderLargeImage();


	//{{AFX_MSG(CSpriteDlg)	
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkSprite( NMHDR * pNMHDR, LRESULT * pResult );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}


#endif
