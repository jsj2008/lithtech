//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// WorldsDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "worldsdlg.h"
#include "editprojectmgr.h"
#include "edithelpers.h"
#include "mainfrm.h"
#include "projectbar.h"
#include "ltamgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CWorldsDlg dialog


CWorldsDlg::CWorldsDlg() : CBaseRezDlg()
{
	//{{AFX_DATA_INIT(CWorldsDlg)
	//}}AFX_DATA_INIT

	m_pIconList=NULL;
	m_pIconList2=NULL;

	m_hCurrentItem = NULL;
	InitBaseRezDlg("lta;ltc;tbw", &m_WorldTree, &m_WorldList, RESTYPE_WORLD);
}

CWorldsDlg::~CWorldsDlg()
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

void CWorldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CMRCSizeDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWorldsDlg)
	DDX_Control(pDX, IDC_BASEREZ_TREE, m_WorldTree);
	DDX_Control(pDX, IDC_BASEREZ_LIST, m_WorldList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWorldsDlg, CBaseRezDlg)
	//{{AFX_MSG_MAP(CWorldsDlg)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_BASEREZ_TREE, OnSelchangedDirectory)
	ON_NOTIFY(NM_DBLCLK, IDC_BASEREZ_LIST, OnDblclkWorldsList)
	ON_WM_CONTEXTMENU()	
	ON_COMMAND(ID_WORLD_OPEN, OnWorldListOpen)
	ON_COMMAND(ID_FILE_NEWWORLD, OnFileNewWorld)
	ON_COMMAND(ID_WORLD_DELETE, OnWorldDelete)
	ON_COMMAND(ID_WORLD_COMPRESS, OnWorldCompress)
	ON_COMMAND(ID_WORLD_DECOMPRESS, OnWorldDecompress)
	ON_COMMAND(ID_WORLDLIST_RUN, OnWorldRun)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldsDlg message handlers

void CWorldsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CMRCSizeDialogBar::OnSize(nType, cx, cy);
	
	// Reposition the controls
	RepositionControls();
}

BOOL CWorldsDlg::OnInitDialogBar() 
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
	m_WorldTree.SetImageList(m_pIconList,TVSIL_NORMAL);

	//list control image list
	m_pIconList2->Create(16,16,ILC_COLOR4,10,5);
	m_pIconList2->SetBkColor(RGB(255,255,255));
	m_pIconList2->Add(AfxGetApp()->LoadIcon(IDI_WORLDS_TAB_ICON));
	m_WorldList.SetImageList(m_pIconList2,LVSIL_SMALL);
	
	CRect rect;
	m_WorldList.GetClientRect( &rect );

	// Clear the world list
	m_WorldList.DeleteAllItems();

	while (m_WorldList.DeleteColumn(0))
	{
	}
	
	m_WorldList.InsertColumn(0,"Name",LVCFMT_LEFT,(rect.Width()-35)/3,-1);
	m_WorldList.InsertColumn(1,"Size",LVCFMT_RIGHT,(rect.Width()-35)/3,-1);
	m_WorldList.InsertColumn(2,"Modified",LVCFMT_LEFT,(rect.Width()-35)/3,-1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWorldsDlg::OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
//	LPPOINT lpPoint=new POINT;
//	UINT* pnFlags=new UINT;
	DDirIdent *pIdent;

//	GetCursorPos(lpPoint);
//	m_WorldTree.ScreenToClient(lpPoint);
//	HTREEITEM hItem=m_WorldTree.HitTest(*lpPoint,pnFlags);
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

void CWorldsDlg::OnDblclkWorldsList(NMHDR* pNMHDR, LRESULT* pResult)
{
	int iItem = m_WorldList.GetNextItem( -1, LVNI_SELECTED );
	DFileIdent *pIdent;
	CString fullName;

	//show a wait cursor for the users.
	CWaitCursor WaitCursor;
	
	if(iItem != -1)
	{
		pIdent = (DFileIdent*)m_WorldList.GetItemData(iItem);
		if(pIdent)
		{

			fullName = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
			GetProjectBar()->OpenRegionDoc(fullName);
		}
	}	

	return;
}

void CWorldsDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	CMenu menu;
	
	if(!GetProjectBar()->IsProjectOpen())
		return;

	CWnd* pWndPopupOwner = GetProjectBar();

	if(pWnd->m_hWnd == m_WorldTree.m_hWnd)
	{
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_WORLDTREE));
	}
	else if(pWnd->m_hWnd == m_WorldList.m_hWnd)
	{
		// Nothing active unless a directory is selected.
		if(!IsDirectorySelected())
		{
			return;
		}

		//make sure that the user is depressing this over an actual world, so that
		//the action is not ambiguious to what it is acting upon

		//get the point in terms of this window
		CPoint ClientPoint(point);
		m_WorldList.ScreenToClient(&ClientPoint);

		int iIndex = m_WorldList.HitTest(ClientPoint);
		//see if we are hitting any items
		if(iIndex == -1)
		{
			//no item under the cursor, so just make a menu that can only create a new map
			VERIFY(menu.LoadMenu(CG_IDR_POPUP_WORLDLIST_NOITEM));
			pWndPopupOwner = this;
		}
		else
		{
			//we have an item, so display all the options that can be done on that item
			VERIFY(menu.LoadMenu(CG_IDR_POPUP_WORLDLIST));
			pWndPopupOwner = this;

			//now we need to see if this file is compressed and only enable the opposite
			//menu item to be enable so the user doesn't get confused
			DFileIdent *pIdent = (DFileIdent*)m_WorldList.GetItemData(iIndex);

			if(CLTAUtil::IsFileCompressed(pIdent->m_Filename))
			{
				menu.RemoveMenu(ID_WORLD_COMPRESS, MF_BYCOMMAND);
			}
			else
			{
				menu.RemoveMenu(ID_WORLD_DECOMPRESS, MF_BYCOMMAND);
			}

		}
	}
	else return;


	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);	
}

void CWorldsDlg::OnWorldListOpen()
{
	OnDblclkWorldsList(NULL,NULL);

	return;
}

//gets the filename of the currently selected list item
CString CWorldsDlg::GetSelectedItemFileName()
{
	//the final name
	CString rv;

	int nSelected = m_WorldList.GetSelectionMark();

	//see if anything is selected
	if(nSelected == -1)
	{
		//this should not be
		ASSERT(false);
		return rv;
	}

	//now we get the file identifier from the selected item
	DFileIdent *pIdent = (DFileIdent*)m_WorldList.GetItemData(nSelected);

	if(pIdent == NULL)
	{
		//this means that the data was not properly associated with the list item
		ASSERT(false);
		return rv;
	}

	//get the final name
	rv = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);

	//success
	return rv;
}


//determines if the specified file exists
//determines if the designated file exists
static BOOL DoesFileExist(const char* pszFileName)
{
	CFile InFile;
	if(InFile.Open(pszFileName, CFile::modeRead) == TRUE)
	{
		InFile.Close();
		return TRUE;
	}

	return FALSE;
}


//swaps the given filename with the appropriate extension. Takes the input file,
//the expected extension for the input file, and the extension for the output file
static CString ConvertExtension(const CString& sInFileName, const char* pszInExt, const char* pszOutExt)
{
	//now we need to figure out the new name
	CString sOutFileName;

	int nInExtLen = strlen(pszInExt);

	//see if it ends with .lta
	if(	(sInFileName.GetLength() > nInExtLen) && 
		((sInFileName.Right(nInExtLen)).CompareNoCase(pszInExt) == 0))
	{
		//it does, so the final filename is simply that, with a changed last letter
		sOutFileName = sInFileName.Left(sInFileName.GetLength() - nInExtLen) + pszOutExt;
	}
	else
	{
		//no .lta extension, just tack it on the end
		sOutFileName = sInFileName + pszOutExt;
	}

	return sOutFileName;
}



void CWorldsDlg::OnWorldCompress()
{
	//first off, we need to get the filename of the selected item
	CString sInFileName = GetSelectedItemFileName();

	int nDotPos = sInFileName.ReverseFind('.');
	if(nDotPos == -1)
		return;

	CString sExt = sInFileName.Mid(nDotPos + 1);
	if(sExt.CompareNoCase("lta") != 0)
		return;

	//make sure it is compressed
	if(CLTAUtil::IsFileCompressed(sInFileName) == true)
	{
		//this shouldn't ever happen. The wrong menu item managed to come up
		ASSERT(false);
		return;
	}

	//make sure that the region we are compressing is not open for editing
	for(uint32 nCurrRegion = 0; nCurrRegion < GetProjectBar()->m_RegionDocs.GetSize(); nCurrRegion++)
	{
		if(sInFileName.CompareNoCase(GetProjectBar()->m_RegionDocs[nCurrRegion]->m_FileName) == 0)
		{
			//they are trying to compress a file that is open, we can't allow this, otherwise
			//when they save, it will save to the wrong filename. Another approach
			//would be to update the region's filename, but I don't think I'll have enough
			//time to thoroughly test this to my satisfaction, so I'll go with the error
			MessageBox(	sInFileName + " is currently open for editing.\nYou must close this file before you compress it",
						"Cannot Compress Open File", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
	}

	CString sOutFileName = ConvertExtension(sInFileName, ".lta", ".ltc");

	//see if the file already exists
	if(DoesFileExist(sOutFileName) == TRUE)
	{
		//prompt the user with an overwrite prompt
		if(MessageBox("The file " + sOutFileName + " already exists.\nWould you like to overwrite this file?",
					"File already exists", MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			//the user didn't want to overwrite the file
			return;
		}
	}

	//now we open up the input file, and compress it
	BeginWaitCursor();

	CLTAFile InFile(sInFileName, true, false);
	//make sure the files opened okay
	if(InFile.IsValid() == false)
	{
		MessageBox(	"Unable to open " + sInFileName + " for reading\nCompression not performed", 
					"Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	CLTAFile OutFile(sOutFileName, false, true);
	//make sure the files opened okay
	if(OutFile.IsValid() == false)
	{
		MessageBox(	"Unable to open " + sOutFileName + " for writing\nCompression not performed", 
					"Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//now we just compress
	uint8 nByte;
	while(InFile.ReadByte(nByte))
	{
		if(OutFile.WriteByte(nByte) == false)
		{
			MessageBox(	"Error while saving to " + sOutFileName + "\nCompression not performed",
						"Error", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
	}

	//close out the files
	InFile.Close();
	OutFile.Close();

	//now we need to delete the input file
	// Delete the world
	if (!DeleteFile(sInFileName))
	{
		MessageBox("Error deleting world" + sInFileName, "Error", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	// Refresh the display list
	PopulateList();

	EndWaitCursor();
}

void CWorldsDlg::OnWorldDecompress()
{

	//first off, we need to get the filename of the selected item
	CString sInFileName = GetSelectedItemFileName();

	int nDotPos = sInFileName.ReverseFind('.');
	if(nDotPos == -1)
		return;

	CString sExt = sInFileName.Mid(nDotPos + 1);
	if(sExt.CompareNoCase("ltc") != 0)
		return;

	//make sure it is compressed
	if(CLTAUtil::IsFileCompressed(sInFileName) == false)
	{
		//this shouldn't ever happen. The wrong menu item managed to come up
		ASSERT(false);
		return;
	}

	//make sure that the region we are compressing is not open for editing
	for(uint32 nCurrRegion = 0; nCurrRegion < GetProjectBar()->m_RegionDocs.GetSize(); nCurrRegion++)
	{
		if(sInFileName.CompareNoCase(GetProjectBar()->m_RegionDocs[nCurrRegion]->m_FileName) == 0)
		{
			//they are trying to compress a file that is open, we can't allow this, otherwise
			//when they save, it will save to the wrong filename. Another approach
			//would be to update the region's filename, but I don't think I'll have enough
			//time to thoroughly test this to my satisfaction, so I'll go with the error
			MessageBox(	sInFileName + " is currently open for editing.\nYou must close this file before you decompress it",
						"Cannot Decompress Open File", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
	}



	CString sOutFileName = ConvertExtension(sInFileName, ".ltc", ".lta");

	//see if the file already exists
	if(DoesFileExist(sOutFileName) == TRUE)
	{
		//prompt the user with an overwrite prompt
		if(MessageBox("The file " + sOutFileName + " already exists.\nWould you like to overwrite this file?",
					"File already exists", MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			//the user didn't want to overwrite the file
			return;
		}
	}

	//now we open up the input file, and compress it
	BeginWaitCursor();

	CLTAFile InFile(sInFileName, true, true);
	//make sure the files opened okay
	if(InFile.IsValid() == false)
	{
		MessageBox(	"Unable to open " + sInFileName + " for reading\nDecompression not performed", 
					"Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	CLTAFile OutFile(sOutFileName, false, false);
	//make sure the files opened okay
	if(OutFile.IsValid() == false)
	{
		MessageBox(	"Unable to open " + sOutFileName + " for writing\nDecompression not performed", 
					"Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//now we just decompress
	uint8 nByte;
	while(InFile.ReadByte(nByte))
	{
		if(OutFile.WriteByte(nByte) == false)
		{
			MessageBox(	"Error while saving to " + sOutFileName + "\nDecompression not performed",
						"Error", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
	}

	//close out the files
	InFile.Close();
	OutFile.Close();

	//now we need to delete the input file
	// Delete the world
	if (!DeleteFile(sInFileName))
	{
		MessageBox("Error deleting world" + sInFileName, "Error", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	// Refresh the display list
	PopulateList();

	EndWaitCursor();
}

void CWorldsDlg::OnFileNewWorld()
{
	GetApp()->OnFileNewWorld();
}

void CWorldsDlg::OnWorldRun()
{
	//get the main frame
	GetMainFrame()->RunWorld(GetSelectedItemFileName());
}

void CWorldsDlg::OnWorldDelete() 
{
	// Figure out which one they want to delete
	int iIndex = m_WorldList.GetSelectionMark();
	if (iIndex < 0)
		return;

	// Display a confirmation dialog
	CString csWorldName = m_WorldList.GetItemText(iIndex, 0);
	CString csMessage = "Are you sure you want to delete the world " + csWorldName + "?";
	if (MessageBox(csMessage, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;

	// Get the full name of the world
	DFileIdent *pIdent;
	pIdent = (DFileIdent*)m_WorldList.GetItemData(iIndex);
	if(!pIdent)
	{
		MessageBox("Internal error deleting world!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	csWorldName = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);

	// Delete the world
	if (!DeleteFile(csWorldName))
	{
		MessageBox("Error deleting world", "Error", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	// Refresh the display list
	PopulateList();
}
