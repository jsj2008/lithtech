#include "bdefs.h"
#include "renameresourcedlg.h"
#include "resource.h"
#include "edithelpers.h"
#include "editprojectmgr.h"
#include "dirdialog.h"
#include "filepalette.h"
#include "texture.h"
#include "ltamgr.h"
#include "regiondoc.h"
#include "worldnode.h"
#include "FileUtils.h"
#include "ProjectBar.h"
#include <direct.h>

#define IMAGE_SIZE			32

#define COL_SOURCEFILE		0
#define COL_ACTION			1
#define COL_DESTNAME		2
#define COL_DESTDIR			3


//---------------------------------------------------------------------------------------
// File utilities
//
// Functions for handling proper formatting and managing of files
//
//---------------------------------------------------------------------------------------


//given a full file name it will return the file name without the path
static CString ExtractFileName(const CString& sFile)
{
	int nPos = sFile.ReverseFind('\\');
	
	//no path
	if(nPos == -1)
		return sFile;

	return sFile.Mid(nPos + 1);
}

//given a full filename it will return the path portion
static CString ExtractFilePath(const CString& sFile)
{
	int nPos = sFile.ReverseFind('\\');
	
	//no path
	if(nPos == -1)
		return sFile;

	return sFile.Left(nPos + 1);
}


//given a full filename, it will return the file name without the project path
//with no leading \'s
static CString RemoveProjectPath(const CString& sFile)
{
	//get the project path
	CString sProjPath = GetProject()->m_BaseProjectDir;

	//make sure that the other file is longer than this
	if(sFile.GetLength() <= sProjPath.GetLength())
		return sFile;

	//see if we have a matching front end
	if(sFile.Left(sProjPath.GetLength()).CompareNoCase(sProjPath) == 0)
	{
		//it matches
		return sFile.Mid(sProjPath.GetLength() + 1);
	}

	return sFile;
}

//given a directory and a name it will build up the final file name
static CString BuildFinalFile(const CString& sDir, const CString& sFile)
{
	CString sFinalDir = sDir;
	CFileUtils::EnsureValidFileName(sFinalDir);

	//trim off any beginning or ending slashes
	sFinalDir.TrimLeft();
	sFinalDir.TrimRight();
	sFinalDir.TrimLeft("\\");
	sFinalDir.TrimRight("\\");

	//add on the project path if necessary
	if(sFinalDir.Find(':') == -1)
	{
		sFinalDir = GetProject()->m_BaseProjectDir + '\\' + sFinalDir;
	}

	//now clean up the filename as well
	CString sCleanFile = sFile;
	CFileUtils::EnsureValidFileName(sCleanFile);
	sCleanFile.TrimLeft();
	sCleanFile.TrimRight();
	sCleanFile.TrimLeft("\\");
	sCleanFile.TrimRight("\\");

	sFinalDir += '\\' + sCleanFile;

	return sFinalDir;
}

//given a directory and a name it will build up the final file name that is relative
//to the project directory
static CString BuildFinalFileRel(const CString& sDir, const CString& sFile)
{
	CString sFinalDir = sDir;
	CFileUtils::EnsureValidFileName(sFinalDir);

	//trim off any beginning or ending slashes
	sFinalDir.TrimLeft("\\");
	sFinalDir.TrimRight("\\");

	//now add on the file name
	if((sFinalDir.GetLength() > 0) && (sFinalDir[sFinalDir.GetLength() - 1] != '\\'))
		sFinalDir += '\\';

	sFinalDir += sFile;

	return sFinalDir;
}

//recursively builds a directory tree
static void BuildDirectoryTree(const char* pszPath)
{
	//determine the path we want to create
	CString sPathLeft(pszPath);

	CString sPathDone = "\\";

	//remove the drive identifier
	sPathLeft = sPathLeft.Mid(sPathLeft.Find(":") + 1);
	sPathLeft.TrimLeft('\\');

	while(1)
	{
		int nPos = sPathLeft.Find('\\');

		if(nPos == -1)
		{
			//none left, all done
			return;
		}

		//we still have parts left, move over the part and create it
		sPathDone += sPathLeft.Left(nPos + 1);
		sPathLeft = sPathLeft.Mid(nPos + 1);

		//create the directory
		_mkdir(sPathDone);
	}
}

//given a source file it will create the full filename
static CString BuildFullSourceFile(const char* pszFile)
{
	CString sRV = GetProject()->m_BaseProjectDir + '\\';
	sRV += pszFile;

	return sRV;
}


//moves the file specified in source to the location in dest. It will create directories
//if needed.
static bool MoveFileTo(const char* pSrc, const char* pDest)
{
	//now we need to ensure that the directory structure is in place
	BuildDirectoryTree(pDest);

	bool bTryCopying = true;

	while(bTryCopying)
	{
		//we don't want to copy again
		bTryCopying = false;

		TRY
		{
			//we first need to copy the file over
			CFile::Rename(pSrc, pDest);
		}
		CATCH( CFileException, e )
		{
			//an error occurred renaming it
			CString sMsg;
			sMsg.Format("An error occurred moving the file %s to %s. Please ensure that a file is not at the destination location and that there is enough space available.", pSrc, pDest);

			if(MessageBox(NULL, sMsg, "Critical Error", MB_RETRYCANCEL | MB_ICONEXCLAMATION) == IDRETRY)
			{
				//ok, we really do want to try again

				bTryCopying = true;
			}
			else
			{
				return false;
			}
		}
		END_CATCH;
	}

	return true;
}

//given a prefab name (absolute) it will return the bitmap filename
static CString GetPrefabBitmapName(const char* pszPrefab)
{
	CString sBitmap(pszPrefab);

	//remove the extension
	sBitmap = sBitmap.Left(sBitmap.GetLength() - 3);

	//add our extension
	sBitmap += "bmp";

	return sBitmap;
}

//----------------------------------------------------------------------------
// CResourceInfo

//different resource types (bit field so they can potentially overlap)
#define RESTYPE_TEXTURE			0x01
#define RESTYPE_PREFAB			0x02

class CResourceInfo
{
public:

	CString			m_sSourceFile;
	CString			m_sDestName;
	CString			m_sDestDir;
	EResourceAction	m_eAction;
	uint32			m_nType;
};

//given a resource action, this will give a human readable string version
static const char* ActionToString(EResourceAction eAction)
{
	if(eAction == eResAction_Move)
		return "Move To";
	if(eAction == eResAction_Replace)
		return "Replace With";

	return "Unknown";
}

//given a resource action, this will give a human readable string version
static DWORD ActionToControl(EResourceAction eAction)
{
	if(eAction == eResAction_Move)
		return IDC_RADIO_ACTIONMOVE;

	return IDC_RADIO_ACTIONREPLACE;
}


//----------------------------------------------------------------------------
// CRenameResourceDlg


BEGIN_MESSAGE_MAP (CRenameResourceDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_RES_RENAME_ADD, OnAddResources)	
	ON_BN_CLICKED(IDC_BUTTON_RES_RENAME_REMOVE, OnRemoveResources)	
	ON_BN_CLICKED(IDC_BUTTON_RES_RENAME_CLEAR, OnClear)

	ON_BN_CLICKED(IDC_BUTTON_RES_RENAME_OUTDIR_BROWSE, OnBrowseDestDir)
	ON_BN_CLICKED(IDC_BUTTON_RES_RENAME_OUTNAME_BROWSE, OnBrowseDestName)

	ON_BN_CLICKED(IDC_RADIO_ACTIONMOVE, OnRadioActionMove)
	ON_BN_CLICKED(IDC_RADIO_ACTIONREPLACE, OnRadioActionReplace)

	ON_BN_CLICKED(IDC_RADIO_RES_RENAME_UPDATE_ALL, UpdateEnabled)
	ON_BN_CLICKED(IDC_RADIO_RES_RENAME_UPDATE_ONLY_OPEN, UpdateEnabled)

	ON_BN_CLICKED(IDC_BUTTON_BROWSE_LEVEL_DIR, OnBrowseLevelDir)

	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_RES_RENAME, OnSelectionChange)

	ON_EN_CHANGE(IDC_EDIT_RES_RENAME_OUTNAME, OnResEditChange)
	ON_EN_CHANGE(IDC_EDIT_RES_RENAME_OUTDIR, OnResEditChange)

END_MESSAGE_MAP()

CRenameResourceDlg::CRenameResourceDlg(CWnd* pParentWnd) :	
	CDialog(IDD_RENAMERESOURCE, pParentWnd),
	m_bIgnoreEditChanges(false),
	m_pIconBitmap(NULL)
{
}

CRenameResourceDlg::~CRenameResourceDlg()
{
	//free up the bitmap
	delete m_pIconBitmap;
}



//---------------------------------------------------------------------------------------
// User Interface
//
// Functions for handling the user interface of the dialog
//
//---------------------------------------------------------------------------------------


//standard button handlers
void CRenameResourceDlg::OnOK()
{
	BeginWaitCursor();

	//determine what levels need to be updated
	bool bUpdateClosed		= ShouldUpdateAllLevels();

	//build up the directory we want to start on
	CString sLevelDir;
	GetDlgItem(IDC_EDIT_LEVEL_DIR)->GetWindowText(sLevelDir);
	sLevelDir = BuildFinalFile(sLevelDir, "");

	//we need to update the LTA files with the changes
	uint32 nNumModified = UpdateAllLevels(sLevelDir, bUpdateClosed);

	//now we need to copy over all the resources to their new locations
	MoveResources();

	//update the project bar
	UpdateProjectBar();

	EndWaitCursor();

	//tell them how many were modified
	CString sMsg;
	sMsg.Format("Resource renaming complete. %d levels were updated.", nNumModified);
	MessageBox(sMsg, "Task Complete", MB_OK);

	FreeAllItems();

	//don't call the base OK....
}

void CRenameResourceDlg::OnCancel()
{
	//don't call the base cancel, but hide ourselves...
	ShowWindow(SW_HIDE);
}

//handle initialization and loading of icons
BOOL CRenameResourceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//setup the column headers
	GetResList()->InsertColumn( COL_SOURCEFILE, "Source", LVCFMT_LEFT );
	GetResList()->InsertColumn( COL_ACTION, "Action", LVCFMT_LEFT );
	GetResList()->InsertColumn( COL_DESTNAME, "Name", LVCFMT_LEFT );
	GetResList()->InsertColumn( COL_DESTDIR, "Directory", LVCFMT_LEFT );

	//setup the widths of the columns to auto fit
	GetResList()->SetColumnWidth( COL_SOURCEFILE, 250 );
	GetResList()->SetColumnWidth( COL_ACTION, 80 );
	GetResList()->SetColumnWidth( COL_DESTNAME, 125 );
	GetResList()->SetColumnWidth( COL_DESTDIR, 220 );

	//make it so the user can select any part of that row. Makes it much easier to use
	GetResList()->SetExtendedStyle(	LVS_EX_FULLROWSELECT );

	//update the enabled status
	UpdateEnabled();

	//initialize the update value
	((CButton*)GetDlgItem(IDC_RADIO_RES_RENAME_UPDATE_ONLY_OPEN))->SetCheck(1);

	//we should scale texture coordinates by default
	((CButton*)GetDlgItem(IDC_CHECK_SCALE_TEXTURE_UVS))->SetCheck(1);

	//setup the icon list
	m_IconList.Create(IMAGE_SIZE,IMAGE_SIZE,ILC_COLOR32,1,1);
	m_IconList.SetBkColor(RGB(0,128,128));
	m_IconList.Add(AfxGetApp()->LoadIcon(IDI_TEXTURE_TAB_ICON));
	GetResList()->SetImageList(&m_IconList, LVSIL_SMALL);

	return TRUE;
}

//determines if based upon the current dialog settings whether or not levels on disk should
//be modified
bool CRenameResourceDlg::ShouldUpdateAllLevels()
{
	return ((CButton*)GetDlgItem(IDC_RADIO_RES_RENAME_UPDATE_ALL))->GetCheck() ? true : false;
}

//set the radio button to a specified action (-1 clears all)
void CRenameResourceDlg::SetAction(int nSelAction)
{
	//clear out all of them
	((CButton*)GetDlgItem(IDC_RADIO_ACTIONMOVE))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_RADIO_ACTIONREPLACE))->SetCheck(0);

	//set the active one
	((CButton*)GetDlgItem(nSelAction))->SetCheck(1);
}

//determines the number of items selected
uint32 CRenameResourceDlg::GetNumSelected()
{
	return (uint32)GetResList()->GetSelectedCount();
}

//handles updating the various controls
void CRenameResourceDlg::UpdateEnabled()
{
	//get the number of items selected in the list
	DWORD nNumSelected = GetNumSelected();

	//if there are 0 items selected, all our input controls are disabled
	//if there is 1 item selected, all of our controls need to be selected,
	//if there are multiple items selected, only the directory controls should be enabled
	
	//the output name
	GetDlgItem(IDC_EDIT_RES_RENAME_OUTNAME)->EnableWindow(nNumSelected == 1);
	GetDlgItem(IDC_BUTTON_RES_RENAME_OUTNAME_BROWSE)->EnableWindow(nNumSelected == 1);

	//the output directory
	GetDlgItem(IDC_EDIT_RES_RENAME_OUTDIR)->EnableWindow(nNumSelected > 0);
	GetDlgItem(IDC_BUTTON_RES_RENAME_OUTDIR_BROWSE)->EnableWindow(nNumSelected > 0);

	//now also handle the remove button which requires a selection
	GetDlgItem(IDC_BUTTON_RES_RENAME_REMOVE)->EnableWindow(nNumSelected > 0);

	//handle the action radio controls
	GetDlgItem(IDC_RADIO_ACTIONMOVE)->EnableWindow(nNumSelected > 0);
	GetDlgItem(IDC_RADIO_ACTIONREPLACE)->EnableWindow(nNumSelected > 0);

	//the level directory is only applicable if we are doing an update on all levels
	bool bUpdateAllLevels = ShouldUpdateAllLevels();
	GetDlgItem(IDC_EDIT_LEVEL_DIR)->EnableWindow(bUpdateAllLevels);
	GetDlgItem(IDC_BUTTON_BROWSE_LEVEL_DIR)->EnableWindow(bUpdateAllLevels);
}


//handle the button for adding resources to the list
void CRenameResourceDlg::OnAddResources()
{
	//create a file dialog and get the list of names
	CString sFilter;
	sFilter.LoadString(IDS_RENAME_RESOURCE_FILTER);

	CFileDialog FileDlg(TRUE, "dtx", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, sFilter);
	if(FileDlg.DoModal() == IDOK)
	{
		//we need to go through and add all these textures to our list
		POSITION Pos = FileDlg.GetStartPosition();

		while(Pos)
		{
			//get the filename and move along
			CString sFile = FileDlg.GetNextPathName(Pos);

			AddNewFile(sFile);
		}

		UpdateIcons();
	}
}

void CRenameResourceDlg::AddNewFile(const char* pszFile)
{
	//make sure that the filename is clean
	CString sFile(pszFile);

	CFileUtils::EnsureValidFileName(sFile);

	//make sure that this file isn't already in the list
	uint32 nNumRes = GetResList()->GetItemCount();

	for(uint32 nCurrRes = 0; nCurrRes < nNumRes; nCurrRes++)
	{
		//get the associated data
		CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nCurrRes);

		if(pInfo->m_sSourceFile.CompareNoCase(sFile) == 0)
		{
			//already in the list
			return;
		}
	}


	//now we need to add it to our list
	CResourceInfo *pNewRes = new CResourceInfo;

	if(pNewRes)
	{
		pNewRes->m_sSourceFile	= RemoveProjectPath(sFile);
		pNewRes->m_sDestDir		= RemoveProjectPath(ExtractFilePath(sFile));
		pNewRes->m_eAction		= eResAction_Replace;
		pNewRes->m_sDestName	= ExtractFileName(sFile);
	}

	//now add an item to the list
	int nItemPos = GetResList()->InsertItem(0, sFile);

	//associate our data with the item
	GetResList()->SetItemData(nItemPos, (DWORD)pNewRes);

	//update the type of this item
	//clear it out
	pNewRes->m_nType = 0;

	//so now check the extension
	if(pNewRes->m_sSourceFile.Right(4).CompareNoCase(".dtx") == 0)
	{
		pNewRes->m_nType |= RESTYPE_TEXTURE;
	}
	else
	{
		pNewRes->m_nType |= RESTYPE_PREFAB;
	}


	//setup the item
	UpdateItem(nItemPos);

	//invalidate our view
	Invalidate();
}

//handle the button that clears the entire list
void CRenameResourceDlg::OnClear()
{
	//remove all items
	FreeAllItems();
}

//handle the button for removing resources from the list
void CRenameResourceDlg::OnRemoveResources()
{
	//need to continually get the first one and remove it to prevent invalid
	//indexing into the list
	while(1)
	{
		//get all selected items and remove them
		POSITION Pos = GetResList()->GetFirstSelectedItemPosition();

		if(Pos)
		{	
			//found an item, remove it
			int nSelItem = GetResList()->GetNextSelectedItem(Pos);
			RemoveItem(nSelItem);
		}
		else
		{
			//no more items to remove
			break;
		}
	}

	UpdateIcons();
}

//given the index to an item, it will update the data in the columns
//to reflect the change
void CRenameResourceDlg::UpdateItem(int nItem)
{
	//first off, get the item data
	CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nItem);

	if(pInfo)
	{
		//see if this file exists
		CString sFile = BuildFinalFile(pInfo->m_sDestDir, pInfo->m_sDestName);

		bool bExists = CFileUtils::DoesFileExist(sFile);

		//setup the column text
		GetResList()->SetItemText(nItem, COL_SOURCEFILE, pInfo->m_sSourceFile);
		GetResList()->SetItemText(nItem, COL_ACTION, ActionToString(pInfo->m_eAction));
		GetResList()->SetItemText(nItem, COL_DESTNAME, pInfo->m_sDestName + (bExists ? "*" : ""));
		GetResList()->SetItemText(nItem, COL_DESTDIR, pInfo->m_sDestDir);
	}
}

//given the index to an item, it will remove it from the list
void CRenameResourceDlg::RemoveItem(int nItem)
{
	//first off get the item data and clean it up
	CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nItem);
	delete pInfo;

	//now remove the item
	GetResList()->DeleteItem(nItem);
}

//handle when the selection changes
void CRenameResourceDlg::OnSelectionChange(NMHDR* pmnh, LRESULT* pResult)
{
	//see if this is actually a change
	NMLISTVIEW* pLVHdr = (NMLISTVIEW*)pmnh;

	if(pLVHdr->uChanged & LVIF_STATE)
	{
		//supress the change from modifying our objects
		m_bIgnoreEditChanges = true;

		UpdateEnabled();
		UpdateResEdits();

		//reenable the changes
		m_bIgnoreEditChanges = false;
	}
}

//frees all the items in the list
void CRenameResourceDlg::FreeAllItems()
{
	//go through and free all items in the list
	for(uint32 nCurrItem = 0; nCurrItem < GetResList()->GetItemCount(); nCurrItem++)
	{
		//get the data and free it
		CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nCurrItem);
		delete pInfo;
	}

	//now clear out the list
	GetResList()->DeleteAllItems();

	//clear out the directory text fields
	GetDlgItem(IDC_EDIT_RES_RENAME_OUTNAME)->SetWindowText("");
	GetDlgItem(IDC_EDIT_RES_RENAME_OUTDIR)->SetWindowText("");

	//and finally update the enabled status
	UpdateEnabled();
}

//handles updating the edit controls to reflect a change in selection
void CRenameResourceDlg::UpdateResEdits()
{
	//get the number of items selected in the list
	DWORD nNumSelected = GetNumSelected();

	//if there are 0 items selected, all fields are blank
	//if there is 1 item selected, all of our controls are filled in
	//if there are multiple items selected, only the directory controls should be filled in
	//    with the first item's directory
	
	CString sDestName, sDestDir;

	//keep track of the action as well
	DWORD	nAction = ActionToControl(eResAction_Replace);

	if(nNumSelected > 0)
	{
		//get the first selected item
		POSITION Pos = GetResList()->GetFirstSelectedItemPosition();
		ASSERT(Pos);

		//get the index
		int nSelItem = GetResList()->GetNextSelectedItem(Pos);
	
		//get the item data
		CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nSelItem);

		//now setup the names appropriately
		if(nNumSelected == 1)
		{
			sDestName = pInfo->m_sDestName;
		}

		sDestDir = pInfo->m_sDestDir;
		nAction = ActionToControl(pInfo->m_eAction);
	}

	//setup the text in the controls
	GetDlgItem(IDC_EDIT_RES_RENAME_OUTNAME)->SetWindowText(sDestName);
	GetDlgItem(IDC_EDIT_RES_RENAME_OUTDIR)->SetWindowText(sDestDir);

	//and update the radio control
	SetAction(nAction);
}

//handle when one of the resource edit fields is changed
void CRenameResourceDlg::OnResEditChange()
{
	//see if we should ignore this
	if(m_bIgnoreEditChanges)
		return;

	//copy the values into the structures of the selected items
	POSITION Pos = GetResList()->GetFirstSelectedItemPosition();

	while(Pos)
	{
		//found an item, update it
		int nSelItem = GetResList()->GetNextSelectedItem(Pos);

		//get the item data
		CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nSelItem);

		if(GetNumSelected() == 1)
		{
			GetDlgItem(IDC_EDIT_RES_RENAME_OUTNAME)->GetWindowText(pInfo->m_sDestName);
		}

		GetDlgItem(IDC_EDIT_RES_RENAME_OUTDIR)->GetWindowText(pInfo->m_sDestDir);

		//now update the item
		UpdateItem(nSelItem);
	}
}


//handle browsing for the starting directory
void CRenameResourceDlg::OnBrowseLevelDir()
{
	CDirDialog Dlg;

	Dlg.m_hwndOwner		= GetSafeHwnd();
	Dlg.m_strTitle		= "Select the directory to update levels under.";
	Dlg.m_strInitDir	= GetProject()->m_BaseProjectDir;

	if(Dlg.DoBrowse())
	{
		//change the edit text
		GetDlgItem(IDC_EDIT_LEVEL_DIR)->SetWindowText(RemoveProjectPath(Dlg.m_strPath));
	}
}

//handle browsing for an output directory
void CRenameResourceDlg::OnBrowseDestDir()
{
	CDirDialog Dlg;

	Dlg.m_hwndOwner		= GetSafeHwnd();
	Dlg.m_strTitle		= "Select the output directory for this resource.";
	Dlg.m_strInitDir	= GetProject()->m_BaseProjectDir;

	if(Dlg.DoBrowse())
	{
		//change the edit text
		GetDlgItem(IDC_EDIT_RES_RENAME_OUTDIR)->SetWindowText(RemoveProjectPath(Dlg.m_strPath));
	}
}

//handle browsing for an output name
void CRenameResourceDlg::OnBrowseDestName()
{
	//get the name currently in the field as well as the current output directory
	CString sCurrName;
	GetDlgItem(IDC_EDIT_RES_RENAME_OUTNAME)->GetWindowText(sCurrName);
	CString sCurrDir;
	GetDlgItem(IDC_EDIT_RES_RENAME_OUTDIR)->GetWindowText(sCurrDir);

	//splice them together into one final filename
	CString sCurrFile = BuildFinalFile(sCurrDir, sCurrName);

	//extract the default extension
	int nDotPos = sCurrFile.ReverseFind('.');
	CString sDefExt = (nDotPos == -1) ? "" : "*." + sCurrFile.Mid(nDotPos);

	//load in the filter
	CString sFilter;
	sFilter.LoadString(IDS_RENAME_RESOURCE_FILTER);

	LPCTSTR pszCurrFile = sCurrFile.IsEmpty() ? NULL :(LPCTSTR) sCurrFile;

	CFileDialog	Dlg(	FALSE, 
						pszCurrFile,
						sCurrFile,
						OFN_PATHMUSTEXIST,
						sFilter);

	//now we let the user do their thing...
	if(Dlg.DoModal() == IDOK)
	{
		//they hit OK, lets now update the name and output directory field
		CString sFullFile = Dlg.GetPathName();

		//and now split that into the directory and file parts
		CString sFileName = ::ExtractFileName(sFullFile);

		//remove the file name from the full file to get the directory
		CString sFileDir = sFullFile.Left(sFullFile.GetLength() - sFileName.GetLength());

		//now remove the project path
		sFileDir = ::RemoveProjectPath(sFileDir);

		//now update the edit boxes appropriately
		GetDlgItem(IDC_EDIT_RES_RENAME_OUTDIR)->SetWindowText(sFileDir);
		GetDlgItem(IDC_EDIT_RES_RENAME_OUTNAME)->SetWindowText(sFileName);

	}
}



void CRenameResourceDlg::UpdateIcons()
{
	CClientDC	dcScreen(this);
	CDC			dcMem;
	CTexture	*pTexture;
	CBitmap		*pOldBitmap;

	BeginWaitCursor( ); // Put the wait cursor on the screen
	m_IconList.SetImageCount(0);  // Empty list out

	dcMem.CreateCompatibleDC(&dcScreen);
	SetStretchBltMode( dcMem.m_hDC, COLORONCOLOR );

	CDC dcPicture;
	dcPicture.CreateCompatibleDC(&dcMem);
	SetStretchBltMode( dcPicture.m_hDC, COLORONCOLOR );

	//clear out the old bitmap
	delete m_pIconBitmap;
	m_pIconBitmap = new CBitmap;

	if(m_pIconBitmap == NULL)
		return;

	//get the number of items in the list
	uint32 nNumItems = GetResList()->GetItemCount();

	//create the bitmap we will be using
	if (!m_pIconBitmap->CreateCompatibleBitmap(&dcScreen, IMAGE_SIZE * nNumItems, IMAGE_SIZE))  
		return;

	pOldBitmap = dcMem.SelectObject(m_pIconBitmap);


	//run through and add our images to it
	uint32 nCurrTex;
	for (nCurrTex = 0; nCurrTex < nNumItems; nCurrTex++)
	{
		//get the associated data
		CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nCurrTex);

		//see if this is a texture
		if(pInfo->m_nType & RESTYPE_TEXTURE)
		{
			
			//we have a texture, add it to the image list
			DFileIdent* pIdent;
			dfm_GetFileIdentifier(GetFileMgr(), pInfo->m_sSourceFile, &pIdent);
			
			pTexture = dib_GetDibTexture(pIdent);

			if (pTexture != NULL)  
				pTexture->m_pDib->Blt(dcMem.m_hDC, 0 + IMAGE_SIZE * nCurrTex, 0, IMAGE_SIZE, IMAGE_SIZE);
		}
		else if(pInfo->m_nType & RESTYPE_PREFAB)
		{
			//load in the bitmap filename
			CString sFile = GetPrefabBitmapName(BuildFullSourceFile(pInfo->m_sSourceFile));

			HBITMAP hPicture = (HBITMAP)LoadImage(0, sFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
			if (!hPicture)
				continue;

			CBitmap *pNewPicture = CBitmap::FromHandle(hPicture);
			if (!pNewPicture)
			{
				continue;
			}

			// Copy the picture into the image list
			CBitmap *pTempBMP = dcPicture.SelectObject(pNewPicture);
			dcMem.BitBlt(IMAGE_SIZE * nCurrTex, 0, IMAGE_SIZE, IMAGE_SIZE, &dcPicture, 0,0, SRCCOPY);
			dcPicture.SelectObject(pTempBMP);
		}
	}


	dcMem.SelectObject(pOldBitmap);

	m_IconList.Add(m_pIconBitmap, RGB(0,0,0));

	for (nCurrTex = 0; nCurrTex < nNumItems; nCurrTex++)
		GetResList()->SetItem( nCurrTex, 0, LVIF_IMAGE, NULL, nCurrTex, 0, 0, 0 );

	EndWaitCursor( ); // End the wait cursor
}

//handles actually changing the action on the selection
void CRenameResourceDlg::OnActionChanged(EResourceAction eAction)
{
	//run through the selection, change it, and update
	POSITION Pos = GetResList()->GetFirstSelectedItemPosition();

	while(Pos)
	{
		//found an item, update it
		int nSelItem = GetResList()->GetNextSelectedItem(Pos);

		//get the item data
		CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nSelItem);
		pInfo->m_eAction = eAction;

		//now update the item
		UpdateItem(nSelItem);
	}
}

//handles the action radio buttons
afx_msg void CRenameResourceDlg::OnRadioActionMove()
{
	OnActionChanged(eResAction_Move);
}

afx_msg void CRenameResourceDlg::OnRadioActionReplace()
{
	OnActionChanged(eResAction_Replace);
}






//---------------------------------------------------------------------------------------
// Updating the level
//
// Functions for after OK has been pressed and the levels need to be updated
//
//---------------------------------------------------------------------------------------

//updates the project bar to reflect changes. 
bool CRenameResourceDlg::UpdateProjectBar()
{
	//clear out the selected texture if it was renamed
	char pszTextureName[MAX_PATH];
	GetProjectBar()->GetCurrentTextureName( pszTextureName, TRUE );

	//valid, now see if we have a match
	uint32 nNumRes = GetResList()->GetItemCount();

	for(uint32 nCurrRes = 0; nCurrRes < nNumRes; nCurrRes++)
	{
		//get the associated data
		CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nCurrRes);

		if(stricmp(pszTextureName, RemoveProjectPath(pInfo->m_sSourceFile)) == 0)
		{
			//we have a match...clear out the selected texture
			GetProjectBar()->SetCurTextureSel(NULL);

			//don't bother looking any more...
			break;
		}
	}

	//update the property pages so they will reflect the new names
	GetProjectBar()->UpdateAll();

	//success
	return true;
}

//this will recursively go through the specified directory and subdirectories
//looking for LTA files, and for each one it will call UpdateLTA
// If open levels are to be updated, any levels upen in the editor will be modified
// If closed files are to be updated, any LTA that is not open will be modified
//returns the number of levels that were modified
uint32 CRenameResourceDlg::UpdateAllLevels(const char* pszDir, bool bUpdateClosedLevels)
{
	//count of how many files have been modified
	uint32 nNumModified = 0;

	//see if we are updating only the open levels
	if(!bUpdateClosedLevels)
	{
		//we can just run through the document list and update
		POSITION Pos;
		for( Pos = GetApp()->m_pWorldTemplate->GetFirstDocPosition(); Pos; )
		{
			//get the document
			CRegionDoc* pDoc = (CRegionDoc*)GetApp()->m_pWorldTemplate->GetNextDoc( Pos );
			
			if(UpdateOpenLevel(pDoc))
			{
				nNumModified++;
			}
		}

		//success
		return nNumModified;
	}

	//get all the levels
	CMoArray<CString> LevelList;

	CFileUtils::GetAllWorldFiles(pszDir, LevelList);

	//now run through and update them

	for(uint32 nCurrLevel = 0; nCurrLevel < LevelList.GetSize(); nCurrLevel++)
	{
		//see if this file is a level
		CString sFile = LevelList[nCurrLevel];

		bool bUpdateLTA = bUpdateClosedLevels;

		//now look in all the open documents and see if any of them
		//are the level we are looking to update
		POSITION Pos;
		for( Pos = GetApp()->m_pWorldTemplate->GetFirstDocPosition(); Pos; )
		{
			//get the document
			CRegionDoc* pDoc = (CRegionDoc*)GetApp()->m_pWorldTemplate->GetNextDoc( Pos );
			
			//see if this is the level we are trying to modify
			if(pDoc->GetPathName().CompareNoCase(sFile) == 0)
			{
				//this file is open, so we need to update the open level
				//and not the LTA file
				if(UpdateOpenLevel(pDoc))
				{
					nNumModified++;
				}

				//we don't want to updat the actual LTA file
				bUpdateLTA = false;
			}
		}

		//this is an LTA file that needs to be converted
		if(bUpdateLTA)
		{
			if(UpdateLTA(sFile))
			{
				nNumModified++;
			}
		}
	}

	return nNumModified;
}

//given an LTA it will open it up, and if it is a world, will modify the changes
//and save it back out to the same file. Returns true if the file was modified
bool CRenameResourceDlg::UpdateLTA(const char* pszFile)
{
	//first off, we need to open up the level, but don't update properties
	CRegionDoc* pLevel = GetProjectBar()->OpenRegionDoc(pszFile, false, false);

	//now we need to update the level
	bool bModified = UpdateOpenLevel(pLevel);

	//now that it has been modified, we need to save it
	if(bModified)
	{
		if(!pLevel->SaveLTA(true))
		{
			CString sMsg;
			sMsg.Format("Unable to save file %s. Please make sure that it is checked out and writable", pszFile);
			
			if(MessageBox(sMsg, "Error", MB_RETRYCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
			{
				return false;
			}
		}
	}

	//and finally, we can close it		
	GetApp()->m_pWorldTemplate->RemoveDocument( pLevel );
	pLevel->OnCloseDocument();

	return bModified;
}

//handles the actual moving of the resources. This will take all items in the
//list, move them from the source to the destination
void CRenameResourceDlg::MoveResources()
{
	//cache the number of items
	uint32 nNumRes = GetResList()->GetItemCount();

	//determine if the user wants the output files to be writable
	bool bMakeWritable = !!((CButton*)GetDlgItem(IDC_CHECK_MAKE_TARGETS_WRITABLE))->GetCheck();

	for(uint32 nCurrRes = 0; nCurrRes < nNumRes; nCurrRes++)
	{
		//get the associated data
		CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nCurrRes);

		//however, if we are only doing a replace, we don't need to actually move the resources
		if(pInfo->m_eAction == eResAction_Replace)
			continue;

		CString sOutFile = BuildFinalFile(pInfo->m_sDestDir, pInfo->m_sDestName);

		MoveFile(BuildFullSourceFile(pInfo->m_sSourceFile), sOutFile);

		//make it writable if applicable
		if(bMakeWritable)
		{
			//get the existing status
			CFileStatus Status;
			CFile::GetStatus(sOutFile, Status);

			//now remove read only
			Status.m_attribute &= ~CFile::readOnly;

			//set the new, writable status
			CFile::SetStatus(sOutFile, Status);			
		}

		//if this is a prefab, we should also try to move the bitmap
		if(pInfo->m_nType & RESTYPE_PREFAB)
		{
			CString sBitmapFile = GetPrefabBitmapName(BuildFullSourceFile(pInfo->m_sSourceFile));

			//see if it has a bitmap
			if(CFileUtils::DoesFileExist(sBitmapFile))
			{
				MoveFile(sBitmapFile, GetPrefabBitmapName(sOutFile));
			}
		}
	}
}

//given a root node of a level it will update the textures to reflect the changes
bool CRenameResourceDlg::UpdateOpenWorldTextures(CRegionDoc* pDoc)
{
	uint32 nNumChanged = 0;

	//cache the number of items
	uint32 nNumRes = GetResList()->GetItemCount();

	//determine if the user wants the UV's to be scaled on textures that change dimensions
	bool bScaleUVs = !!((CButton*)GetDlgItem(IDC_CHECK_SCALE_TEXTURE_UVS))->GetCheck();

	//valid, now see if we have a match
	for(uint32 nCurrRes = 0; nCurrRes < nNumRes; nCurrRes++)
	{
		//get the associated data
		CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nCurrRes);

		nNumChanged += pDoc->GetRegion()->RenameTexture(pInfo->m_sSourceFile, 
														BuildFinalFileRel(pInfo->m_sDestDir, pInfo->m_sDestName), 
														bScaleUVs && (pInfo->m_eAction == eResAction_Replace));
	}

	return (nNumChanged > 0);
}

//recursively changes the matching texture names of the patch textures. Returns the number
//of textures changed
static uint32 RecusivelyReplacePrefabs(CWorldNode* pNode, const char* pszInName, const char* pszOutName)
{
	//count of how many have changed
	uint32 nNumChanged = 0;

	//if this node is a patch, update it
	if(pNode->GetType() == Node_PrefabRef)
	{
		CPrefabRef* pPrefab = (CPrefabRef*)pNode;

		//see if the texture matches
		if(CHelpers::UpperStrcmp(pPrefab->GetPrefabFilename(), pszInName))
		{
			nNumChanged++;

			pPrefab->SetPrefabFilename(pszOutName);
		}
	}

	//now recurse on all its children
	GPOS Pos;
	for( Pos = pNode->m_Children; Pos; )
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);
		nNumChanged += RecusivelyReplacePrefabs(pChild, pszInName, pszOutName);
	}

	return nNumChanged;
}



//given a level it will update the prefabs to reflect the changes in resource names
bool CRenameResourceDlg::UpdateOpenWorldPrefabs(CRegionDoc* pDoc)
{
	uint32 nNumChanged = 0;
	
	//cache the number of items
	uint32 nNumRes = GetResList()->GetItemCount();

	//valid, now see if we have a match
	for(uint32 nCurrRes = 0; nCurrRes < nNumRes; nCurrRes++)
	{
		//get the associated data
		CResourceInfo* pInfo = (CResourceInfo*)GetResList()->GetItemData(nCurrRes);

		nNumChanged += RecusivelyReplacePrefabs(pDoc->GetRegion()->GetRootNode(), pInfo->m_sSourceFile, BuildFinalFileRel(pInfo->m_sDestDir, pInfo->m_sDestName));
	}

	return (nNumChanged > 0);
}

//given the document of a currently open world, it will update it to reflect the new
//changes. Returns if it was modified or not
bool CRenameResourceDlg::UpdateOpenLevel(CRegionDoc* pDoc)
{
	bool bModified = false;

	bModified = bModified || UpdateOpenWorldTextures(pDoc);
	bModified = bModified || UpdateOpenWorldPrefabs(pDoc);

	//if it was modified, set the document's modified flag
	if(bModified)
	{
		pDoc->SetModifiedFlag(TRUE);
		pDoc->SetTitle(TRUE);
	}

	return bModified;
}
