// PrefabDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "prefabdlg.h"
#include "editprojectmgr.h"
#include "edithelpers.h"
#include "mainfrm.h"
#include "projectbar.h"
#include "regionview.h"
#include "RenameResourceDlg.h"
#include "fileutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// File utilities
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


/////////////////////////////////////////////////////////////////////////////
// CPrefabDlg dialog


CPrefabDlg::CPrefabDlg() : 
	CBaseImgDlg()
{
	//{{AFX_DATA_INIT(CPrefabDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_hCurrentItem = NULL;
	InitBaseRezDlg("lta;ltc;tbw", &m_PrefabTree, &m_PrefabList, RESTYPE_PREFAB);
}

CPrefabDlg::~CPrefabDlg()
{
}

void CPrefabDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseRezDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefabDlg)
	DDX_Control(pDX, IDC_BASEREZ_TREE, m_PrefabTree);
	DDX_Control(pDX, IDC_BASEREZ_LIST, m_PrefabList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefabDlg, CBaseImgDlg)
	//{{AFX_MSG_MAP(CPrefabDlg)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_BASEREZ_TREE, OnSelchangedDirectory)
	ON_NOTIFY(NM_DBLCLK, IDC_BASEREZ_LIST, OnDblclkPrefabList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_BASEREZ_LIST, OnListSelChanged)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefabDlg message handlers

void CPrefabDlg::OnSize(UINT nType, int cx, int cy) 
{
	CMRCSizeDialogBar::OnSize(nType, cx, cy);
	
	// Reposition the controls
	RepositionControls();
}

BOOL CPrefabDlg::OnInitDialogBar() 
{
	CMRCSizeDialogBar::OnInitDialogBar();

	CBaseImgDlg::InitBaseImgDlg((CListCtrl*)GetDlgItem(IDC_BASEREZ_LIST), (CTreeCtrl*)GetDlgItem(IDC_BASEREZ_TREE), IDI_WORLDS_TAB_ICON);

	CRect rect;
	m_PrefabList.GetClientRect( &rect );

	// Clear the world list
	m_PrefabList.DeleteAllItems();

	while (m_PrefabList.DeleteColumn(0))
	{
	}
	
	m_PrefabList.InsertColumn(0,"Name",LVCFMT_LEFT,rect.Width(),-1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrefabDlg::OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult) 
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

void CPrefabDlg::OnDblclkPrefabList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int iItem = m_PrefabList.GetNextItem( -1, LVNI_SELECTED );
	DFileIdent *pIdent;
	
	if(iItem != -1)
	{
		pIdent = (DFileIdent*)m_PrefabList.GetItemData(iItem);
		if(pIdent)
		{
			// Get the document
			CRegionDoc *pDoc=GetActiveRegionDoc();
			if (pDoc)
			{
				CRegionDoc *pDoc=GetActiveRegionDoc();
				if (!pDoc)
					return;

				// Get the edit region for convenience
				CEditRegion *pRegion = pDoc->GetRegion();

				// Get the region view
				POSITION pos = pDoc->GetFirstViewPosition();
				CRegionView* pView = (CRegionView *)pDoc->GetNextView( pos );
				if(!pView)
				{
					ASSERT(FALSE);
					return;
				}

				// Set up the name
				char aNameBuff[256];
				m_PrefabList.GetItemText(iItem, 0, aNameBuff, sizeof(aNameBuff));
				// Find one that isn't already in use
				char aTempName[256];
				int nNameOffset = 0;
				do
				{
					sprintf(aTempName, "%s%02d", aNameBuff, nNameOffset);
					++nNameOffset;
				} while ((pRegion->FindNodeByName(aTempName) != LTNULL) && (nNameOffset < 100));
				strcpy(aNameBuff, aTempName);

				// Create the prefab ref
				CPrefabRef *pPrefabRef = pRegion->GetPrefabMgr()->CreateRef(pRegion, pRegion->GetActiveParentNode(), pIdent->m_Filename, aNameBuff);
				if (!pPrefabRef)
				{
					ASSERT(!"Unable to create prefab reference object from filename");
					return;
				}

				// Select it for convenience
				pDoc->SelectNode(pPrefabRef);

				// Move it to where the crosshairs are
				pPrefabRef->SetPos(pRegion->GetMarker());
				
				// Create an undo list
				PreActionList actionList;
				actionList.AddTail(new CPreAction(ACTION_ADDEDNODE, pPrefabRef));
				pDoc->Modify(&actionList, TRUE);

				pDoc->NotifySelectionChange();
			}
		}
	}	

	return;
}

void CPrefabDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu menu;
	
	if(!GetProjectBar()->IsProjectOpen())
		return;

	if(pWnd->m_hWnd == m_PrefabTree.m_hWnd)
	{
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_WORLDTREE));
	}
	else if(pWnd->m_hWnd == m_PrefabList.m_hWnd)
	{
		// Nothing active unless a directory is selected.
		if(!IsDirectorySelected())
			return;

		VERIFY(menu.LoadMenu(CG_IDR_POPUP_PREFAB));
	}
	else return;

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	CWnd* pWndPopupOwner = GetProjectBar();

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pWndPopupOwner);	
}

void CPrefabDlg::OnPrefabOpen() 
{
	int iItem = m_PrefabList.GetNextItem( -1, LVNI_SELECTED );

	if(iItem != -1)
	{
		DFileIdent *pIdent = (DFileIdent*)m_PrefabList.GetItemData(iItem);
		if(pIdent)
		{
			CString fullName = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
			GetProjectBar()->OpenRegionDoc(fullName);
		}
	}		
}

void CPrefabDlg::OnReplaceSelectedPrefabs()
{
	int nItem = m_PrefabList.GetNextItem( -1, LVNI_SELECTED );

	if(nItem == -1)
	{
		//no item selected
		return;
	}

	//get the filename of this prefab
	CString sFile = ((DFileIdent*)m_PrefabList.GetItemData(nItem))->m_Filename;

	//also grab just the textual name
	CString sTextName = m_PrefabList.GetItemText(nItem, 0);
	

	//run through the active document and replace any selected prefabs
	CRegionDoc* pDoc = ::GetActiveRegionDoc();

	//safety check
	if(pDoc == NULL)
		return;

	
	uint32 nCurrSel;

	//we need to first create an undo for all the selected prefabs
	PreActionList actionList;
	for(nCurrSel = 0; nCurrSel < pDoc->GetRegion()->m_Selections.GetSize(); nCurrSel++)
	{
		CWorldNode* pNode = pDoc->GetRegion()->m_Selections[nCurrSel];

		//see if this is a prefab
		if(pNode->GetType() != Node_PrefabRef)
			continue;

		actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pNode));
	}
	pDoc->Modify(&actionList, TRUE);


	//now run through the selections
	for(nCurrSel = 0; nCurrSel < pDoc->GetRegion()->m_Selections.GetSize(); nCurrSel++)
	{
		CWorldNode* pNode = pDoc->GetRegion()->m_Selections[nCurrSel];

		//see if this is a prefab
		if(pNode->GetType() != Node_PrefabRef)
			continue;

		//it is a prefab, so lets change its type
		CPrefabRef* pPrefab = (CPrefabRef*)pNode;

		//set the filename
		pPrefab->SetPrefabFilename(sFile);

		//clear out the binding
		pPrefab->SetPrefabTree(NULL);

		//now bind this new prefab
		pDoc->GetRegion()->GetPrefabMgr()->BindRef(pPrefab);

		//finally we need to update the name to reflect the new prefab. This avoids
		//confusion...
		// Find one that isn't already in use
		char pszTempName[256];
		uint32 nNameOffset = 0;
		do
		{
			sprintf(pszTempName, "%s%02d", sTextName, nNameOffset);
			++nNameOffset;
		} 
		while ((pDoc->GetRegion()->FindNodeByName(pszTempName) != LTNULL) && (nNameOffset < 100));

		//we have a unique name, rename that node
		pPrefab->SetName(pszTempName);

		//need to update the node view as well
		GetNodeView()->UpdateTreeItemLabel(pPrefab);
		
	}

	//also need to update the selection box in case the size of the prefabs is different
	pDoc->UpdateSelectionBox();

	//redraw
	pDoc->RedrawAllViews();
}

void CPrefabDlg::OnRenamePrefab()
{

	//add all selected prefabs

	int nItem = m_PrefabList.GetNextItem( -1, LVNI_SELECTED );
	uint32 nNumAdded = 0;

	while(nItem != -1)
	{
		nNumAdded++;

		//get the filename of this prefab
		CString sFile = ((DFileIdent*)m_PrefabList.GetItemData(nItem))->m_Filename;


		//setup the initial list
		GetRenameResourceDlg()->AddNewFile(sFile);

		nItem = m_PrefabList.GetNextItem( nItem, LVNI_SELECTED );
	}

	if(nNumAdded > 0)
	{
		//now make sure it is visible
		GetRenameResourceDlg()->UpdateIcons();
		GetRenameResourceDlg()->ShowWindow(SW_SHOW);
	}
}


void CPrefabDlg::PopulateList()
{
	CBaseRezDlg::PopulateList();

	if(GetProjectBar()->m_bShowThumbnails)
		UpdateThumbnails();
}

bool CPrefabDlg::RecursivelySelectPrefabDir(HTREEITEM hParent, const char* pszNextDir)
{
	//determine what our string is
	CString sPath(pszNextDir);

	//try and get the trailing slash
	int nSlashPos = sPath.Find('\\');

	CString sFilename;

	if(nSlashPos == -1)
	{
		//couldn't find the slash, see if we have any text at all
		if(sPath.GetLength() == 0)
			return true;

		sFilename = sPath;
		sPath = "";
	}
	else
	{
		sFilename = sPath.Left(nSlashPos);
		sPath = sPath.Mid(nSlashPos + 1);
	}

	//alright, now we can run through the list of items and try and find our child node
	HTREEITEM hMatch = NULL;

	//see if this item is a match
	if(m_PrefabTree.GetItemText(hParent).CompareNoCase(sFilename) == 0)
	{
		//it is a match
		hMatch = hParent;
	}
	else
	{
		HTREEITEM hCurrSib = hParent;

		//we need to run through all the siblings
		while((hCurrSib = m_PrefabTree.GetNextSiblingItem(hCurrSib)) != NULL)
		{
			//see if this item matches
			if(m_PrefabTree.GetItemText(hCurrSib).CompareNoCase(sFilename) == 0)
			{
				//it does
				hMatch = hCurrSib;
				break;
			}
		}
	}

	//see if we found a match
	if(!hMatch)
	{
		//we failed
		return false;
	}

	//select the item if we have no more to possibly select
	if(sPath.IsEmpty())
	{
		m_PrefabTree.SelectItem(hMatch);
		return true;
	}

	HTREEITEM hChild = m_PrefabTree.GetChildItem(hMatch);

	if(hChild)
	{
		return RecursivelySelectPrefabDir(hChild, sPath);
	}
	else
	{
		//no chilren to recurse into, but still have paths left
		return false;
	}
}


bool CPrefabDlg::SelectPrefab(const char* pszPrefabName)
{
	//we must first extract the filename
	CString sPrefabName(pszPrefabName);
	CFileUtils::EnsureValidFileName(sPrefabName);

	CString sPathname = ExtractFilePath(sPrefabName);

	//now run through our directory tree, and select the appropriate directory
	if(!RecursivelySelectPrefabDir(m_PrefabTree.GetRootItem(), sPathname))
	{
		return false;
	}

	CString sFilename = ExtractFileName(sPrefabName);

	//strip off the file extension
	int nDotPos = sFilename.ReverseFind('.');
	if(nDotPos != -1)
		sFilename = sFilename.Left(nDotPos);

	//we now need to run through this list and select the actual prefab
	for(uint32 nCurrItem = 0; nCurrItem < m_PrefabList.GetItemCount(); nCurrItem++)
	{
		if(m_PrefabList.GetItemText(nCurrItem, 0).CompareNoCase(sFilename) == 0)
		{
			m_PrefabList.SetSelectionMark(nCurrItem);
			m_PrefabList.SetItemState(nCurrItem, LVIS_SELECTED, LVIS_SELECTED);
			break;
		}
	}

	//success
	return true;
}

//this must be overridden by a derived class to render the icon for the appropriate
//list item
bool CPrefabDlg::RenderIcon(HDC BlitTo, uint32 nXOff, uint32 nImgSize, uint32 nItem)
{
	DFileIdent *pIdent = (DFileIdent*)m_PrefabList.GetItemData(nItem);
	if (!pIdent)
		return false;

	CString fullName = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
	int nExtInFilename = fullName.ReverseFind('.');
	if (nExtInFilename < 0)
		return false;

	fullName = fullName.Left(nExtInFilename + 1) + "BMP";
	HBITMAP hPicture = (HBITMAP)LoadImage(0, fullName, IMAGE_BITMAP, nImgSize, nImgSize, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (!hPicture)
		return false;

	CBitmap *pNewPicture = CBitmap::FromHandle(hPicture);
	if (!pNewPicture)
	{
		return false;
	}

	CDC dcPicture;
	dcPicture.CreateCompatibleDC(CDC::FromHandle(BlitTo));
	SetStretchBltMode( dcPicture.m_hDC, COLORONCOLOR );

	// Copy the picture into the image list
	CBitmap *pTempBMP = dcPicture.SelectObject(pNewPicture);
	BitBlt(BlitTo, nXOff, 0, nImgSize, nImgSize, dcPicture.m_hDC, 0,0, SRCCOPY);
	dcPicture.SelectObject(pTempBMP);

	DeleteObject(hPicture);

	return true;
}

//this must be overridden by a derived class to render the large selected image
void CPrefabDlg::RenderLargeImage()
{
	//setup the rectangle we are going to blit to
	CRect InitialRect = CBaseImgDlg::InitLargeImageRect();
	CRect DrawRect = InitialRect;

	//get a DC
	CDC *pDC = GetDC();
	if(!pDC)
		return;

	HICON hDefaultIcon = AfxGetApp()->LoadIcon(IDI_WORLDS_TAB_ICON);

	POSITION Pos = m_PrefabList.GetFirstSelectedItemPosition();
	if(!Pos)
		return;

	int nItem = m_PrefabList.GetNextSelectedItem(Pos);

	DFileIdent *pIdent = (DFileIdent*)m_PrefabList.GetItemData(nItem);
	if (!pIdent)
		return;

	CString fullName = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
	int nExtInFilename = fullName.ReverseFind('.');
	if (nExtInFilename < 0)
		return;

	int nImgWid = DrawRect.Width() * 2 / 3;
	int nImgHgt = DrawRect.Height() * 2 / 3;

	nImgWid = (nImgWid < nImgHgt) ? nImgWid : nImgHgt;
	nImgHgt = (nImgWid < nImgHgt) ? nImgWid : nImgHgt;

	//bail if it is too small
	if(nImgWid < 4)
		return;


	fullName = fullName.Left(nExtInFilename + 1) + "BMP";
	HBITMAP hPicture = (HBITMAP)LoadImage(0, fullName, IMAGE_BITMAP, nImgWid, nImgHgt, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (!hPicture)
		return;

	CBitmap *pNewPicture = CBitmap::FromHandle(hPicture);
	if (!pNewPicture)
	{
		return;
	}

	CDC dcPicture;
	dcPicture.CreateCompatibleDC(pDC);
	SetStretchBltMode( dcPicture.m_hDC, COLORONCOLOR );

	//adjust the origin of the texture
	int nTmp = (DrawRect.top + (DrawRect.bottom/6)) - (nImgHgt / 2);
	if(nTmp > DrawRect.top)
		DrawRect.top = nTmp;

	DrawRect.left = (DrawRect.right/2) - (nImgWid/2);
	
	nTmp = DrawRect.top + nImgHgt;
	if(nTmp < DrawRect.bottom)
		DrawRect.bottom = nTmp;

	DrawRect.right = DrawRect.left + nImgWid;

	// Copy the picture into the screen list
	CBitmap *pTempBMP = dcPicture.SelectObject(pNewPicture);
	SetStretchBltMode( pDC->m_hDC, COLORONCOLOR );
	BitBlt(pDC->m_hDC, DrawRect.left, DrawRect.top, DrawRect.Width()+1, DrawRect.Height()+1, dcPicture.m_hDC, 0,0, SRCCOPY);
	dcPicture.SelectObject(pTempBMP);

	DeleteObject(hPicture);


	// Draw the texture name.
	char pszToPrint[MAX_PATH];
	CHelpers::ExtractNames( fullName, NULL, NULL, pszToPrint, NULL );

	CBaseImgDlg::PrintLargeImageText(pDC->m_hDC, InitialRect, pszToPrint);

	ReleaseDC(pDC);
}

void CPrefabDlg::OnListSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	RenderLargeImage();
}

/************************************************************************/
// This is called to reposition the controls
void CPrefabDlg::RepositionControls()
{
	if( ::IsWindow(m_hWnd))
	{
		CBaseImgDlg::RepositionControls();

		// Adjust columns
		if (m_PrefabList)
		{
			CRect rect;
			m_PrefabList.GetClientRect( &rect );

			m_PrefabList.SetColumnWidth(0, rect.Width());
		}
	}		
}