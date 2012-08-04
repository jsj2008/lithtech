//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// AllTextureDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "alltexturedlg.h"
#include "projectbar.h"
#include "stringdlg.h"
#include "texture.h"
#include "renameresourcedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAllTextureDlg dialog


CAllTextureDlg::CAllTextureDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAllTextureDlg::IDD, pParent),
	m_cPalette(IDS_TEXTUREPALETTE_FILE_HEADER),
	m_csPaletteFileName(""),
	m_bPaletteChanged(FALSE),
	m_eCurMode(MODE_VIEWALL),
	m_nTotalTextureSize(0)
{
	//{{AFX_DATA_INIT(CAllTextureDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAllTextureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAllTextureDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAllTextureDlg, CDialog)
	//{{AFX_MSG_MAP(CAllTextureDlg)
	ON_WM_SIZE()
	ON_COMMAND(ID_POPUP_TEXTUREPALETTE_LOAD, OnPopupTexturePaletteLoad)
	ON_COMMAND(ID_POPUP_TEXTUREPALETTE_SAVE, OnPopupTexturePaletteSave)
	ON_COMMAND(ID_POPUP_TEXTUREPALETTE_VIEWALL, OnPopupTexturePaletteViewAll)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_POPUP_TEXTUREPALETTE_NEW, OnPopupTexturePaletteNew)
	ON_COMMAND(ID_POPUP_TEXTUREPALETTE_DELETE, OnPopupTexturePaletteDelete)
	ON_COMMAND(ID_POPUP_TEXTUREPALETTE_REMOVE, OnPopupTexturePaletteRemove)
	ON_COMMAND(ID_POPUP_TEXTUREPALETTE_IMPORT, OnPopupTexturePaletteImport)
	ON_COMMAND(ID_POPUP_TEXTUREPALETTE_SORT, OnPopupTexturePaletteSort)
	ON_COMMAND(ID_POPUP_TEXTUREPALETTE_RENAMETEX, OnPopupTexturePaletteRenameTextures)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAllTextureDlg message handlers

BOOL CAllTextureDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(AfxGetApp()->LoadIcon(IDI_TEXTURE_TAB_ICON), TRUE);

	CRect			rcTextureList;

	GetDlgItem(IDC_TEXTURELB)->GetWindowRect( &rcTextureList );
	ScreenToClient( &rcTextureList );

	m_TextureList.m_pNotifier = this;

	m_TextureList.Create(WS_VISIBLE | WS_CHILD | LBS_MULTICOLUMN | LBS_HASSTRINGS | LBS_OWNERDRAWFIXED | WS_HSCROLL |
					   LBS_NOINTEGRALHEIGHT | WS_BORDER | LBS_DISABLENOSCROLL | LBS_NOTIFY,
					   rcTextureList, this, 3456);

	m_TextureList.m_bDrawNumbers = FALSE;

	m_TextureList.SetContextMenu(CG_IDR_POPUP_TEXTUREPALETTE);

	FillTextureList();

	CRect cClientRect;
	GetClientRect(cClientRect);
	// Only initialize the anchors on the first initialize
	if (m_cResizer.GetAnchorList().GetSize() == 0)
	{
		// Set up the resizing anchors
		m_cResizer.AddAnchor(new CResizeAnchorOffset(3456, CResizeAnchorOffset::eAnchorSize));

		// Lock the resizing anchors
		m_cResizer.Lock(this, cClientRect);
	}
	else
		// Reset the sizes if this isn't the first initialization
		m_cResizer.Resize(this, cClientRect);

	// Refresh the caption
	SetPaletteChanged(GetPaletteChanged());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set the palette change flag
void CAllTextureDlg::SetPaletteChanged(BOOL bChanged)
{
	// Save the value
	m_bPaletteChanged = bChanged;

	// Update the window caption
	if (IsWindow(m_hWnd) && (GetViewMode() == MODE_PALETTE))
	{
		CString csNewTitle;
		csNewTitle.Format("Texture Palette \"%s\"%s", m_cPalette.GetName(), bChanged ? " *" : "");
		SetWindowText(csNewTitle);
	}
}

void CAllTextureDlg::AddTextureToPalette(LPCTSTR pTexture)
{
	// Jump out if the dialog's currently in "all textures" mode
	if (GetViewMode() != MODE_PALETTE)
	{
		AppMessageBox( "You must have a palette file open to perform this operation.", MB_OK );
		return;
	}

	// Add the texture to the list
	m_cPalette.AddEntry(pTexture);

	// Refresh the texture list
	FillTextureList();

	// Set the change flag
	SetPaletteChanged(TRUE);
}

void CAllTextureDlg::FillTextureList()
{
	DFileIdent	*pIdent;
	CTexture	*pTex;

	if (!GetProjectBar()->VerifyProjectIsOpen())  return;

	m_TextureList.ResetContent();
	m_nTotalTextureSize = 0;

	// Check for "view all textures" mode
	if (GetViewMode() == MODE_VIEWALL)
	{
		// Fill the texture list from the texture dialog
		CTextureDlg* pParent = GetTextureDlg();
		CString searchSpec, fullName;
		CFileIterator iterator;

		if((!pParent) || (!pParent->GetSelectedDirectory()) || (!IsWindow(pParent->m_hWnd)))
		{
			AppMessageBox( "Please select a directory in the Textures tab of the Project Window.", MB_OK );
			return;
		}

		// Add each .spr file to the list (with its FileIdent as the data).
		searchSpec = dfm_BuildName(pParent->GetSelectedDirectory()->m_Filename, "*.dtx");
		while(iterator.Next(searchSpec, TRUE))
		{
			fullName = dfm_BuildName(pParent->GetSelectedDirectory()->m_Filename, iterator.GetFilename());
			dfm_GetFileIdentifier(GetFileMgr(), fullName, &pIdent);
			if(pIdent)
			{
				// Find a location for the entry
				for (int iIndex = 0; iIndex < m_TextureList.GetCount(); iIndex++)
				{
					CString csHold;
					m_TextureList.GetText(iIndex, csHold);
					if (csHold.CompareNoCase(iterator.GetFilename()) > 0)
						break;
				}
				m_TextureList.InsertString(iIndex, iterator.GetFilename() );
				m_TextureList.SetItemDataPtr(iIndex, pIdent);

				pTex = dib_GetDibTexture(pIdent);
				if (pTex)  m_nTotalTextureSize += pTex->m_MemorySize;
			}
		}

		CString csNewTitle;
		csNewTitle.Format("Textures in Directory: \"%s\" (No Palette File Loaded)", pParent->GetSelectedDirectory()->m_Filename);

		SetWindowText(csNewTitle);
	}
	else
	{
		// Fill the texture list from the palette list
		for (DWORD uLoop = 0; uLoop < m_cPalette.GetSize(); uLoop++)
		{
			dfm_GetFileIdentifier(GetFileMgr(), m_cPalette[uLoop], &pIdent);
			if (pIdent)
			{
				int index = m_TextureList.AddString(m_cPalette[uLoop]);
				m_TextureList.SetItemDataPtr(index, pIdent);

				pTex = dib_GetDibTexture(pIdent);
				if (pTex)  m_nTotalTextureSize += pTex->m_MemorySize;
			}
		}
	}

	CString str;
	str.Format("%dKB in entire palette", m_nTotalTextureSize);
	SetDlgItemText(IDC_ALLTEXTURE_EDIT4, str);
}

void CAllTextureDlg::DeleteSelectedTexture()
{
	// Jump out if a palette isn't loaded
	if (GetViewMode() != MODE_PALETTE)
	{
		AppMessageBox( "You must have a palette file open to perform this operation.", MB_OK );
		return;
	}

	int iCurSel = m_TextureList.GetCurSel();
	
	// Make sure a texture is selected
	if (iCurSel == LB_ERR) 
	{
		MessageBox("You must have a texture selected to remove it.", "Please select a texture", MB_OK);
		return;
	}

	// Decrease palette memory size
	DFileIdent *pListItem	= (DFileIdent *)m_TextureList.GetItemData(iCurSel);
	CTexture   *pTex		= dib_GetDibTexture(pListItem);
	if (pTex)  m_nTotalTextureSize -= pTex->m_MemorySize;

	CString str;
	str.Format("");
	SetDlgItemText(IDC_ALLTEXTURE_EDIT1, str);
	SetDlgItemText(IDC_ALLTEXTURE_EDIT2, str);
	SetDlgItemText(IDC_ALLTEXTURE_EDIT3, str);
	str.Format("%dKB in entire palette", m_nTotalTextureSize);
	SetDlgItemText(IDC_ALLTEXTURE_EDIT4, str);

	// Remove it from the palette list
	m_cPalette.Remove(iCurSel);

	// Remove it from the viewing list
	m_TextureList.DeleteString(iCurSel);

	// Set the change flag
	SetPaletteChanged(TRUE);
}

int CAllTextureDlg::ConfirmSave(char *pMessage)
{
	if (!GetPaletteChanged())
		return IDOK;

	CString csMessage;
	if (!pMessage)
		csMessage.Format("The palette \"%s\" has not been saved!  Would you like to save it now?", m_cPalette.GetName());
	else
		csMessage = pMessage;

	int iResult = MessageBox(csMessage, "Confirmation",  MB_ICONINFORMATION | MB_YESNOCANCEL);
	
	switch (iResult)
	{
		case IDYES :
			OnPopupTexturePaletteSave();
			break;
	}
	return iResult;
}


void CAllTextureDlg::OnCancel()
{
	DestroyWindow( );

	return;
}

void CAllTextureDlg::NotifyDblClk( CFrameList *pList, int curSel )
{
	GetTextureDlg()->DoTextureProperties(); // Open texture properties dialog

	return;
}

void CAllTextureDlg::NotifySelChange( CFrameList *pList, int curSel )
{
	// Try to find it in the current directory
	DFileIdent *pListItem = (DFileIdent *)pList->GetItemData(curSel);
	CString csTemp(pListItem->m_Filename);
	CListCtrl *pDlgList = &(GetTextureDlg()->m_TextureList);

	// Set status text
	CTexture *pTex;
	CString str;

	SetDlgItemText(IDC_ALLTEXTURE_EDIT1, csTemp);

	pTex = dib_GetDibTexture(pListItem);

	if (pTex)
	{
	str.Format("%dx%d", pTex->m_pDib->GetWidth(), pTex->m_pDib->GetHeight());
	SetDlgItemText(IDC_ALLTEXTURE_EDIT2, str);

	str.Format("%dKB", pTex->m_MemorySize);
	SetDlgItemText(IDC_ALLTEXTURE_EDIT3, str);
	}



	int iSelItem = -1;
	for (int iLoop = 0; (iLoop < pDlgList->GetItemCount()) && (iSelItem < 0); iLoop++)
	{
		DFileIdent *pSearchItem = (DFileIdent *)pDlgList->GetItemData(iLoop);
		if (!csTemp.CompareNoCase(pSearchItem->m_Filename))
			iSelItem = iLoop;
	}

	// Just change the selection if it's already in the right directory
	if (iSelItem >= 0)
		GetTextureDlg()->ChangeSelection(iSelItem);
	else
	{
		GetTextureDlg()->FindTexture(pListItem);
		// Re-update the selection pointer because of the directory change
		m_TextureList.SetCurSel(curSel);
	}

	return;
}


void CAllTextureDlg::NotifyReCreate(CFrameList *pList)
{
	m_TextureList.DestroyWindow();
	OnInitDialog();
}

void CAllTextureDlg::NotifyDirChange()
{
	// Don't do anything if it's not in view all mode
	if (GetViewMode() != MODE_VIEWALL)
		return;

	// Don't do anything if the window isn't valid
	if(!IsWindow(m_hWnd))
		return;

	// Re-load the texture list
	FillTextureList();
}

void CAllTextureDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// Don't resize if the window doesn't really exist..
	if (!IsWindow(m_hWnd))
		return;

	// Resize the controls
	CRect cNewSize(0,0, cx, cy);
	m_cResizer.Resize(this, cNewSize);

	int32 editInterval = (cx-20)/2;

	if (GetDlgItem(IDC_ALLTEXTURE_EDIT1))
		GetDlgItem(IDC_ALLTEXTURE_EDIT1)->MoveWindow( 10, cy-45, cx-20, 20, TRUE );
	if (GetDlgItem(IDC_ALLTEXTURE_EDIT2))
		GetDlgItem(IDC_ALLTEXTURE_EDIT2)->MoveWindow( 10, cy-25, editInterval/2, 20, TRUE );
	if (GetDlgItem(IDC_ALLTEXTURE_EDIT3))
		GetDlgItem(IDC_ALLTEXTURE_EDIT3)->MoveWindow( 10 + editInterval/2, cy-25, editInterval/2, 20, TRUE );
	if (GetDlgItem(IDC_ALLTEXTURE_EDIT4))
		GetDlgItem(IDC_ALLTEXTURE_EDIT4)->MoveWindow( 10 + editInterval, cy-25, editInterval, 20, TRUE );

}

void CAllTextureDlg::OnPopupTexturePaletteLoad() 
{
	// Make sure a changed palette doesn't accidentally get lost
	if (ConfirmSave() == IDCANCEL)
		return;

	// Display the load dialog
	CHelperFileDlg	dlg( TRUE, "*.pal", m_csPaletteFileName, OFN_FILEMUSTEXIST, "Palette Files (*.pal)|*.pal||", this );
	dlg.m_ofn.lpstrInitialDir = GetProject()->m_BaseProjectDir;
	if( dlg.DoModal() != IDOK )
		return;

	// Load the texture palette
	if (!m_cPalette.Load(dlg.GetPathName()))
		return;

	// Switch to palette mode
	SetViewMode(MODE_PALETTE);

	// Display the loaded palette
	FillTextureList();

	// Turn off the change flag
	SetPaletteChanged(FALSE);
}

void CAllTextureDlg::OnPopupTexturePaletteSave() 
{
	// Display the save dialog
	CString csFileName(m_csPaletteFileName);
	if (!m_csPaletteFileName.GetLength())
	{
		csFileName = m_cPalette.GetName();
		csFileName += ".pal";
	}
	CHelperFileDlg	dlg(FALSE, "*.pal", csFileName, OFN_OVERWRITEPROMPT, "Palette Files (*.pal)|*.pal||", this );
	dlg.m_ofn.lpstrInitialDir = GetProject()->m_BaseProjectDir;
	if( dlg.DoModal() != IDOK )
		return;

	// Change the file name if it was empty before
	if (!m_csPaletteFileName.GetLength())
		m_csPaletteFileName = dlg.GetPathName();

	// Save the texture palette
	if (!m_cPalette.Save(m_csPaletteFileName))
		return;

	// Turn off the change flag
	SetPaletteChanged(FALSE);
}

void CAllTextureDlg::OnPopupTexturePaletteViewAll() 
{
	// Make sure a changed palette doesn't accidentally get lost
	if (ConfirmSave() == IDCANCEL)
		return;

	// Clear the palette list
	m_cPalette.Term();
	m_cPalette.SetName("");
	m_csPaletteFileName = "";

	// Go to "view all" mode
	SetViewMode(MODE_VIEWALL);

	// Load the texture list from the project bar
	FillTextureList();

	// Turn off the change flag
	SetPaletteChanged(FALSE);
}

void CAllTextureDlg::OnPopupTexturePaletteNew() 
{
	// Make sure a changed palette doesn't accidentally get lost
	if (ConfirmSave() == IDCANCEL)
		return;

	// Get a palette name
	CStringDlg		dlg;

	dlg.m_bAllowFile = TRUE;
	dlg.m_MaxStringLen = 70;
	dlg.m_bBeeping = TRUE;

	if( dlg.DoModal(IDS_TEXTUREPALETTE_NEW_TITLE, IDS_TEXTUREPALETTE_NEW_CAPTION) != IDOK )
		return;

	// Set up for an empty palette
	CString csTemp(dlg.m_EnteredText);
	if (!csTemp.GetLength())
		m_cPalette.SetName("Unnamed");
	else
		m_cPalette.SetName(csTemp);
	m_csPaletteFileName = "";
	m_cPalette.Term();

	// Go to palette mode
	SetViewMode(MODE_PALETTE);

	// Clear the texture list
	FillTextureList();

	// Set the change flag
	SetPaletteChanged(TRUE);
}

void CAllTextureDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
		case VK_DELETE :
			DeleteSelectedTexture();
			break;
	}

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CAllTextureDlg::DestroyWindow(bool bAcceptCancel) 
{
	if ((ConfirmSave() == IDCANCEL) && bAcceptCancel)
		return false;

	// Make sure the change dialog never comes up again
	SetPaletteChanged(FALSE);

	if (!IsWindow(m_hWnd))
		return true;

	return CDialog::DestroyWindow();
}

void CAllTextureDlg::OnPopupTexturePaletteDelete() 
{
	DeleteSelectedTexture();
}

// Removes the selected texture from the world and from the palette
void CAllTextureDlg::OnPopupTexturePaletteRemove()
{
	// Get the active document
	CRegionDoc *pDoc = GetActiveRegionDoc();
	if (!pDoc)
	{
		MessageBox("You must have a world open to remove the texture from.", "Please open a world", MB_OK);
		return;
	}

	if (m_TextureList.GetCurSel() == LB_ERR) 
	{
		MessageBox("You must have a texture selected to remove it.", "Please select a texture", MB_OK);
		return;
	}

	if(MessageBox("Are you sure you want to clear all surfaces\nwith this texture in the world?",
					"Remove Texture?", MB_ICONQUESTION | MB_YESNO) == IDNO)		return;

	// Show the hourglass cursor
	BeginWaitCursor();

	DFileIdent *pListItem = (DFileIdent *)m_TextureList.GetItemData(m_TextureList.GetCurSel());

	// Check if item selected was valid
	if ((int)pListItem == LB_ERR)  return;

	// Go through the brush list
	DFileIdent *pIdent;
	CLinkedList<CEditBrush*> *pBrushList = &(pDoc->m_Region.m_Brushes);
	LPOS lpFinger;
	CEditPoly *pPoly;
	CMoArray<CEditPoly *> cPolyList;
	PreActionList actionList;
	for (lpFinger = pBrushList->GetHeadPosition(); lpFinger; )
	{
		CEditBrush *pBrush = pBrushList->GetNext(lpFinger);

		// Go through the polygon list
		DWORD uLoop;
		for (uLoop = 0; uLoop < pBrush->m_Polies.GetSize(); uLoop++)
		{
			// If this poly has the texture we're removing
			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				if (_stricmp(pListItem->m_Filename, pBrush->m_Polies[uLoop]->GetTexture(nCurrTex).m_pTextureName) == 0)
				{
					// Add an undo.
					AddToActionListIfNew(&actionList, new CPreAction(ACTION_MODIFYNODE, pBrush), TRUE);
					// Add it to the modify list
					cPolyList.Add(pBrush->m_Polies[uLoop]);
				}
			}
		}
	}

	// Save the undo list.
	pDoc->Modify(&actionList, TRUE);

	// Actually modify the polies
	for(uint32 i = 0; i < cPolyList.GetSize(); i++)
	{
	
		// If this poly has the texture we're removing
		for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
		{
			if (_stricmp(pListItem->m_Filename, cPolyList[i]->GetTexture(nCurrTex).m_pTextureName) == 0)
			{
				pPoly = cPolyList[i];
				pPoly->GetTexture(nCurrTex).m_pTextureName = pDoc->GetRegion()->m_pStringHolder->AddString("Default");
				pPoly->GetTexture(nCurrTex).UpdateTextureID();
			}
		}
	}

	pDoc->RedrawAllViews();

	// Remove this texture from the palette as well
	if (GetViewMode() == MODE_PALETTE)	DeleteSelectedTexture();
}

//adds all the textures in the specified world, will recurse into prefabs if specified
void CAllTextureDlg::AddWorldTextures(const CWorldNode* pNode, bool bAddPrefabs, bool& bChanged)
{
	//bail if it not valid
	if(!pNode)
		return;

	//see if this node is a prefab
	if(pNode->GetType() == Node_PrefabRef && bAddPrefabs)
	{
		CPrefabRef* pPrefab = (CPrefabRef*)pNode;
		AddWorldTextures(pPrefab->GetPrefabTree(), bAddPrefabs, bChanged);
	}
	else if(pNode->GetType() == Node_Brush)
	{
		CEditBrush *pBrush = pNode->AsBrush();

		// Go through the polygon list
		for (uint32 uLoop = 0; uLoop < pBrush->m_Polies.GetSize(); uLoop++)
		{
			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				CString csTextureName(pBrush->m_Polies[uLoop]->GetTexture(nCurrTex).m_pTextureName);
				// Skip over untextured polygons
				if (!csTextureName.GetLength())
					continue;
				if (!csTextureName.Compare("Default"))
					continue;
				// Skip over sprites
				CString csTemp;
				csTemp = csTextureName.Right(3);
				if (!csTemp.CompareNoCase("SPR"))
					continue;
				// Add the texture
				if (m_cPalette.AddEntry(csTextureName))
				{
					DFileIdent *pIdent;

					dfm_GetFileIdentifier(GetFileMgr(), csTextureName, &pIdent);
					if (pIdent)
					{
						int index = m_TextureList.AddString(csTextureName);
						m_TextureList.SetItemDataPtr(index, pIdent);
						m_TextureList.UpdateWindow();
						bChanged = true;

						CTexture *pTex = dib_GetDibTexture(pIdent);
						if (pTex)  
							m_nTotalTextureSize += pTex->m_MemorySize;
					}
				}
			}
		}
	}


	//now recurse on all of the children
	if(!pNode->m_Children.IsEmpty())
	{
		GPOS Pos = pNode->m_Children.GetHead(); 
		while(Pos)
		{
			CWorldNode* pChild = pNode->m_Children.GetNext(Pos);
			AddWorldTextures(pChild, bAddPrefabs, bChanged);
		}
	}
}

void CAllTextureDlg::OnPopupTexturePaletteImport() 
{
	// Jump out if the dialog's not in palette mode
	if (GetViewMode() != MODE_PALETTE)
	{
		AppMessageBox( "You must have a palette file open to perform this operation.", MB_OK );
		return;
	}

	// Get the active document
	CRegionDoc *pDoc = GetActiveRegionDoc();
	if (!pDoc)
		return;

	// Show the hourglass cursor
	BeginWaitCursor();

	// Go through the brush list
	bool bChanged = false;

	AddWorldTextures(pDoc->GetRegion()->GetRootNode(), true, bChanged);

	// Show palette size
	CString str;
	str.Format("%dKB in entire palette", m_nTotalTextureSize);
	SetDlgItemText(IDC_ALLTEXTURE_EDIT4, str);

	// Update the change flag
	if (bChanged)
		SetPaletteChanged(TRUE);

	// Bring back the old cursor
	EndWaitCursor();
}

void CAllTextureDlg::OnPopupTexturePaletteSort() 
{
	// Jump out if the dialog's not in palette mode
	if (GetViewMode() != MODE_PALETTE)
	{
		AppMessageBox( "You must have a palette file open to perform this operation.", MB_OK );
		return;
	}

	// Jump out if nothing's in the palette to sort
	if (m_cPalette.GetSize() <= 1)
		return;

	BeginWaitCursor();

	// Use a simple bubble sort on the palette..  (Not optimal, but whatever, it's a short list..)
	for (DWORD uTop = 0; uTop < (m_cPalette.GetSize() - 1); uTop++)
	{
		CString csLowest = m_cPalette[uTop];
		for (DWORD uSearch = uTop + 1; uSearch < m_cPalette.GetSize(); uSearch++)
		{
			if (csLowest.CompareNoCase(m_cPalette[uSearch]) > 0)
			{
				// Swap the entries
				CString csTemp = m_cPalette[uSearch];
				m_cPalette[uSearch] = csLowest;
				csLowest = csTemp;
			}
		}
		// Store the lowest entry in the array
		m_cPalette[uTop] = csLowest;
	}

	// Refill the palette list
	FillTextureList();

	// Set the change flag
	SetPaletteChanged(TRUE);

	EndWaitCursor();
}

void CAllTextureDlg::OnPopupTexturePaletteRenameTextures()
{
	//add all of our textures to the list
	for(uint32 nCurrTex = 0; nCurrTex < m_TextureList.GetCount(); nCurrTex++)
	{
		DFileIdent *pItemData = (DFileIdent *)m_TextureList.GetItemData(nCurrTex);
		GetRenameResourceDlg()->AddNewFile(pItemData->m_Filename);
	}

	GetRenameResourceDlg()->UpdateIcons();
	GetRenameResourceDlg()->ShowWindow(SW_SHOW);
}
