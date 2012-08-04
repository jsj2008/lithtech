
#include "bdefs.h"
#include "dedit.h"
#include "proppageoptionsprefabs.h"
#include "optionsprefabs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun property page

IMPLEMENT_DYNCREATE(CPropPageOptionsPrefabs, CPropertyPage)

BEGIN_MESSAGE_MAP(CPropPageOptionsPrefabs, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageOptionsPrefabs)
	ON_BN_CLICKED(IDC_RADIO_PREFAB_BOX, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_PREFAB_GEOMETRY, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_PREFAB_NONE, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_PREFAB_OUTLINE, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_PREFAB_ORIENTATION, DefaultHandler)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPropPageOptionsPrefabs::CPropPageOptionsPrefabs() : CPropertyPage(CPropPageOptionsPrefabs::IDD)
{
	COptionsPrefabs *pOptions=GetApp()->GetOptions().GetPrefabsOptions();
	if (pOptions)
	{
		m_nDrawContents		= pOptions->GetContentsView();
		m_bShowOutline		= pOptions->IsShowOutline();
		m_bShowOrientation	= pOptions->IsShowOrientation();
	}
}

CPropPageOptionsPrefabs::~CPropPageOptionsPrefabs()
{
}

void CPropPageOptionsPrefabs::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageOptionsPrefab)
	DDX_Check(pDX, IDC_CHECK_PREFAB_OUTLINE, m_bShowOutline);
	DDX_Check(pDX, IDC_CHECK_PREFAB_ORIENTATION, m_bShowOrientation);
	DDX_Radio(pDX, IDC_RADIO_PREFAB_BOX, m_nDrawContents);
	//}}AFX_DATA_MAP
}


/************************************************************************/
// Saves the options
void CPropPageOptionsPrefabs::SaveOptions()
{
	UpdateData(TRUE);

	// Get the options class
	COptionsPrefabs *pOptions=GetApp()->GetOptions().GetPrefabsOptions();
	if (pOptions)
	{
		pOptions->SetContentsView((COptionsPrefabs::EViewMode)m_nDrawContents);
		pOptions->SetShowOutline(m_bShowOutline);
		pOptions->SetShowOrientation(m_bShowOrientation);
	}
}

//handles updating the enabled status of the controls
void CPropPageOptionsPrefabs::UpdateEnableStatus()
{
	UpdateData(TRUE);
}

//saves the data, and updates the enabled states
void CPropPageOptionsPrefabs::UpdateAll()
{
	SaveOptions();
	UpdateEnableStatus();
}

void CPropPageOptionsPrefabs::DefaultHandler()
{
	UpdateAll();
}


/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun message handlers

BOOL CPropPageOptionsPrefabs::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	

	// Load the options
	UpdateData(FALSE);

	UpdateAll();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
