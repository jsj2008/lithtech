#include "bdefs.h"
#include "dedit.h"
#include "proppageoptionsmisc.h"
#include "optionsmisc.h"
#include "keyconfigdlg.h"
#include "globalhotkeydb.h"
#include "projectbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun property page

IMPLEMENT_DYNCREATE(CPropPageOptionsMisc, CPropertyPage)

BEGIN_MESSAGE_MAP(CPropPageOptionsMisc, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageOptionsRun)
	ON_BN_CLICKED(IDC_CHECK_PARENT_FOLDER, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ICONS, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_SHOW_THUMBNAILS, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_AUTO_EXTRACT_ICONS, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_AUTO_EXTRACT_HELP, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_SHOW_FULL_TITLE, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_AUTO_LOAD_PROJ, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_COMPRESSED, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_LOAD_LYT_FILE, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_UNDO_FREEZE_HIDE, DefaultHandler)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPropPageOptionsMisc::CPropPageOptionsMisc() : CPropertyPage(CPropPageOptionsMisc::IDD)
{
	//{{AFX_DATA_INIT(CUndoPage)
	m_dwUndos = 40;
	//}}AFX_DATA_INIT

	COptionsMisc *pOptions=GetApp()->GetOptions().GetMiscOptions();
	if (pOptions)
	{
		m_bParentFolder		= pOptions->IsParentFolder()		? TRUE : FALSE;
		m_bShowIcons		= pOptions->GetShowIcons()			? TRUE : FALSE;
		m_bShowThumbnails	= pOptions->GetShowThumbnails()		? TRUE : FALSE;
		m_bAutoExtractIcons	= pOptions->IsAutoExtractIcons()	? TRUE : FALSE;
		m_bAutoExtractHelp  = pOptions->IsAutoExtractClassHelp()? TRUE : FALSE;
		m_bShowUndoWarnings	= pOptions->GetShowUndoWarnings()	? TRUE : FALSE;
		m_bShowFullPath		= pOptions->IsShowFullPathTitle()	? TRUE : FALSE;
		m_bDefaultCompressed= pOptions->IsDefaultCompressed()	? TRUE : FALSE;
		m_bAutoLoadProj		= pOptions->IsAutoLoadProj()		? TRUE : FALSE;
		m_bLoadLYTFile		= pOptions->IsLoadLYTFile()			? TRUE : FALSE;
		m_bUndoFreezeHide	= pOptions->IsUndoFreezeHide()		? TRUE : FALSE;
		m_dwUndos			= pOptions->GetNumUndos();
	}

	m_dwOriginalUndos = m_dwUndos;
}

CPropPageOptionsMisc::~CPropPageOptionsMisc()
{
}

void CPropPageOptionsMisc::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageOptionsMisc)
	DDX_Check(pDX, IDC_CHECK_AUTO_EXTRACT_ICONS,	m_bAutoExtractIcons);
	DDX_Check(pDX, IDC_CHECK_PARENT_FOLDER,			m_bParentFolder);
	DDX_Check(pDX, IDC_CHECK_SHOW_ICONS,			m_bShowIcons);
	DDX_Check(pDX, IDC_CHECK_SHOW_THUMBNAILS,		m_bShowThumbnails);
	DDX_Check(pDX, IDC_CHECK_SHOW_UNDO_WARNING,		m_bShowUndoWarnings);
	DDX_Check(pDX, IDC_CHECK_AUTO_EXTRACT_HELP,		m_bAutoExtractHelp);
	DDX_Check(pDX, IDC_CHECK_SHOW_FULL_TITLE,		m_bShowFullPath);
	DDX_Check(pDX, IDC_CHECK_AUTO_LOAD_PROJ,		m_bAutoLoadProj);
	DDX_Check(pDX, IDC_CHECK_DEFAULT_COMPRESSED,	m_bDefaultCompressed);
	DDX_Check(pDX, IDC_CHECK_LOAD_LYT_FILE,			m_bLoadLYTFile);
	DDX_Check(pDX, IDC_CHECK_UNDO_FREEZE_HIDE,		m_bUndoFreezeHide);
	DDX_Text(pDX, IDC_NUM_UNDOS, m_dwUndos);
	//}}AFX_DATA_MAP
}


/************************************************************************/
// Saves the options
void CPropPageOptionsMisc::SaveOptions()
{
	UpdateData(TRUE);

	if (GetProjectBar()->m_bShowIcons		!= (bool)m_bShowIcons)		  OnSetShowIcons();
	if (GetProjectBar()->m_bShowThumbnails	!= (bool)m_bShowThumbnails)	  OnSetShowThumbnails();

	// Get the options class
	COptionsMisc *pOptions=GetApp()->GetOptions().GetMiscOptions();
	if (pOptions)
	{
		if (pOptions->GetShowUndoWarnings()	!= (bool)m_bShowUndoWarnings) OnSetShowUndoWarnings();

		pOptions->SetAutoExtractIcons(m_bAutoExtractIcons);
		pOptions->SetParentFolder(m_bParentFolder);
		pOptions->SetShowIcons(m_bShowIcons);
		pOptions->SetShowThumbnails(m_bShowThumbnails);
		pOptions->SetShowUndoWarnings(m_bShowUndoWarnings);
		pOptions->SetAutoExtractClassHelp(m_bAutoExtractHelp);
		pOptions->SetNumUndos(m_dwUndos);
		pOptions->SetShowFullPathTitle(m_bShowFullPath);
		pOptions->SetAutoLoadProj(m_bAutoLoadProj);
		pOptions->SetDefaultCompressed(m_bDefaultCompressed);
		pOptions->SetLoadLYTFile(m_bLoadLYTFile);
		pOptions->SetUndoFreezeHide(m_bUndoFreezeHide);
	}
}

//handles updating the enabled status of the controls
void CPropPageOptionsMisc::UpdateEnableStatus()
{
	UpdateData(TRUE);
}

//saves the data, and updates the enabled states
void CPropPageOptionsMisc::UpdateAll()
{
	UpdateEnableStatus();
}

void CPropPageOptionsMisc::DefaultHandler()
{
	UpdateAll();
}

void CPropPageOptionsMisc::OnSetShowIcons()
{
	GetProjectBar()->m_bShowIcons = !m_bShowIcons; // This is get negated again in OnShowIcons()

	GetProjectBar()->OnShowIcons();
}

void CPropPageOptionsMisc::OnSetShowThumbnails()
{
	GetProjectBar()->m_bShowThumbnails = m_bShowThumbnails;

	if( GetProjectBar()->IsProjectOpen() )
	{
		GetTextureDlg()->DoShowThumbnails(m_bShowThumbnails);
		GetPrefabDlg()->DoShowThumbnails(m_bShowThumbnails);
		GetSpriteDlg()->DoShowThumbnails(m_bShowThumbnails);
	}
}

void CPropPageOptionsMisc::OnSetShowUndoWarnings()
{
	if( GetProjectBar()->IsProjectOpen() )
	{
		for(uint32 i=0; i < GetProjectBar()->m_RegionDocs; i++)
		{
			GetProjectBar()->m_RegionDocs[i]->m_UndoMgr.m_bShowNodeCountWarning = m_bShowUndoWarnings;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun message handlers

BOOL CPropPageOptionsMisc::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	SetDlgItemInt(IDC_NUM_UNDOS, m_dwUndos);
	m_dwOriginalUndos = m_dwUndos;

	// Load the options
	UpdateData(FALSE);

	UpdateAll();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CPropPageOptionsMisc::OnOK()
{
	UpdateData();

	// If the user change the size of the undo buffer, we must confirm if they actually intend to 
	// delete it before resizing.
	if (DidUndoValueChange()) 
	{
		if (MessageBox("If you change the undo buffer size,\nthe undo buffer will be erased.\nIs this okay?",
					   "WARNING!",MB_YESNO|MB_ICONEXCLAMATION) == IDNO)  
		{
			// Reset the change
			m_dwUndos = m_dwOriginalUndos;
			SetDlgItemInt(IDC_NUM_UNDOS, m_dwUndos);
		}
	}

	SaveOptions(); 

	CDialog::OnOK();
}
