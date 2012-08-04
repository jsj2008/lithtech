//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// SoundDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "sounddlg.h"
#include "editprojectmgr.h"
#include "edithelpers.h"
#include "mainfrm.h"
#include "projectbar.h"
#include "resnewdir.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSoundDlg dialog


CSoundDlg::CSoundDlg()
{
	//{{AFX_DATA_INIT(CSoundDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pIconList=NULL;
	m_pIconList2=NULL;

	InitBaseRezDlg("wav", &m_SoundTree, &m_SoundList, RESTYPE_SOUND);
}

CSoundDlg::~CSoundDlg()
{
	if (m_pIconList)
	{
		delete m_pIconList;
	}
	if (m_pIconList2)
	{
		delete m_pIconList2;
	}
}

void CSoundDlg::DoDataExchange(CDataExchange* pDX)
{
	CMRCSizeDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundDlg)
	DDX_Control(pDX, IDC_BASEREZ_TREE, m_SoundTree);
	DDX_Control(pDX, IDC_BASEREZ_LIST, m_SoundList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSoundDlg, CMRCSizeDialogBar)
	//{{AFX_MSG_MAP(CSoundDlg)
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_BASEREZ_TREE, OnSelchangedDirectory)
	ON_NOTIFY( NM_DBLCLK, IDC_BASEREZ_LIST, OnDblClickSound )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSoundDlg message handlers

BOOL CSoundDlg::OnInitDialogBar() 
{
	CMRCSizeDialogBar::OnInitDialogBar();
	
	// TODO: Add extra initialization here
	char szSysPath[MAX_PATH];
	char szIconDLL[MAX_PATH];

	GetSystemDirectory(szSysPath,MAX_PATH);
	sprintf(szIconDLL,"%s\\shell32.dll",szSysPath);

	if (m_pIconList)
	{
		delete m_pIconList;
	}
	if (m_pIconList2)
	{
		delete m_pIconList2;
	}
	m_pIconList = new CImageList;
	m_pIconList2 = new CImageList;

	//tree control image list
	m_pIconList->Create(16,16,ILC_COLOR16,10,5);
	m_pIconList->SetBkColor(RGB(255,255,255));
	m_pIconList->Add(ExtractIcon(AfxGetInstanceHandle(),szIconDLL,3));	//closed folder
	m_pIconList->Add(ExtractIcon(AfxGetInstanceHandle(),szIconDLL,4));	//open folder
	m_SoundTree.SetImageList(m_pIconList,TVSIL_NORMAL);

	//list control image list
	m_pIconList2->Create(16,16,ILC_COLOR4,10,5);
	m_pIconList2->SetBkColor(RGB(255,255,255));
	m_pIconList2->Add(AfxGetApp()->LoadIcon(IDI_SOUNDS_TAB_ICON));
	m_SoundList.SetImageList(m_pIconList2,LVSIL_SMALL);
	
	CRect rect;
	m_SoundList.GetClientRect( &rect );

	m_SoundList.InsertColumn(0,"Name",LVCFMT_LEFT,(rect.Width()-35)/3,-1);
	m_SoundList.InsertColumn(1,"Size",LVCFMT_RIGHT,(rect.Width()-35)/3,-1);
	m_SoundList.InsertColumn(2,"Modified",LVCFMT_LEFT,(rect.Width()-35)/3,-1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSoundDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	CMenu menu;
	
	if(!GetProjectBar()->IsProjectOpen())
		return;

	if(pWnd->m_hWnd == m_SoundTree.m_hWnd)
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_WORLDTREE));
	else if(pWnd->m_hWnd == m_SoundList.m_hWnd)
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_SOUNDS_DLG));
	else return;

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	CWnd* pWndPopupOwner = GetProjectBar();

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);			
}

void CSoundDlg::OnSize(UINT nType, int cx, int cy) 
{
	CMRCSizeDialogBar::OnSize(nType, cx, cy);
	
	// Reposition the controls
	RepositionControls();
}

void CSoundDlg::OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
//	LPPOINT lpPoint=new POINT;
//	UINT* pnFlags=new UINT;
	DDirIdent *pIdent;

//	GetCursorPos(lpPoint);
//	m_SoundTree.ScreenToClient(lpPoint);
//	HTREEITEM hItem=m_SoundTree.HitTest(*lpPoint,pnFlags);
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

	//cleanup
//	delete lpPoint;
//	delete pnFlags;

	*pResult = 0;
}

void CSoundDlg::OnDblClickSound( NMHDR * pNMHDR, LRESULT * pResult )
{
	POINT point;
	UINT nFlags;
	DFileIdent *pIdent;
	int nItem;
	CString fullName;

	GetCursorPos(&point);
	m_SoundList.ScreenToClient(&point);

	nItem=m_SoundList.HitTest(point,&nFlags);

	if(nItem >= 0)
	{
		pIdent = (DFileIdent*)m_SoundList.GetItemData(nItem);
		fullName = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
		
		ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), "open", fullName, NULL, "", SW_SHOW);
	}

	*pResult = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDlg::DoImportOperation
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

BOOL CSoundDlg::DoImportOperation()
{
	CString			importExt, fileMask, newExtension, sFileName;
	CString			pathName, targetFilename, tempStr, relativeTargetFilename;

	POSITION		pos;
	char			fileTitle[256];
	char			fileName[256], fileExt[256];
	
	int				status;
	CMoFileIO		file;

	char szTemp[MAX_PATH];
	CAbstractIO* pFile = NULL;
	WORD width, height,nMipMaps; 
	int dummy[4]; 


	// If no directory is selected, then we can't import anything.
	if(!IsDirectorySelected())
		return FALSE;


	importExt.LoadString( IDS_WAV_EXTENSION );
	fileMask.LoadString( IDS_WAV_FILEMASK );
	newExtension.LoadString( IDS_WAV_EXTENSION );
	sFileName = GetProject()->m_BaseProjectDir + "\\Sounds\\*" + importExt;

	CHelperFileDlg	dlg( TRUE, importExt, (LPCTSTR)sFileName, OFN_ALLOWMULTISELECT|OFN_OVERWRITEPROMPT, fileMask, this );
	if( dlg.DoModal() == IDOK )
	{
		// Note: pPos is NOT used as the filename here!!!
		for( pos=dlg.GetStartPosition(); pos != NULL; )
		{		
			pathName = dlg.GetNextPathName( pos );
			
			GetFileTitle( (LPCTSTR)pathName, fileTitle, 256 );
			CHelpers::ExtractFileNameAndExtension( fileTitle, fileName, fileExt );
		
			// Does the file already exist?
			relativeTargetFilename = dfm_BuildName(m_csCurrentDir, fileTitle);
			targetFilename = dfm_GetFullFilename(GetFileMgr(), relativeTargetFilename);

			if( file.Open(targetFilename, "rb") )
			{
				file.Close();

				tempStr.FormatMessage( IDS_SOUND_ALREADY_EXISTS, fileName );
				status = MessageBox( tempStr, AfxGetAppName(), MB_YESNOCANCEL );
			
				if( status == IDYES )
				{
					CopyFile( pathName, targetFilename, FALSE );
					AddFileToList(relativeTargetFilename);
				}
				else if( status == IDCANCEL )
				{
					break;
				}
			}
			else
			{
				CopyFile( pathName, targetFilename, FALSE );
				AddFileToList(relativeTargetFilename);
			}
		}

		return TRUE;
	}

	return FALSE;
}
