// BaseSelectionFolder.cpp: implementation of the CBaseSelectionFolder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseSelectionFolder.h"
#include "ClientRes.h"
#include "InterfaceMgr.h"
#include "FolderCommands.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;




namespace
{
    LTBOOL   bFinishedSetup = LTFALSE;
}
	LTIntPt  g_UpArrowOffset;
	LTIntPt  g_DownArrowOffset;


char	CBaseSelectionFolder::m_sSelectStr[32] = "";
char	CBaseSelectionFolder::m_sUnselectStr[32] = "";
char	CBaseSelectionFolder::m_sRequiredStr[32] = "";
char	CBaseSelectionFolder::m_sEquippedStr[32] = "";
char	CBaseSelectionFolder::m_sUnequippedStr[32] = "";
char	CBaseSelectionFolder::m_sEmptyStr[32] = "";

const int CBaseSelectionFolder::kEmptySlot = 9999;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseSelectionFolder::CBaseSelectionFolder()
{

	m_nNumSlots = 0;
	m_nFirstSlot = 0;
	m_nSlotsFilled = 0;
	m_nSlotsLocked = 0;
	m_bSaveSelection = LTFALSE;
	m_nLastListItem = kNoSelection;
	m_hPhotoSurf = LTNULL;

    m_bReadLayout = LTFALSE;

	m_szModel[0] = NULL;
	m_szSkin[0] = NULL;
	m_fSFXRot = 0.0f;
	m_vOffset = LTVector(0.0f,0.0f,0.0f);
	m_fScale = 1.0f;


}

CBaseSelectionFolder::~CBaseSelectionFolder()
{

}


LTBOOL CBaseSelectionFolder::Build()
{
	if (!m_bReadLayout)
	{
        m_bReadLayout       = LTTRUE;
		m_SlotOffset		= g_pLayoutMgr->GetSlotOffset((eFolderID)m_nFolderID);
		m_ListRect			= g_pLayoutMgr->GetListRect((eFolderID)m_nFolderID);
		m_NameRect 			= g_pLayoutMgr->GetNameRect((eFolderID)m_nFolderID);
		m_PhotoPos	 		= g_pLayoutMgr->GetPhotoPos((eFolderID)m_nFolderID);
		m_DescriptionRect 	= g_pLayoutMgr->GetDescriptionRect((eFolderID)m_nFolderID);
		g_UpArrowOffset		= g_pLayoutMgr->GetUpArrowOffset((eFolderID)m_nFolderID);
		g_DownArrowOffset	= g_pLayoutMgr->GetDownArrowOffset((eFolderID)m_nFolderID);
	}

	CLTGUIFont *pFont;

	//name control
    LTIntPt NamePos(m_NameRect.left,m_NameRect.top);
	if (g_pLayoutMgr->GetNameFontSize((eFolderID)m_nFolderID) == 0)
		pFont = GetSmallFont();
	else if (g_pLayoutMgr->GetNameFontSize((eFolderID)m_nFolderID) == 1)
		pFont = GetMediumFont();
	else
		pFont = GetLargeFont();

    m_pName = CreateTextItem(IDS_SPACER,LTNULL,LTNULL,LTTRUE,pFont);
    m_pName->Enable(LTFALSE);
	m_pName->SetFixedWidth(m_NameRect.right - m_NameRect.left);
    AddFixedControl(m_pName,NamePos,LTFALSE);


	//description control
    LTIntPt DescriptionPos(m_DescriptionRect.left,m_DescriptionRect.top);
	if (g_pLayoutMgr->GetDescriptionFontSize((eFolderID)m_nFolderID) == 0)
		pFont = GetSmallFont();
	else if (g_pLayoutMgr->GetDescriptionFontSize((eFolderID)m_nFolderID) == 1)
		pFont = GetMediumFont();
	else
		pFont = GetLargeFont();
    m_pDescription = CreateStaticTextItem(IDS_SPACER,LTNULL,LTNULL,(m_DescriptionRect.right - m_DescriptionRect.left),0,LTTRUE,pFont);
    m_pDescription->Enable(LTFALSE);
    AddFixedControl(m_pDescription,DescriptionPos,LTFALSE);


	if (strlen(m_sSelectStr) == 0)
	{
        HSTRING hTemp = g_pLTClient->FormatString(IDS_SELECT);
        char *pTemp = g_pLTClient->GetStringData(hTemp);
		strncpy(m_sSelectStr,pTemp,ARRAY_LEN(m_sSelectStr));
        g_pLTClient->FreeString(hTemp);

        hTemp = g_pLTClient->FormatString(IDS_UNSELECT);
        pTemp = g_pLTClient->GetStringData(hTemp);
		strncpy(m_sUnselectStr,pTemp,ARRAY_LEN(m_sUnselectStr));
        g_pLTClient->FreeString(hTemp);

        hTemp = g_pLTClient->FormatString(IDS_REQUIRED);
        pTemp = g_pLTClient->GetStringData(hTemp);
		strncpy(m_sRequiredStr,pTemp,ARRAY_LEN(m_sRequiredStr));
        g_pLTClient->FreeString(hTemp);

        hTemp = g_pLTClient->FormatString(IDS_EQUIPPED);
        pTemp = g_pLTClient->GetStringData(hTemp);
		strncpy(m_sEquippedStr,pTemp,ARRAY_LEN(m_sEquippedStr));
        g_pLTClient->FreeString(hTemp);

        hTemp = g_pLTClient->FormatString(IDS_UNEQUIPPED);
        pTemp = g_pLTClient->GetStringData(hTemp);
		strncpy(m_sUnequippedStr,pTemp,ARRAY_LEN(m_sUnequippedStr));
        g_pLTClient->FreeString(hTemp);

	}

    UseArrows(LTTRUE);

    LTBOOL bSuccess = CBaseFolder::Build();

	m_PageRect = m_ListRect;

	m_UpArrowPos.x = m_ListRect.left + g_UpArrowOffset.x;
	m_UpArrowPos.y = m_ListRect.top + g_UpArrowOffset.y;
	m_pUpArrow->SetPos(m_UpArrowPos);

	m_DownArrowPos.x = m_ListRect.left + g_DownArrowOffset.x;
	m_DownArrowPos.y = m_ListRect.bottom + g_DownArrowOffset.y;
	m_pDownArrow->SetPos(m_DownArrowPos);

	return bSuccess;

}

void CBaseSelectionFolder::Escape()
{
    m_bSaveSelection = LTFALSE;
	CBaseFolder::Escape();
}

void CBaseSelectionFolder::Term()
{

	CBaseFolder::Term();
}


// Renders the folder to a surface
LTBOOL CBaseSelectionFolder::Render(HSURFACE hDestSurf)
{
	int xo = g_pInterfaceResMgr->GetXOffset();
	int yo = g_pInterfaceResMgr->GetYOffset();
	if (m_nLastListItem != kNoSelection)
	{
/*
		CLTGUICtrl *pCtrl = GetControl(m_nLastListItem);
		if (pCtrl)
		{
            LTIntPt pos = pCtrl->GetPos();
			pos.x += xo;
			pos.y += yo;
			HSURFACE hSelSurf = g_pInterfaceResMgr->GetSharedSurface("interface\\SelectBox.pcx");
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hDestSurf,hSelSurf,LTNULL,pos.x,pos.y,m_hTransparentColor);
		}
		
*/
		
	}

	if (!CBaseFolder::Render(hDestSurf))
	{
        return LTFALSE;
	}



	if (m_hPhotoSurf)
        g_pLTClient->DrawSurfaceToSurface(hDestSurf,m_hPhotoSurf,LTNULL,m_PhotoPos.x+xo,m_PhotoPos.y+yo);

/*	for (int slot = 0; slot < m_nNumSlots; slot++)
	{
		CLTGUICtrl *pCtrl = GetControl(m_nFirstSlot - slot);
		if (pCtrl && pCtrl->GetParam2() && !pCtrl->IsSelected())
		{
			CGroupCtrl	*pGroup = (CGroupCtrl*)pCtrl;
            LTIntPt pos = pCtrl->GetPos();
			LTIntPt off = pGroup->GetControlOffset(1);
			pos.x += xo + off.x;
			pos.y += yo + off.y;
			CLTGUITextItemCtrl *pTxt = (CLTGUITextItemCtrl *)pGroup->GetControl(1);
			
			GetSmallRedFont()->Draw(pTxt->GetString(0), hDestSurf, pos.x, pos.y, LTF_JUSTIFY_LEFT);
			

		}
	}

*/
    return LTTRUE;

}

// This is called when the folder gets or loses focus
void CBaseSelectionFolder::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{

        m_bSaveSelection = LTTRUE;
		ClearSelection();


		LTBOOL bAlreadySelected = (m_nSelection != kNoSelection);


		if (!bAlreadySelected)
		{
			if (m_controlArray.GetSize() > 0)
				SetSelection(0);
			else if (m_nFirstSlot < 0)
				SetSelection(m_nFirstSlot);
			else
				SetSelection(kNoSelection);
		}

		UpdateSelection();
        bFinishedSetup = LTTRUE;

		if (!bAlreadySelected)
			SetSelection(GetIndex(m_pContinue));
		ForceMouseUpdate();

	}
	else
	{
		SetSelection(kNoSelection);
		m_pDescription->Purge();
		ClearSlots();
        bFinishedSetup = LTFALSE;

		while (m_sharedSurfaceArray.GetSize() > 0)
		{
			g_pInterfaceResMgr->FreeSharedSurface(m_sharedSurfaceArray[0]);
			m_sharedSurfaceArray.Remove(0);
		}

	}
	CBaseFolder::OnFocus(bFocus);
}

	// Handles a key press.  Returns FALSE if the key was not processed through this method.
	// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
LTBOOL CBaseSelectionFolder::HandleKeyDown(int key, int rep)
{
	if (CBaseFolder::HandleKeyDown(key,rep))
	{
		UpdateSelection();
        return LTTRUE;
	}
    return LTFALSE;

}

uint32 CBaseSelectionFolder::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
    LTBOOL bHandled = LTFALSE;
	switch(dwCommand)
	{
	case FOLDER_CMD_LEFT_ARROW:
		{
			PreviousPage();
			SetSelection(m_nFirstDrawn);
			UpdateSelection();
            bHandled = LTTRUE;
		} break;
	case FOLDER_CMD_RIGHT_ARROW:
		{
			NextPage();
			SetSelection(m_nFirstDrawn);
			UpdateSelection();
            bHandled = LTTRUE;
		} break;

	}

	if (bHandled)
		return 1;
	else
		return  CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);

}

	// Mouse messages
LTBOOL CBaseSelectionFolder::OnLButtonDown(int x, int y)
{
	return CBaseFolder::OnLButtonDown(x, y);
}

LTBOOL CBaseSelectionFolder::OnLButtonUp(int x, int y)
{
	return CBaseFolder::OnLButtonUp(x, y);
}

LTBOOL CBaseSelectionFolder::OnLButtonDblClick(int x, int y)
{
	return CBaseFolder::OnLButtonDblClick(x, y);
}

LTBOOL CBaseSelectionFolder::OnRButtonDown(int x, int y)
{
	return CBaseFolder::OnRButtonDown(x, y);
}

LTBOOL CBaseSelectionFolder::OnRButtonUp(int x, int y)
{
	return CBaseFolder::OnRButtonUp(x, y);
}

LTBOOL CBaseSelectionFolder::OnRButtonDblClick(int x, int y)
{
	return CBaseFolder::OnRButtonDblClick(x,y);
}

LTBOOL CBaseSelectionFolder::OnMouseMove(int x, int y)
{
	if (CBaseFolder::OnMouseMove(x,y))
	{
		UpdateSelection();
        return LTTRUE;
	}
    return LTFALSE;
}

	// Handle input
LTBOOL CBaseSelectionFolder::OnUp()
{
    LTBOOL bHandled = CBaseFolder::OnUp();
	if (bHandled)
		UpdateSelection();
	return bHandled;
}

LTBOOL CBaseSelectionFolder::OnDown()
{
    LTBOOL bHandled = CBaseFolder::OnDown();
	if (bHandled)
		UpdateSelection();
	return bHandled;
}

LTBOOL CBaseSelectionFolder::OnLeft()
{
	return CBaseFolder::OnLeft();
}

LTBOOL CBaseSelectionFolder::OnRight()
{
	return CBaseFolder::OnRight();
}

LTBOOL CBaseSelectionFolder::OnEnter()
{
	return CBaseFolder::OnEnter();
}

LTBOOL CBaseSelectionFolder::OnPageUp()
{
    LTBOOL bHandled = CBaseFolder::OnPageUp();
	if (bHandled)
		UpdateSelection();
	return bHandled;
}

LTBOOL CBaseSelectionFolder::OnPageDown()
{
    LTBOOL bHandled = CBaseFolder::OnPageDown();
	if (bHandled)
		UpdateSelection();
	return bHandled;
}


LTBOOL CBaseSelectionFolder::UpdateSelection()
{
	int nItem = GetSelection();
	if (nItem == m_nLastListItem) return LTFALSE;
	int nLastSlot = (m_nFirstSlot - m_nNumSlots) + 1;
	if (nItem >= 0 || (nItem <= m_nFirstSlot && nItem >= nLastSlot))
	{
		if (GetSelectedControl()->GetParam1() != kEmptySlot)
		{
			m_nLastListItem = nItem;
			return LTTRUE;
		}
	}
	return LTFALSE;
}

void CBaseSelectionFolder::ClearSelection()
{
	m_nLastListItem = kNoSelection;

	m_pName->RemoveAll();

	m_pDescription->RemoveString();

    SetPhotoBitmap(LTNULL);

}

void CBaseSelectionFolder::ClearSlots()
{
	for (int slot = 0; slot < m_nNumSlots; slot++)
	{
		CLTGUICtrl *pCtrl = GetControl(m_nFirstSlot);
		if (pCtrl)
			RemoveFixedControl(pCtrl);
	}
	m_nNumSlots = 0;
	m_nSlotsFilled = 0;
	m_nFirstSlot = 0;

	//reset control positions
	m_PageRect = m_ListRect;


	m_UpArrowPos.x = m_ListRect.left + g_UpArrowOffset.x;
	m_UpArrowPos.y = m_ListRect.top + g_UpArrowOffset.y;
	m_pUpArrow->SetPos(m_UpArrowPos);

}


eFolderID GetNextSelectionFolder(eFolderID eCurrent,int *pnHelpID)
{
	_ASSERT(pnHelpID);

	eFolderID folder[]	=
	{
		FOLDER_ID_OBJECTIVES,
		FOLDER_ID_WEAPONS,
		FOLDER_ID_GADGETS,
		FOLDER_ID_MODS,
		FOLDER_ID_GEAR,
		FOLDER_ID_INVENTORY,
		FOLDER_ID_NONE
	};
	int help[]	=
	{
		IDS_HELP_CONTINUE,
		IDS_HELP_WEAPONS,
		IDS_HELP_GADGETS,
		IDS_HELP_MODS,
		IDS_HELP_GEAR,
		IDS_HELP_INVENTORY,
        LTNULL
	};

    CBaseFolder* pFolder = LTNULL;
	int f = 0;
	while (folder[f] != FOLDER_ID_NONE && folder[f] != eCurrent)
		f++;

	if (folder[f] == FOLDER_ID_NONE)
	{
		*pnHelpID = 0;
		return FOLDER_ID_NONE;
	}

    LTBOOL bFound = LTFALSE;
	f++;
	while (folder[f] != FOLDER_ID_NONE && !bFound)
	{
		pFolder = g_pInterfaceMgr->GetFolderMgr()->GetFolderFromID(folder[f]);
		if (pFolder && pFolder->IsAvailable())
		{
            bFound = LTTRUE;
			*pnHelpID = help[f];
		}
		else
			f++;
	}
	return folder[f];
}

eFolderID GetPreviousSelectionFolder(eFolderID eCurrent)
{

	eFolderID folder[]	=
	{
		FOLDER_ID_OBJECTIVES,
		FOLDER_ID_WEAPONS,
		FOLDER_ID_GADGETS,
		FOLDER_ID_MODS,
		FOLDER_ID_GEAR,
		FOLDER_ID_INVENTORY,
		FOLDER_ID_NONE
	};

    CBaseFolder* pFolder = LTNULL;
	int f = 0;
	while (folder[f] != FOLDER_ID_NONE && folder[f] != eCurrent)
		f++;

	if (folder[f] == FOLDER_ID_NONE || f == 0)
	{
		return FOLDER_ID_OBJECTIVES;
	}

    LTBOOL bFound = LTFALSE;
	f--;
	while (folder[f] != FOLDER_ID_OBJECTIVES && !bFound)
	{
		pFolder = g_pInterfaceMgr->GetFolderMgr()->GetFolderFromID(folder[f]);
		if (pFolder && pFolder->IsAvailable())
		{
            bFound = LTTRUE;
		}
		else
			f--;
	}
	return folder[f];
}


void CBaseSelectionFolder::SetPhotoBitmap(char *pszPhoto)
{
	if (!pszPhoto || !strlen(pszPhoto) )
	{
        m_hPhotoSurf = LTNULL;
		return;
	}
	m_hPhotoSurf = g_pInterfaceResMgr->GetSharedSurface(pszPhoto);
	AddPhotoBitmap(m_hPhotoSurf);
}

void CBaseSelectionFolder::AddPhotoBitmap(HSURFACE hSurf)
{
	if (m_sharedSurfaceArray.FindElement(hSurf) >= m_sharedSurfaceArray.GetSize())
	{
		m_sharedSurfaceArray.Add(hSurf);
	}
}

int CBaseSelectionFolder::FindItemIndex(int nItemId)
{
	if (m_nFirstSlot < 0)
	{
		for (int slot = 0; slot < m_nNumSlots; slot++)
		{
			CLTGUICtrl *pCtrl = GetControl(m_nFirstSlot - slot);
			if (pCtrl && (int)pCtrl->GetParam1() == nItemId)
			{
				return (m_nFirstSlot - slot);
			}
		}
	}

	for (int i = 0; i < (int)m_controlArray.GetSize(); i++)
	{
		if ((int)m_controlArray[i]->GetParam1() == nItemId)
			return i;
	}

	return kNoSelection;

}

int CBaseSelectionFolder::AddToSlot(int nItemId, int nNameId, LTBOOL bRequired)
{
	if (m_nSlotsFilled >= m_nNumSlots) return kNoSelection;
	int index = FindItemIndex(nItemId);
	if (index < 0 && index != kNoSelection) return index;

	CLTGUIFont *pFont = GetLargeFont();
	if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 0)
		pFont = GetSmallFont();
	else if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 1)
		pFont = GetMediumFont();


	CLTGUITextItemCtrl* pCtrl = CreateTextItem(nNameId,FOLDER_CMD_SELECT_SLOT, nNameId, LTFALSE, pFont);
	if (bRequired)
		pCtrl->SetColor(kWhite,SETRGB(255,0,0),SETRGB(96,32,32));
	CBitmapCtrl *pBmp = debug_new(CBitmapCtrl);
	if (bRequired)
        pBmp->Create(g_pLTClient,"interface\\slot_lock.pcx");
	else
        pBmp->Create(g_pLTClient,"interface\\slot_full.pcx");
		

	LTIntPt offset(0,0);
	int nWidth = (m_ListRect.right - m_ListRect.left) + (2*m_SlotOffset.x);
	int nHeight = Max(pBmp->GetHeight(),pCtrl->GetHeight()) + 2;
	CGroupCtrl*	pGroup = CreateGroup(nWidth,nHeight,nNameId);

	offset.y = (nHeight - pBmp->GetHeight()) / 2;
	pGroup->AddControl(pBmp,offset);

	offset.x = pBmp->GetWidth() + 4;
	offset.y = (nHeight - pCtrl->GetHeight()) / 2;
	pGroup->AddControl(pCtrl,offset,LTTRUE);

	pCtrl->SetParam1(nItemId);
	pCtrl->SetParam2(bRequired);
	pGroup->SetParam1(nItemId);
	pGroup->SetParam2(bRequired);

	// add new control to list
	m_nSlotsFilled++;

	//look for the proper slot
	int nSlotIndex = kNoSelection;
	if (m_nFirstSlot < 0)
	{
		LTBOOL bFound = LTFALSE;
		for (int slot = 0; !bFound && slot < m_nNumSlots; slot++)
		{
			CLTGUICtrl *pCtrl = GetControl(m_nFirstSlot - slot);
			if (pCtrl)
			{
				LTBOOL bPriority = bRequired && !pCtrl->GetParam2();
				LTBOOL bEqual = (bRequired  == (LTBOOL)pCtrl->GetParam2());
				if (bPriority  || (bEqual && (int)pCtrl->GetParam1() > nItemId) || pCtrl->GetParam1() == kEmptySlot)
				{
					nSlotIndex = m_nFirstSlot - slot;
					bFound = LTTRUE;
				}
				
			}
		}
	}
	
	if (nSlotIndex == kNoSelection)
	{
		//this item goes at the end, so add a new slot there
		LTIntPt slotPos(GetPageLeft(), GetPageTop());
		slotPos.x -= m_SlotOffset.x;
		slotPos.y -= m_SlotOffset.y;

		m_PageRect.top += (nHeight + 2);
		m_UpArrowPos.y = m_PageRect.top + g_UpArrowOffset.y;

		if (m_pUpArrow)
		{
			m_pUpArrow->SetPos(m_UpArrowPos);
		}

		int nNewIndex = AddFixedControl(pGroup,slotPos);
		if (m_nFirstSlot == 0) m_nFirstSlot = nNewIndex;
		return nNewIndex;
	}
	else
	{
		//we're inserting in the middle of the list
		int nFixedIndex = FixedIndex(nSlotIndex);
		// remember what's in the slot right now
		CGroupCtrl *pTemp = (CGroupCtrl*)m_fixedControlArray[nFixedIndex];

		// if we'not holding an empty slot...
		while (pTemp && pGroup->GetParam1() != kEmptySlot)
		{
			//put the item we're holding into the current slot
			m_fixedControlArray.SetAt(nFixedIndex,pGroup);
			LTIntPt slotPos	= pTemp->GetPos();
			pGroup->SetPos(slotPos);

			// hold onto the item we just displaced
			pGroup = pTemp;

			//look at the next item
			nFixedIndex++;
			if (nFixedIndex < (int)m_fixedControlArray.GetSize())
				pTemp = (CGroupCtrl*)m_fixedControlArray[nFixedIndex];
			else
				pTemp = LTNULL;
		}

		if (pGroup && pGroup->GetParam1() == kEmptySlot)
		{
			//were holding an extra empty slot, so just get rid of it
			debug_delete(pGroup);
		}
		else
		{
			//we didn't find an empty slot, so add a new one
			LTIntPt slotPos(GetPageLeft(), GetPageTop());
			slotPos.x -= m_SlotOffset.x;
			slotPos.y -= m_SlotOffset.y;

			m_PageRect.top += (nHeight + 2);
			m_UpArrowPos.y = m_PageRect.top + g_UpArrowOffset.y;

			if (m_pUpArrow)
			{
				m_pUpArrow->SetPos(m_UpArrowPos);
			}

			int nNewIndex = AddFixedControl(pGroup,slotPos);
		}
		return nSlotIndex;

	}


}


int CBaseSelectionFolder::AddEmptySlot()
{
	CLTGUIFont *pFont;
	if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 0)
		pFont = GetSmallFont();
	else if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 1)
		pFont = GetMediumFont();
	else
		pFont = GetLargeFont();

	CLTGUITextItemCtrl* pCtrl = CreateTextItem(IDS_EMPTY_SLOT,LTNULL,LTNULL,LTFALSE,pFont);
	CBitmapCtrl *pBmp = debug_new(CBitmapCtrl);
    pBmp->Create(g_pLTClient,"interface\\slot_empty.pcx");
		

	LTIntPt offset(0,0);
	int nWidth = (m_ListRect.right - m_ListRect.left) + (2*m_SlotOffset.x);
	int nHeight = Max(pBmp->GetHeight(),pCtrl->GetHeight()) + 2;
	CGroupCtrl*	pGroup = CreateGroup(nWidth,nHeight,LTNULL);

	offset.y = (nHeight - pBmp->GetHeight()) / 2;
	pGroup->AddControl(pBmp,offset);

	offset.x = pBmp->GetWidth() + 4;
	offset.y = (nHeight - pCtrl->GetHeight()) / 2;
	pGroup->AddControl(pCtrl,offset,LTTRUE);

	pGroup->SetParam1(kEmptySlot);
	pGroup->SetParam2(0);

	LTIntPt slotPos(GetPageLeft(), GetPageTop());
	slotPos.x -= m_SlotOffset.x;
	slotPos.y -= m_SlotOffset.y;

	m_PageRect.top += (nHeight + 2);
	m_UpArrowPos.y = m_PageRect.top + g_UpArrowOffset.y;;
	if (m_pUpArrow)
	{
		m_pUpArrow->SetPos(m_UpArrowPos);
	}

	int nNewIndex = AddFixedControl(pGroup,slotPos);
	if (m_nFirstSlot == 0) m_nFirstSlot = nNewIndex;
	return nNewIndex;


}



void CBaseSelectionFolder::RemoveFromSlot(int nItemId)
{
	int index = FindItemIndex(nItemId);
	if (index >= 0 || index == kNoSelection) return;

	//create a new empty slot to replace whatever we're removing
	CLTGUIFont *pFont;
	if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 0)
		pFont = GetSmallFont();
	else if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 1)
		pFont = GetMediumFont();
	else
		pFont = GetLargeFont();

	CLTGUITextItemCtrl* pCtrl = CreateTextItem(IDS_EMPTY_SLOT,LTNULL,LTNULL,LTFALSE,pFont);
	CBitmapCtrl *pBmp = debug_new(CBitmapCtrl);
    pBmp->Create(g_pLTClient,"interface\\slot_empty.pcx");
		

	LTIntPt offset(0,0);
	int nWidth = (m_ListRect.right - m_ListRect.left) + (2*m_SlotOffset.x);
	int nHeight = Max(pBmp->GetHeight(),pCtrl->GetHeight()) + 2;
	CGroupCtrl*	pGroup = CreateGroup(nWidth,nHeight,LTNULL);

	offset.y = (nHeight - pBmp->GetHeight()) / 2;
	pGroup->AddControl(pBmp,offset);

	offset.x = pBmp->GetWidth() + 4;
	offset.y = (nHeight - pCtrl->GetHeight()) / 2;
	pGroup->AddControl(pCtrl,offset,LTTRUE);

	pCtrl->SetParam1(kEmptySlot);
	pCtrl->SetParam2(0);
	pGroup->SetParam1(kEmptySlot);
	pGroup->SetParam2(0);

	int nFixedIndex = FixedIndex(index);
	//remember where the item to be removed is
	CLTGUICtrl *pOldCtrl = m_fixedControlArray[nFixedIndex];
	LTIntPt slotPos	= pOldCtrl->GetPos();
	LTIntPt nextPos;

	//get rid of the item
	debug_delete(pOldCtrl);

	//if there are non-empty slots, slide them up
	int nNextIndex = nFixedIndex + 1;
	if (nNextIndex < (int)m_fixedControlArray.GetSize())
	{
		CLTGUICtrl *pNextCtrl = m_fixedControlArray[nNextIndex];
		//while there is a next slot and it's not empty
		while (nNextIndex < (int)m_fixedControlArray.GetSize() && pNextCtrl && pNextCtrl->GetParam1() != kEmptySlot)
		{
			//remember where it was
			nextPos	= pNextCtrl->GetPos();
			//move the control up
			m_fixedControlArray.SetAt(nFixedIndex,pNextCtrl);
			pNextCtrl->SetPos(slotPos);
			// go on to the next
			slotPos = nextPos;
			nNextIndex++; 
			nFixedIndex++;
			if (nNextIndex < (int)m_fixedControlArray.GetSize())
				pNextCtrl = m_fixedControlArray[nNextIndex];
			else
				pNextCtrl = LTNULL;

		}
	}

	//add the empty slot
	pGroup->SetPos(slotPos);
	m_fixedControlArray.SetAt(nFixedIndex,pGroup);
	m_nSlotsFilled--;


}



int CBaseSelectionFolder::AddItem(int nItemId, int nNameId)
{

	if (m_nFirstSlot < 0)
	{
		for (int slot = 0; slot < m_nNumSlots; slot++)
		{
			CLTGUICtrl *pCtrl = GetControl(m_nFirstSlot - slot);
			if (pCtrl && (int)pCtrl->GetParam1() == nItemId)
			{
				return kNoSelection;
			}
		}
	}

	int i = 0;
	while (i < (int)m_controlArray.GetSize() && (int)m_controlArray[i]->GetParam1() < nItemId)
		 i++;
	if (i < (int)m_controlArray.GetSize() && (int)m_controlArray[i]->GetParam1() == nItemId)
		return i;

	CLTGUIFont *pFont;
	if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 0)
		pFont = GetSmallFont();
	else if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 1)
		pFont = GetMediumFont();
	else
		pFont = GetLargeFont();
	CLTGUITextItemCtrl* pCtrl = CreateTextItem(nNameId,FOLDER_CMD_SELECT_ITEM,nNameId,LTFALSE,pFont);
	pCtrl->SetParam1(nItemId);

	if (i >= (int)m_controlArray.GetSize())
	{
		return AddFreeControl(pCtrl);
	}
	else
	{
		m_controlArray.Insert(i,pCtrl);
		return i;
	}

}


void CBaseSelectionFolder::RemoveItem(int nItemId)
{
	int index = FindItemIndex(nItemId);
	if (index < 0) return;
	if (m_nSelection == index)
		SetSelection(kNoSelection);
	m_controlArray[index]->Destroy();
	debug_delete(m_controlArray[index]);
	m_controlArray.Remove(index);
}


void CBaseSelectionFolder::ItemToSlot(int nItemId, int nNameId)
{
	if (m_nSlotsFilled >= m_nNumSlots) return;
	SetSelection(kNoSelection);
	RemoveItem(nItemId);
	int nIndex = AddToSlot(nItemId,nNameId,LTFALSE);
	CalculateLastDrawn();
	SetSelection(nIndex);
	UpdateSelection();
	CheckArrows();
}
void CBaseSelectionFolder::SlotToItem(int nItemId, int nNameId)
{
	SetSelection(kNoSelection);
	RemoveFromSlot(nItemId);
	int nIndex = AddItem(nItemId,nNameId);
	CalculateLastDrawn();
	SetSelection(nIndex);
	UpdateSelection();
	CheckArrows();
}

