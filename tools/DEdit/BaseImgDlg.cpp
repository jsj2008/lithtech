// TextureDlg.cpp : implementation file
//

#include "bdefs.h"
#include "baseimgdlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IMAGE_SIZE		32

/////////////////////////////////////////////////////////////////////////////
// CBaseImgDlg dialog


CBaseImgDlg::CBaseImgDlg()
{
	//{{AFX_DATA_INIT(CTextureDlg)
	//}}AFX_DATA_INIT

	m_pListCtrl			= NULL;
	m_pTreeCtrl			= NULL;
	m_pTreeIcons		= NULL;
	m_pListIcons		= NULL;
	m_pThumbnails		= NULL;
	m_hFont				= NULL;

	m_nDefaultIcon		= 0;
	m_bShowThumbnails	= true;
}

CBaseImgDlg::~CBaseImgDlg()
{
	if (m_pTreeIcons)
	{
		delete m_pTreeIcons;
	}
	if (m_pListIcons)
	{
		delete m_pListIcons;
	}
	if (m_pThumbnails)  
	{
		delete m_pThumbnails;
	}

	//clean up the font
	DeleteObject(m_hFont);
}


BEGIN_MESSAGE_MAP(CBaseImgDlg, CBaseRezDlg)
	//{{AFX_MSG_MAP(CBaseImgDlg)
	ON_WM_SIZE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextureDlg message handlers

void CBaseImgDlg::InitBaseImgDlg(CListCtrl* pItemList, CTreeCtrl* pTree, DWORD nDefaultIcon) 
{
	//make sure params are valid
	ASSERT(pItemList);
	ASSERT(pTree);

	//save these values
	m_pTreeCtrl			= pTree;
	m_pListCtrl			= pItemList;
	m_nDefaultIcon		= nDefaultIcon;

	CMRCSizeDialogBar::OnInitDialogBar();
	
	// TODO: Add extra initialization here
	char szSysPath[MAX_PATH];
	char szIconDLL[MAX_PATH];

	GetSystemDirectory(szSysPath,MAX_PATH);
	sprintf(szIconDLL,"%s\\shell32.dll",szSysPath);

	if (m_pTreeIcons)
	{
		delete m_pTreeIcons;
	}
	if (m_pListIcons)
	{
		delete m_pListIcons;
	}

	m_pTreeIcons=new CImageList;
	m_pListIcons=new CImageList;

	//tree control image list
	m_pTreeIcons->Create(16,16,ILC_COLOR16,10,5);
	m_pTreeIcons->SetBkColor(RGB(255,255,255));
	m_pTreeIcons->Add(ExtractIcon(AfxGetInstanceHandle(),szIconDLL,3));	//closed folder
	m_pTreeIcons->Add(ExtractIcon(AfxGetInstanceHandle(),szIconDLL,4));	//open folder
	m_pTreeCtrl->SetImageList(m_pTreeIcons,TVSIL_NORMAL);
	
	//list control image list
	m_pListIcons->Create(IMAGE_SIZE, IMAGE_SIZE, ILC_COLOR32,1,1);
	m_pListIcons->SetBkColor(RGB(0,128,128));
	m_pListIcons->Add(AfxGetApp()->LoadIcon(m_nDefaultIcon));
	m_pListCtrl->SetImageList(m_pListIcons,LVSIL_SMALL);
	
	//create the font for drawing the texture's name
	CDC* pDC = GetDC();
	//height of the font
	int32 nPointSize	= 10;
	int32 nHeight		= -MulDiv(nPointSize, pDC->GetDeviceCaps(LOGPIXELSY), 72);

	m_hFont = CreateFont(	nHeight, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH,
							"Ariel");

	//clean up
	ReleaseDC(pDC);
}

void CBaseImgDlg::OnSize(UINT nType, int cx, int cy) 
{
	CMRCSizeDialogBar::OnSize(nType, cx, cy);
	
	// Reposition the controls
	RepositionControls();
}

/************************************************************************/
// This is called to reposition the controls
void CBaseImgDlg::RepositionControls()
{
	if( ::IsWindow(m_hWnd))
	{
		CRect rect;
		GetClientRect( &rect );

		// Move the tree
		if (m_pTreeCtrl)
		{
			m_pTreeCtrl->MoveWindow( 0, 0, rect.Width(), rect.Height()/3 );
		}

		// Move the list
		if (m_pListCtrl)
		{
			m_pListCtrl->MoveWindow( 0, rect.Height()/3, rect.Width(), rect.Height() / 3);
		}		
	}		
}

void CBaseImgDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	//let the derived class render its large image
	RenderLargeImage();
}

//can be used by a derived class in order to find the rectangle that needs to be blitted
//to for the rendering of the large image, it will also clear the rect
CRect CBaseImgDlg::InitLargeImageRect()
{
	CRect DrawRect;
	GetClientRect( &DrawRect );
	//bring the height down so we are dealing with just the bottom 1/3 of the dlg
	DrawRect.top = (DrawRect.bottom * 2 / 3) ;

	//make a region to clear that covers the rect
	CRgn Rgn;
	Rgn.CreateRectRgn(DrawRect.left,DrawRect.top,DrawRect.right,DrawRect.bottom);

	//get the DC
	CDC *pDC = GetDC();
	ASSERT(pDC);

	//clear the region
	CBrush Brush(RGB(0,0,0));
	pDC->FillRgn(&Rgn,&Brush);

	//clean up
	ReleaseDC(pDC);

	return DrawRect;
}

//called in order to render the specified text to the upper left corner of the large
//image
void CBaseImgDlg::PrintLargeImageText(HDC hDC, CRect Area, const char* pszText)
{
	// Draw the texture name.
	if( m_hFont )
	{
		//just to keep it a bit away from the borders
		Area.DeflateRect(2, 2, 0, 0);
		
		::SetBkMode( hDC, TRANSPARENT );
		::SetTextColor( hDC, RGB(255,255,255) );

		HFONT hOldFont = (HFONT)::SelectObject( hDC, m_hFont );
		DrawText( hDC, pszText, strlen(pszText), &Area, DT_SINGLELINE );
		::SelectObject( hDC, hOldFont );
	}
}


void CBaseImgDlg::UpdateThumbnails()
{
	CClientDC	dcScreen(this);
	CDC			dcMem;
	CBitmap		*pOldBitmap;

	BeginWaitCursor( ); // Put the wait cursor on the screen
	m_pListIcons->SetImageCount(0);  // Empty list out

	dcMem.CreateCompatibleDC(&dcScreen);
	SetStretchBltMode( dcMem.m_hDC, COLORONCOLOR );

	if (m_pThumbnails)  
		delete m_pThumbnails;

	//create the new image
	m_pThumbnails = new CBitmap;

	ASSERT(m_pThumbnails);

	//create the bitmap surface
	int32 nNumIcons = m_pListCtrl->GetItemCount();
	if (!m_pThumbnails->CreateCompatibleBitmap(&dcScreen, IMAGE_SIZE * nNumIcons, IMAGE_SIZE))  
		return;

	pOldBitmap = dcMem.SelectObject(m_pThumbnails);

	//render all of the icons
	uint32 nCurrIcon;
	for (nCurrIcon = 0; nCurrIcon < nNumIcons; nCurrIcon++)
	{
		if(!m_bShowThumbnails || !RenderIcon(dcMem.m_hDC, IMAGE_SIZE * nCurrIcon, IMAGE_SIZE, nCurrIcon))
		{
			//they failed to draw the image, so we need to draw the default
			//icon in its place
			HICON hDefaultIcon = AfxGetApp()->LoadIcon(m_nDefaultIcon);

			// Default to the default icon
			DrawIcon(dcMem.m_hDC, IMAGE_SIZE * nCurrIcon, 0, hDefaultIcon);
		}
	}

	dcMem.SelectObject(pOldBitmap);

	//set these new icons as our image list
	m_pListIcons->Add(m_pThumbnails, RGB(0,0,0));

	//update the lsit to reflect the new icons
	for (nCurrIcon = 0; nCurrIcon < nNumIcons; nCurrIcon++)
	{
		m_pListCtrl->SetItem( nCurrIcon, 0, LVIF_IMAGE, NULL, nCurrIcon, 0, 0, 0 );
	}

	m_pListCtrl->Update(0);

	EndWaitCursor( ); // End the wait cursor
}

void CBaseImgDlg::DoShowThumbnails(bool bShow)
{
	m_bShowThumbnails = bShow;

	if (bShow)
	{
		UpdateThumbnails();
	}
	else
	{
		m_pListIcons->SetImageCount(0);  // Empty list out
		m_pListIcons->Add(AfxGetApp()->LoadIcon(m_nDefaultIcon));

		// Set all list items to the texture icon
		for (uint32 nCurrIcon = 0; nCurrIcon < m_pListCtrl->GetItemCount(); nCurrIcon++)
			m_pListCtrl->SetItem( nCurrIcon, 0, LVIF_IMAGE, NULL, 0, 0, 0, 0 );

		m_pListCtrl->Update(0); // Update the first item, since its image data doesn't change
	}

}