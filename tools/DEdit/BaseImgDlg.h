#ifndef __BASEIMGDLG_H__
#define __BASEIMGDLG_H__

#include "baserezdlg.h"

/////////////////////////////////////////////////////////////////////////////
// CBaseImgDlg dialog

class CBaseImgDlg : public CBaseRezDlg
{
// Construction
public:
	CBaseImgDlg();			// standard constructor
	virtual ~CBaseImgDlg();	// destructor

	void			DoShowThumbnails(bool bShow);
	void			UpdateThumbnails();


	//called to initialize the basic controls of the texture dialog, such as the 
	//icons for the tree controls
	void			InitBaseImgDlg(CListCtrl* pItemList, CTreeCtrl* pTree, DWORD nDefaultIcon);

	//this must be overridden by a derived class to render the icon for the appropriate
	//list item. If it returns false, it will draw the default icon over the image
	virtual bool	RenderIcon(HDC BlitTo, uint32 nXOff, uint32 nImgSize, uint32 nItem) = 0; 

	//this must be overridden by a derived class to render the large selected image
	virtual void	RenderLargeImage() = 0;


// Implementation
protected:

	//can be used by a derived class in order to find the rectangle that needs to be blitted
	//to for the rendering of the large image, it will also clear the rect
	CRect			InitLargeImageRect();

	//called in order to render the specified text to the upper left corner of the large
	//image
	void			PrintLargeImageText(HDC hDC, CRect Area, const char* pszText);

	//keep track of if we are supposed to show thumbnails or not
	bool			m_bShowThumbnails;

	//ID of the default icon
	DWORD			m_nDefaultIcon;

	//the tree control that holds the directory structure
	CTreeCtrl		*m_pTreeCtrl;

	//the list control that holds each file
	CListCtrl		*m_pListCtrl;

	//the icon lists
	CImageList		*m_pTreeIcons;
	CImageList		*m_pListIcons;

	//the font used to draw the item's name
	HFONT			m_hFont;

	//the bitmap that holds the thumbnails for the list icons
	CBitmap			*m_pThumbnails;

	// This is called to reposition the controls
	virtual void	RepositionControls();

	// Generated message map functions
	//{{AFX_MSG(CTextureDlg)	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif
