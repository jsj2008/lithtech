// winpackerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "winpacker.h"
#include "winpackerDlg.h"
#include "IPackerImpl.h"
#include "ParamList.h"
#include "PackerProperty.h"
#include "RegistryUtil.h"
#include "StringPrompt.h"
#include "PackerRegLoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//ID for the property Wnd control
#define PROPERTY_WND_ID					15000

//this will open up a file and convert it into a CString. If it fails, it will
//return false
BOOL ConvertFileToString(const char* pszFilename, CString& sFile)
{
	//try and open up the file
	CFile File;
	
	if(File.Open(pszFilename, CFile::modeRead) == FALSE)
	{
		return FALSE;
	}
	
	//load in the file

	DWORD nFileSize = (DWORD) File.GetLength();

// why seek :(
//	DWORD nFileSize = File.SeekToEnd();
//	File.SeekToBegin();

	//create a string from the file
	char* pszBuffer = new char[nFileSize + 1];
	File.Read(pszBuffer, nFileSize);
	pszBuffer[nFileSize] = '\0';


	sFile = pszBuffer;

	//clean up
	delete [] pszBuffer;
	File.Close();

	return TRUE;
}

//builds up the command stream from the command line parameters as well as the
//saved packer options, etc.
CString BuildFinalCommandString(const char* pszPackerName, 
								const char* pszCommandLine)
{
	//the return value string
	CString sRV;

	//load up the registry settings for the command line
	
	//build up the registry key name
	CString sKeyName = PACKER_REGISTRY_DIRECTORY;
	sKeyName += pszPackerName;

	//get the registry
	sRV = GetRegistryKey(HKEY_CURRENT_USER, sKeyName, "");


	//build up the command line parser
	CParamList ParamList(pszCommandLine);

	//run through the parameter list and load all the specified options, and
	//append them to the command string
	if(ParamList.GetNumParams() > 0)
	{
		for(uint32 nCurrParam = 0; nCurrParam < ParamList.GetNumParams() - 1; nCurrParam++)
		{
			if(stricmp("-LoadOptions", ParamList.GetParameter(nCurrParam)) == 0)
			{
				//we need to load in these options
				CString sLoadedOptions;
				if(ConvertFileToString(ParamList.GetParameter(nCurrParam + 1), sLoadedOptions))
				{
					//loaded the options, now tack them on
					sRV += " ";
					sRV += sLoadedOptions;
				}
			}
		}
	}
	
	//finally, add the command line on the end
	sRV += pszCommandLine;

	return sRV;
}

CWinpackerDlg::CWinpackerDlg(IPackerImpl* pIPacker, CWnd* pParent) :
	CDialog(CWinpackerDlg::IDD, pParent),
	m_pIPacker(pIPacker),
	m_PropertyWnd(pIPacker, &m_PropMgr),
	m_bSkipDlg(FALSE)
{
	//{{AFX_DATA_INIT(CWinpackerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//load the icons for the buttons
	m_hSaveIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SAVEICON));
	m_hLoadIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_LOADICON));

	//make sure that the packer is valid
	ASSERT(m_pIPacker);
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinpackerDlg dialog


void CWinpackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWinpackerDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWinpackerDlg, CDialog)
	//{{AFX_MSG_MAP(CWinpackerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_GROUPS, OnGroupChanged)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, OnButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_ADD_PRESET, OnAddPreset)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_PRESET, OnRemovePreset)
	ON_CBN_SELCHANGE(IDC_COMBO_PRESETS, OnPresetChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinpackerDlg message handlers


BOOL CWinpackerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//also add the icons to the buttons
	((CButton*)(GetDlgItem(IDC_BUTTON_SAVE)))->SetIcon(m_hSaveIcon);
	((CButton*)(GetDlgItem(IDC_BUTTON_LOAD)))->SetIcon(m_hLoadIcon);
	
	//we need to notify our packer that the properties need to be filled out
	m_pIPacker->RequestUserOptions(&m_PropMgr);

	//now we want to load in the values specified by the user
	CString sFinalCommand = BuildFinalCommandString(m_sPackerName, m_sCommandLine);
	LoadUserDefaults(sFinalCommand);

	//update the groups
	ResetGroupList();

	//now we need to create our property window
	CreatePropertyWindow();	
	m_PropertyWnd.SetFocus();

	//set the first group to be selected
	GetGroupTabs()->SetCurSel(0);

	//refresh the property window
	ResetPropertyList();

	//update the presets list
	LoadPresets();

	//make the remove preset button disabled since no preset can be selected yet
	GetDlgItem(IDC_BUTTON_REMOVE_PRESET)->EnableWindow(FALSE);

	//set the title bar text
	CString sTitle;
	sTitle.Format("%s - %s", m_sPackerName, m_sFileToPack);
	SetWindowText(sTitle);

	//setup the tooltips
	m_ToolTip.Create(this);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_BUTTON_SAVE), IDS_TOOLTIP_SAVE_SETTINGS);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_BUTTON_LOAD), IDS_TOOLTIP_LOAD_SETTINGS);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_COMBO_PRESETS), IDS_TOOLTIP_PRESET_LIST);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_BUTTON_ADD_PRESET), IDS_TOOLTIP_ADD_PRESET);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_BUTTON_REMOVE_PRESET), IDS_TOOLTIP_REMOVE_PRESET);
	m_ToolTip.AddWindowTool(GetDlgItem(IDOK), IDS_TOOLTIP_PROCESS);
	
	//now after all that hard work, see if we want to just skip this dialog (not all
	//of it was in vain though, the properties were loaded
	if(m_bSkipDlg)
		OnOK();
	
	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CWinpackerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWinpackerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWinpackerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//updates the group list to reflect the groups in the propmgr
void CWinpackerDlg::ResetGroupList()
{
	CTabCtrl* pTab = GetGroupTabs();

	//clear the list
	pTab->DeleteAllItems();
	
	//add in the new groups
	for(uint32 nCurrGroup = 0; nCurrGroup < m_PropMgr.GetNumGroups(); nCurrGroup++)
	{
		CPropertyGroup* pGroup = m_PropMgr.GetGroup(nCurrGroup);
		pTab->InsertItem(TCIF_TEXT | TCIF_PARAM, nCurrGroup, pGroup->GetName(), 0, (LPARAM)pGroup);
	}
}

//updates the property list when a group is changed
void CWinpackerDlg::ResetPropertyList()
{
	CPropertyGroup* pGroup = GetSelectedGroup();

	if(pGroup == NULL)
		return;

	//we need to find the selected group, and update the property window
	m_PropertyWnd.CreatePropertyList(pGroup);
}

//creates the property window
void CWinpackerDlg::CreatePropertyWindow()
{
	//first off, find the place holder, get the rectangle, and remove the holder
	CStatic* pHolder = (CStatic*)GetDlgItem(IDC_STATIC_PROP_LIST_HOLDER);

	//we want to make this control invisible
	pHolder->ModifyStyle(WS_VISIBLE, 0);


	//get the rectangle of the control
	CRect rHolder;
	pHolder->GetWindowRect(rHolder);

	//get the rectangle of the dialog
	CRect rDialog;
	GetWindowRect(rDialog);

	//now we need to remove the title bar part....a lot of work considering this is just
	//to get an offset in the dialog....stupid windows.
	CRect rDlgClient;
	GetClientRect(rDlgClient);

	//now we need to offset the holder rectangle by the dialog's upper left corner
	rHolder.OffsetRect(	-(rDialog.left + (rDialog.Width() - rDlgClient.Width()) - GetSystemMetrics(SM_CXDLGFRAME)),
						-(rDialog.top  + (rDialog.Height() - rDlgClient.Height()) - GetSystemMetrics(SM_CYDLGFRAME)));

	//now we create our actual control
	m_PropertyWnd.CreateEx(	0,//WS_EX_CLIENTEDGE, 
							AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW), 
												(HBRUSH)(COLOR_3DFACE + 1), NULL), 
							"Property List", 
							WS_VSCROLL | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
							rHolder, this, PROPERTY_WND_ID);

	//we want to use the same font in the property list
	LOGFONT LogFont;
	(pHolder->GetFont())->GetLogFont(&LogFont);
	m_PropertyWnd.m_Font.CreateFontIndirect(&LogFont);
}

//gets the selected group (NULL if none selected)
CPropertyGroup* CWinpackerDlg::GetSelectedGroup()
{
	int nSel = GetGroupTabs()->GetCurSel();

	//check for no selection
	if(nSel == -1)
		return NULL;

	//now get the name of the selected tab
	TCITEM tcItem;
	tcItem.mask = TCIF_PARAM;
	GetGroupTabs()->GetItem(nSel, &tcItem);
	
	return (CPropertyGroup*)tcItem.lParam;
}

void CWinpackerDlg::OnGroupChanged(NMHDR* pNmhdr, LRESULT* pResult) 
{
	ResetPropertyList();

	m_PropertyWnd.Invalidate();

	*pResult = 0;
}


void CWinpackerDlg::OnOK() 
{
	//we want to save these settings to the registry
	CString sSettings = BuildSettingsString();

	//build up the key name
	CString sRegKeyName = PACKER_REGISTRY_DIRECTORY;
	sRegKeyName += m_sPackerName;

	//save it to the registry
	SetRegistryKey(HKEY_CURRENT_USER, sRegKeyName, sSettings);

	//turn off the tooltips in the property wnd...this avoids an assert
	m_PropertyWnd.EmptyPropertyList();

	CDialog::OnOK();
}

void CWinpackerDlg::OnCancel()
{
	//turn off the tooltips in the property wnd...this avoids an assert
	m_PropertyWnd.EmptyPropertyList();

	CDialog::OnCancel();
}


CPackerPropList* CWinpackerDlg::GetPropList()
{
	return &m_PropMgr.GetPropList();
}

//this will take the command string, break it apart, and use that to override values
//in the property list
bool CWinpackerDlg::LoadUserDefaults(const char* pszDefaults)
{
	//load up the initial parameters
	CParamList ParamList;
	ParamList.Init(pszDefaults);

	//now run through looking for strings that start with a -
	for(uint32 nCurrParam = 0; nCurrParam < ParamList.GetNumParams(); nCurrParam++)
	{
		CString sParam = ParamList.GetParameter(nCurrParam);

		//remove any whitespace
		sParam.TrimLeft();
		sParam.TrimRight();

		//see if it starts with a minus
		if(sParam.GetLength() && (sParam[0] == '-'))
		{
			//trim off the minus, and see if this is an actual property
			sParam.TrimLeft("-");

			for(uint32 nCurrProp = 0; nCurrProp < m_PropMgr.GetPropList().GetNumProperties(); nCurrProp++)
			{
				CPackerProperty* pProp = m_PropMgr.GetPropList().GetProperty(nCurrProp);
				if(stricmp(sParam, pProp->GetName()) == 0)
				{
					//we have a match, let the property load itself
					pProp->LoadValue(	ParamList.GetParameterList() + (nCurrParam + 1), 
										ParamList.GetNumParams() - nCurrParam - 1);
				}
			}
		}
	}

	return true;
}

//this function will build up a string that can be saved out to persist the settings
//for this packer
#define SETTINGS_BUFFER_SIZE			2048

CString CWinpackerDlg::BuildSettingsString()
{
	CString sRV;

	//buffer for values to be saved into
	char pszBuffer[SETTINGS_BUFFER_SIZE + 1];

	for(uint32 nCurrProp = 0; nCurrProp < m_PropMgr.GetPropList().GetNumProperties(); nCurrProp++)
	{
		CPackerProperty* pProp = m_PropMgr.GetPropList().GetProperty(nCurrProp);

		//skip over UI properties
		if(pProp->GetType() == PROPERTY_INTERFACE)
			continue;

		//tack on the property name
		sRV += "-";
		sRV += pProp->GetName();
		sRV += " ";

		//now write out its value
		pProp->SaveValue(pszBuffer, SETTINGS_BUFFER_SIZE);
		sRV += pszBuffer;
		sRV += " ";
	}

	//success
	return sRV;
}


void CWinpackerDlg::OnButtonSave() 
{
	CFileDialog Dlg(FALSE, "*.opt", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
					"Packer Options File (*.opt)|*.opt|All Files (*.*)|*.*||");

	if(Dlg.DoModal() == IDOK)
	{
		//try and open up the file
		CFile File;
		if(File.Open(Dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite) == FALSE)
		{
			MessageBox("Error");
			return;
		}

		//build up the string
		CString sOptions = BuildSettingsString();

		File.Write((const char*)sOptions, sOptions.GetLength());

		File.Close();
	}		
}

void CWinpackerDlg::OnButtonLoad() 
{
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, 
					"Packer Options File (*.opt)|*.opt|All Files (*.*)|*.*||");

	if(Dlg.DoModal() == IDOK)
	{
		//try and open up the file
		CString sFile;

		if(ConvertFileToString(Dlg.GetPathName(), sFile) == FALSE)
		{
			MessageBox("Error reading specified file");
			return;
		}
		
		//process the file
		LoadUserDefaults(sFile);

		//make sure the control states are in sync
		m_PropertyWnd.CreatePropertyList(m_PropertyWnd.GetAssociatedGroup());
	}	
}

//loads the presets in from the registry for this packer
void CWinpackerDlg::LoadPresets()
{
	//now get the key value
	CString sPresetList = GetRegistryKey(HKEY_CURRENT_USER, GetPresetListKeyName(), "");

	//now that we have the list, pull it apart into items and add them to the dropdown
	GetPresetCombo()->ResetContent();

	do
	{
		//each preset name is separated by a newline
		int nPos = sPresetList.Find('\n');

		//no more presets
		if(nPos == -1)
			break;

		//extract the preset
		GetPresetCombo()->AddString(sPresetList.Left(nPos));
		sPresetList = sPresetList.Mid(nPos + 1);
	}
	while(1);  //bails when no more presets are found
}

void CWinpackerDlg::OnAddPreset() 
{
	//need to prompt this user for a preset name

	CStringPrompt Dlg;

	Dlg.m_sTitle.LoadString(IDS_ENTER_PRESET_NAME_TITLE);
	Dlg.m_sPrompt.LoadString(IDS_ENTER_PRESET_NAME_PROMPT);

	while(1)
	{
		if(Dlg.DoModal() == IDCANCEL)
		{
			//user changed their mind
			return;
		}

		//don't allow empty strings
		if(Dlg.m_sString.IsEmpty())
			continue;

		//check for a unique name
		bool bNameUnique = true;

		for(uint32 nCurrItem = 0; nCurrItem < (uint32)GetPresetCombo()->GetCount(); nCurrItem++)
		{
			CString sItemName;
			GetPresetCombo()->GetLBText(nCurrItem, sItemName);

			if(stricmp(Dlg.m_sString, sItemName) == 0)
			{
				bNameUnique = false;
				break;
			}
		}

		//we have a string, make sure it isn't taken
		if(bNameUnique)
		{
			//we have a unique string, save the new preset and update the preset list

			//update the list
			//now get the key value
			CString sPresetList = GetRegistryKey(HKEY_CURRENT_USER, GetPresetListKeyName(), "");
			sPresetList += Dlg.m_sString;
			sPresetList += '\n';

			//save the list
			SetRegistryKey(HKEY_CURRENT_USER, GetPresetListKeyName(), sPresetList);

			//now save the actual configuration
			CString sPresetName;
			sPresetName.Format("%s%s\\%s", PACKER_REGISTRY_DIRECTORY, m_sPackerName, Dlg.m_sString);
			SetRegistryKey(HKEY_CURRENT_USER, sPresetName, BuildSettingsString());

			//reset the list to reflect this new item
			LoadPresets();

			//select the newly added item
			GetPresetCombo()->SelectString(-1, Dlg.m_sString);

			//enable the remove button
			GetDlgItem(IDC_BUTTON_REMOVE_PRESET)->EnableWindow(TRUE);

			//all done
			return;
		}
		else
		{

			CString sErrorTitle, sErrorText;
			sErrorTitle.LoadString(IDS_ERROR_PRESET_NOT_UNIQUE_TITLE);
			sErrorText.Format(IDS_ERROR_PRESET_NOT_UNIQUE_TEXT, Dlg.m_sString);
			
			//not a unique name, just inform the user and prompt them for another name
			if(MessageBox(sErrorText, sErrorTitle, MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
				return;
		}
	}	
}

void CWinpackerDlg::OnRemovePreset() 
{
	//get the selected item text
	int nSel = GetPresetCombo()->GetCurSel();

	//make sure there is a selection
	if(nSel == CB_ERR)
		return;

	CString sSelText;
	GetPresetCombo()->GetLBText(nSel, sSelText);

	//make sure we have a string
	if(sSelText.IsEmpty())
		return;

	//make sure that the user indeed does want to delete this preset
	CString sConfirmTitle, sConfirmPrompt;
	sConfirmTitle.LoadString(IDS_CONFIRM_REMOVE_PRESET_TITLE);
	sConfirmPrompt.Format(IDS_CONFIRM_REMOVE_PRESET_PROMPT, sSelText);

	if(MessageBox(sConfirmPrompt, sConfirmTitle, MB_ICONQUESTION | MB_YESNO) == IDNO)
	{
		//they canceled
		return;
	}

	//delete the preset
	CString sPresetName;
	sPresetName.Format("%s%s\\%s", PACKER_REGISTRY_DIRECTORY, m_sPackerName, sSelText);

	DeleteRegistryKey(HKEY_CURRENT_USER, sPresetName);

	//now get the preset list
	CString sPresetList = GetRegistryKey(HKEY_CURRENT_USER, GetPresetListKeyName(), "");
	
	//remove this item
	sPresetList.Replace(sSelText + '\n', "");
	
	//save the list
	SetRegistryKey(HKEY_CURRENT_USER, GetPresetListKeyName(), sPresetList);

	//update the list
	LoadPresets();

	//disable the button
	GetDlgItem(IDC_BUTTON_REMOVE_PRESET)->EnableWindow(FALSE);

	//success
	return;
}

//gets the name of the registry string that holds the preset list
CString CWinpackerDlg::GetPresetListKeyName() const
{
	//we need to open up the registry key holding the list of presets
	CString sRegKey;
	sRegKey.Format("%s%s\\Preset List", PACKER_REGISTRY_DIRECTORY, m_sPackerName);

	return sRegKey;
}

void CWinpackerDlg::OnPresetChanged() 
{
	int nSel = GetPresetCombo()->GetCurSel();

	//enable/disable the remove button accordingly
	GetDlgItem(IDC_BUTTON_REMOVE_PRESET)->EnableWindow((nSel == CB_ERR) ? FALSE : TRUE);

	//can't do much else without a selection
	if(nSel == CB_ERR)
		return;

	CString sSelText;
	GetPresetCombo()->GetLBText(nSel, sSelText);

	//load up the settings
	CString sPresetName;
	sPresetName.Format("%s%s\\%s", PACKER_REGISTRY_DIRECTORY, m_sPackerName, sSelText);

	CString sPreset = GetRegistryKey(HKEY_CURRENT_USER, sPresetName, "");

	//load up the options
	LoadUserDefaults(sPreset);
	
	//update everything
	m_PropertyWnd.PropertyModified(NULL);

}
