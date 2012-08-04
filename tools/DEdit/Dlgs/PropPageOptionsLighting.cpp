#include "bdefs.h"
#include "dedit.h"
#include "proppageoptionslighting.h"
#include "optionslighting.h"
#include "optionsdisplay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsLighting property page

IMPLEMENT_DYNCREATE(CPropPageOptionsLighting, CPropertyPage)

BEGIN_MESSAGE_MAP(CPropPageOptionsLighting, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageOptionsRun)
	ON_BN_CLICKED(IDC_CHECK_LAMBERTIAN, DefaultHandler)
	ON_BN_CLICKED(IDC_CHECK_SHADOWS, DefaultHandler)

	ON_BN_CLICKED(IDC_RADIO_UNSHADED, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_SHADED, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_VERTEX_LIT, DefaultHandler)
	ON_BN_CLICKED(IDC_RADIO_LIGHTMAPPED, DefaultHandler)

	ON_EN_CHANGE(IDC_EDIT_LIGHT_TIME_SLICE, DefaultHandler)
	ON_EN_CHANGE(IDC_EDIT_LM_MAX_SIZE, DefaultHandler)
	ON_EN_CHANGE(IDC_EDIT_LM_MIN_SIZE, DefaultHandler)
	ON_EN_CHANGE(IDC_EDIT_MIN_LM_TEXEL, DefaultHandler)
	ON_EN_CHANGE(IDC_EDIT_LIGHT_LEAK_DIST, DefaultHandler)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPropPageOptionsLighting::CPropPageOptionsLighting() : CPropertyPage(CPropPageOptionsLighting::IDD)
{
	m_bInit = FALSE;
}

CPropPageOptionsLighting::~CPropPageOptionsLighting()
{
}

void CPropPageOptionsLighting::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageOptionsRun)
	DDX_Control(pDX, IDC_SPIN_LIGHT_TIME_SLICE,		m_spinTimeSlice);
	DDX_Control(pDX, IDC_SPIN_LM_MAX_SIZE,			m_spinLMMaxSize);
	DDX_Control(pDX, IDC_SPIN_LM_MIN_SIZE,			m_spinLMMinSize);
	DDX_Control(pDX, IDC_SPIN_MIN_LM_TEXEL,			m_spinTexelSize);
	DDX_Control(pDX, IDC_SPIN_LIGHT_LEAK_DIST,		m_spinLeakDist);

	DDX_Check(pDX, IDC_CHECK_LAMBERTIAN,		m_bLambertian);
	DDX_Check(pDX, IDC_CHECK_SHADOWS,			m_bShadows);

	DDX_Text (pDX, IDC_EDIT_LIGHT_TIME_SLICE,	m_nTimeSlice);
	DDX_Text (pDX, IDC_EDIT_LM_MAX_SIZE,		m_nLightMapSize);
	DDX_Text (pDX, IDC_EDIT_LM_MIN_SIZE,		m_nMinLightMapSize);
	DDX_Text (pDX, IDC_EDIT_MIN_LM_TEXEL,		m_nTexelSize);
	DDX_Text (pDX, IDC_EDIT_LIGHT_LEAK_DIST,	m_nLightLeakDist);
	
	DDX_Radio(pDX, IDC_RADIO_UNSHADED,			m_nLightMode);
	//}}AFX_DATA_MAP
}

/************************************************************************/
// Saves the options
void CPropPageOptionsLighting::SaveOptions()
{
	UpdateData(TRUE);

	// Get the options class
	COptionsLighting *pOptions		= GetApp()->GetOptions().GetLightingOptions();
	COptionsDisplay  *pDispOptions	= GetApp()->GetOptions().GetDisplayOptions();

	if (pOptions)
	{
		pOptions->SetLambertian(m_bLambertian);
		pOptions->SetShadows(m_bShadows);
		pOptions->SetMaxLMSize(m_nLightMapSize);
		pOptions->SetLMTexelSize(m_nTexelSize);
		pOptions->SetTimeSlice(m_nTimeSlice);
		pOptions->SetMinLMSize(m_nMinLightMapSize);
		pOptions->SetLightLeakDist(m_nLightLeakDist);

		pOptions->SetVertex((m_nLightMode == LIGHT_VERTEX) || (m_nLightMode == LIGHT_LIGHTMAPPED));
		pOptions->SetLightMap(m_nLightMode == LIGHT_LIGHTMAPPED);

		pDispOptions->SetShadePolygons(m_nLightMode == LIGHT_SHADED);
	}
}

//handles updating the enabled status of the controls
void CPropPageOptionsLighting::UpdateEnableStatus()
{
	UpdateData(TRUE);

	BOOL bEnableLM = (m_nLightMode == LIGHT_LIGHTMAPPED);
	BOOL bEnableVertex = (m_nLightMode == LIGHT_VERTEX) || bEnableLM;
	BOOL bEnableShadow = ((CButton*)GetDlgItem(IDC_CHECK_SHADOWS))->GetCheck() && bEnableVertex;

	GetDlgItem(IDC_CHECK_LAMBERTIAN)->EnableWindow(bEnableVertex);
	GetDlgItem(IDC_CHECK_SHADOWS)->EnableWindow(bEnableVertex);
	GetDlgItem(IDC_EDIT_LIGHT_TIME_SLICE)->EnableWindow(bEnableVertex);
	GetDlgItem(IDC_EDIT_LM_MAX_SIZE)->EnableWindow(bEnableLM);
	GetDlgItem(IDC_EDIT_LM_MIN_SIZE)->EnableWindow(bEnableLM);
	GetDlgItem(IDC_EDIT_MIN_LM_TEXEL)->EnableWindow(bEnableLM);
	GetDlgItem(IDC_EDIT_LIGHT_LEAK_DIST)->EnableWindow(bEnableShadow);
}

//saves the data, and updates the enabled states
void CPropPageOptionsLighting::UpdateAll()
{
	if(m_bInit)
	{
		SaveOptions();
		UpdateEnableStatus();
	}
}

void CPropPageOptionsLighting::DefaultHandler()
{
	UpdateAll();
}


/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun message handlers

BOOL CPropPageOptionsLighting::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	COptionsLighting *pOptions		= GetApp()->GetOptions().GetLightingOptions();
	COptionsDisplay  *pDispOptions	= GetApp()->GetOptions().GetDisplayOptions();

	if (pOptions)
	{
		m_bLambertian			= pOptions->IsLambertian() ? TRUE : FALSE;
		m_bShadows				= pOptions->IsShadows() ? TRUE : FALSE;
		m_nLightMapSize			= pOptions->GetMaxLMSize();
		m_nTexelSize			= pOptions->GetLMTexelSize();
		m_nTimeSlice			= pOptions->GetTimeSlice();
		m_nMinLightMapSize		= pOptions->GetMinLMSize();
		m_nLightLeakDist		= pOptions->GetLightLeakDist();

		if(pOptions->IsLightMap())
		{
			m_nLightMode = LIGHT_LIGHTMAPPED;
		}
		else if(pOptions->IsVertex())
		{
			m_nLightMode = LIGHT_VERTEX;
		}
		else if(pDispOptions->IsShadePolygons())
		{
			m_nLightMode = LIGHT_SHADED;
		}
		else
		{
			m_nLightMode = LIGHT_NONE;
		}
	}
	
	m_spinTimeSlice.SetRange(0, 200);
	m_spinLMMaxSize.SetRange(2, 128);
	m_spinLMMinSize.SetRange(1, 128);
	m_spinTexelSize.SetRange(1, 256);
	m_spinLeakDist.SetRange(0, 256);

	// Load the options
	UpdateData(FALSE);

	m_bInit = TRUE;

	UpdateAll();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
