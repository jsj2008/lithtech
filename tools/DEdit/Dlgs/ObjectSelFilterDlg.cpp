#include "bdefs.h"
#include "dedit.h"
#include "editobjects.h"
#include "resource.h"
#include "objectselfilterdlg.h"
#include "edithelpers.h"
#include "editprojectmgr.h"
#include "stringdlg.h"

#define PREFAB_CLASS_NAME "Prefab"

//--------------------------------------------------------------------
// Preset parsing utilities

#define CURRENT_PRESET_VERSION		1
#define PRESET_LIST_KEY				"PresetListV1"

CString NextPresetString(CString& str)
{
	//find the first comma
	int nComma = str.Find(',');

	CString sRV;

	if(nComma == -1)
	{
		sRV = str;
		str.Empty();
	}
	else
	{
		sRV = str.Left(nComma);
		str = str.Mid(nComma + 1);

		str.TrimLeft();
	}

	sRV.TrimLeft();
	sRV.TrimRight();

	return sRV;
}

int32 NextPresetInt(CString& str)
{
	CString sVal = NextPresetString(str);
	return atoi(sVal);
}

void AddPresetString(CString& sPreset, const char* pszVal)
{
	sPreset += pszVal;
	sPreset += ',';
}

void AddPresetInt(CString& sPreset, int32 nVal)
{
	CString sFormat;
	sFormat.Format("%d", nVal);
	AddPresetString(sPreset, sFormat);
}

//--------------------------------------------------------------------
// CObjectSelFilterDlg

BEGIN_MESSAGE_MAP(CObjectSelFilterDlg, CDialog)

	ON_BN_CLICKED(IDC_BUTTON_SELALL, OnReset)
	ON_BN_CLICKED(IDC_BUTTON_SELNONE, OnClear)
	ON_BN_CLICKED(IDC_BUTTON_SELINVERT, OnInvert)
	ON_BN_CLICKED(IDC_CHECK_MATCH_NAME, OnUpdateData)
	ON_BN_CLICKED(IDC_CHECK_MATCH_CLASS, OnUpdateData)
	ON_BN_CLICKED(IDC_BUTTON_ADDPRESET, OnAddPreset)
	ON_BN_CLICKED(IDC_BUTTON_REMOVEPRESET, OnRemovePreset)
	ON_EN_CHANGE(IDC_EDIT_OBJECTNAME, OnUpdateData)
	ON_CBN_SELCHANGE(IDC_COMBO_PRESETS, OnPresetChanged)

	ON_NOTIFY(LVN_ITEMCHANGING, IDC_LIST_CLASSES, OnItemStateChanged)

END_MESSAGE_MAP()


CObjectSelFilterDlg::CObjectSelFilterDlg() :
	CDialog(IDD_OBJECTSELFILTER)
{
	m_bCheckName			= FALSE;
	m_bCheckClass			= FALSE;
	m_bIgnoreCheckChanges	= false;
}

CObjectSelFilterDlg::~CObjectSelFilterDlg()
{
}

BOOL CObjectSelFilterDlg::OnInitDialog()
{
	//get the list
	CListCtrl*	pList = GetClassList();

	RECT rList;
	pList->GetWindowRect(&rList);

	pList->InsertColumn(0, "Col0");

	//setup some list styles
	pList->SetExtendedStyle(	pList->GetExtendedStyle() |
								LVS_EX_CHECKBOXES |
								LVS_EX_FULLROWSELECT);

	OnUpdateEnabled();

	FillPresetList();

	//disable the remove preset button by default
	GetRemovePresetButton()->EnableWindow(FALSE);

	return TRUE;
}

void CObjectSelFilterDlg::OnOK()
{
	ShowWindow(SW_HIDE);
}

void CObjectSelFilterDlg::OnCancel()
{
	ShowWindow(SW_HIDE);
}

bool CObjectSelFilterDlg::InsertClass(const char* pszClassName, uint32& nMaxExtent)
{
	int nItem = GetClassList()->InsertItem(0, pszClassName);

	//update our max extent
	if(nItem == -1)
	{
		return false;
	}

	//to make things even more wonderful, you can't get the width of the checkbox
	//since it is different than the standard dims and is not the same as the icon
	//size, so we have a hardcoded value
	static const uint32 knIconWidth = 25;
	uint32 nItemExtents = knIconWidth + GetClassList()->GetStringWidth(pszClassName);

	nMaxExtent = LTMAX(nItemExtents, nMaxExtent);

	return true;
}

//updates the class listing based upon the existing list of classes as held by the project
bool CObjectSelFilterDlg::UpdateClassListing()
{
	if(!GetProject())
		return false;

	//alright, we now need to convert the prefab file list over to structures we can use
	uint32 nNumClasses = GetProject()->m_nClassDefs;

	//our maximum extent
	uint32 nMaxExtent = 0;

	for(uint32 nCurrClass = 0; nCurrClass < nNumClasses; nCurrClass++)
	{
		if(!(GetProject()->m_ClassDefs[nCurrClass].m_ClassFlags & CF_HIDDEN))
		{
			InsertClass(GetProject()->m_ClassDefs[nCurrClass].m_ClassName, nMaxExtent);
		}
	}

	InsertClass(PREFAB_CLASS_NAME, nMaxExtent);

	//update the column width to our extents
	GetClassList()->SetColumnWidth(0, nMaxExtent);

	//default them all to checked
	OnReset();

	return true;
}

//clears the class listing
bool CObjectSelFilterDlg::ClearClassListing()
{
	GetClassList()->DeleteAllItems();
	return true;
}

//querying whether or not an object passes the criteria for selection
bool CObjectSelFilterDlg::CanSelectObject(CWorldNode* pObj)
{
	//make sure that the object is an object or a prefab
	if((pObj->GetType() != Node_Object) && (pObj->GetType() != Node_PrefabRef))
	{
		//this shouldn't actually ever happen
		assert(false);
		return true;
	}

	//filter on the name
	if(m_bCheckName)
	{
		const char* pszName = pObj->GetName();

		//run through doing a custom comparison so we can expand out any * on the end
		uint32 nStrLen = m_sNameFilter.GetLength();

		//ignore empty strings as a wildcard
		if(nStrLen)
		{
			bool bMatch = false;

			for(int32 nCurrChar = 0; nCurrChar < nStrLen; nCurrChar++)
			{
				//see if we have an asterisk to allow for anything
				if(m_sNameFilter[nCurrChar] == '*')
				{
					//we have a successful match at this point
					bMatch = true;
					break;
				}

				//check for any mismatch, the name filter should already be uppercase
				if(toupper(pszName[nCurrChar]) != m_sNameFilter[nCurrChar])
				{
					return false;
				}
			}

			if(!bMatch)
			{
				//make sure the lenghts are equivilant
				if(strlen(pszName) != nStrLen)
					return false;
			}
		}
	}

	//see if we are filtering on the class
	if(m_bCheckClass)
	{
		LVFINDINFO FindInfo;
		FindInfo.flags	= LVFI_STRING;

		if(pObj->GetType() == Node_PrefabRef)
		{
			FindInfo.psz	= PREFAB_CLASS_NAME;
		}
		else
		{
			FindInfo.psz	= pObj->GetClassName();
		}

		int nFind = GetClassList()->FindItem(&FindInfo);

		//see if we couldn't find a match
		if(nFind != -1)
		{
			//alright, we found a match, now don't allow it if it is not checked
			if(!GetClassList()->GetCheck(nFind))
				return false;			
		}
	}

	//it has made it past all the checks
	return true;
}

void CObjectSelFilterDlg::OnUpdateData()
{
	m_bCheckName	= (GetUseNameButton()->GetCheck() == 1);
	m_bCheckClass	= (GetUseClassButton()->GetCheck() == 1);

	GetObjNameEdit()->GetWindowText(m_sNameFilter);

	//uppercase this once so we don't have to for each query
	m_sNameFilter.MakeUpper();
	m_sNameFilter.TrimLeft();
	m_sNameFilter.TrimRight();

	OnUpdateEnabled();
}

void CObjectSelFilterDlg::OnUpdateEnabled()
{
	//grey out the name field if we aren't filtering on the name
	GetObjNameEdit()->EnableWindow(m_bCheckName);

	//and grey out the class controls if we aren't filtering on those
	GetClassList()->EnableWindow(m_bCheckClass);
	GetResetClassButton()->EnableWindow(m_bCheckClass);
	GetInvertClassButton()->EnableWindow(m_bCheckClass);
	GetClearClassButton()->EnableWindow(m_bCheckClass);
}

void CObjectSelFilterDlg::OnClear()
{
	//run through and uncheck all items
	uint32 nNumItems = GetClassList()->GetItemCount();

	m_bIgnoreCheckChanges = true;

	for(uint32 nCurrItem = 0; nCurrItem < nNumItems; nCurrItem++)
	{
		GetClassList()->SetCheck(nCurrItem, FALSE);
	}

	m_bIgnoreCheckChanges = false;
}

void CObjectSelFilterDlg::OnReset()
{
	//run through and check all items
	uint32 nNumItems = GetClassList()->GetItemCount();

	m_bIgnoreCheckChanges = true;

	for(uint32 nCurrItem = 0; nCurrItem < nNumItems; nCurrItem++)
	{
		GetClassList()->SetCheck(nCurrItem, TRUE);
	}

	m_bIgnoreCheckChanges = false;
}

void CObjectSelFilterDlg::OnInvert()
{
	//run through and check all items
	uint32 nNumItems = GetClassList()->GetItemCount();

	m_bIgnoreCheckChanges = true;

	for(uint32 nCurrItem = 0; nCurrItem < nNumItems; nCurrItem++)
	{
		GetClassList()->SetCheck(nCurrItem, !GetClassList()->GetCheck(nCurrItem));
	}

	m_bIgnoreCheckChanges = false;
}


void CObjectSelFilterDlg::OnItemStateChanged(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	//since windows controls can do absolutely nothing right, we need to handle the checkbox
	//changing so it will apply it to all selected nodes to make UI much better
	NMLISTVIEW* pChange = (NMLISTVIEW*)pNotifyStruct;

	if(m_bIgnoreCheckChanges)
		return;

	m_bIgnoreCheckChanges = true;

	//we need to determine if the changed value is the image state
	if((pChange->uNewState & LVIS_STATEIMAGEMASK) != (pChange->uOldState & LVIS_STATEIMAGEMASK) && (pChange->iItem != -1))
	{
		//see if this item is even selected
		if(GetClassList()->GetItemState(pChange->iItem, LVIS_SELECTED) == 0)
		{
			//we weren't selected, so we need to clear any current selection and select this one
			POSITION Pos = GetClassList()->GetFirstSelectedItemPosition();

			while(Pos)
			{
				int nItem = GetClassList()->GetNextSelectedItem(Pos);
				GetClassList()->SetItemState(nItem, 0, LVIS_SELECTED);
			}

			//and select us
			GetClassList()->SetItemState(pChange->iItem, LVIS_SELECTED, LVIS_SELECTED);
			GetClassList()->SetSelectionMark(pChange->iItem);
		}

		//this is a change to the check, we need to apply this change to ALL selections
		BOOL bCheck = !GetClassList()->GetCheck(pChange->iItem);

		//don't bother setting it on the current item since the control will handle this already
		//but run through all the selections
		POSITION Pos = GetClassList()->GetFirstSelectedItemPosition();

		while(Pos)
		{
			int nItem = GetClassList()->GetNextSelectedItem(Pos);
			GetClassList()->SetCheck(nItem, bCheck);
		}
	}

	m_bIgnoreCheckChanges = false;
}

bool CObjectSelFilterDlg::FillPresetList()
{
	//clear out any old data
	GetPresetCombo()->ResetContent();

	//get the preset string list
	CString sPresetList = ::GetApp()->GetOptions().GetStringValue(PRESET_LIST_KEY);

	while(!sPresetList.IsEmpty())
	{
		CString sPreset = NextPresetString(sPresetList);
		sPreset.TrimLeft();
		sPreset.TrimRight();

		if(!sPreset.IsEmpty())
		{
			GetPresetCombo()->AddString(sPreset);
		}
	}

	return true;
}

void CObjectSelFilterDlg::OnAddPreset()
{
	//prompt the user for a name
	CStringDlg Dlg;
	Dlg.m_bAllowLetters = TRUE;
	if(Dlg.DoModal(IDS_OBJSEL_ADDPRESETTITLE, IDS_OBJSEL_ADDPRESETTEXT) != IDOK)
	{
		return;
	}

	//alright, now lets get our list and add our preset onto it, which is , delimited
	CString sPresetList = ::GetApp()->GetOptions().GetStringValue(PRESET_LIST_KEY);

	CString sPresetWithEnd = Dlg.m_EnteredText + ',';

	//see if one already exists
	CString sUpperList(sPresetList);
	CString sUpperVal(sPresetWithEnd);
	sUpperList.MakeUpper();
	sUpperVal.MakeUpper();

	if(sUpperList.Find(sUpperVal) == -1)
	{
		//it is new, add it onto our list and save it out
		sPresetList += sPresetWithEnd;
		::GetApp()->GetOptions().SetStringValue(PRESET_LIST_KEY, sPresetList);
	}

	//and now we need to save our actual string out.
	CString sPreset;
	AddPresetInt(sPreset, CURRENT_PRESET_VERSION);

	//write out the name data
	AddPresetInt(sPreset, m_bCheckName);
	AddPresetString(sPreset, m_sNameFilter);

	AddPresetInt(sPreset, m_bCheckClass);

	if(m_bCheckClass)
	{
		//run through and find all checked items, and add them to the list
		for(uint32 nCurrItem = 0; nCurrItem < GetClassList()->GetItemCount(); nCurrItem++)
		{
			if(GetClassList()->GetCheck(nCurrItem))
			{
				AddPresetString(sPreset, GetClassList()->GetItemText(nCurrItem, 0));
			}
		}
	}

	//and now save our preset
	::GetApp()->GetOptions().SetStringValue(Dlg.m_EnteredText, sPreset);

	FillPresetList();

	//select that item
	int nToSel = GetPresetCombo()->FindString(-1, Dlg.m_EnteredText);
	if(nToSel != CB_ERR)
	{
		GetPresetCombo()->SetCurSel(nToSel);
	}
}

void CObjectSelFilterDlg::OnRemovePreset()
{
	int nSel = GetPresetCombo()->GetCurSel();

	//can't do much else without a selection
	if(nSel == CB_ERR)
		return;

	CString sSelText;
	GetPresetCombo()->GetLBText(nSel, sSelText);

	//load in the list of presets
	CString sPresetList = ::GetApp()->GetOptions().GetStringValue(PRESET_LIST_KEY);

	//remove our preset from the list
	sPresetList.Replace(sSelText + ',', "");

	//save it back out
	::GetApp()->GetOptions().SetStringValue(PRESET_LIST_KEY, sPresetList);

	FillPresetList();
}

void CObjectSelFilterDlg::OnPresetChanged() 
{
	int nSel = GetPresetCombo()->GetCurSel();

	//enable/disable the remove button accordingly
	GetRemovePresetButton()->EnableWindow((nSel == CB_ERR) ? FALSE : TRUE);

	//can't do much else without a selection
	if(nSel == CB_ERR)
		return;

	CString sSelText;
	GetPresetCombo()->GetLBText(nSel, sSelText);

	CString sPreset = ::GetApp()->GetOptions().GetStringValue(sSelText);

	int nVersion = NextPresetInt(sPreset);

	if(nVersion != CURRENT_PRESET_VERSION)
	{
		MessageBox("This preset is an incorrect version. Please remove this preset", "Invalid Version", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	//split it apart in csv format and parse
	GetUseNameButton()->SetCheck(NextPresetInt(sPreset) ? 1 : 0);

	//now the edit name
	GetObjNameEdit()->SetWindowText(NextPresetString(sPreset));

	//uncheck all items
	OnClear();

	GetUseClassButton()->SetCheck(NextPresetInt(sPreset) ? 1 : 0);

	//now read in the selections one at a time
	m_bIgnoreCheckChanges = true;

	while(!sPreset.IsEmpty())
	{
		CString sClass = NextPresetString(sPreset);

		LVFINDINFO FindInfo;
		FindInfo.flags	= LVFI_STRING;
		FindInfo.psz	= sClass;
		
		int nFind = GetClassList()->FindItem(&FindInfo);

		if(nFind != -1)
		{
			GetClassList()->SetCheck(nFind, TRUE);
		}
	}

	//make sure to reload data since windows only selectively sends changed messages for items
	//that are explicitly changed. Another great move
	OnUpdateData();
}