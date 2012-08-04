// AdvancedSelectPropertyDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "advancedselectpropertydlg.h"
#include "colorpicker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSelectPropertyDlg dialog


CAdvancedSelectPropertyDlg::CAdvancedSelectPropertyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAdvancedSelectPropertyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvancedSelectPropertyDlg)
	m_sName = _T("");
	m_sValue = _T("");
	m_nSelPropertyIndex = 0;
	//}}AFX_DATA_INIT
}


void CAdvancedSelectPropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvancedSelectPropertyDlg)
	DDX_Text(pDX, IDC_EDIT_PROPERTY_NAME, m_sName);
	DDX_Text(pDX, IDC_EDIT_PROPERTY_VALUE, m_sValue);
	DDX_CBIndex(pDX, IDC_COMBO_PROPERTY_TYPES, m_nSelPropertyIndex);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvancedSelectPropertyDlg, CDialog)
	//{{AFX_MSG_MAP(CAdvancedSelectPropertyDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_PROPERTY_TYPES, OnSelchangeComboPropertyTypes)
	ON_EN_CHANGE(IDC_EDIT_PROPERTY_VALUE, OnChangeEditPropertyValue)
	ON_EN_CHANGE(IDC_EDIT_PROPERTY_NAME, OnChangeEditPropertyName)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_PICK, OnButtonColorPick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSelectPropertyDlg message handlers

BOOL CAdvancedSelectPropertyDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Update the formatting text
	GetDlgItem(IDC_STATIC_FORMATTING_TEXT)->SetWindowText(GetSelectedFormattingText());
	
	// Update the enabled status of the controls
	UpdateEnabledStatus();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvancedSelectPropertyDlg::OnSelchangeComboPropertyTypes() 
{
	UpdateData();

	// Update the formatting text
	GetDlgItem(IDC_STATIC_FORMATTING_TEXT)->SetWindowText(GetSelectedFormattingText());

	// Clear the Value string
	m_sValue="";
	UpdateData(FALSE);

	// Update the enabled status of the controls
	UpdateEnabledStatus();
}

/************************************************************************/
// This allocates a property object and fills it in with the
// data from the dialog box.  The function returns NULL if
// a valid property cannot be made.
CBaseProp *CAdvancedSelectPropertyDlg::AllocPropertyFromData()
{
	// Make sure that the value string is valid
	if (!IsValueStringValid())
	{
		return NULL;
	}

	// Build the property 
	CBaseProp *pProperty=NULL;

	switch (GetPropertyTypeFromIndex(m_nSelPropertyIndex))
	{
	case PT_STRING:
		{
			// Allocate the property
			pProperty=new CStringProp(m_sName);

			// Set the property data
			((CStringProp *)pProperty)->SetString(m_sValue);
			break;
		}
	case PT_REAL:
		{
			// Allocate the property
			pProperty=new CRealProp(m_sName);

			// Set the property data						
			((CRealProp *)pProperty)->SetValue(atof(m_sValue));
			break;
		}
	case PT_LONGINT:
		{
			// Allocate the property			
			pProperty=new CRealProp(m_sName);

			// Set the property data						
			((CRealProp *)pProperty)->SetValue(atof(m_sValue));

			// Set the property type
			pProperty->m_Type=PT_LONGINT;
			break;
		}
	case PT_VECTOR:
		{
			// Allocate the property
			pProperty=new CVectorProp(m_sName);

			// Set the property data
			CVector v;
			sscanf(m_sValue, "%f %f %f", &v.x, &v.y, &v.z);
			((CVectorProp *)pProperty)->SetVector(v);
			break;
		}
	case PT_ROTATION:
		{
			// Allocate the property
			pProperty=new CRotationProp(m_sName);

			// Set the property data
			DRotation rotation;
			sscanf(m_sValue, "%f %f %f", &rotation.m_Vec.y, &rotation.m_Vec.x, &rotation.m_Vec.z);

			// Convert from degrees to radians...
			VEC_MULSCALAR(rotation.m_Vec, rotation.m_Vec, MATH_CIRCLE / 360.0f);

			((CRotationProp *)pProperty)->m_Rotation=rotation;
			break;
		}
	case PT_COLOR:
		{
			// Allocate the property
			pProperty=new CColorProp(m_sName);

			// Set the property data
			CVector vColor;
			sscanf(m_sValue, "%f %f %f", &vColor.x, &vColor.y, &vColor.z);
			((CVectorProp *)pProperty)->SetVector(vColor);
			break;
		}
	case PT_BOOL:
		{
			// Allocate the property
			pProperty=new CBoolProp(m_sName);

			// Set the property data
			CString sUpperValue=m_sValue;
			sUpperValue.MakeUpper();
			if (sUpperValue == "TRUE")
			{
				((CBoolProp *)pProperty)->SetValue(TRUE);
			}
			else
			{
				((CBoolProp *)pProperty)->SetValue(FALSE);
			}			
			break;
		}
	default:
		{
			ASSERT(FALSE);			
		}
	}
	return pProperty;
}

/************************************************************************/
// Returns the property type for a specific index in the combo box
int	CAdvancedSelectPropertyDlg::GetPropertyTypeFromIndex(int nIndex)
{
	switch (nIndex)
	{
	case 0:
		{
			return PT_STRING;
		}
	case 1:
		{
			return PT_REAL;
		}
	case 2:
		{
			return PT_LONGINT;
		}
	case 3:
		{
			return PT_VECTOR;
		}
	case 4:
		{
			return PT_ROTATION;
		}
	case 5:
		{
			return PT_COLOR;
		}
	case 6:
		{
			return PT_BOOL;
		}
	default:
		{
			ASSERT(FALSE);
			return -1;
		}
	}
}

/************************************************************************/
// Returns the formatting text for the selected property type
CString	CAdvancedSelectPropertyDlg::GetSelectedFormattingText()
{
	// Determine the formatting text
	switch (GetPropertyTypeFromIndex(m_nSelPropertyIndex))
	{
	case PT_STRING:
		{
			return "";
		}
	case PT_REAL:
		{
			return "";
		}
	case PT_LONGINT:
		{
			return "";
		}
	case PT_VECTOR:
		{
			return "x y z";
		}
	case PT_ROTATION:
		{
			return "yaw pitch roll";
		}
	case PT_COLOR:
		{
			return "r g b";
		}
	case PT_BOOL:
		{
			return "true/false";
		}
	default:
		{
			ASSERT(FALSE);
			return "";
		}
	}
}

/************************************************************************/
// This returns TRUE if the value text is valid with the
// selected property type.
BOOL CAdvancedSelectPropertyDlg::IsValueStringValid()
{
	// Determine the formatting text
	switch (GetPropertyTypeFromIndex(m_nSelPropertyIndex))
	{
	case PT_STRING:
		{
			// Any string is always valid
			return TRUE;
			break;
		}		
	case PT_REAL:
	case PT_LONGINT:
		{			
			// Check to see if we can get a number from the string
			float fNumber=0;
			if (sscanf(m_sValue, "%f", &fNumber) > 0)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
			break;
		}
	case PT_VECTOR:
	case PT_ROTATION:
	case PT_COLOR:
		{
			// Make sure that the vector/rotation/color is in the correct format
			float fX, fY, fZ;
			if (sscanf(m_sValue, "%f %f %f", &fX, &fY, &fZ) >= 3)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
			break;
		}
	case PT_BOOL:
		{
			// Convert the string to uppercase for comparison reasons
			CString sUpperValue=m_sValue;
			sUpperValue.MakeUpper();

			if (sUpperValue == "TRUE" || sUpperValue == "FALSE")
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
			break;
		}
	default:
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}
}

/************************************************************************/
// This updates the enabled status of controls in the dialog
// depending on the values entered into the controls.
void CAdvancedSelectPropertyDlg::UpdateEnabledStatus()
{
	// Start out as enabled
	BOOL bEnable=TRUE;

	// Make sure that there is a string typed into the name field
	if (m_sName.GetLength() <= 0)
	{
		bEnable=FALSE;
	}

	// Make sure that the value field is valid
	if (!IsValueStringValid())
	{
		bEnable=FALSE;
	}

	// Update the OK button
	GetDlgItem(IDOK)->EnableWindow(bEnable);

	// Check to see if the color type is selected.  If so, then show
	// the Pick button.
	if (GetPropertyTypeFromIndex(m_nSelPropertyIndex) == PT_COLOR)
	{
		GetDlgItem(IDC_BUTTON_COLOR_PICK)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_COLOR_PICK)->ShowWindow(SW_HIDE);
	}
}

void CAdvancedSelectPropertyDlg::OnChangeEditPropertyValue() 
{
	// Update the data
	UpdateData();

	// Update the enabled status of the controls
	UpdateEnabledStatus();
}

void CAdvancedSelectPropertyDlg::OnChangeEditPropertyName() 
{
	// Update the enabled status of the controls
	UpdateEnabledStatus();	
}

void CAdvancedSelectPropertyDlg::OnButtonColorPick() 
{
	// Update the data
	UpdateData();

	// Load the color picker
	CColorPicker colorPicker;
	colorPicker.Init();
	if (colorPicker.DoModal() == IDOK)
	{
		// Get the color
		COLORREF cr=colorPicker.GetColor();

		// Set the color
		m_sValue.Format("%d %d %d", GetRValue(cr), GetGValue(cr), GetBValue(cr));
		UpdateData(FALSE);
	}

	// Update the enabled status of the controls
	UpdateEnabledStatus();	
}
