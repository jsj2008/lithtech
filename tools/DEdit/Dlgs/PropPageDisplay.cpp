//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// PropPageDisplay.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "proppagedisplay.h"
#include "optionsdisplay.h"
#include "edithelpers.h"
#include "colorpicker.h"
#include "regiondoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageDisplay property page

IMPLEMENT_DYNCREATE(CPropPageDisplay, CPropertyPage)

CPropPageDisplay::CPropPageDisplay() : CPropertyPage(CPropPageDisplay::IDD)
{
	//{{AFX_DATA_INIT(CPropPageDisplay)
	m_bShowSurfaceColor		= FALSE;
	m_nHandleSize			= 3;
	m_nVertexSize			= 2;
	m_nVertexDrawRule		= 0;
	m_nPerspectiveFarZ		= 0;
	m_nClassIconSize		= 30;
	m_bOrientObjectBoxes	= TRUE;
	m_bTintSelected			= TRUE;
	m_bTintFrozen			= TRUE;
	m_bShadePolygons		= TRUE;
	m_bShowSelectedDecals	= TRUE;
	//}}AFX_DATA_INIT
	m_bInit=FALSE;
}

CPropPageDisplay::~CPropPageDisplay()
{
}

void CPropPageDisplay::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageDisplay)
	DDX_Control(pDX, IDC_SPIN_VERTEX_SIZE, m_spinVertexSize);
	DDX_Control(pDX, IDC_SPIN_HANDLE_SIZE, m_spinHandleSize);
	DDX_Control(pDX, IDC_SPIN_CLASS_ICON_SIZE, m_spinClassIconSize);
	DDX_Control(pDX, IDC_LIST_COLOR_CHOICES, m_listColorChoices);
	DDX_Check(pDX, IDC_CHECK_SHOW_SURFACE_COLOR, m_bShowSurfaceColor);
	DDX_Check(pDX, IDC_CHECK_SHOW_SELECTED_DECALS, m_bShowSelectedDecals);
	DDX_Check(pDX, IDC_CHECK_ORIENT_OBJECT_BOXES, m_bOrientObjectBoxes);
	DDX_Check(pDX, IDC_CHECK_TINT_SELECTED, m_bTintSelected);
	DDX_Check(pDX, IDC_CHECK_TINT_FROZEN, m_bTintFrozen);
	DDX_Check(pDX, IDC_CHECK_SHADE_POLYGONS, m_bShadePolygons);
	DDX_Check(pDX, IDC_CHECK_DISPLAY_CLASS_ICONS, m_bShowClassIcons);
	DDX_Text(pDX, IDC_EDIT_CLASS_ICON_SIZE, m_nClassIconSize);
	DDV_MinMaxInt(pDX, m_nClassIconSize, 1, 256);
	DDX_Text(pDX, IDC_EDIT_HANDLE_SIZE, m_nHandleSize);
	DDX_Text(pDX, IDC_EDIT_PERSPECTIVE_FAR_Z, m_nPerspectiveFarZ);
	DDX_Text(pDX, IDC_EDIT_CLASS_ICON_DIR, m_sClassIconDir);
	DDV_MinMaxInt(pDX, m_nHandleSize, 1, 25);
	DDX_Text(pDX, IDC_EDIT_VERTEX_SIZE, m_nVertexSize);
	DDV_MinMaxInt(pDX, m_nVertexSize, 1, 25);
	DDX_Radio(pDX, IDC_RADIO_VERTEX_RULE, m_nVertexDrawRule);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropPageDisplay, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageDisplay)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_COLOR, OnButtonChangeColor)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_LIST_COLOR_CHOICES, OnSelchangeListColorChoices)
	ON_LBN_DBLCLK(IDC_LIST_COLOR_CHOICES, OnDblclkListColorChoices)
	ON_BN_CLICKED(IDC_STATIC_COLOR_BOX, OnStaticColorBox)
	ON_BN_CLICKED(IDC_CHECK_SHOW_SURFACE_COLOR, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_CHECK_AUTO_LOAD_LIGHTMAPS, UpdateDataHandler)
	ON_BN_CLICKED(IDC_CHECK_SHOW_SELECTED_DECALS, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_CHECK_ORIENT_OBJECT_BOXES, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_CHECK_PREFABS_AS_BOXES, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_RADIO_SELECTED_BRUSHES, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_RADIO_VERTEX_RULE, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnButtonReset)
	ON_EN_CHANGE(IDC_EDIT_HANDLE_SIZE, UpdateDataHandlerAndDraw)
	ON_EN_CHANGE(IDC_EDIT_VERTEX_SIZE, UpdateDataHandlerAndDraw)
	ON_EN_CHANGE(IDC_EDIT_CLASS_ICON_SIZE, UpdateDataHandlerAndDraw)
	ON_EN_CHANGE(IDC_EDIT_PERSPECTIVE_FAR_Z, UpdateDataHandlerAndDraw)
	ON_EN_CHANGE(IDC_EDIT_CLASS_ICON_DIR, UpdateDataHandler)
	ON_BN_CLICKED(IDC_CHECK_TINT_SELECTED, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_CHECK_TINT_INVALID, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_CHECK_TINT_FROZEN, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_CHECK_TINT_DETAIL, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_CHECK_DISPLAY_CLASS_ICONS, UpdateDataHandlerAndDraw)
	ON_BN_CLICKED(IDC_BROWSE_CLASS_ICON_DIR, OnBrowseClassIcons)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageDisplay message handlers

/************************************************************************/
// Dialog initialization
BOOL CPropPageDisplay::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// The display options class
	COptionsDisplay *pDisplayOptions=GetApp()->GetOptions().GetDisplayOptions();
	if (!pDisplayOptions)
	{
		ASSERT(FALSE);
		return TRUE;
	}

	// Fill the listbox with the color choices
	int i;
	for (i=0; i < COptionsDisplay::kNumColorOptions; i++)
	{
		m_listColorChoices.AddString(pDisplayOptions->GetColorItem(i)->GetDisplayName());
	}

	// Select the first item in the list
	m_listColorChoices.SetCurSel(0);
	
	// Get the other display options
	m_bShowSurfaceColor		= pDisplayOptions->IsShowSurfaceColor();
	m_nVertexSize			= pDisplayOptions->GetVertexSize();
	m_nHandleSize			= pDisplayOptions->GetHandleSize();
	m_nVertexDrawRule		= pDisplayOptions->GetVertexDrawRule();
	m_nPerspectiveFarZ		= pDisplayOptions->GetPerspectiveFarZ();
	m_bOrientObjectBoxes	= pDisplayOptions->IsOrientObjectBoxes();
	m_bTintSelected			= pDisplayOptions->IsTintSelected();
	m_bTintFrozen			= pDisplayOptions->IsTintFrozen();
	m_bShadePolygons		= pDisplayOptions->IsShadePolygons();
	m_bShowClassIcons		= pDisplayOptions->IsShowClassIcons();
	m_sClassIconDir			= pDisplayOptions->GetClassIconsDir();
	m_nClassIconSize		= pDisplayOptions->GetClassIconSize();
	m_bShowSelectedDecals	= pDisplayOptions->IsShowSelectedDecals();

	// Setup the spin controls
	m_spinVertexSize.SetRange(1, 25);
	m_spinHandleSize.SetRange(1, 25);
	m_spinClassIconSize.SetRange(1, 256);

	UpdateData(FALSE);

	m_bInit=TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/************************************************************************/
// Handle the paint message
void CPropPageDisplay::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// The display options class
	COptionsDisplay *pDisplayOptions=GetApp()->GetOptions().GetDisplayOptions();
	if (!pDisplayOptions)
	{
		ASSERT(FALSE);
		return;
	}

	// Make sure that an item is selected
	int nSelectedItem=m_listColorChoices.GetCurSel();
	if (nSelectedItem == LB_ERR)
	{
		return;
	}

	// Get the client rectangle for the color box
	CRect rcColor;
	GetDlgItem(IDC_STATIC_COLOR_BOX)->GetWindowRect(rcColor);
	ScreenToClient(rcColor);
	
	// Get the color to paint with
	COLORREF crColor=pDisplayOptions->GetColor(nSelectedItem);
	
	// Draw the color box
	dc.FillRect(rcColor, &CBrush(crColor));
}

/************************************************************************/
// The "Change Color" button was pressed
void CPropPageDisplay::OnButtonChangeColor() 
{
	// The display options class
	COptionsDisplay *pDisplayOptions=GetApp()->GetOptions().GetDisplayOptions();
	if (!pDisplayOptions)
	{
		ASSERT(FALSE);
		return;
	}

	// Make sure that an item is selected
	int nSelectedItem=m_listColorChoices.GetCurSel();
	if (nSelectedItem == LB_ERR)
	{
		return;
	}

	// Load the color picker
	CColorPicker colorPicker;
	colorPicker.Init();
	colorPicker.SetCurrentColor(pDisplayOptions->GetColor(nSelectedItem));

	if (colorPicker.DoModal() == IDOK)
	{
		pDisplayOptions->GetColorItem(nSelectedItem)->SetColor(colorPicker.GetColor());
		Invalidate();
	}
	
	// Redraw all of the documents
	RedrawAllDocuments();

	// Save the color options
	pDisplayOptions->Save();
}

/************************************************************************/
// The color choices dialog has been double clicked
void CPropPageDisplay::OnDblclkListColorChoices() 
{
	OnButtonChangeColor();	
}

/************************************************************************/
// The color box has been clicked
void CPropPageDisplay::OnStaticColorBox() 
{
	OnButtonChangeColor();	
}

/************************************************************************/
// The selected color item has changed
void CPropPageDisplay::OnSelchangeListColorChoices() 
{
	// Get the client rectangle for the color box
	CRect rcColor;
	GetDlgItem(IDC_STATIC_COLOR_BOX)->GetWindowRect(rcColor);	
	ScreenToClient(rcColor);

	InvalidateRect(rcColor);
}

/************************************************************************/
// The user is looking for a directory for the class icons
void CPropPageDisplay::OnBrowseClassIcons()
{
	//we want an open file dialog that will look for DTX files
	CFileDialog Dlg(TRUE, "dtx", NULL, 0, "DTX Files (*.dtx)|*.dtx|All Files (*.*)|*.*||");

	if(Dlg.DoModal() == IDOK)
	{
		//first, strip off the filename if needed
		CString sPath = Dlg.GetPathName();
		sPath.Replace(Dlg.GetFileName(), "");

		COptionsDisplay *pDisplayOptions=GetApp()->GetOptions().GetDisplayOptions();

		//first set this value
		pDisplayOptions->SetClassIconsDir(sPath);

		//set the text for the edit box
		((CEdit*)GetDlgItem(IDC_EDIT_CLASS_ICON_DIR))->SetWindowText(pDisplayOptions->GetClassIconsDir());
	}
}

/************************************************************************/
// Updates the display options class with the dialog data
void CPropPageDisplay::UpdateDisplayOptions()
{
	if (!m_bInit)
	{
		return;
	}

	UpdateData();
	
	// The display options class
	COptionsDisplay *pDisplayOptions=GetApp()->GetOptions().GetDisplayOptions();
	if (!pDisplayOptions)
	{
		ASSERT(FALSE);
		return;
	}

	// Set the option in the display class options	
	pDisplayOptions->SetShowSurfaceColor(m_bShowSurfaceColor);
	pDisplayOptions->SetVertexSize(m_nVertexSize);
	pDisplayOptions->SetHandleSize(m_nHandleSize);
	pDisplayOptions->SetPerspectiveFarZ(m_nPerspectiveFarZ);
	pDisplayOptions->SetOrientObjectBoxes(m_bOrientObjectBoxes);
	pDisplayOptions->SetTintSelected(m_bTintSelected);
	pDisplayOptions->SetTintFrozen(m_bTintFrozen);
	pDisplayOptions->SetShowClassIcons(m_bShowClassIcons);
	pDisplayOptions->SetClassIconsDir(m_sClassIconDir);
	pDisplayOptions->SetClassIconSize(m_nClassIconSize);
	pDisplayOptions->SetShowSelectedDecals(m_bShowSelectedDecals);
	pDisplayOptions->SetShadePolygons(m_bShadePolygons);

	// Set the vertex rule
	switch (m_nVertexDrawRule)
	{
	case 0:
		{
			pDisplayOptions->SetVertexDrawRule(COptionsDisplay::kVertexDrawAll);
			break;
		}
	case 1:
		{
			pDisplayOptions->SetVertexDrawRule(COptionsDisplay::kVertexDrawSelectedOnly);
			break;
		}
	default:
		{
			ASSERT(FALSE);
			break;
		}
	}
}

//**********************************************************************/
// defualt handler that will update the data
void CPropPageDisplay::UpdateDataHandler()
{
	// Update the display options
	UpdateDisplayOptions();
}

//**********************************************************************/
// defualt handler that will update the data
void CPropPageDisplay::UpdateDataHandlerAndDraw()
{
	// Update the display options
	UpdateDisplayOptions();

	// Redraw all of the documents
	RedrawAllDocuments();
}


/************************************************************************/
// The window is being destroyed
void CPropPageDisplay::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	
	m_bInit=FALSE;	
}

/************************************************************************/
// The reset button was pressed
void CPropPageDisplay::OnButtonReset() 
{
	// The display options class
	COptionsDisplay *pDisplayOptions=GetApp()->GetOptions().GetDisplayOptions();
	if (!pDisplayOptions)
	{
		ASSERT(FALSE);
		return;
	}

	// Display the warning message box
	CString sMessage="Do you want to reset your colors to the default settings?";
	if (MessageBox(sMessage, "Reset", MB_YESNO) == IDYES)
	{
		// Reset the colors
		pDisplayOptions->ResetToDefaultColors();

		// Invalidate the dialog
		Invalidate();

		// Redraw all of the documents
		RedrawAllDocuments();

		// Save the color options
		pDisplayOptions->Save();
	}
}
