
#include "bdefs.h"
#include "dedit.h"
#include "proppageoptionsmodels.h"
#include "optionsmodels.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun property page

IMPLEMENT_DYNCREATE(CPropPageOptionsModels, CPropertyPage)

BEGIN_MESSAGE_MAP(CPropPageOptionsModels, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageOptionsRun)
	ON_BN_CLICKED(IDC_CHECK_DISPLAY_MODELS_AS_BOXES, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_ALWAYS_SHOW_MODELS, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_MAXIMUM_MODEL_MEMORY_USE, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_LOW_PRIORITY_MODEL_THREAD, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_PERSPECTIVE_BOX, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_PERSPECTIVE_WIREFRAME, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_PERSPECTIVE_SOLID, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_PERSPECTIVE_TEXTURED, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_ORTHOGRAPHIC_BOX, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_ORTHOGRAPHIC_WIREFRAME, DefaultHandler)
	ON_EN_CHANGE(IDC_EDIT_MODEL_DISPLAY_BOX_DISTANCE, DefaultHandler)
	ON_EN_CHANGE(IDC_EDIT_MODEL_MAX_MEMORY, DefaultHandler)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPropPageOptionsModels::CPropPageOptionsModels() : CPropertyPage(CPropPageOptionsModels::IDD)
{
	COptionsModels *pOptions=GetApp()->GetOptions().GetModelsOptions();
	if (pOptions)
	{
		m_bDrawBoxAtDist		= pOptions->IsRenderBoxAtDist() ? TRUE : FALSE;
		m_nDrawBoxDist			= pOptions->GetRenderBoxDist();
		m_bLimitMemUse			= pOptions->IsLimitMemoryUse() ? TRUE : FALSE;
		m_nMaxMemUse			= pOptions->GetMaxMemoryUse();
		m_bLowPriorityLoading	= pOptions->IsRunLowPriority() ? TRUE : FALSE;
		m_nPerspective			= pOptions->GetPerspectiveMode();
		m_nOrthographic			= pOptions->GetOrthoMode();
		m_bAlwaysShowModels		= pOptions->IsAlwaysShowModels() ? TRUE : FALSE;
	}
}

CPropPageOptionsModels::~CPropPageOptionsModels()
{
}

void CPropPageOptionsModels::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageOptionsRun)
	DDX_Check(pDX, IDC_CHECK_DISPLAY_MODELS_AS_BOXES, m_bDrawBoxAtDist);
	DDX_Check(pDX, IDC_CHECK_ALWAYS_SHOW_MODELS,	m_bAlwaysShowModels);
	DDX_Text (pDX, IDC_EDIT_MODEL_DISPLAY_BOX_DISTANCE, m_nDrawBoxDist);
	DDX_Check(pDX, IDC_CHECK_MAXIMUM_MODEL_MEMORY_USE, m_bLimitMemUse);
	DDX_Text (pDX, IDC_EDIT_MODEL_MAX_MEMORY, m_nMaxMemUse);
	DDX_Check(pDX, IDC_CHECK_LOW_PRIORITY_MODEL_THREAD, m_bLowPriorityLoading);
	DDX_Radio(pDX, IDC_RADIO_PERSPECTIVE_BOX, m_nPerspective);
	DDX_Radio(pDX, IDC_RADIO_ORTHOGRAPHIC_BOX, m_nOrthographic);
	//}}AFX_DATA_MAP
}


/************************************************************************/
// Saves the options
void CPropPageOptionsModels::SaveOptions()
{
	UpdateData(TRUE);

	// Get the options class
	COptionsModels *pOptions=GetApp()->GetOptions().GetModelsOptions();
	if (pOptions)
	{
		pOptions->SetRenderBoxAtDist(m_bDrawBoxAtDist);
		pOptions->SetRenderBoxDist(m_nDrawBoxDist);
		pOptions->SetLimitMemoryUse(m_bLimitMemUse);
		pOptions->SetMaxMemoryUse(m_nMaxMemUse);
		pOptions->SetRunLowPriority(m_bLowPriorityLoading);
		pOptions->SetPerspectiveMode(m_nPerspective);
		pOptions->SetOrthoMode(m_nOrthographic);
		pOptions->SetAlwaysShowModels(m_bAlwaysShowModels);
	}
}

//handles updating the enabled status of the controls
void CPropPageOptionsModels::UpdateEnableStatus()
{
	UpdateData(TRUE);

	BOOL bEnableModelBoxes = (m_nPerspective == COptionsModels::VIEWMODEL_BOX) ? FALSE : TRUE;
	GetDlgItem(IDC_CHECK_DISPLAY_MODELS_AS_BOXES)->EnableWindow(bEnableModelBoxes);
	GetDlgItem(IDC_EDIT_MODEL_DISPLAY_BOX_DISTANCE)->EnableWindow(bEnableModelBoxes && m_bDrawBoxAtDist);

	GetDlgItem(IDC_EDIT_MODEL_MAX_MEMORY)->EnableWindow(m_bLimitMemUse);
}

//saves the data, and updates the enabled states
void CPropPageOptionsModels::UpdateAll()
{
	SaveOptions();
	UpdateEnableStatus();
}

void CPropPageOptionsModels::DefaultHandler()
{
	UpdateAll();
}


/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun message handlers

BOOL CPropPageOptionsModels::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	

	// Load the options
	UpdateData(FALSE);

	UpdateAll();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
