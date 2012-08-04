#include "bindkeydlg.h"
#include "resource.h"
#include "hotkeydb.h"
#include "extendedvk.h"

#define MAX_FAMILY_NAME_LEN		256	

struct SCodeToString
{
	uint32 m_nCode;
	char   m_pszName[20];
};

SCodeToString g_MouseToString[] = {	{ UI_MOUSELEFT, "Left Mouse" },
									{ UI_MOUSERIGHT, "Right Mouse" },
									{ UI_MOUSEMIDDLE, "Middle Mouse" }
								};

SCodeToString g_VKToString[] = {	{ VK_BACK, "Back" },
									{ VK_TAB, "Tab" },
									{ VK_RETURN, "Return" },
									{ VK_SHIFT, "Shift" },
									{ VK_CONTROL, "Ctrl" },
									{ VK_PAUSE, "Pause" },
									{ VK_KANA, "Kana" },
									{ VK_KANJI, "Kanji" },
									{ VK_ESCAPE, "Esc" },
									{ VK_SPACE, "Space" },
									{ VK_PRIOR, "Pg Up" },
									{ VK_NEXT, "Pg Down" },
									{ VK_END, "End" },
									{ VK_HOME, "Home" },
									{ VK_LEFT, "Left" },
									{ VK_UP, "Up" },
									{ VK_RIGHT, "Right" },
									{ VK_DOWN, "Down" },
									{ VK_SELECT, "Select" },
									{ VK_PRINT, "Print" },
									{ VK_EXECUTE, "Execute" },
									{ VK_INSERT, "Insert" },
									{ VK_DELETE, "Delete" },
									{ VK_HELP, "Help" },
									{ VK_NUMPAD0, "Numpad 0" },
									{ VK_NUMPAD1, "Numpad 1" },
									{ VK_NUMPAD2, "Numpad 2" },
									{ VK_NUMPAD3, "Numpad 3" },
									{ VK_NUMPAD4, "Numpad 4" },
									{ VK_NUMPAD5, "Numpad 5" },
									{ VK_NUMPAD6, "Numpad 6" },
									{ VK_NUMPAD7, "Numpad 7" },
									{ VK_NUMPAD8, "Numpad 8" },
									{ VK_NUMPAD9, "Numpad 9" },
									{ VK_MULTIPLY, "Numpad *" },
									{ VK_ADD, "Numpad +" },
									{ VK_SUBTRACT, "Numpad -" },
									{ VK_DECIMAL, "Numpad ." },
									{ VK_DIVIDE, "Numpad /" },
									{ VK_F1, "F1" },
									{ VK_F2, "F2" },
									{ VK_F3, "F3" },
									{ VK_F4, "F4" },
									{ VK_F5, "F5" },
									{ VK_F6, "F6" },
									{ VK_F7, "F7" },
									{ VK_F8, "F8" },
									{ VK_F9, "F9" },
									{ VK_F10, "F10" },
									{ VK_F11, "F11" },
									{ VK_F12, "F12" },
									{ VK_F13, "F13" },
									{ VK_F14, "F14" },
									{ VK_F15, "F15" },
									{ VK_F16, "F16" },
									{ VK_F17, "F17" },
									{ VK_F18, "F18" },
									{ VK_F19, "F19" },
									{ VK_F20, "F20" },
									{ VK_F21, "F21" },
									{ VK_F22, "F22" },
									{ VK_F23, "F23" },
									{ VK_F24, "F24" },
									{ VK_NUMLOCK,			"Num Lock" },
									{ VK_SCROLL,			"Scroll Lock" },
									{ EXTVK_MINUS,			"-" },
									{ EXTVK_EQUALS,			"=" },
									{ EXTVK_OPENBRACKET,	"[" },
									{ EXTVK_CLOSEBRACKET,	"]" },
									{ EXTVK_SEMICOLON,		";" },
									{ EXTVK_SINGLEQUOTE,	"\'" },
									{ EXTVK_COMMA,			"," },
									{ EXTVK_PERIOD,			"." },
									{ EXTVK_SLASH,			"/" },
									{ EXTVK_BACKSLASH,		"\\" },
									{ EXTVK_BACKQUOTE,		"`" }
								 };


#define MOUSE_MASK		0x80000000

BEGIN_MESSAGE_MAP (CBindKeyDlg, CDialog)
END_MESSAGE_MAP()

//constructor that takes a hotkey to initialize its data to
CBindKeyDlg::CBindKeyDlg(	const CHotKey* pHotKey, 
							CHotKeyDB* pDB,
							CWnd* pParentWnd) :	
	CDialog(IDD_BINDKEYDLG, pParentWnd)
{
	if(pHotKey)
	{
		m_HotKey = *pHotKey;
	}

	//save the pointer to the database
	m_pDB = pDB;
}


//given a string that is a hotkey name, it will determine the family name. 
//if it is at global scope, the family name will be empty
static void GetFamilyName(const char* pszHotKey, char* pszFamily, uint32 nBuffLen)
{
	ASSERT(nBuffLen > 0);

	//clear out the family string
	pszFamily[0] = '\0';

	//run through backwards looking for a period
	uint32 nStrLen = strlen(pszHotKey);

	int32 nCurrPos;
	for(nCurrPos = (int32)nStrLen - 1; nCurrPos >= 0; nCurrPos--)
	{
		if(pszHotKey[nCurrPos] == '.')
		{
			break;
		}
	}

	//see if we found one
	if(nCurrPos <= 0)
	{
		return;
	}

	strncpy(pszFamily, pszHotKey, LTMIN(nBuffLen, nCurrPos));
	pszFamily[nCurrPos] = '\0';
}


//standard button handlers
void CBindKeyDlg::OnOK()
{
	//clear all the current keys
	m_HotKey.ResetEvents();

	//add the modifiers first
	if( ((CButton*)GetDlgItem(IDC_MOD_CTRL))->GetCheck() )
	{
		m_HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	}
	if( ((CButton*)GetDlgItem(IDC_MOD_SHIFT))->GetCheck() )
	{
		m_HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}

	//now go through all the dropdowns and add those keys
	int nDropDownList[] = {  IDC_KEYSELECT1, IDC_KEYSELECT2, IDC_KEYSELECT3 };
	uint32 nKey[3];

	for(uint32 nCurrDrop = 0; nCurrDrop < sizeof(nDropDownList) / sizeof(nDropDownList[0]); nCurrDrop++)
	{
		CComboBox* pCombo = (CComboBox*)GetDlgItem(nDropDownList[nCurrDrop]);
		if(pCombo)
		{
			//get the selected item's data
			nKey[nCurrDrop] = pCombo->GetItemData(pCombo->GetCurSel());

			//make sure that there aren't any duplicate keys, so they can't have
			// like Q + Q + Q, which just doesn't make sense
			for(uint32 nCurrCheck = 0; nCurrCheck < nCurrDrop; nCurrCheck++)
			{
				if(nKey[nCurrCheck] == nKey[nCurrDrop])
				{
					//found a duplicate, so set this key to an invalid value,
					//so that it will be skipped
					nKey[nCurrDrop] = 0;
					break;
				}
			}

					

			if(nKey[nCurrDrop] > 0)
			{
				if(nKey[nCurrDrop] & MOUSE_MASK)
				{
					m_HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, 
											nKey[nCurrDrop] & (~MOUSE_MASK)));
				}
				else
				{
					m_HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, nKey[nCurrDrop]));
				}
			}
		}
	}

	if( m_HotKey.GetNumEvents() > 0 )
	{
		// see if the key combination is being used by an accelerator:
		HACCEL hAccel = ::LoadAccelerators( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDR_MAINFRAME ) );
		if (hAccel )
		{
			ACCEL accel[ 512 ];
			int nAccel = CopyAcceleratorTable( hAccel, accel, 256 );
			DestroyAcceleratorTable( hAccel );
			for( uint32 nCurrCommand = 0; nCurrCommand < nAccel; ++nCurrCommand )
			{
				// Create a CHotKey from the current accelerator, so that
				// CHotKey::operator== can be used to see if the hotkeys are the same:
				CHotKey accelKey;
				if( accel[ nCurrCommand ].fVirt & FCONTROL )
				{
					accelKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
				}
				if( accel[ nCurrCommand ].fVirt & FSHIFT )
				{
					accelKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
				}
				if( accel[ nCurrCommand ].fVirt & FALT )
				{
					accelKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_MENU));
				}
				accelKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, accel[ nCurrCommand ].key));

				if( m_HotKey.HasSameStartingEvents(accelKey) )
				{
					int result = MessageBox( "That key combination is already being used by an accelerator.  Do you want to use it anyway?", "Warning", MB_YESNOCANCEL );
					if( result != IDYES )
					{
						if( result == IDNO )
						{
							//they chose no, so reset it back to the original
							m_HotKey = m_OriginalHotKey;
							CDialog::OnOK();
						}
						return;
					}
				}
			}
		}

		// see if another hotkey is using the same key combination:
		if(m_pDB)
		{
			//keeps track of the number of key conflicts
			uint32 nNumKeyConflicts = 0;
			CString sConflictWith;

			char pszCurrFamily[MAX_FAMILY_NAME_LEN];
			char pszOtherFamily[MAX_FAMILY_NAME_LEN];

			//get the current key's family name
			GetFamilyName(m_HotKey.GetEventName(), pszCurrFamily, MAX_FAMILY_NAME_LEN);

			for( uint32 nCurrCommand = 0; nCurrCommand < m_pDB->GetNumHotKeys(); ++nCurrCommand )
			{
				//get the current hotkey
				const CHotKey* pCurrKey = m_pDB->GetHotKey(nCurrCommand);
				
				//skip over the key comparison if it has the same event name
				if( stricmp(pCurrKey->GetEventName(), m_HotKey.GetEventName()) == 0 )
					continue;

				//get the family name of this
				GetFamilyName(pCurrKey->GetEventName(), pszOtherFamily, MAX_FAMILY_NAME_LEN);

				//now see if the family names conflict, which can occur if 
				//they are the same in the same family
				if(stricmp(pszCurrFamily, pszOtherFamily) != 0)
				{
					//these keys can't conflict
					continue;
				}

				if( m_HotKey.HasSameStartingEvents(*pCurrKey) )
				{
					if(nNumKeyConflicts > 0)
						sConflictWith += ", ";
					sConflictWith += pCurrKey->GetEventName();
					nNumKeyConflicts++;
				}
			}

			//see if we had any conflicts
			if(nNumKeyConflicts > 0)
			{
				
				CString str;
				str.Format( "That key combination is conflicts with %s.  Do you want to use it anyway?", sConflictWith );
				int result = MessageBox( str, "Warning", MB_YESNOCANCEL );
				if( result != IDYES )
				{
					if( result == IDNO )
					{
						//they chose no, so reset it back to the original
						m_HotKey = m_OriginalHotKey;
						CDialog::OnOK();
					}
					return;
				}
			}
		}
	}


	CDialog::OnOK();
}

void CBindKeyDlg::OnCancel()
{
	CDialog::OnCancel();
}


CString CBindKeyDlg::EventToString(const CUIEvent& Event)
{
	CString rv("ERROR");

	if((Event.GetType() == UIEVENT_KEYDOWN) || (Event.GetType() == UIEVENT_KEYUP))
	{
		//get the key
		uint32 nKey = ((CUIKeyEvent&)Event).GetCode();

		//see if we can find the string in the key to string listing
		for(uint32 nCurrKey = 0; nCurrKey < sizeof(g_VKToString) / sizeof(g_VKToString[0]); nCurrKey++)
		{
			if(nKey == g_VKToString[nCurrKey].m_nCode)
			{
				rv = g_VKToString[nCurrKey].m_pszName;
				return rv;
			}
		}
		//couldn't find it, so assume that the code is actually a letter like A or 1
		rv = (char)nKey;
		return rv;
	}
	else if((Event.GetType() == UIEVENT_MOUSEDOWN) || (Event.GetType() == UIEVENT_MOUSEUP))
	{
		//see if it is a mouse event
		for(uint32 nCurrMouse = 0; nCurrMouse < sizeof(g_MouseToString) / sizeof(g_MouseToString[0]); nCurrMouse++)
		{
			if(g_MouseToString[nCurrMouse].m_nCode == ((CUIMouseEvent&)Event).GetButton())
			{
				rv = g_MouseToString[nCurrMouse].m_pszName;
				return rv;
			}
		}
	}

	return rv;	
}

CString CBindKeyDlg::HotKeyToString(const CHotKey& Key)
{
	CString rv;

	//go through all the keys and add them to the string along with + separating them
	for(uint32 nCurrEvent = 0; nCurrEvent < Key.GetNumEvents(); nCurrEvent++)
	{
		rv += EventToString(Key.GetEvent(nCurrEvent));
		
		if(nCurrEvent < Key.GetNumEvents() - 1)
		{
			rv += "+";
		}
	}

	return rv;
}

//handle initialization and loading of icons
BOOL CBindKeyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//setup the check boxes to correspond with the current hotkey
	SetCheckIfKeyUsed(IDC_MOD_CTRL, VK_CONTROL);
	SetCheckIfKeyUsed(IDC_MOD_SHIFT, VK_SHIFT);

	//add all the items to the dropdown boxes
	FillDropDown(IDC_KEYSELECT1);
	FillDropDown(IDC_KEYSELECT2);
	FillDropDown(IDC_KEYSELECT3);

	//select the appropriate item in each box
	uint32 nCurrDropDown = 0;
	int nDropDownList[] = {  IDC_KEYSELECT1, IDC_KEYSELECT2, IDC_KEYSELECT3 };

	//set the static controls to show the event name and description
	CStatic* pEventName = ((CStatic*)GetDlgItem(IDC_EVENT_NAME));
	pEventName->SetWindowText(m_HotKey.GetEventName());

	//set the description of the event
	CEdit* pDescription = ((CEdit*)GetDlgItem(IDC_EVENT_DESCRIPTION));
	
	CString sDescription;
	pDescription->SetWindowText(m_HotKey.GetDescription());


	for(uint32 nCurrEvent = 0; nCurrEvent < m_HotKey.GetNumEvents(); nCurrEvent++)
	{
		const CUIEvent& Event = m_HotKey.GetEvent(nCurrEvent);

		//skip over control and shift
		if( (Event == CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL)) ||
			(Event == CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT)) )
		{
			continue;
		}

		//lets see if we can find the string
		CComboBox* pDropDown = (CComboBox*)GetDlgItem(nDropDownList[nCurrDropDown]);

		if(pDropDown)
		{
			//find the item to the list and set it as the current selection
			int nItem = pDropDown->FindString(-1, EventToString(Event));

			if(nItem >= 0)
			{
				pDropDown->SetCurSel(nItem);
				//move on to the next dropdown box
				nCurrDropDown++;

				if(nCurrDropDown >= sizeof(nDropDownList) / sizeof(nDropDownList[0]))
				{
					break;
				}
			}
		}
	}
	
	//save the original hotkey
	m_OriginalHotKey = m_HotKey;

	return TRUE;
}

//sets the specified check box control to checked if the current hotkey
//has the specified key included
void CBindKeyDlg::SetCheckIfKeyUsed(int nControl, uint32 nKey)
{
	if(m_HotKey.ContainsEvent(CUIKeyEvent(UIEVENT_KEYDOWN, nKey)))
	{
		CButton* pCheckBox = (CButton*)GetDlgItem(nControl);
		if(pCheckBox)
		{
			pCheckBox->SetCheck(1);
		}
	}
}

//fills the specified dropdown box with all the keys possible to select
void CBindKeyDlg::FillDropDown(int nControl)
{
	//get the control and ensure it is valid
	CComboBox* pCombo = (CComboBox*)GetDlgItem(nControl);

	if(pCombo == NULL)
	{
		return;
	}

	//fill it with all the letters

	//this is used as a temp var to load the letter into a string
	//so that the function (which takes a string) will work
	CString sLetToStr;

	for(uint32 nCurrLet = 0; nCurrLet < 26; nCurrLet++)
	{
		sLetToStr = (char)('A' + nCurrLet);
		int nItem = pCombo->AddString(sLetToStr);
		pCombo->SetItemData(nItem, 'A' + nCurrLet);
	}

	//do the same for numbers
	for(uint32 nCurrNum = 0; nCurrNum < 10; nCurrNum++)
	{
		sLetToStr = (char)('0' + nCurrNum);
		int nItem = pCombo->AddString(sLetToStr);
		pCombo->SetItemData(nItem, '0' + nCurrNum);
	}

	//now add all the items
	for(uint32 nCurrKey = 0; nCurrKey < sizeof(g_VKToString) / sizeof(g_VKToString[0]); nCurrKey++)
	{
		int nItem = pCombo->AddString(g_VKToString[nCurrKey].m_pszName);
		pCombo->SetItemData(nItem, g_VKToString[nCurrKey].m_nCode);
	}
	//now the mouse commands
	for(uint32 nCurrMouse = 0; nCurrMouse < sizeof(g_MouseToString) / sizeof(g_MouseToString[0]); nCurrMouse++)
	{
		int nItem = pCombo->AddString(g_MouseToString[nCurrMouse].m_pszName);
		pCombo->SetItemData(nItem, g_MouseToString[nCurrMouse].m_nCode | MOUSE_MASK);	
	}

	//insert and default the NONE string
	pCombo->InsertString(0, "--NONE--");
	pCombo->SetCurSel(0);
	pCombo->SetItemData(0, 0);
}
