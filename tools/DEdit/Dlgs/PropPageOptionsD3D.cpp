//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// PropPageOptionsD3D.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "proppageoptionsd3d.h"
#include "optionsdisplay.h"
#include "drawmgr.h"
#include "texture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsD3D property page

IMPLEMENT_DYNCREATE(CPropPageOptionsD3D, CPropertyPage)

CPropPageOptionsD3D::CPropPageOptionsD3D() : CPropertyPage(CPropPageOptionsD3D::IDD)
{
	//{{AFX_DATA_INIT(CPropPageOptionsD3D)
	m_bBlurryTextures				= FALSE;
	m_bZBufferLines					= FALSE;
	m_bSaturateLightmaps			= TRUE;
	m_bDetailTexEnable				= TRUE;
	m_bDetailTexAdditive			= TRUE;
	m_sDetailTexScale				= "1.0";
	m_sDetailTexAngle				= "0.0";
	//}}AFX_DATA_INIT
}

CPropPageOptionsD3D::~CPropPageOptionsD3D()
{
}

void CPropPageOptionsD3D::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageOptionsD3D)
	DDX_Control(pDX, IDC_COMBO_D3D_MODE, m_comboD3Dmode);
	DDX_Control(pDX, IDC_COMBO_D3DDEVICES, m_comboD3Ddevices);
	DDX_Check(pDX, IDC_BLURRY_TEXTURES, m_bBlurryTextures);
	DDX_Check(pDX, IDC_CHECK_ZBUFFER_LINES, m_bZBufferLines);
	DDX_Check(pDX, IDC_CHECK_DETAIL_TEX_ENABLE, m_bDetailTexEnable);
	DDX_Check(pDX, IDC_CHECK_DETAIL_TEX_ADDITIVE, m_bDetailTexAdditive);
	DDX_Check(pDX, IDC_CHECK_SATURATE_LIGHTMAPS, m_bSaturateLightmaps);
	DDX_Text(pDX, IDC_DETAIL_TEX_SCALE, m_sDetailTexScale);
	DDX_Text(pDX, IDC_DETAIL_TEX_ANGLE, m_sDetailTexAngle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropPageOptionsD3D, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageOptionsD3D)
	ON_BN_CLICKED(IDC_CHECK_ZBUFFER_LINES, GenericUpdateAll)
	ON_BN_CLICKED(IDC_CHECK_DETAIL_TEX_ENABLE,	GenericUpdateAll)
	ON_BN_CLICKED(IDC_CHECK_DETAIL_TEX_ADDITIVE, GenericUpdateAll)
	ON_BN_CLICKED(IDC_CHECK_SATURATE_LIGHTMAPS, GenericUpdateAll)
	ON_EN_CHANGE(IDC_DETAIL_TEX_SCALE, GenericUpdateAll)
	ON_EN_CHANGE(IDC_DETAIL_TEX_ANGLE, GenericUpdateAll)
	ON_CBN_SELCHANGE(IDC_COMBO_D3DDEVICES, GenericUpdateAll)
	ON_CBN_SELCHANGE(IDC_COMBO_D3D_MODE, GenericUpdateAll)
	ON_BN_CLICKED(IDC_BLURRY_TEXTURES, OnBlurryTextures)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsD3D message handlers

BOOL CPropPageOptionsD3D::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// The display options class
	COptionsDisplay *pDisplayOptions=GetApp()->GetOptions().GetDisplayOptions();
	if (!pDisplayOptions)
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	// Fill the combo box with the device names
	char *deviceNames[4];
	memset(deviceNames, 0, sizeof(deviceNames));
	int nDevices=dm_GetDirectDrawDeviceNames(deviceNames, 4);

	int i;
	for (i=0; i < nDevices; i++)
	{
		m_comboD3Ddevices.AddString(deviceNames[i]);
		delete deviceNames[i];
	}

	// Add the different D3D modes
	m_comboD3Dmode.AddString("Hardware Acceleration");
	m_comboD3Dmode.AddString("Software MMX");
	m_comboD3Dmode.AddString("Software 8-bit Emulation");
	m_comboD3Dmode.AddString("Software RGB Emulation");

	// Get the D3D values
	m_bZBufferLines				= pDisplayOptions->IsZBufferLines();	
	m_bSaturateLightmaps		= pDisplayOptions->IsSaturateLightmaps();
	m_bBlurryTextures			= (BOOL)pDisplayOptions->GetMipMapOffset();
	
	// Set up the detail texture info
	m_bDetailTexEnable			= pDisplayOptions->IsDetailTexEnabled();
	m_bDetailTexAdditive		= pDisplayOptions->IsDetailTexAdditive();

	m_sDetailTexScale.Format("%.2f", pDisplayOptions->GetDetailTexScale());
	m_sDetailTexAngle.Format("%.2f", pDisplayOptions->GetDetailTexAngle());

	m_comboD3Ddevices.SetCurSel(pDisplayOptions->GetDefaultD3DDevice());
	m_comboD3Dmode.SetCurSel(pDisplayOptions->GetDefaultD3DMode());

	// Update the enabled status of the controls
	UpdateEnabledStatus();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/************************************************************************/
// Updates the display options with the current dialog data
void CPropPageOptionsD3D::UpdateDisplayOptions()
{
	// Update the data
	UpdateData();

	// The display options class
	COptionsDisplay *pDisplayOptions=GetApp()->GetOptions().GetDisplayOptions();
	if (!pDisplayOptions)
	{
		ASSERT(FALSE);
		return;
	}

	// Set the Z buffer line status
	pDisplayOptions->SetZBufferLines(m_bZBufferLines);	
	pDisplayOptions->SetSaturateLightmaps(m_bSaturateLightmaps);

	pDisplayOptions->SetMipMapOffset(m_bBlurryTextures ? 1 : 0);

	// Set the default device
	int nSelection=m_comboD3Ddevices.GetCurSel();
	if (nSelection != CB_ERR)
	{
		pDisplayOptions->SetDefaultD3DDevice(nSelection);
	}

	// Set the default mode
	nSelection=m_comboD3Dmode.GetCurSel();
	if (nSelection != CB_ERR)
	{
		pDisplayOptions->SetDefaultD3DMode(nSelection);
	}

	//set up the detail texture options
	pDisplayOptions->SetDetailTexEnabled(m_bDetailTexEnable);
	pDisplayOptions->SetDetailTexAdditive(m_bDetailTexAdditive);
	pDisplayOptions->SetDetailTexScale(atof(m_sDetailTexScale));
	pDisplayOptions->SetDetailTexAngle(atof(m_sDetailTexAngle));
}

/************************************************************************/
// Update the enabled status of the controls
void CPropPageOptionsD3D::UpdateEnabledStatus()
{
	//detail texture settings
	GetDlgItem(IDC_CHECK_DETAIL_TEX_ADDITIVE)->EnableWindow(m_bDetailTexEnable);
	GetDlgItem(IDC_DETAIL_TEX_SCALE)->EnableWindow(m_bDetailTexEnable);
	GetDlgItem(IDC_DETAIL_TEX_ANGLE)->EnableWindow(m_bDetailTexEnable);

	GetDlgItem(IDC_STATIC_DETAIL_TEX_SCALE)->EnableWindow(m_bDetailTexEnable);
	GetDlgItem(IDC_STATIC_DETAIL_TEX_ANGLE)->EnableWindow(m_bDetailTexEnable);
}

void CPropPageOptionsD3D::UpdateAll(bool bRedrawViews)
{
	// Update the data
	UpdateData();
	
	// Update the display options
	UpdateDisplayOptions();

	// Update the enabled status of the controls
	UpdateEnabledStatus();

	if(bRedrawViews)
	{
		RedrawAllDocuments();
	}
}


void CPropPageOptionsD3D::GenericUpdateAll()
{
	UpdateAll(true);
}

void CPropPageOptionsD3D::OnBlurryTextures()
{
	// Update the data
	UpdateData();
	
	// Update the display options
	UpdateDisplayOptions();

	// Update the enabled status of the controls
	UpdateEnabledStatus();

	dib_FreeAllTextures();

	RedrawAllDocuments();
}
