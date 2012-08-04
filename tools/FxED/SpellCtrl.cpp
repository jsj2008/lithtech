// SpellCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "SpellEd.h"
#include "SpellCtrl.h"
#include "StringDlg.h"
#include "mainfrm.h"
#include "SpellEdDoc.h"
#include "SpellEdView.h"
#include "ChildFrm.h"
#include "ImportSpellsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Includes....

#include "SpellsDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellCtrl

CSpellCtrl::CSpellCtrl()
{
	m_pCopySpell = NULL;
}

CSpellCtrl::~CSpellCtrl()
{
}


BEGIN_MESSAGE_MAP(CSpellCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CSpellCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_COMMAND(ID_ADDGROUP, OnAddGroup)
	ON_COMMAND(ID_DELETEGROUP, OnDeleteGroup)
	ON_COMMAND(ID_ADDSPELL, OnAddSpell)
	ON_COMMAND(ID_DELETESPELL, OnDeleteSpell)
	ON_COMMAND(ID_DELETESPELLFROMGROUP, OnDeleteSpellFromGroup)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_ADDSPELLSBYREF, OnAddSpellsByReference)
	ON_COMMAND(ID_RENAMESPELL, OnRenameSpell)
	ON_COMMAND(ID_RENAMEGROUP, OnRenameGroup)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblClk)
	ON_COMMAND(ID_EDITSPELL, OnEditSpell)
	ON_COMMAND(ID_GROUP_MAKECOPY, OnGroupMakeCopy)
	ON_COMMAND(ID_IMPORTSPELL, OnImportSpell)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellCtrl message handlers

//------------------------------------------------------------------
//
//   FUNCTION : AddGroup()
//
//   PURPOSE  : Adds a new group to the list of spells
//
//------------------------------------------------------------------

BOOL CSpellCtrl::AddGroup(HTREEITEM hItem, char *sName)
{
	// Add a new group 

	HTREEITEM hNewItem = InsertItem(sName, 0, 1, hItem, TVI_LAST);
	EnsureVisible(hNewItem);

	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnLButtonDown()
//
//   PURPOSE  : Handles WM_LBUTTONDOWN
//
//------------------------------------------------------------------

void CSpellCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CTreeCtrl::OnLButtonDown(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRButtonDown()
//
//   PURPOSE  : Handles WM_RBUTTONDOWN
//
//------------------------------------------------------------------

void CSpellCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// Get the current selection
	
	CTreeCtrl::OnRButtonDown(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRButtonUp()
//
//   PURPOSE  : Handles WM_RBUTTONUP
//
//------------------------------------------------------------------

void CSpellCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{

	CTreeCtrl::OnRButtonUp(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRClick()
//
//   PURPOSE  : Handles right mouse button clicking
//
//------------------------------------------------------------------

void CSpellCtrl::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	
	CPoint ptCursor;
	GetCursorPos(&ptCursor);

	CPoint point = ptCursor;
	ScreenToClient(&point);
	UINT flags = 0;
	HTREEITEM hSelectedItem = HitTest(point, &flags);
	if (!hSelectedItem) return;

	HTREEITEM hParent = GetParentItem(hSelectedItem);

	int iImage;
	int iSelectedImage;

	GetItemImage(hSelectedItem, iImage, iSelectedImage);

	// Invoke the right mouse button menu

	{
		CMenu rbMenu;
		rbMenu.CreatePopupMenu();
			
		BOOL bNeedSeparator = FALSE;
		
		if (iImage == 0)
		{
			rbMenu.AppendMenu(MF_STRING, ID_ADDGROUP, "&Add Group");
			if (hSelectedItem != GetRootItem()) 
			{
				rbMenu.AppendMenu(MF_STRING, ID_DELETEGROUP, "Delete Group");
				rbMenu.AppendMenu(MF_STRING, ID_RENAMEGROUP, "Rename &Group");
			}
			
			bNeedSeparator = TRUE;
		}

		if (bNeedSeparator) rbMenu.AppendMenu(MF_SEPARATOR, 0, "");

		if (iImage != 0)
		{
			rbMenu.AppendMenu(MF_STRING, ID_EDITSPELL, "&Edit FX");

			if (hParent != GetRootItem())
			{
				rbMenu.AppendMenu(MF_STRING, ID_DELETESPELLFROMGROUP, "Delete FX From &Group");
			}
			else
			{
				rbMenu.AppendMenu(MF_STRING, ID_GROUP_MAKECOPY, "&Make Copy Of FX");
				rbMenu.AppendMenu(MF_STRING, ID_DELETESPELL, "&Delete FX");
			}

			rbMenu.AppendMenu(MF_STRING, ID_RENAMESPELL, "&Rename FX");
		}

		if (iImage == 0)
		{
			rbMenu.AppendMenu(MF_STRING, ID_ADDSPELL, "Add &FX");

			if ((hSelectedItem != GetRootItem()) && (pApp->GetSpellMgr()->GetSpells()->GetSize()))
			{
				rbMenu.AppendMenu(MF_STRING, ID_ADDSPELLSBYREF, "Add FX By &Reference");
			}
		}

		rbMenu.AppendMenu(MF_SEPARATOR, 0, "");
		rbMenu.AppendMenu(MF_STRING, ID_IMPORTSPELL, "&Import FX");

		rbMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptCursor.x, ptCursor.y, this, NULL);		
	}

	SelectItem(hSelectedItem);

	Invalidate();
	UpdateWindow();
	
	*pResult = 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnAddGroup()
//
//   PURPOSE  : Adds a group of spells
//
//------------------------------------------------------------------

void CSpellCtrl::OnAddGroup() 
{
	// Add a new group

	HTREEITEM hSelectedItem = GetSelectedItem();

	if (hSelectedItem)
	{
		if (hSelectedItem == GetRootItem())
		{
			HTREEITEM hNewItem = InsertItem("New FX Group", 0, 1, NULL, TVI_LAST);
			EnsureVisible(hNewItem);
		}
		else
		{
			HTREEITEM hNewItem = InsertItem("New FX Group", 0, 1, hSelectedItem, TVI_LAST);
			EnsureVisible(hNewItem);
		}

		FullSortTree(GetRootItem());
	}

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDeleteGroup()
//
//   PURPOSE  : Deletes a group of spells
//
//------------------------------------------------------------------

void CSpellCtrl::OnDeleteGroup() 
{
	// Delete a group
	
	HTREEITEM hSelectedItem = GetSelectedItem();
	
	if (hSelectedItem)
	{
		BOOL bDelete = TRUE;

		if (ItemHasChildren(hSelectedItem))
		{
			int ret = AfxMessageBox("Warning, all groupings will be lost !!!, Continue ?", MB_YESNO | MB_ICONEXCLAMATION);
			if (ret == IDNO) bDelete = FALSE;
		}

		if (bDelete) DeleteItem(hSelectedItem);
	}	

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnAddSpell()
//
//   PURPOSE  : Adds a spell
//
//------------------------------------------------------------------

void CSpellCtrl::OnAddSpell() 
{
	HTREEITEM hSelectedItem = GetSelectedItem();
	if (!hSelectedItem) return;
	
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	CSpell *pNewSpell = pApp->GetSpellMgr()->AddSpell();
	if (!pNewSpell) return;

	// Add this spell to the tree control

	TV_INSERTSTRUCT tvItem;

	tvItem.hParent			   = hSelectedItem;
	tvItem.hInsertAfter		   = TVI_LAST;
	tvItem.item.mask		   = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE;
	tvItem.item.pszText		   = (char *)(LPCSTR)pNewSpell->GetName();
	tvItem.item.cchTextMax	   = strlen(tvItem.item.pszText);
	tvItem.item.iImage		   = 2;
	tvItem.item.iSelectedImage = 2;
	tvItem.item.lParam		   = (LPARAM)pNewSpell;
	
	HTREEITEM hNewSpell = InsertItem(&tvItem);
	if (hNewSpell) EnsureVisible(hNewSpell);

	if (hSelectedItem != GetRootItem())
	{
		// Need to add this one to the global list as well....

		tvItem.hParent = GetRootItem();
		InsertItem(&tvItem);

		FullSortTree(GetRootItem());
	}

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDeleteSpell()
//
//   PURPOSE  : Deletes a spell ENTIRELY
//
//------------------------------------------------------------------

void CSpellCtrl::OnDeleteSpell() 
{
	// Big time delete....

	HTREEITEM hSelectedItem = GetSelectedItem();
	if (!hSelectedItem) return;

	CSpell *pSpell = (CSpell *)GetItemData(hSelectedItem);
	if (!pSpell) return;

	int ret = AfxMessageBox("Deletion is permanent !!, Continue ?", MB_ICONEXCLAMATION | MB_YESNO);

	if (ret == IDYES)
	{
		CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

		// Before we delete the spell we need to close any documents associated with
		// this spell

		CDocTemplate *pTemplate = pApp->GetDocTemplate();

		POSITION docPos = pTemplate->GetFirstDocPosition();

		while (docPos)
		{
			CSpellEdDoc *pDoc = (CSpellEdDoc *)pTemplate->GetNextDoc(docPos);

			POSITION viewPos = pDoc->GetFirstViewPosition();
	
			CSpellEdView *pView = (CSpellEdView *)pDoc->GetNextView(viewPos);

			if (pView)
			{
				if (pView->GetSpell() == pSpell)
				{
					// Remove the document
					
					pTemplate->RemoveDocument(pDoc);
					
					pDoc->OnCloseDocument();
				}
			}
		}
		
		// Nuke that spell !!!

		pApp->GetSpellMgr()->DeleteSpell(pSpell);

		DeleteItem(hSelectedItem);
		Invalidate(TRUE);

		FullUpdate(GetRootItem());
	}

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDeleteSpellFromGroup()
//
//   PURPOSE  : Removes spell from group
//
//------------------------------------------------------------------

void CSpellCtrl::OnDeleteSpellFromGroup() 
{
	HTREEITEM hSelectedItem = GetSelectedItem();
	if (!hSelectedItem) return;

	DeleteItem(hSelectedItem);
	Invalidate(TRUE);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnKeyDown()
//
//   PURPOSE  : Handles WM_KEYDOWN
//
//------------------------------------------------------------------

void CSpellCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//see if the user is hitting delete
	if(nChar == VK_DELETE)
	{
		//forward it on to the menu handler...
		OnDeleteSpell();
	}

	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnAddSpellsByReference()
//
//   PURPOSE  : Adds a series of spells by reference
//
//------------------------------------------------------------------

void CSpellCtrl::OnAddSpellsByReference() 
{
	HTREEITEM hSelectedItem = GetSelectedItem();
	if (!hSelectedItem) return;

	CSpellsDlg dlg;

	if (dlg.DoModal() == IDOK)
	{
		for (int i = 0; i < dlg.m_nSpells; i ++)
		{
			CSpell *pSpell = dlg.m_pSpells[i];

			// Check to make sure we don't already have a ref to this spell in
			// this group

			BOOL bOkayToAdd = TRUE;

			HTREEITEM hChild = GetChildItem(hSelectedItem);

			while (hChild)
			{
				int iImage, iSelectedImage;
				GetItemImage(hChild, iImage, iSelectedImage);

				if (iImage == 2)
				{
					// This is a spell

					CSpell *pCheckSpell = (CSpell *)GetItemData(hChild);

					if (pCheckSpell == pSpell) bOkayToAdd = FALSE;
				}

				hChild = GetNextSiblingItem(hChild);
			}

			if (bOkayToAdd)
			{
				// Add this spell to the current group
				
				TV_INSERTSTRUCT tvItem;

				tvItem.hParent			   = hSelectedItem;
				tvItem.hInsertAfter		   = TVI_LAST;
				tvItem.item.mask		   = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE;
				tvItem.item.pszText		   = (char *)(LPCSTR)pSpell->GetName();
				tvItem.item.cchTextMax	   = strlen(tvItem.item.pszText);
				tvItem.item.iImage		   = 2;
				tvItem.item.iSelectedImage = 2;
				tvItem.item.lParam		   = (LPARAM)pSpell;

				InsertItem(&tvItem);

				FullSortTree(GetRootItem());
			}
		}

		if (dlg.m_nSpells)
		{
			Expand(hSelectedItem, TVE_EXPAND);
		}
	}

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRenameSpell()
//
//   PURPOSE  : Renames a spell
//
//------------------------------------------------------------------

void CSpellCtrl::OnRenameSpell() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	HTREEITEM hSelectedItem = GetSelectedItem();
	if (!hSelectedItem) return;

	CStringDlg dlg("Enter FX Name");

	dlg.m_sText = GetItemText( hSelectedItem );
	if (dlg.DoModal() == IDOK)
	{
		// Check to make sure that we don't have a spell already named this

		if (pApp->GetSpellMgr()->FindSpellByName(dlg.m_sText))
		{
			AfxMessageBox("An FX group with this name already exists !!", MB_ICONEXCLAMATION | MB_OK);
		}
		else
		{
			// Retrieve the spell to modify...

			CSpell *pSpell = (CSpell *)GetItemData(hSelectedItem);

			pSpell->SetName(dlg.m_sText.GetBuffer(dlg.m_sText.GetLength()));
			pSpell->SetGuid(dlg.m_sText.GetBuffer(dlg.m_sText.GetLength()));

			FullUpdate(GetRootItem());
			pApp->UpdateViewNames();

			FullSortTree(GetRootItem());
		}
	}

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRenameGroup()
//
//   PURPOSE  : Renames a group
//
//------------------------------------------------------------------

void CSpellCtrl::OnRenameGroup() 
{
	HTREEITEM hSelectedItem = GetSelectedItem();
	if (!hSelectedItem) return;

	CStringDlg dlg("Enter Group Name");

	dlg.m_sText = GetItemText( hSelectedItem );
	if (dlg.DoModal() == IDOK)
	{
		SetItemText(hSelectedItem, dlg.m_sText);
	}

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : FullUpdate()
//
//   PURPOSE  : Goes through the entire tree control and makes 
//				sure all names etc are valid
//
//------------------------------------------------------------------

void UpdateNames(CTreeCtrl *pCtrl, HTREEITEM hItem)
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	while (hItem)
	{
		HTREEITEM hDelItem = NULL;

		int iImage, iSelectedImage;
		pCtrl->GetItemImage(hItem, iImage, iSelectedImage);

		if (iImage == 2)
		{
			// Spell, update
			
			CSpell *pSpell = (CSpell *)pCtrl->GetItemData(hItem);

			// Check to see if the spell still exists, if not then delete
			// this reference

			if (pApp->GetSpellMgr()->SpellExists(pSpell))
			{
				pCtrl->SetItemText(hItem, pSpell->GetGuid());
			}
			else
			{
				hDelItem = hItem;
			}
		}

		if (pCtrl->ItemHasChildren(hItem)) UpdateNames(pCtrl, pCtrl->GetChildItem(hItem));
		
		hItem = pCtrl->GetNextSiblingItem(hItem);

		if (hDelItem) pCtrl->DeleteItem(hDelItem);
	}
}

BOOL CSpellCtrl::FullUpdate(HTREEITEM hItem)
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	// Update the names

	CTreeCtrl *pCtrl = this;
	UpdateNames(pCtrl, hItem);

	// Now, go through the all spells list and make sure we have every one listed

	CLinkListNode<CSpell *> *pSpellNode = pApp->GetSpellMgr()->GetSpells()->GetHead();

	while (pSpellNode)
	{
		HTREEITEM hItem = GetChildItem(GetRootItem());

		if (hItem)
		{
			BOOL bHaveSpell = FALSE;
			
			while (hItem)
			{
				CSpell *pSpell = (CSpell *)GetItemData(hItem);

				if (pSpellNode->m_Data == pSpell) bHaveSpell = TRUE;
				
				hItem = GetNextSiblingItem(hItem);
			}

			if (!bHaveSpell)
			{
				TV_INSERTSTRUCT tvItem;

				tvItem.hParent			   = GetRootItem();
				tvItem.hInsertAfter		   = TVI_LAST;
				tvItem.item.mask		   = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE;
				tvItem.item.pszText		   = (char *)(LPCSTR)pSpellNode->m_Data->GetName();
				tvItem.item.cchTextMax	   = strlen(tvItem.item.pszText);
				tvItem.item.iImage		   = 2;
				tvItem.item.iSelectedImage = 2;
				tvItem.item.lParam		   = (long)pSpellNode->m_Data;
				
				HTREEITEM hNewSpell = InsertItem(&tvItem);
			}
		}

		pSpellNode = pSpellNode->m_pNext;
	}

	FullSortTree(GetRootItem());

 	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDblClk()
//
//   PURPOSE  : Handles WM_LBUTTONDBLCLK
//
//------------------------------------------------------------------

void CSpellCtrl::OnDblClk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnEditSpell();
	
	*pResult = 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnEditSpell()
//
//   PURPOSE  : Edits a spell
//
//------------------------------------------------------------------

void CSpellCtrl::OnEditSpell() 
{
	// Get the current selection

	HTREEITEM hSelectedItem = GetSelectedItem();
	if (!hSelectedItem) return;

	// Retrieve the images

	int iImage, iSelectedImage;
	GetItemImage(hSelectedItem, iImage, iSelectedImage);

	if (iImage == IM_SPELL)
	{
		CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
		CFrameWnd *pFrameWnd = (CMainFrame *)AfxGetMainWnd();

		// It's a spell so open a view with this spell....

		CSpell *pSpell = (CSpell *)GetItemData(hSelectedItem);

		// See if we are already editing it, if we are then bring it to the front

		CSpellEdView *pView = pApp->GetViewBySpell(pSpell);
		
		if (pView)
		{
			pView->GetParent()->BringWindowToTop();
		}
		else
		{
			// No window open so go ahead and create one....

			CSpellEdDoc *pDoc = (CSpellEdDoc *)pApp->GetDocTemplate()->CreateNewDocument();
			CChildFrame *pFrame = (CChildFrame *)pApp->GetDocTemplate()->CreateNewFrame(pDoc, NULL);
			pFrame->GetView()->Init(pSpell);
			pFrame->GetView()->GetDocument()->SetTitle(pSpell->GetName());
			pApp->GetDocTemplate()->InitialUpdateFrame(pFrame, NULL);
			// See if any windows are maximized

			BOOL bMoveWindow = TRUE;

			POSITION pos = pApp->GetDocTemplate()->GetFirstDocPosition();

			while (pos)
			{
				CDocument *pDoc = pApp->GetDocTemplate()->GetNextDoc(pos);

				if (pDoc)
				{
					POSITION viewPos = pDoc->GetFirstViewPosition();
					CSpellEdView *pView = (CSpellEdView *)pDoc->GetNextView(viewPos);

					if (pView->GetParent()->IsZoomed()) bMoveWindow = FALSE;
				}
			}			
			if (bMoveWindow) pFrame->MoveWindow(0, 0, 700, 600);
			pFrameWnd->RecalcLayout();
		}
		
		Invalidate();
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnGroupMakeCopy()
//
//   PURPOSE  : Copys a spell
//
//------------------------------------------------------------------

void CSpellCtrl::OnGroupMakeCopy() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	HTREEITEM hSelectedItem = GetSelectedItem();
	if (!hSelectedItem) return;

	if (!IsSpell(hSelectedItem)) return;

	CSpell *pSpell = (CSpell *)GetItemData(hSelectedItem);
	
	// Make a new spell

	CSpell *pCopy = pApp->GetSpellMgr()->AddSpell();

	// Copy all the stuff

	CString sNewName = pSpell->GetName();
	sNewName = sNewName + "_1";

	while (pApp->GetSpellMgr()->FindSpellByName(sNewName))
	{
		sNewName += "_1";
	}

	pCopy->SetName(sNewName.GetBuffer(sNewName.GetLength()));
	pCopy->SetDesc(pSpell->GetDesc());
	pCopy->SetCastCost(pSpell->GetCastCost());
	pCopy->SetRadius(pSpell->GetRadius());
	pCopy->SetHowOften(pSpell->GetHowOften());
	pCopy->SetType(pSpell->GetType());
	pCopy->SetTargetType(pSpell->GetTargetType());

	// Copy the effects
/*
	CLinkListNode<CBaseEffect *> *pNode = pSpell->GetEffects()->GetHead();

	while (pNode)
	{
		pCopy->GetEffects()->AddTail(pNode->m_Data->CopyEffect());
		
		pNode = pNode->m_pNext;
	}
*/
	// Copy the phases

	for (int i = 0; i < 3; i ++)
	{
		CPhase *pSourcePhase = pSpell->GetPhase(i);
		CPhase *pTargetPhase = pCopy->GetPhase(i);

		pTargetPhase->SetPhaseLength(pSourcePhase->GetPhaseLength());

		CLinkListNode<CTrack *> *pSourceTrack = pSourcePhase->GetTracks()->GetHead();
		
		while (pSourceTrack)
		{						
			CTrack *pNewTrack = new CTrack;

			CLinkListNode<CKey *> *pSourceKey = pSourceTrack->m_Data->GetKeys()->GetHead();

			// Add all the keys
			
			while (pSourceKey)
			{
				BOOL bLink = pSourceKey->GetData()->IsLinked();
				DWORD dwLinkedID = pSourceKey->GetData()->GetLinkedID();
				char sNodeName[32];
				memcpy(sNodeName, pSourceKey->GetData()->GetLinkedNodeName(), 32);
				
				CKey *pNewKey = pSourceKey->GetData()->Clone();

				pNewKey->SetID(pSourceKey->GetData()->GetID());
				if(bLink)
				{
					pNewKey->SetLink(TRUE, dwLinkedID, sNodeName);
				}
				else
				{
					pNewKey->SetLink(FALSE, 0, "");
				}

				pNewTrack->GetKeys()->AddTail(pNewKey);
				
				pSourceKey = pSourceKey->m_pNext;
			}

			pTargetPhase->GetTracks()->AddTail(pNewTrack);
			
			pSourceTrack = pSourceTrack->m_pNext;
		}

		pTargetPhase->SetupUniqueID();
	}

	InsertSpell(GetParentItem(hSelectedItem), pCopy);

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnImportSpell
//
//   PURPOSE  : Imports spells from another .dic file
//
//------------------------------------------------------------------

void CSpellCtrl::OnImportSpell() 
{
	char sDir[256];

	// Retrieve the selection

	HTREEITEM hInsert = GetSelectedItem();
	if (!hInsert) return;

	// Move up one if it's a spell
	
	if (IsSpell(hInsert)) hInsert = GetParentItem(hInsert);

	GetCurrentDirectory(256, sDir);
	CFileDialog dlg(TRUE, "*.fxf", "*.fxf", OFN_OVERWRITEPROMPT, "FX Group Files (*.fxf)|*.fxf||");
	dlg.m_ofn.lpstrInitialDir = sDir;

	if (dlg.DoModal() == IDOK)
	{
		CString sName = dlg.GetPathName();

		CImportSpellsDlg dlg(sName);

		if (dlg.DoModal() == IDOK)
		{
			CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

			CSpellMgr *pSource = &dlg.m_spellMgr;
			CSpellMgr *pTarget = pApp->GetSpellMgr();
			
			// Add the spells....

			CLinkListNode<CSpell *> *pNode = dlg.m_selectedSpells.GetHead();

			while (pNode)
			{
				CSpell *pSpell = pNode->m_Data;

				// Remove this spell from the source manager

				pSource->GetSpells()->Remove(pSpell);

				// And add it to the target manager

				pTarget->GetSpells()->AddTail(pSpell);
				
				// Add it to the group

				InsertSpell(hInsert, pSpell);
				if (hInsert != GetRootItem()) InsertSpell(GetRootItem(), pSpell);

				pNode = pNode->m_pNext;
			}
		}

	}

	Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : SortTree()
//
//   PURPOSE  : Sorts the tree
//
//------------------------------------------------------------------

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (!lParam1)
	{
		return 1;
	}

	if ((lParam1) && (!lParam2))
	{
		return -1;
	}

	if ((lParam1) && (lParam2))
	{
		CSpell *pSpell1 = (CSpell *)lParam1;
		CSpell *pSpell2 = (CSpell *)lParam2;		
		
		return stricmp(pSpell1->GetName(), pSpell2->GetName());
	}

	return 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : FullSortTree()
//
//   PURPOSE  : Fully sorts the tree....
//
//------------------------------------------------------------------

void CSpellCtrl::FullSortTree(HTREEITEM hItem)
{
	while (hItem)
	{
		if (ItemHasChildren(hItem))
		{
			TVSORTCB cb;
			cb.hParent	   = hItem;
			cb.lParam	   = NULL;
			cb.lpfnCompare = CompareFunc;

			FullSortTree(GetChildItem(hItem));

			SortChildrenCB(&cb);
		}
		
		hItem = GetNextSiblingItem(hItem);
	}	
}
