#include "keyconfigdlg.h"
#include "resource.h"
#include "bindkeydlg.h"
#include "globalhotkeydb.h"

BEGIN_MESSAGE_MAP (CKeyConfigDlg, CDialog)
	ON_BN_CLICKED(ID_BIND, OnBind)
	ON_BN_CLICKED(ID_CLEAR, OnClear)
	ON_BN_CLICKED(ID_RESET, OnReset)
	ON_BN_CLICKED(ID_SAVE, OnSave)
	ON_BN_CLICKED(ID_LOAD, OnLoad)
	ON_BN_CLICKED(ID_RESETALL, OnResetAll)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_KEYLIST, OnKeyActivated)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_KEYLIST, OnKeySelected)
END_MESSAGE_MAP()

//constructor that takes a configuration to use as a default configuration
//this will be used as the starting values for all the keys. Also takes a 
//pointer to the tracker of which will be set with the final values if the
//user presses OK
CKeyConfigDlg::CKeyConfigDlg(	const CHotKeyDB& CurrConfig, 
								CUITrackerMgr*	pUITrackerMgr, 
								CWnd* pParentWnd) :	
	CDialog(IDD_KEYCONFIG, pParentWnd),
	m_pUITrackerMgr(pUITrackerMgr),
	m_Config(CurrConfig)
{
	//load the icons for the buttons
	m_hSaveIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SAVE_ICON));
	m_hLoadIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_LOAD_ICON));
}

CKeyConfigDlg::~CKeyConfigDlg()
{
}


//standard button handlers
void CKeyConfigDlg::OnOK()
{
	CDialog::OnOK();
}

void CKeyConfigDlg::OnCancel()
{
	CDialog::OnCancel();
}

//handle initialization and loading of icons
BOOL CKeyConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//set the icon for the save button
	CButton* pSaveButton = (CButton*)(GetDlgItem(ID_SAVE));
	if(pSaveButton)
	{	
		pSaveButton->SetIcon(m_hSaveIcon);
	}	

	//set the icon for the load button
	CButton* pLoadButton = (CButton*)(GetDlgItem(ID_LOAD));
	if(pLoadButton)
	{	
		pLoadButton->SetIcon(m_hLoadIcon);
	}	

	//setup the column headers
	CString sHeader;

	sHeader.LoadString( IDS_KEYCONFIG_COMMAND );
	GetKeyList()->InsertColumn( 0, sHeader, LVCFMT_LEFT );
	sHeader.LoadString( IDS_KEYCONFIG_KEY );
	GetKeyList()->InsertColumn( 1, sHeader, LVCFMT_LEFT );
	sHeader.LoadString( IDS_KEYCONFIG_DESCRIPTION );
	GetKeyList()->InsertColumn( 2, sHeader, LVCFMT_LEFT );

	//realize the list in the list control
	InitKeyList();

	//setup the widths of the columns to auto fit
	GetKeyList()->SetColumnWidth( 0, LVSCW_AUTOSIZE );
	GetKeyList()->SetColumnWidth( 1, LVSCW_AUTOSIZE );
	GetKeyList()->SetColumnWidth( 2, LVSCW_AUTOSIZE );

	//disable the action buttons since an item may isn't currently selected
	EnableActionButtons(FALSE);

	//make it so the user can select any part of that row. Makes it much easier to use
	GetKeyList()->SetExtendedStyle(	LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	return TRUE;
}

//handle buttons for mapping keys
void CKeyConfigDlg::OnBind()
{
	//make sure an item is selected
	if(GetKeyList()->GetSelectedCount() == 0)
	{
		return;
	}

	//get the selected item position
	POSITION pos = GetKeyList()->GetFirstSelectedItemPosition();
	//now get the index
	uint32 nItem = GetKeyList()->GetNextSelectedItem(pos);

	//get the pointer to the hotkey
	CHotKey* pKey = (CHotKey*)GetKeyList()->GetItemData(nItem);

	//create the bind key dialog and display it
	CBindKeyDlg BindDlg(pKey, &m_Config);

	//run it and see if the user selected OK to apply changes
	if(BindDlg.DoModal() == IDOK)
	{
		//save the changed hotkey back into the original
		*pKey = BindDlg.m_HotKey;

		pKey->SortEvents();

		//update the item
		SetupItem(nItem, pKey);
	}

}

void CKeyConfigDlg::OnResetAll()
{
	//prompt the user and give them a chance to back out
	if(MessageBox("This will reset you current configuration and all changes will be lost",
				"Are you sure?", MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL)
	{
		//user hit cancel
		return;
	}

	m_Config.ClearKeys();

	//reset the configuration
	CGlobalHotKeyDB::SetDefaults(m_Config);

	//realize the list in the list control
	InitKeyList();
}

void CKeyConfigDlg::OnClear()
{
	//make sure an item is selected
	if(GetKeyList()->GetSelectedCount() == 0)
	{
		return;
	}

	//get the selected item position
	POSITION pos = GetKeyList()->GetFirstSelectedItemPosition();
	//now get the index
	uint32 nItem = GetKeyList()->GetNextSelectedItem(pos);

	//get the pointer to the hotkey
	CHotKey* pKey = (CHotKey*)GetKeyList()->GetItemData(nItem);

	//clear out the keys
	pKey->ResetEvents();

	//update the item
	SetupItem(nItem, pKey);

}

void CKeyConfigDlg::OnReset()
{
	//make sure an item is selected
	if(GetKeyList()->GetSelectedCount() == 0)
	{
		return;
	}

	//get the selected item position
	POSITION pos = GetKeyList()->GetFirstSelectedItemPosition();
	//now get the index
	uint32 nItem = GetKeyList()->GetNextSelectedItem(pos);

	//get the pointer to the hotkey
	CHotKey* pKey = (CHotKey*)GetKeyList()->GetItemData(nItem);

	//we want to reset this key to the default one
	CGlobalHotKeyDB::SetKeyDefault(*pKey);

	//update the items
	SetupItem(nItem, pKey);
}

//handle loading and saving configurations
void CKeyConfigDlg::OnLoad()
{
	//create the filters and extension needed from the string table
	//so it could be localized if needed
	CString	sExtension;
	CString sFilter;

	sExtension.LoadString( IDS_KEYCONFIG_EXTENSION );
	sFilter.LoadString( IDS_KEYCONFIG_FILTER );
	
	//setup the dialog
	CFileDialog		dlg( TRUE, sExtension, NULL, 
						 OFN_FILEMUSTEXIST, 
						 sFilter);

	//if they selected ok, then save it
	if( dlg.DoModal() == IDOK )
	{
		//reinit the configuration with the global one
		CGlobalHotKeyDB::SetDefaults(m_Config);
		m_Config.Load(dlg.GetPathName(), false);
	}

	//now realize the list
	InitKeyList();

}

void CKeyConfigDlg::OnSave()
{
	//create the filters and extension needed from the string table
	//so it could be localized if needed
	CString	sExtension;
	CString sFilter;

	sExtension.LoadString( IDS_KEYCONFIG_EXTENSION );
	sFilter.LoadString( IDS_KEYCONFIG_FILTER );

	//setup the dialog
	CFileDialog		dlg( FALSE, sExtension, NULL, 
						 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
						 sFilter);

	//if they selected ok, then save it
	if( dlg.DoModal() == IDOK )
	{
		m_Config.Save(dlg.GetPathName());
	}
}

CListCtrl* CKeyConfigDlg::GetKeyList()
{
	return (CListCtrl*)GetDlgItem(IDC_KEYLIST);
}

void CKeyConfigDlg::EnableActionButtons(BOOL bEnable)
{
	((CButton*)(GetDlgItem(ID_BIND)))->EnableWindow(bEnable);
	((CButton*)(GetDlgItem(ID_CLEAR)))->EnableWindow(bEnable);
	((CButton*)(GetDlgItem(ID_RESET)))->EnableWindow(bEnable);
}

//initializes the list control with all the keys in the current database
void CKeyConfigDlg::InitKeyList()
{
	//clear the entire list
	GetKeyList()->DeleteAllItems();

	for(uint32 nCurrCommand = 0; nCurrCommand < m_Config.GetNumHotKeys(); nCurrCommand++)
	{
		//get the current hotkey
		const CHotKey* pCurrKey = m_Config.GetHotKey(nCurrCommand);

		if(pCurrKey && pCurrKey->IsUserChangable())
		{
			//set the command name
			int nItem = GetKeyList()->InsertItem(  nCurrCommand, pCurrKey->GetEventName() ); 
			
			SetupItem(nItem, pCurrKey);
		}
	}
}




void CKeyConfigDlg::OnKeyActivated(NMHDR* pmnh, LRESULT* pResult)
{
	//this is called when an item is double clicked, so lets just pretend
	//they selected the bind option
	OnBind();
}

void CKeyConfigDlg::OnKeySelected(NMHDR* pmnh, LRESULT* pResult)
{
	int nNumSelected = GetKeyList()->GetSelectedCount();

	EnableActionButtons((nNumSelected > 0) ? TRUE : FALSE);
}

//sets up all the information about an item
void CKeyConfigDlg::SetupItem(uint32 nIndex, const CHotKey* pKey)
{
	//set the command name
	GetKeyList()->SetItemText( nIndex, 0, pKey->GetEventName() ); 
	//set the key data
	GetKeyList()->SetItemText( nIndex, 1, CBindKeyDlg::HotKeyToString(*pKey) );
	//set the description
	GetKeyList()->SetItemText( nIndex, 2, pKey->GetDescription());

	//set the hotkey as the data
	GetKeyList()->SetItemData( nIndex, (unsigned long)pKey );	
}
