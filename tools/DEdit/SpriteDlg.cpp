//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// SpriteDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "spritedlg.h"
#include "editprojectmgr.h"
#include "edithelpers.h"
#include "mainfrm.h"
#include "projectbar.h"
#include "resnewdir.h"
#include "texture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSpriteDlg dialog


CSpriteDlg::CSpriteDlg()
{
	//{{AFX_DATA_INIT(CSpriteDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	InitBaseRezDlg("spr", &m_SpriteTree, &m_SpriteList, RESTYPE_SPRITE);
}

CSpriteDlg::~CSpriteDlg()
{
}

void CSpriteDlg::DoDataExchange(CDataExchange* pDX)
{
	CMRCSizeDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpriteDlg)
	DDX_Control(pDX, IDC_BASEREZ_TREE, m_SpriteTree);
	DDX_Control(pDX, IDC_BASEREZ_LIST, m_SpriteList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpriteDlg, CBaseImgDlg)
	//{{AFX_MSG_MAP(CSpriteDlg)
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_BASEREZ_TREE, OnSelchangedDirectory)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_BASEREZ_LIST, OnListSelChanged)
	ON_NOTIFY( NM_DBLCLK, IDC_BASEREZ_LIST, OnDblClkSprite )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpriteDlg message handlers

BOOL CSpriteDlg::OnInitDialogBar() 
{
	CMRCSizeDialogBar::OnInitDialogBar();

	InitBaseImgDlg((CListCtrl*)GetDlgItem(IDC_BASEREZ_LIST), (CTreeCtrl*)GetDlgItem(IDC_BASEREZ_TREE), IDI_SPRITES_TAB_ICON);

	CRect rect;
	m_SpriteList.GetClientRect( &rect );

	m_SpriteList.InsertColumn(0,"Name",LVCFMT_LEFT,(rect.Width()-35)/3,-1);
	m_SpriteList.InsertColumn(1,"Size",LVCFMT_RIGHT,(rect.Width()-35)/3,-1);
	m_SpriteList.InsertColumn(2,"Modified",LVCFMT_LEFT,(rect.Width()-35)/3,-1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpriteDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	CMenu menu;
	
	if(!GetProjectBar()->IsProjectOpen())
		return;

	if(pWnd->m_hWnd == m_SpriteTree.m_hWnd)
	{
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_WORLDTREE));
	}
	else if(pWnd->m_hWnd == m_SpriteList.m_hWnd)
	{
		// Nothing active unless a directory is selected.
		if(!IsDirectorySelected())
			return;

		VERIFY(menu.LoadMenu(CG_IDR_POPUP_SPRITE_DLG));
	}
	else return;

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	CWnd* pWndPopupOwner = GetProjectBar();

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);				
}

void CSpriteDlg::OnSize(UINT nType, int cx, int cy) 
{
	CMRCSizeDialogBar::OnSize(nType, cx, cy);
	
	// Reposition the controls
	RepositionControls();
}

void CSpriteDlg::OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	DDirIdent *pIdent;

	HTREEITEM hItem=pNMTreeView->itemNew.hItem;

	if(hItem)
	{
		pIdent = (DDirIdent*)m_pTree->GetItemData(hItem);
		if(pIdent)
		{
			m_csCurrentDir = pIdent->m_Filename;
			m_hCurrentItem = hItem;

			PopulateList();
		}
	}

	*pResult = 0;
}

void CSpriteDlg::OnDblClkSprite( NMHDR * pNMHDR, LRESULT * pResult )
{
	POINT point;
	UINT nFlags;
	DFileIdent *pIdent;
	int nItem;

	GetCursorPos(&point);
	m_SpriteList.ScreenToClient(&point);

	nItem = m_SpriteList.HitTest(point,&nFlags);

	if(nItem >= 0)
	{
		pIdent = (DFileIdent*)m_SpriteList.GetItemData(nItem);
		if(pIdent)
		{
			GetProjectBar()->EditSpriteFile(pIdent);
		}
	}

	*pResult = 0;
}

void CSpriteDlg::RepositionControls()
{
	if( ::IsWindow(m_hWnd))
	{
		CBaseImgDlg::RepositionControls();

		// Adjust columns
		if (m_SpriteList)
		{
			CRect rect;
			m_SpriteList.GetClientRect( &rect );

			m_SpriteList.SetColumnWidth(0,(rect.Width())/3);
			m_SpriteList.SetColumnWidth(1,(rect.Width())/3);
			m_SpriteList.SetColumnWidth(2,(rect.Width())/3);
		}
	}		
}

void CSpriteDlg::PopulateList()
{
	CBaseRezDlg::PopulateList();

	if(GetProjectBar()->m_bShowThumbnails)
		UpdateThumbnails();
}

//given an absolute sprite filename, it will open it up and find the name of the 
//first texture used in the sprite
static CString GetFirstSpriteTexture(const char* pszFile)
{
	CMoFileIO		File;
	DWORD			nFrames, nJunk;
	char			str[MAX_PATH];

	CString			sRV;


	if(dfm_OpenFileRelative(GetFileMgr(), pszFile, File))
	{
		if( File.GetLen() > 0 )
		{
			File >> nFrames;
			File >> nJunk >> nJunk >> nJunk >> nJunk;	 //frame rate, transparent, translucent, key
	
			if(nFrames > 0)
			{
				if( File.ReadString(str, MAX_PATH) )
				{
					sRV = str;
				}
			}
		}
		
		File.Close();
	}

	return sRV;
}

//this must be overridden by a derived class to render the icon for the appropriate
//list item
bool CSpriteDlg::RenderIcon(HDC BlitTo, uint32 nXOff, uint32 nImgSize, uint32 nItem)
{
	//get the name of the texture
	CString sFile = GetFirstSpriteTexture(((DFileIdent*)m_SpriteList.GetItemData(nItem))->m_Filename);

	if(sFile.IsEmpty())
		return false;

	DFileIdent* pFile;
	::dfm_GetFileIdentifier(GetFileMgr(), sFile, &pFile);

	CTexture* pTexture = dib_GetDibTexture(pFile);

	if(!pTexture)
		return false;

	pTexture->m_pDib->Blt(BlitTo, nXOff, 0, nImgSize, nImgSize);

	return true;
}

//this must be overridden by a derived class to render the large selected image
void CSpriteDlg::RenderLargeImage()
{
	//setup the rectangle we are going to blit to
	CRect InitialRect = CBaseImgDlg::InitLargeImageRect();
	CRect DrawRect = InitialRect;

	//get a DC
	CDC *pDC = GetDC();
	if(!pDC)
		return;

	//get the first selection
	POSITION Pos = m_SpriteList.GetFirstSelectedItemPosition();
	if(!Pos)
		return;

	int nItem = m_SpriteList.GetNextSelectedItem(Pos);

	//get the name of the texture
	CString sFile = GetFirstSpriteTexture(((DFileIdent*)m_SpriteList.GetItemData(nItem))->m_Filename);

	if(sFile.IsEmpty())
		return;

	DFileIdent* pRez;
	::dfm_GetFileIdentifier(GetFileMgr(), sFile, &pRez);

	if(pRez == NULL)
	{
		ReleaseDC(pDC);
		return;
	}

	CTexture *pTexture = dib_GetDibTexture(pRez);
	if(!pTexture)
	{
		ReleaseDC(pDC);
		return;
	}

	CDib *pDib = pTexture->m_pDib;
	CRect texRect(0,0,pDib->GetWidth(),pDib->GetHeight());
	texRect.right >>= pTexture->m_UIMipmapOffset;
	texRect.bottom >>= pTexture->m_UIMipmapOffset;


	//adjust the origin of the texture
	int nTmp = (DrawRect.top + (DrawRect.bottom/6)) - (texRect.bottom/2);
	if(nTmp > DrawRect.top)
		DrawRect.top = nTmp;

	DrawRect.left = (DrawRect.right/2) - (texRect.right/2);
	
	nTmp = DrawRect.top + texRect.bottom;
	if(nTmp < DrawRect.bottom)
		DrawRect.bottom = nTmp;

	DrawRect.right = DrawRect.left + texRect.right;

	// Blit the texture in.
	SetStretchBltMode( pDC->m_hDC, COLORONCOLOR );
	pDib->Blt( pDC->m_hDC, DrawRect.left, DrawRect.top, DrawRect.Width()+1, DrawRect.Height()+1 );
	
	// Draw the texture name.
	char pszToPrint[MAX_PATH];
	CHelpers::ExtractNames( ((DFileIdent*)m_SpriteList.GetItemData(nItem))->m_Filename, NULL, NULL, pszToPrint, NULL );

	CBaseImgDlg::PrintLargeImageText(pDC->m_hDC, InitialRect, pszToPrint);

	ReleaseDC(pDC);
}

void CSpriteDlg::OnListSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	RenderLargeImage();
}

