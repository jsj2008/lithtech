#include "bdefs.h"
#include "dedit.h"
#include "proppageoptionscontrols.h"
#include "optionscontrols.h"
#include "keyconfigdlg.h"
#include "globalhotkeydb.h"

#include "keydefaultaggregate.h"
#include "keydefaultfactory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun property page

IMPLEMENT_DYNCREATE(CPropPageOptionsControls, CPropertyPage)

BEGIN_MESSAGE_MAP(CPropPageOptionsControls, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageOptionsRun)
	ON_BN_CLICKED(IDC_CHECK_INVERT_MOUSE_Y, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_ZOOM_TO_CURSOR, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_ORBIT_AROUND_SEL, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_AUTO_CAPTURE_FOCUS, DefaultHandler)
	ON_BN_CLICKED(IDC_BUTTON_HOT_KEYS, OnButtonHotKeys)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_STYLE, OnApplyStyle)
	ON_CBN_SELCHANGE(IDC_COMBO_PRESET_STYLES, OnStyleChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPropPageOptionsControls::CPropPageOptionsControls() : CPropertyPage(CPropPageOptionsControls::IDD)
{
	
}

CPropPageOptionsControls::~CPropPageOptionsControls()
{
}

void CPropPageOptionsControls::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageOptionsRun)
	DDX_Check(pDX, IDC_CHECK_INVERT_MOUSE_Y, m_bInvertMouseY);
	DDX_Check(pDX, IDC_CHECK_ZOOM_TO_CURSOR, m_bZoomToCursor);
	DDX_Check(pDX, IDC_CHECK_ORBIT_AROUND_SEL, m_bOrbitAroundSel);
	DDX_Check(pDX, IDC_CHECK_AUTO_CAPTURE_FOCUS, m_bAutoCaptureFocus);
	//}}AFX_DATA_MAP
}


/************************************************************************/
// Saves the options
void CPropPageOptionsControls::SaveOptions()
{
	UpdateData(TRUE);

	// Get the options class
	COptionsControls *pOptions=GetApp()->GetOptions().GetControlsOptions();
	if (pOptions)
	{
		pOptions->SetInvertMouseY(m_bInvertMouseY);
		pOptions->SetZoomToCursor(m_bZoomToCursor);
		pOptions->SetOrbitAroundSel(m_bOrbitAroundSel);
		pOptions->SetAutoCaptureFocus(m_bAutoCaptureFocus);
	}
}

void CPropPageOptionsControls::LoadOptions()
{
	COptionsControls *pOptions=GetApp()->GetOptions().GetControlsOptions();
	if (pOptions)
	{
		m_bInvertMouseY		= pOptions->IsInvertMouseY() ? TRUE : FALSE;
		m_bZoomToCursor		= pOptions->IsZoomToCursor() ? TRUE : FALSE;
		m_bOrbitAroundSel	= pOptions->IsOrbitAroundSel() ? TRUE : FALSE;
		m_bAutoCaptureFocus = pOptions->IsAutoCaptureFocus() ? TRUE : FALSE;
	}

	UpdateData(FALSE);
}

//handles updating the enabled status of the controls
void CPropPageOptionsControls::UpdateEnableStatus()
{
	UpdateData(TRUE);
}

//saves the data, and updates the enabled states
void CPropPageOptionsControls::UpdateAll()
{
	SaveOptions();
	UpdateEnableStatus();
}

void CPropPageOptionsControls::DefaultHandler()
{
	UpdateAll();
}

void CPropPageOptionsControls::OnButtonHotKeys()
{
	CKeyConfigDlg KeyConfigDlg(CGlobalHotKeyDB::m_DB);

	if(KeyConfigDlg.DoModal() == IDOK)
	{
		GetApp()->SetHotKeyDB(KeyConfigDlg.m_Config);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun message handlers

BOOL CPropPageOptionsControls::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// Load the options
	LoadOptions();

	UpdateAll();

	UpdateStyleText();

	//set the combo box to have the first item selected
	((CComboBox*)GetDlgItem(IDC_COMBO_PRESET_STYLES))->SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPropPageOptionsControls::OnApplyStyle()
{
	//need to apply the style. Get the name of the style first
	CString sStyle = GetStyleName();

	//now try and create it (we do this first before warning, just in
	//case the style is bad, then we don't need to warn the user)
	CKeyDefaultAggregate* pAggregate = CKeyDefaultFactory::CreateDefault(sStyle);

	if(pAggregate == NULL)
	{
		CString sStyleError;
		sStyleError.LoadString(IDS_APPLY_STYLE_FAILED);
		MessageBox(sStyleError, "Error", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	//first warn the user of their actions.
	CString sWarning;
	CString sWarningTitle;

	sWarningTitle.LoadString(IDS_APPLY_STYLE_WARNING_TITLE);
	sWarning.Format(IDS_APPLY_STYLE_WARNING, GetStyleName());

	if(MessageBox(sWarning, sWarningTitle, MB_OK | MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		//the user aborted
		delete pAggregate;
		return;
	}

	//the user actually wants to apply this style

	//add the aggregate
	CGlobalHotKeyDB::ClearAggregateList();
	CGlobalHotKeyDB::AddAggregate(pAggregate);

	//reset the keys to reflect these defaults
	CHotKeyDB NewDB;
	CGlobalHotKeyDB::SetDefaults(NewDB);

	//now set this to the app (this resets internal state, etc)
	GetApp()->SetHotKeyDB(NewDB);

	//now give the configuration a chance to modify the global options
	CKeyDefaultFactory::UpdateGlobalOptions(sStyle);

	//now that they have done that, we need to refresh this dialog
	LoadOptions();

	//for the last thing, we need to save this setting so that it can be loaded up
	//the next time we run
	GetApp()->GetOptions().GetControlsOptions()->SetStringValue("ControlStyle", sStyle);

	// success
}

//gets the text from the style dropdown
CString CPropPageOptionsControls::GetStyleName()
{
	//first off get the text of the dropdown
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PRESET_STYLES);

	CString sStyle;
	pCombo->GetWindowText(sStyle);

	return sStyle;
}

//updates the edit box to reflect the description of the selected style
void CPropPageOptionsControls::UpdateStyleText()
{
	//now get the appropriate text
	CString sText = CKeyDefaultFactory::GetDefaultText(GetStyleName());

	//apply that text to the edit control
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STYLE_DESCRIPTION);
	pEdit->SetWindowText(sText);
}

//the style changed
void CPropPageOptionsControls::OnStyleChanged()
{
	UpdateStyleText();
}
