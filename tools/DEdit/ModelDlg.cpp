//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ModelDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "modeldlg.h"
#include "d_filemgr.h"
#include "projectbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CModelDlg dialog


CModelDlg::CModelDlg() : CBaseRezDlg()
{
	//{{AFX_DATA_INIT(CModelDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pIconList=NULL;
	m_pIconList2=NULL;

	InitBaseRezDlg("lta;ltc", &m_ModelTree, &m_ModelList, RESTYPE_MODEL);
}

CModelDlg::~CModelDlg()
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

void CModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CProjectTabControlBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModelDlg)
	DDX_Control(pDX, IDC_BASEREZ_TREE, m_ModelTree);
	DDX_Control(pDX, IDC_BASEREZ_LIST, m_ModelList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModelDlg, CBaseRezDlg)
	//{{AFX_MSG_MAP(CModelDlg)
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_BASEREZ_TREE, OnSelchangedDirectory)
	ON_NOTIFY( NM_DBLCLK, IDC_BASEREZ_LIST, OnDblClkModel )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModelDlg message handlers

BOOL CModelDlg::OnInitDialogBar() 
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
	m_pIconList=new CImageList;
	m_pIconList2=new CImageList;

	//tree control image list
	m_pIconList->Create(16,16,ILC_COLOR16,10,5);
	m_pIconList->SetBkColor(RGB(255,255,255));
	m_pIconList->Add(ExtractIcon(AfxGetInstanceHandle(),szIconDLL,3));	//closed folder
	m_pIconList->Add(ExtractIcon(AfxGetInstanceHandle(),szIconDLL,4));	//open folder
	m_ModelTree.SetImageList(m_pIconList,TVSIL_NORMAL);

	//list control image list
	m_pIconList2->Create(16,16,ILC_COLOR4,10,5);
	m_pIconList2->SetBkColor(RGB(255,255,255));
	m_pIconList2->Add(AfxGetApp()->LoadIcon(IDI_MODELS_TAB_ICON));
	m_ModelList.SetImageList(m_pIconList2,LVSIL_SMALL);
	
	CRect rect;
	m_ModelList.GetClientRect( &rect );

	m_ModelList.InsertColumn(0,"Name",LVCFMT_LEFT,(rect.Width()-35)/3,-1);
	m_ModelList.InsertColumn(1,"Size",LVCFMT_RIGHT,(rect.Width()-35)/3,-1);
	m_ModelList.InsertColumn(2,"Modified",LVCFMT_LEFT,(rect.Width()-35)/3,-1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModelDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	CMenu menu;
	
	if(!GetProjectBar()->IsProjectOpen())
		return;

	if(pWnd->m_hWnd == m_ModelTree.m_hWnd)
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_WORLDTREE));
	else if(pWnd->m_hWnd == m_ModelList.m_hWnd)
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_MODELS_DLG));
	else return;

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	CWnd* pWndPopupOwner = GetProjectBar();

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);		
}

void CModelDlg::OnSize(UINT nType, int cx, int cy) 
{
	CProjectTabControlBar::OnSize(nType, cx, cy);
	
	// Reposition the controls
	RepositionControls();
}

void CModelDlg::OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
//	LPPOINT lpPoint=new POINT;
//	UINT* pnFlags=new UINT;
	DDirIdent *pIdent;

//	GetCursorPos(lpPoint);
//	m_ModelTree.ScreenToClient(lpPoint);
//	HTREEITEM hItem=m_ModelTree.HitTest(*lpPoint,pnFlags);
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

void CModelDlg::OnDblClkModel( NMHDR * pNMHDR, LRESULT * pResult )
{
	POINT point;
	UINT nFlags;
	DFileIdent *pIdent;
	int nItem;
	CString fullName;

	GetCursorPos(&point);
	m_ModelList.ScreenToClient(&point);

	nItem = m_ModelList.HitTest(point,&nFlags);

	if(nItem >= 0)
	{
		pIdent = (DFileIdent*)m_ModelList.GetItemData(nItem);
		fullName = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
		
		ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), "open", fullName, NULL, "", SW_SHOW);
	}

	*pResult = 0;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelDlg::DoImportOperation
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

BOOL CModelDlg::DoImportOperation()
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


	importExt.LoadString( IDS_MODEL_EXTENSION );
	fileMask.LoadString( IDS_MODEL_FILEMASK );
	newExtension.LoadString( IDS_MODEL_EXTENSION );
	sFileName = GetProject()->m_BaseProjectDir + "\\Models\\*" + importExt;

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

				tempStr.FormatMessage( IDS_MODEL_ALREADY_EXISTS, fileName );
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


