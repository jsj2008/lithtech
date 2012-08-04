//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// SpriteEditDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "spriteeditdlg.h"
#include "projectbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpriteEditDlg dialog


CSpriteEditDlg::CSpriteEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpriteEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpriteEditDlg)
	m_bTransparent = FALSE;
	m_bTranslucent = FALSE;
	//}}AFX_DATA_INIT

	m_bTranslucent = m_bTransparent = FALSE;
	m_Key = 0;
//	m_pTextureDir = NULL;

}

CSpriteEditDlg::~CSpriteEditDlg()
{
	Term( );
}

void CSpriteEditDlg::Term( )
{
	m_Frames.Term( );
	m_StringHolder.ClearStrings( );
}

BOOL CSpriteEditDlg::Create( CWnd *pParent )
{
	Term( );
	return CDialog::Create( IDD_SPRITEEDIT_DLG, NULL );
}


void CSpriteEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpriteEditDlg)
	DDX_Control(pDX, IDC_FRAMERATE_SPIN, m_FrameRate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpriteEditDlg, CDialog)
	//{{AFX_MSG_MAP(CSpriteEditDlg)
	ON_BN_CLICKED(ID_SAVE_BUTTON, OnSaveButton)
	ON_NOTIFY(NM_CLICK, IDC_FRAMERATE_SPIN, OnClickFramerateSpin)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CSpriteEditDlg custom functions



void CSpriteEditDlg::LoadSpriteFile()
{
	CMoFileIO		file;
	DWORD			i, nFrames, frameRate, bTransparent, bTranslucent;
	char			str[256];


	if(dfm_OpenFileRelative(GetFileMgr(), m_pFile->m_Filename, file))
	{
		if( file.GetLen() > 0 )
		{
			file >> nFrames;
			file >> frameRate;
			file >> bTransparent;
			file >> bTranslucent;
			file >> m_Key;
	
			for( i=0; i < nFrames; i++ )
			{
				if( !file.ReadString(str, 256) )
					break;

				m_Frames.Append( m_StringHolder.AddString(str) );
			}

			m_FrameRate.SetPos( frameRate );
			m_bTransparent = bTransparent;
			m_bTranslucent = bTranslucent;
		}
		
		file.Close();
	}

	UpdateData( FALSE );
}


void CSpriteEditDlg::SaveSpriteFile()
{
	CMoFileIO file;
	DWORD i;
	DWORD frameRate;
	CString fullName;


	UpdateData();

	fullName = dfm_GetFullFilename(GetFileMgr(), m_pFile->m_Filename);
	if( file.Open(fullName, "wb") )
	{
		file << m_Frames.GetSize() << (DWORD)m_FrameRate.GetPos() << (DWORD)m_bTransparent << (DWORD)m_bTranslucent << m_Key;

		for( i=0; i < m_Frames; i++ )
			file.WriteString( m_Frames[i] );
		
		file.Close();
	}
	else
		AppMessageBox( IDS_ERROR_SAVING_SPRITE, MB_OK );
}

/*
void CSpriteEditDlg::FillTextureList()
{
	DWORD i;
	int index;
	CString searchSpec, fullName;
	HANDLE handle;
	WIN32_FIND_DATA findData;
	DFileIdent *pIdent;
	CFileIterator iterator;

	m_TextureList.ResetContent();
	if(!m_pTextureDir)
		return;
	
	// Add each .spr file to the list (with its FileIdent as the data).
	searchSpec = dfm_BuildName(m_pTextureDir->m_Filename, "*.dtx");
	while(iterator.Next(searchSpec, TRUE))
	{
		fullName = dfm_BuildName(m_pTextureDir->m_Filename, iterator.GetFilename());
		dfm_GetFileIdentifier(GetFileMgr(), fullName, &pIdent);
		if(pIdent)
		{
			index = m_TextureList.AddString( "" );
			m_TextureList.SetItemDataPtr(index, pIdent);
		}
	}
}
*/

void CSpriteEditDlg::FillSpriteList()
{
	DWORD i;
	int index;
	DFileIdent *pIdent;
	CListBox *pList;
	
	m_SpriteList.ResetContent();
	pList = (CListBox*)GetDlgItem(IDC_SPRITETEXTURENAMES);
	if(!pList)
		return;

	pList->ResetContent();
	for(i=0; i < m_Frames; i++)
	{
		index = m_SpriteList.AddString( "" );
		if(index != LB_ERR)
		{
			dfm_GetFileIdentifier(GetFileMgr(), m_Frames[i], &pIdent);
			if(pIdent)
			{
				m_SpriteList.SetItemDataPtr(index, pIdent);
			}
		}

		pList->AddString(m_Frames[i]);
	}
}


void CSpriteEditDlg::AddTexture( DFileIdent *pIdent )
{
	DWORD insertAt;
	int spriteCurSel = m_SpriteList.GetCurSel();
	
	if( spriteCurSel == LB_ERR )
		insertAt = 0;
	else
		insertAt = spriteCurSel+1;

	if(pIdent)
	{
		m_Frames.Insert(insertAt, pIdent->m_Filename);
		FillSpriteList();
		m_SpriteList.SetCurSel( spriteCurSel+1 );
	}
}



void CSpriteEditDlg::NotifyDblClk( CFrameList *pList, int curSel )
{
	m_Frames.Remove( curSel );
	FillSpriteList();
	m_SpriteList.SetCurSel( curSel );
}

void CSpriteEditDlg::NotifyReCreate(CFrameList *pList)
{
	m_SpriteList.DestroyWindow();
	InitSpriteList();

	CRect cClientRect;
	GetClientRect(cClientRect);
	m_cResizer.Resize(this, cClientRect);
}

/////////////////////////////////////////////////////////////////////////////
// CSpriteEditDlg message handlers

BOOL CSpriteEditDlg::OnInitDialog() 
{
	CString			str;
	char			path[256], fullFilename[256], fileName[256], ext[256];
	CWnd *pWnd;
	
	
	CDialog::OnInitDialog();


	if(pWnd = GetDlgItem(IDC_FRAMERATE_VALUE))
		m_FrameRate.SetBuddy(pWnd);

	m_FrameRate.SetRange(1, 30);
	m_FrameRate.SetPos(15);


/*
	m_TextureList.m_pNotifier = this;
	m_TextureList.Create(WS_VISIBLE | WS_CHILD | LBS_MULTICOLUMN | LBS_HASSTRINGS | LBS_OWNERDRAWFIXED | WS_HSCROLL |
					   LBS_NOINTEGRALHEIGHT | WS_BORDER | LBS_DISABLENOSCROLL | LBS_NOTIFY,
					   rcFrameList,
					   this,
					   3455);
*/

	LoadSpriteFile();

	InitSpriteList();
//	m_TextureList.m_bDrawNumbers = FALSE;


	
	// Set the title.
	CHelpers::ExtractPathAndFileName( m_pFile->m_Filename, path, fullFilename );
	CHelpers::ExtractFileNameAndExtension( fullFilename, fileName, ext );
	str.FormatMessage( IDS_SPRITE_EDITOR_TITLE, fileName );
	SetWindowText( str );

	
//	FillTextureList();

	GetProjectBar( )->SetTab( CMainFrame::CB_TEXTUREVIEW );


	CRect cClientRect;
	GetClientRect(cClientRect);
	// Only initialize the anchors on the first initialize
	if (m_cResizer.GetAnchorList().GetSize() == 0)
	{
		CResizeAnchorOffset *pAnchor;

		// Set up the resizing anchors
			// The texture list box
		m_cResizer.AddAnchor(new CResizeAnchorOffset(3456, CResizeAnchorOffset::eAnchorSize));
			// The "Textures" title
		pAnchor = new CResizeAnchorOffset(IDC_STATIC_SPRITE_TEXTURES, CResizeAnchorOffset::eAnchorPosition);
		pAnchor->SetBinding(ANCHOR_LEFT, ANCHOR_LEFT);
		pAnchor->SetBinding(ANCHOR_RIGHT, ANCHOR_LEFT);
		m_cResizer.AddAnchor(pAnchor);
			// The texture list
		pAnchor = new CResizeAnchorOffset(IDC_SPRITETEXTURENAMES, CResizeAnchorOffset::eAnchorPosition);
		pAnchor->SetBinding(ANCHOR_LEFT, ANCHOR_LEFT);
		m_cResizer.AddAnchor(pAnchor);
			// The "Framerate" title
		m_cResizer.AddAnchor(new CResizeAnchorOffset(IDC_FRAMERATE_TEXT, CResizeAnchorOffset::eAnchorPosition));
			// The framerate spin controls
		m_cResizer.AddAnchor(new CResizeAnchorOffset(IDC_FRAMERATE_VALUE, CResizeAnchorOffset::eAnchorPosition));
		m_cResizer.AddAnchor(new CResizeAnchorOffset(IDC_FRAMERATE_SPIN, CResizeAnchorOffset::eAnchorPosition));
			// The buttons on the right side
		m_cResizer.AddAnchor(new CResizeAnchorOffset(IDOK, CResizeAnchorOffset::eAnchorPosition));
		m_cResizer.AddAnchor(new CResizeAnchorOffset(ID_SAVE_BUTTON, CResizeAnchorOffset::eAnchorPosition));
		m_cResizer.AddAnchor(new CResizeAnchorOffset(IDCANCEL, CResizeAnchorOffset::eAnchorPosition));

		// Lock the resizing anchors
		m_cResizer.Lock(this, cClientRect);
	}
	else
		// Reset the sizes if this isn't the first initialization
		m_cResizer.Resize(this, cClientRect);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpriteEditDlg::InitSpriteList()
{
	CRect			rcSpriteFrameList;

	// Make sure and create the sprite list only once
	if (!m_SpriteList.m_hWnd)
	{
		GetDlgItem(IDC_SPRITEFRAMELISTPOS)->GetWindowRect( &rcSpriteFrameList );
		ScreenToClient( &rcSpriteFrameList );

		m_SpriteList.m_pNotifier = this;
		m_SpriteList.Create(WS_VISIBLE | WS_CHILD | LBS_MULTICOLUMN | LBS_HASSTRINGS | LBS_OWNERDRAWFIXED | WS_HSCROLL |
						   LBS_NOINTEGRALHEIGHT | WS_BORDER | LBS_DISABLENOSCROLL | LBS_NOTIFY,
						   rcSpriteFrameList,
						   this,
						   3456);

		m_SpriteList.m_bDrawNumbers = TRUE;
	}
	FillSpriteList();
}

void CSpriteEditDlg::OnOK() 
{
	SaveSpriteFile();
	Term( );
	DestroyWindow( );
	GetProjectBar( )->SetTab( CMainFrame::CB_SPRITEVIEW );
}


void CSpriteEditDlg::OnCancel() 
{
	Term( );
	DestroyWindow( );
	GetProjectBar( )->SetTab( CMainFrame::CB_SPRITEVIEW );
}


void CSpriteEditDlg::OnSaveButton() 
{
	SaveSpriteFile();	
}


BOOL CSpriteEditDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	return CDialog::OnCommand(wParam, lParam);
}







void CSpriteEditDlg::OnClickFramerateSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
}

void CSpriteEditDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// Don't resize if the window doesn't really exist..
	if (!IsWindow(m_hWnd))
		return;

	// Resize the controls
	CRect cNewSize(0,0, cx, cy);
	m_cResizer.Resize(this, cNewSize);

	// Redraw to clean up resize artifacts
	RedrawWindow();
}
