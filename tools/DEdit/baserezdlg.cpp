//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#include "bdefs.h"
#include "baserezdlg.h"
#include "d_filemgr.h"
#include "edithelpers.h"


IMPLEMENT_DYNAMIC(CBaseRezDlg, CMRCSizeDialogBar);


CBaseRezDlg::CBaseRezDlg()	
{
	m_pTree = NULL;
	m_pList = NULL;
	m_hCurrentItem = NULL;
	m_ResourceType = RESTYPE_PROJECT;
}


void CBaseRezDlg::InitBaseRezDlg(CString ext, CTreeCtrl *pTree, CListCtrl *pList, resource_type resType)
{
	m_Extension = ext;
	m_pTree = pTree;
	m_pList = pList;
	m_ResourceType = resType;
}
	

BOOL CBaseRezDlg::RecurseAndAddDirs(HTREEITEM hParent)
{
	DDirIdent *pIdent, *pParentIdent;
	CString searchSpec, dirName;
	HTREEITEM hItem;
	CFileIterator iterator;
	BOOL bRet, bIsType, bChildrenIsType;

	DFILEMGR pFileMgr=GetFileMgr();
	if (!pFileMgr)
	{
		return FALSE;
	}

	if(hParent)
	{
		pParentIdent = (DDirIdent*)m_pTree->GetItemData(hParent);
	}
	else
	{
		dfm_GetDirIdentifier(pFileMgr, "", &pParentIdent);
	}

	ASSERT(pParentIdent);
	if(!pParentIdent)
		return FALSE;

	// Add and recurse into all directories that match our resource type.
	bRet = FALSE;
	searchSpec = dfm_BuildName(pParentIdent->m_Filename, "*.*");
	while(iterator.Next(searchSpec, TRUE))
	{
		if(iterator.GetAttributes() & FILE_ATTRIBUTE_DIRECTORY)
		{
			dirName = dfm_BuildName(pParentIdent->m_Filename, iterator.GetFilename());
			
			//if(CResourceMgr::GetDirType(dirName) == m_ResourceType)
			bIsType = CResourceMgr::IsDirType(dirName, m_ResourceType);
			bRet |= bIsType;

			dfm_GetDirIdentifier(GetFileMgr(), dirName, &pIdent);
			if(pIdent)
			{
				hItem = m_pTree->InsertItem(iterator.GetFilename(), 0, 1, hParent, TVI_SORT);
				if(hItem)
				{
					m_pTree->SetItemData(hItem, (DWORD)pIdent);
					bChildrenIsType = RecurseAndAddDirs(hItem);
					bRet |= bChildrenIsType;

					if(!bIsType && !bChildrenIsType)
					{
						m_pTree->DeleteItem(hItem);
					}
				}
			}
		}
	}

	return bRet;
}


void CBaseRezDlg::UpdateDirectories()
{
	DDirIdent *pIdent;

	if(!m_pTree || !m_pList)
		return;

	BeginWaitCursor( );
	ClearAll();

	RecurseAndAddDirs(NULL);

	EndWaitCursor();
}


void CBaseRezDlg::SetListItemText(int nItem, CString &relativeFilename)
{
	CString fullFilename;
	WIN32_FIND_DATA findData;
	HANDLE handle;
	char szTemp[MAX_PATH];
	SYSTEMTIME systime;
	DWORD dwSize;


	fullFilename = dfm_GetFullFilename(GetFileMgr(), relativeFilename);


	// Get the FindData info.
	handle = FindFirstFile(fullFilename, &findData);
	if(handle == INVALID_HANDLE_VALUE)
		return;

	// insert the file size
	dwSize = (findData.nFileSizeHigh * MAXDWORD) + findData.nFileSizeLow;
	if(dwSize < 1024)
	{
		m_pList->SetItemText(nItem, 1, "1KB");
	}
	else
	{
		sprintf(szTemp,"%dKB",dwSize/1024);
		m_pList->SetItemText(nItem, 1, szTemp);
	}

	//insert the last modified date
	FileTimeToSystemTime(&findData.ftLastWriteTime, &systime);
	sprintf(szTemp, "%d/%d/%d", systime.wMonth, systime.wDay, systime.wYear);
	m_pList->SetItemText(nItem, 2, szTemp);

	FindClose(handle);
}


int CBaseRezDlg::AddFileToList(CString relativeFilename)
{
	DFileIdent *pFileIdent;
	int nIndex;
	char fileTitle[MAX_PATH];


	CHelpers::ExtractNames(relativeFilename, NULL, NULL, fileTitle, NULL);
	
	dfm_GetFileIdentifier(GetFileMgr(), relativeFilename, &pFileIdent);
	if(!pFileIdent)
	{
		return -1;
	}

	// insert the file name
	nIndex = m_pList->InsertItem(0, fileTitle, 0);
	if(nIndex == -1)
	{
		return -1;
	}

	m_pList->SetItemData(nIndex, (DWORD)pFileIdent);
	SetListItemText(nIndex, relativeFilename);

	return nIndex;
}

int CBaseRezDlg::FindFileInList( DFileIdent *pFileIdent )
{
	DFileIdent *pIterator;
	int nIndex, nNext;
	char fileTitle[MAX_PATH];

	if(!pFileIdent)
	{
		return -1;
	}

	nIndex = -1;
	while(( nIndex = m_pList->GetNextItem( nIndex, LVNI_ALL )) != -1 )
	{
		pIterator = ( DFileIdent * )m_pList->GetItemData( nIndex );
		if( pIterator == pFileIdent )
			break;
	}

	return nIndex;
}

BOOL CBaseRezDlg::SelectFileInList( DFileIdent *pFileIdent, BOOL bSelect )
{
	int item;

	item = FindFileInList( pFileIdent );
	if( item != -1 )
	{
		LV_ITEM lvItem;
		lvItem.mask = LVIF_STATE;
		lvItem.iItem = item;

		m_pList->GetItem( &lvItem );
		if( bSelect )
			lvItem.state |= LVIF_STATE;
		else
			lvItem.state &= ~LVIF_STATE;
		m_pList->SetItem( item, 0, LVIF_STATE, NULL, 0, lvItem.state, 0, 0 );

		return TRUE;
	}

	return FALSE;
}

#define MAX_EXTENSIONS			16
#define EXTENSION_SEPARATOR		';'

void CBaseRezDlg::PopulateList()
{
	CFileIterator iterator;
	CString searchSpec, relativeFilename;
	HANDLE hFile;
	char fileName[MAX_PATH];
	char ext[32];

	ListView_SetExtendedListViewStyle(m_pList->m_hWnd, LVS_EX_FULLROWSELECT);

	//build up the list of possible extensions
	CString sExtensions[MAX_EXTENSIONS];
	uint32 nNumExtensions = 0;

	//preserve the original
	CString sOrigExt = m_Extension;

	//split the string apart based upon the separator
	while(!sOrigExt.IsEmpty())
	{
		int32 nSeparatorPos = sOrigExt.Find(EXTENSION_SEPARATOR);

		//sanity check, so if we ever go past the limit, we will know to change the
		//code to reflect it
		if(nNumExtensions >= MAX_EXTENSIONS)
		{
			//too many extensions, increase MAX_EXTENSIONS
			ASSERT(false);
		}

		if(nSeparatorPos >= 0)
		{
			//we hav a separator, break that chunk off
			sExtensions[nNumExtensions] = sOrigExt.Left(nSeparatorPos);
			nNumExtensions++;			
			sOrigExt = sOrigExt.Mid(nSeparatorPos + 1);
		}
		else
		{
			//no more separators, the rest of the string is the extension
			sExtensions[nNumExtensions] = sOrigExt;
			nNumExtensions++;
			sOrigExt = "";
		}
	}

	
	if(!m_pTree || !m_pList)
		return;

	BeginWaitCursor();
	m_pList->DeleteAllItems();
	
	searchSpec = dfm_BuildName(m_csCurrentDir, "*.*");
	while(iterator.Next(searchSpec, TRUE))
	{
		if(!(iterator.m_Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			CHelpers::ExtractFileNameAndExtension(iterator.GetFilename(), fileName, ext);

			//now run through all the possible file extensions
			for(uint32 nCurrExt = 0; nCurrExt < nNumExtensions; nCurrExt++)
			{
				if(stricmp(ext, (LPCTSTR)sExtensions[nCurrExt]) == 0)
				{
					//found a matching extension, add it to the tree
					relativeFilename = dfm_BuildName(m_csCurrentDir, iterator.GetFilename());
					AddFileToList(relativeFilename);
					break;
				}
			}
		}
	}

	RepositionControls(); // Make sure columns in the list view align properly

	EndWaitCursor();
}


DDirIdent* CBaseRezDlg::GetFirstDirectory()
{
	HTREEITEM hChild;

	if(m_pTree)
	{
		hChild = m_pTree->GetChildItem(NULL);
		if(hChild)
		{
			return (DDirIdent*)m_pTree->GetItemData(hChild);
		}
	}

	return NULL;
}


DDirIdent* CBaseRezDlg::GetSelectedDirectory()
{
	if(m_hCurrentItem && m_pTree)
	{
		return (DDirIdent*)m_pTree->GetItemData(m_hCurrentItem);
	}
	else
	{
		return NULL;
	}
}


void RecurseAndNullItems(CTreeCtrl *pTree, HTREEITEM hRoot)
{
	HTREEITEM hCur;

	if(!hRoot)
		return;

	// Recurse into the children of all the siblings.
	hCur = hRoot;
	do
	{
		pTree->SetItemData(hCur, 0);
		RecurseAndNullItems(pTree, pTree->GetChildItem(hCur));
	} while(hCur = pTree->GetNextSiblingItem(hCur));
}


void CBaseRezDlg::ClearAll()
{
	if(m_pTree && m_pTree->GetSafeHwnd())
	{
		// It nulls out all the item data for each item in the tree here
		// so when they get TVN_SELCHANGED messages, they don't repopulate the lists.
		RecurseAndNullItems(m_pTree, m_pTree->GetRootItem());
		m_pTree->DeleteAllItems();
	}

	if(m_pList)
	{
		m_pList->DeleteAllItems();
	}

	m_hCurrentItem = NULL;
	m_csCurrentDir = "";
}


/************************************************************************/
// This is called when the window is docked
void CBaseRezDlg::OnSizedOrDocked(int cx, int cy, BOOL bFloating, int flags)
{
	// Reposition the controls
	RepositionControls();
}

/************************************************************************/
// This is called to reposition the controls
void CBaseRezDlg::RepositionControls()
{
	if( ::IsWindow(m_hWnd))
	{
		CRect rect;
		GetClientRect( &rect );

		// Move the tree
		CWnd *pTree=GetDlgItem(IDC_BASEREZ_TREE);
		if (pTree)
		{
			pTree->MoveWindow( 0, 0, rect.Width(), rect.Height()/2 );
		}

		// Move the list
		CWnd *pList=GetDlgItem(IDC_BASEREZ_LIST);
		if (pList)
		{
			pList->MoveWindow( 0, rect.Height()/2, rect.Width(), rect.Height() / 2);
		}	
		
		// Adjust columns in list control
		if (m_pList && m_pList->m_hWnd)
		{
			m_pList->GetClientRect( &rect );
			CHeaderCtrl* pHeader = m_pList->GetHeaderCtrl();
			int32 tabCount = pHeader->GetItemCount();
			ASSERT(tabCount >= 1);

			for (uint32 i=0; i < tabCount; i++)
				m_pList->SetColumnWidth(i, rect.Width()/tabCount);
		}
	}		
}