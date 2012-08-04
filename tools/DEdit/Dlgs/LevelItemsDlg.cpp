#include "bdefs.h"
#include "regiondoc.h"
#include "levelitemsdlg.h"
#include "edithelpers.h"

#if _MSC_VER >= 1300
#include <fstream>
#else
#include <fstream.h>
#endif

static bool GetRealProp(CWorldNode* pNode, const char* pPropName, CReal& fVal)
{
	ASSERT(pNode);

	CBaseProp* pProp = pNode->m_PropList.GetProp(pPropName);

	if(pProp == NULL)
		return false;

	if((pProp->GetType() == LT_PT_REAL) || (pProp->GetType() == LT_PT_LONGINT))
		fVal = ((CRealProp*)pProp)->m_Value;
	else
		return false;

	return true;
}


static bool GetStringProp(CWorldNode* pNode, const char* pPropName, CString& sStr)
{
	ASSERT(pNode);

	CBaseProp* pProp = pNode->m_PropList.GetProp(pPropName);

	if(pProp == NULL)
		return false;

	if(pProp->GetType() == LT_PT_STRING)
		sStr = ((CStringProp*)pProp)->m_String;
	else
		return false;

	return true;
}


//fill out the message map

BEGIN_MESSAGE_MAP(CLevelItemsDlg, CDialog)

	//buttons
	ON_BN_CLICKED(IDC_BUTTON_SCAN_LEVEL, OnScanLevel)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnSave)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_ITEM_OBJECT, OnSelectItemObject)
	
	//item list
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ITEMS, OnItemSelectionChanged)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_ITEMS, OnSortItems)

	//item object list
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_ITEM_OBJECTS, OnSortItemObjects)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ITEM_OBJECTS, OnItemObjectSelectionChanged)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST_ITEM_OBJECTS, OnActivateItemObject)

END_MESSAGE_MAP()


CLevelItemsDlg::CLevelItemsDlg() :
	CDialog(IDD_LEVEL_ITEMS),
	m_pSrcDoc(NULL)
{
}

CLevelItemsDlg::~CLevelItemsDlg()
{
}

//used for handling the specific object types
void CLevelItemsDlg::ProcessAmmoBox(CBaseEditObj* pObject, CWorldNode* pSelNode)
{
	uint32 nCurrAmmo = 1;

	while(1)
	{
		CString sAmmo;
		CReal fAmmoCount;

		CString sAmmoPropName, sAmmoCountPropName;
		sAmmoPropName.Format("AmmoType%d", nCurrAmmo);
		sAmmoCountPropName.Format("AmmoCount%d", nCurrAmmo);

		if(	GetStringProp(pObject, sAmmoPropName, sAmmo) &&
			GetRealProp(pObject, sAmmoCountPropName, fAmmoCount) &&
			(sAmmo.CompareNoCase("<none>") != 0))
		{
			AddItemObject(pObject, pSelNode, sAmmo, (uint32)(fAmmoCount + 0.5f));
		}
		else
		{
			break;
		}

		nCurrAmmo++;
	}
}

void CLevelItemsDlg::ProcessWeaponItem(CBaseEditObj* pObject, CWorldNode* pSelNode)
{
	CString sWeapon;	
	if(GetStringProp(pObject, "WeaponType", sWeapon))
	{
		AddItemObject(pObject, pSelNode, sWeapon, 1);
	}
}

void CLevelItemsDlg::ProcessGearItem(CBaseEditObj* pObject, CWorldNode* pSelNode)
{
	CString sGear;
	if(GetStringProp(pObject, "GearType", sGear))
	{
		AddItemObject(pObject, pSelNode, sGear, 1);
	}
}

void CLevelItemsDlg::ProcessModItem(CBaseEditObj* pObject, CWorldNode* pSelNode)
{
	CString sMod;
	if(GetStringProp(pObject, "ModType", sMod))
	{
		AddItemObject(pObject, pSelNode, sMod, 1);
	}
}

//used for handling the specific object types
void CLevelItemsDlg::ProcessObject(CBaseEditObj* pObject, CWorldNode* pSelNode)
{
	//filter based upon the class type
	if(stricmp(pObject->GetClassName(), "AmmoBox") == 0)
		ProcessAmmoBox(pObject, pSelNode);
	else if(stricmp(pObject->GetClassName(), "WeaponItem") == 0)
		ProcessWeaponItem(pObject, pSelNode);
	else if(stricmp(pObject->GetClassName(), "GearItem") == 0)
		ProcessGearItem(pObject, pSelNode);
	else if(stricmp(pObject->GetClassName(), "ModItem") == 0)
		ProcessModItem(pObject, pSelNode);
}

//used for searching through the node heirarchy for objects to add into the list
void CLevelItemsDlg::AddObjectsR(CWorldNode* pCurrNode, CWorldNode* pSelNode)
{
	//see if this is an object
	if(pCurrNode->GetType() == Node_Object)
	{
		ProcessObject(pCurrNode->AsObject(), (pSelNode) ? pSelNode : pCurrNode);
	}
	else if(pCurrNode->GetType() == Node_PrefabRef)
	{
		AddObjectsR(const_cast<CWorldNode*>(((CPrefabRef*)pCurrNode)->GetPrefabTree()), (pSelNode) ? pSelNode : pCurrNode);
	}

	//recurse into the children
	for(GPOS pos = pCurrNode->m_Children; pos; )
	{
		AddObjectsR(pCurrNode->m_Children.GetNext(pos), pSelNode);
	}
}

//message handlers
void CLevelItemsDlg::OnScanLevel()
{
	//get the active document
	CRegionDoc* pDoc = ::GetActiveRegionDoc();

	//we can't scan a level if there is no level!
	if(pDoc == NULL)
	{
		MessageBox("A level must be opened before this operation can be performed", "Error Scanning Level", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	//show the user a wait cursor
	CWaitCursor WaitCursor;

	//clear out the old item array
	m_Items.SetSize(0);

	//add all the objects into the list
	AddObjectsR(pDoc->GetRegion()->GetRootNode(), NULL);

	m_pSrcDoc = pDoc;	

	//update the list
	UpdateList();
}

//updates the list with all the items
void CLevelItemsDlg::UpdateList()
{
	//clear out the lists
	GetItemList()->DeleteAllItems();
	GetItemObjectList()->DeleteAllItems();

	//now run through and fill in the item list
	for(uint32 nCurrItem = 0; nCurrItem < m_Items.GetSize(); nCurrItem++)
	{
		int nNewPos = GetItemList()->InsertItem(0, m_Items[nCurrItem].m_sName);

		CString sCount;
		sCount.Format("%d", m_Items[nCurrItem].m_nCount);
		GetItemList()->SetItemText(nNewPos, 1, sCount);
		GetItemList()->SetItemData(nNewPos, (DWORD)&m_Items[nCurrItem]);
	}

	//disable the selection button
	GetDlgItem(IDC_BUTTON_SELECT_ITEM_OBJECT)->EnableWindow(FALSE);
}

void CLevelItemsDlg::OnSelectItemObject()
{
	//must have a source doc
	if((m_pSrcDoc == NULL) || (m_pSrcDoc->GetRegion() == NULL))
	{
		return;
	}

	//clear all existing selections
	m_pSrcDoc->GetRegion()->ClearSelections();

	//run through the selection and for each item selected, in the list,
	//we need to select it in the world
	CListCtrl* pList = GetItemObjectList();

	POSITION Pos = pList->GetFirstSelectedItemPosition();
	while(Pos)
	{
		//get the error object associated with this item
		int nIndex				= pList->GetNextSelectedItem(Pos);
		DWORD nItemData			= pList->GetItemData(nIndex);
		CItemObject* pObject	= (CItemObject*)nItemData;

		//now we need to select that node in the world
		CWorldNode* pNode = m_pSrcDoc->GetRegion()->FindNodeByID(pObject->m_nObjectID);
		if(pNode)
		{
			m_pSrcDoc->GetRegion()->SelectNode(pNode, TRUE);
		}
	}

	m_pSrcDoc->NotifySelectionChange( );
	m_pSrcDoc->RedrawAllViews( );
}


void CLevelItemsDlg::OnOK()
{
	//hide the window
	ShowWindow(SW_HIDE);
}

void CLevelItemsDlg::OnCancel()
{
	//hide the window
	ShowWindow(SW_HIDE);
}

//retreives the list control for the items
CListCtrl* CLevelItemsDlg::GetItemList()
{
	return ((CListCtrl*)GetDlgItem(IDC_LIST_ITEMS));
}

//retreives the list control for the item objects
CListCtrl* CLevelItemsDlg::GetItemObjectList()
{
	return ((CListCtrl*)GetDlgItem(IDC_LIST_ITEM_OBJECTS));
}

//notification when the dialog needs to be set up
BOOL CLevelItemsDlg::OnInitDialog()
{
	DWORD nAdditionalStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;

	//setup the item list first
	CListCtrl* pItemList = GetItemList();

	pItemList->SetExtendedStyle(pItemList->GetExtendedStyle() | nAdditionalStyle);

	pItemList->InsertColumn(0, "Type",	LVCFMT_LEFT, 130);
	pItemList->InsertColumn(1, "Count",	LVCFMT_LEFT, 68);

	//now setup the item object list
	CListCtrl* pObjectList = GetItemObjectList();

	pObjectList->SetExtendedStyle(pObjectList->GetExtendedStyle() | nAdditionalStyle);

	pObjectList->InsertColumn(0, "Object",	LVCFMT_LEFT, 120);
	pObjectList->InsertColumn(1, "Type",	LVCFMT_LEFT, 80);
	pObjectList->InsertColumn(2, "Ammo",	LVCFMT_LEFT, 70);


	((CButton*)GetDlgItem(IDC_BUTTON_SELECT_ITEM_OBJECT))->EnableWindow(FALSE);

	UpdateList();

	return TRUE;
}

//update the object list
void CLevelItemsDlg::UpdateObjectList(CItem* pItem)
{
	//clear out all the old item objects
	GetItemObjectList()->DeleteAllItems();

	if(!pItem)
		return;

	for(uint32 nCurrObject = 0; nCurrObject < pItem->m_Objects.GetSize(); nCurrObject++)
	{
		CItemObject& Object = pItem->m_Objects[nCurrObject];

		CString sCount;
		sCount.Format("%d", Object.m_nCount);

		int nIndex = GetItemObjectList()->InsertItem(0, Object.m_sName);
		GetItemObjectList()->SetItemText(nIndex, 1, Object.m_sClass);
		GetItemObjectList()->SetItemText(nIndex, 2, sCount);

		//and setup the pointer
		GetItemObjectList()->SetItemData(nIndex, (DWORD)&Object);
	}
}

void CLevelItemsDlg::OnItemSelectionChanged(NMHDR * pNotifyStruct, LRESULT * pResult )
{
	CListCtrl* pList = GetItemList();

	//see if any can be selected
	POSITION Pos = pList->GetFirstSelectedItemPosition();
	while(Pos)
	{
		//get the error object associated with this item
		int nIndex = pList->GetNextSelectedItem(Pos);

		//get the item data
		CItem* pItem = (CItem*)pList->GetItemData(nIndex);
		UpdateObjectList(pItem);
	}
}

void CLevelItemsDlg::OnItemObjectSelectionChanged(NMHDR * pNotifyStruct, LRESULT * pResult )
{
	bool bEnabled = (GetItemObjectList()->GetFirstSelectedItemPosition() != NULL);

	//enable/disable buttons based upon the count
	GetDlgItem(IDC_BUTTON_SELECT_ITEM_OBJECT)->EnableWindow(bEnabled);
}

int CALLBACK CLevelItemsDlg::SortItemCallback(LPARAM lParam1, LPARAM lParam2, LPARAM nCol)
{
	CItem* p1 = (CItem*)lParam1;
	CItem* p2 = (CItem*)lParam2;

	//see what we are sorting upon
	switch(nCol)
	{
	case 0:
		//the name
		return strcmp(p1->m_sName, p2->m_sName);
		break;

	case 1:
		//the count
		return (p1->m_nCount - p2->m_nCount);
		break;

	default:
		//huh?
		break;
	}

	return 0;
}

int CALLBACK CLevelItemsDlg::SortItemObjectsCallback(LPARAM lParam1, LPARAM lParam2, LPARAM nCol)
{
	CItemObject* p1 = (CItemObject*)lParam1;
	CItemObject* p2 = (CItemObject*)lParam2;

	//see what we are sorting upon
	switch(nCol)
	{
	case 0:
		//the name
		return strcmp(p1->m_sName, p2->m_sName);
		break;

	case 1:
		//the type
		return strcmp(p1->m_sClass, p2->m_sClass);
		break;

	case 2:
		//the count
		return (p1->m_nCount - p2->m_nCount);
		break;

	default:
		//huh?
		break;
	}

	return 0;
}

void CLevelItemsDlg::OnSortItems(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pNotifyStruct; 

	//figure out which column was clicked
	DWORD nCol = pnmv->iSubItem;

	//now we need to sort the list based upon that column
	GetItemList()->SortItems(SortItemCallback, nCol);
}

void CLevelItemsDlg::OnSortItemObjects(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pNotifyStruct; 

	//figure out which column was clicked
	DWORD nCol = pnmv->iSubItem;

	//now we need to sort the list based upon that column
	GetItemObjectList()->SortItems(SortItemObjectsCallback, nCol);
}


void CLevelItemsDlg::OnActivateItemObject(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	OnSelectItemObject();
}

//this should be called when a region document is closed so that the dialog can
//potentially clear out its list
void CLevelItemsDlg::NotifyDocumentClosed(CRegionDoc* pDoc)
{
	if(pDoc == m_pSrcDoc)
	{
		//our document is going away
		//clear out the lists
		m_Items.SetSize(0);
		UpdateList();
		m_pSrcDoc = NULL;
	}
}

void CLevelItemsDlg::OnSave()
{
	CFileDialog Dlg(FALSE, ".csv", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "CSV Files(*.csv)|*.csv|All Files(*.*)|*.*||");

	if(Dlg.DoModal() != IDOK)
		return;

#if _MSC_VER >= 1300
	std::ofstream OutFile(Dlg.GetPathName());
#else
	ofstream OutFile(Dlg.GetPathName());
#endif

	if(!OutFile.good())
	{
		CString sError;
		sError.Format("Error opening file %s for writing", Dlg.GetPathName());
		MessageBox(sError, "Error opening file", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

#if _MSC_VER >= 1300
	OutFile << "Item, Total, Object, Type, Count" << std::endl;
#else
	OutFile << "Item, Total, Object, Type, Count" << endl;
#endif

	//alright, now go ahead and save all items and their objects
	for(uint32 nCurrItem = 0; nCurrItem < m_Items.GetSize(); nCurrItem++)
	{
		CItem& Item = m_Items[nCurrItem];

#if _MSC_VER >= 1300
		OutFile << Item.m_sName << ", " << Item.m_nCount << std::endl;
#else
		OutFile << Item.m_sName << ", " << Item.m_nCount << endl;
#endif

		for(uint32 nCurrObject = 0; nCurrObject < Item.m_Objects.GetSize(); nCurrObject++)
		{
			CItemObject& Object = Item.m_Objects[nCurrObject];

#if _MSC_VER >= 1300
			OutFile << ",, " << Object.m_sName << ", " << Object.m_sClass << ", " << Object.m_nCount << std::endl;
#else
			OutFile << ",, " << Object.m_sName << ", " << Object.m_sClass << ", " << Object.m_nCount << endl;
#endif
		}
	}
}

//called to add an object that references a specific object
void CLevelItemsDlg::AddItemObject(CBaseEditObj* pObject, CWorldNode* pSelNode, const char* pszItemName, uint32 nItemCount)
{
	//see if we have a matching item
	CItem* pItem = NULL;

	for(uint32 nCurrItem = 0; nCurrItem < m_Items.GetSize(); nCurrItem++)
	{
		if(m_Items[nCurrItem].m_sName.CompareNoCase(pszItemName) == 0)
		{
			//we have a match
			pItem = &m_Items[nCurrItem];
			break;
		}
	}

	//see if we need to add a new item
	if(pItem == NULL)
	{
		m_Items.Add(CItem());

		pItem = &m_Items[m_Items.GetSize() - 1];
		pItem->m_sName	= pszItemName;
		pItem->m_nCount = 0;
	}

	//alright, so lets add our object to the item
	pItem->m_nCount += nItemCount;

	bool bAddNewObject = true;

	//see if we can just add it to an existing one
	for(uint32 nCurrObject = 0; nCurrObject < pItem->m_Objects.GetSize(); nCurrObject++)
	{
		CItemObject& Object = pItem->m_Objects[nCurrObject];

		if(	(Object.m_sName.CompareNoCase(pObject->GetName()) == 0) &&
			(Object.m_sClass.CompareNoCase(pObject->GetClassName()) == 0))
		{
			//we have a match
			Object.m_nCount += nItemCount;
			bAddNewObject = false;
			break;
		}
	}

	if(bAddNewObject)
	{
		CItemObject NewObject;
		NewObject.m_sClass		= pObject->GetClassName();
		NewObject.m_sName		= pObject->GetName();
		NewObject.m_nObjectID	= pSelNode->GetUniqueID();
		NewObject.m_nCount		= nItemCount;

		pItem->m_Objects.Add(NewObject);
	}
}

