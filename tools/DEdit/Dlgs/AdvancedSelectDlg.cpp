//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// AdvancedSelectDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "advancedselectdlg.h"
#include "regiondoc.h"
#include "editprojectmgr.h"
#include "edithelpers.h"
#include "optionsadvancedselect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AdvancedSelectDlg dialog


AdvancedSelectDlg::AdvancedSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AdvancedSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(AdvancedSelectDlg)
	m_bNodesOfClass = false;
	m_ObjectName = _T("");
	m_ClassName = _T("");
	m_bObjectsWithName = false;
	m_nSelect = 0;
	m_bMatchWholeWord = false;
	m_bShowResults = false;
	m_bNodesWithProperty = false;
	m_sPropName = _T("Property Name");
	m_nPropType = 0;
	m_sPropValue = _T("true/false");
	m_bMatchValue = false;
	//}}AFX_DATA_INIT
}

// Loads the settings from the registry
void AdvancedSelectDlg::LoadRegistrySettings()
{
	// Get the options class
	COptionsAdvancedSelect *pOptions=GetApp()->GetOptions().GetAdvancedSelectOptions();
	if (!pOptions)
	{
		return;
	}

	// Set the member variables to their corresponding option value
	m_bNodesOfClass=pOptions->GetUseClassField();
	m_ClassName=pOptions->GetClassField();
	m_bMatchWholeWord=pOptions->GetMatchWholeName();

	m_bObjectsWithName=pOptions->GetUseNameField();
	m_ObjectName=pOptions->GetNameField();	

	m_bNodesWithProperty=pOptions->GetUsePropertyField();
	m_bMatchValue=pOptions->GetMatchValue();
	m_nPropType=pOptions->GetPropType();
	m_sPropName=pOptions->GetPropName();
	m_sPropValue=pOptions->GetPropValue();
	
	m_nSelect=pOptions->GetSelectionOperation();	

	m_bShowResults=pOptions->GetShowResults();
}

// Saves the settings from the registry
void AdvancedSelectDlg::SaveRegistrySettings()
{
	// Get the options class
	COptionsAdvancedSelect *pOptions=GetApp()->GetOptions().GetAdvancedSelectOptions();
	if (!pOptions)
	{
		return;
	}

	// Set the member variables to their corresponding option value
	pOptions->SetUseClassField(m_bNodesOfClass);
	pOptions->SetClassField(m_ClassName);
	pOptions->SetMatchWholeName(m_bMatchWholeWord);

	pOptions->SetUseNameField(m_bObjectsWithName);
	pOptions->SetNameField(m_ObjectName);	

	pOptions->SetUsePropertyField(m_bNodesWithProperty);
	pOptions->SetMatchValue(m_bMatchValue);
	pOptions->SetPropType(m_nPropType);
	pOptions->SetPropName(m_sPropName);
	pOptions->SetPropValue(m_sPropValue);
	
	pOptions->SetSelectionOperation(m_nSelect);	

	pOptions->SetShowResults(m_bShowResults);
}

/************************************************************************/
// Updates the enabled/disabled status of the controls
void AdvancedSelectDlg::UpdateEnabledStatus()
{
	UpdateData();

	BOOL bEnable;

	// Update the name controls
	bEnable=m_bObjectsWithName;
	GetDlgItem(IDC_OBJECTNAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_MATCH_WHOLE_WORD)->EnableWindow(bEnable);

	// Update the class controls	
	GetDlgItem(IDC_CLASSCOMBO)->EnableWindow(m_bNodesOfClass);

	// Update the property controls

	bEnable=m_bNodesWithProperty;
	GetDlgItem(IDC_PROPNAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_MATCH_VALUE)->EnableWindow(bEnable);

	bEnable = (m_bNodesWithProperty && m_bMatchValue);
	GetDlgItem(IDC_PROPTYPECOMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_PROPVALUE)->EnableWindow(bEnable);
	
	// Update the select button and the operation options
	if ((m_bObjectsWithName || m_bNodesOfClass || m_bNodesWithProperty) &&
		!(m_bNodesWithProperty && m_bMatchValue && !IsValueStringValid()))
	{
		bEnable=true;		
	}
	else
	{
		bEnable=false;		
	}

	GetDlgItem(IDOK)->EnableWindow(bEnable);
	GetDlgItem(IDC_RADIO_SELECT)->EnableWindow(bEnable);
	GetDlgItem(IDC_RADIO_DESELECT)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_OPERATION)->EnableWindow(bEnable);
}

void AdvancedSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AdvancedSelectDlg)
	DDX_Control(pDX, IDC_CLASSCOMBO, m_comboClass);
	DDX_Check(pDX, IDC_NODESOFCLASS, m_bNodesOfClass);
	DDX_Text(pDX, IDC_OBJECTNAME, m_ObjectName);
	DDX_CBString(pDX, IDC_CLASSCOMBO, m_ClassName);
	DDX_Check(pDX, IDC_OBJECTSWITHNAME, m_bObjectsWithName);
	DDX_Radio(pDX, IDC_RADIO_SELECT, m_nSelect);
	DDX_Check(pDX, IDC_CHECK_MATCH_WHOLE_WORD, m_bMatchWholeWord);
	DDX_Check(pDX, IDC_SHOWRESULTS, m_bShowResults);
	DDX_Check(pDX, IDC_NODESWITHPROPERTY, m_bNodesWithProperty);
	DDX_Check(pDX, IDC_MATCH_VALUE, m_bMatchValue);
	DDX_Text(pDX, IDC_PROPNAME, m_sPropName);
	DDX_Control(pDX, IDC_PROPTYPECOMBO, m_comboPropType);
	DDX_CBIndex(pDX, IDC_PROPTYPECOMBO, m_nPropType);
	DDX_Text(pDX, IDC_PROPVALUE, m_sPropValue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AdvancedSelectDlg, CDialog)
	//{{AFX_MSG_MAP(AdvancedSelectDlg)
	ON_BN_CLICKED(IDC_NODESOFCLASS, OnNodesOfClass)
	ON_BN_CLICKED(IDC_OBJECTSWITHNAME, OnObjectsWithName)
	ON_BN_CLICKED(IDC_NODESWITHPROPERTY, OnObjectsWithName) // does the same thing
	ON_BN_CLICKED(IDC_MATCH_VALUE, OnObjectsWithName) // does the same thing too
	ON_EN_CHANGE(IDC_PROPVALUE, OnObjectsWithName) // does the same thing too
	ON_CBN_SELCHANGE(IDC_PROPTYPECOMBO, OnChangePropType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AdvancedSelectDlg message handlers

BOOL AdvancedSelectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	//SCHLEGZ 12/31/97 Changed the classes edit to a combo box and filled it up with classes
	CProjectClass		*pRootClass;

	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_CLASSCOMBO);
	
	for(int i=0; i < GetProject()->m_Classes; i++)
	{
		pRootClass = GetProject()->m_Classes[i];
		if(pRootClass)
		{
			pBox->AddString(pRootClass->m_pClass->m_ClassName);
			AddItemsToBox( pRootClass->m_Children );
		}
	}

	//in addition we need to add artificial classes for prefab refs since they
	//don't really belong to any classes
	pBox->AddString("PrefabRef");


	pBox = (CComboBox*)GetDlgItem(IDC_PROPTYPECOMBO);
	
	pBox->AddString("Bool");
	pBox->AddString("Color");
	pBox->AddString("Flags");
	pBox->AddString("Long Int");
	pBox->AddString("Real");
	pBox->AddString("Rotation");
	pBox->AddString("String");
	pBox->AddString("Vector");
	

	// Select the proper string in the combobox
	m_comboClass.SelectString(-1, m_ClassName);

	m_comboPropType.SetCurSel(m_nPropType);

	// Update the enabled status
	UpdateEnabledStatus();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void AdvancedSelectDlg::AddItemsToBox( CMoArray<CProjectClass*> &classes )
{
	ClassDef *pClass;
	DWORD i;
	
	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_CLASSCOMBO);

	for(i=0; i < classes; i++ )
	{
		pClass = classes[i]->m_pClass;

		if(pClass->m_ClassFlags & CF_HIDDEN)
		{
			AddItemsToBox(classes[i]->m_Children);
		}
		else
		{
			pBox->AddString(pClass->m_ClassName);		
			AddItemsToBox(classes[i]->m_Children);
		}
	}
}

/************************************************************************/
// The "nodes" checkbox was clicked
void AdvancedSelectDlg::OnNodesOfClass() 
{
	// Update the enabled status of the controls
	UpdateEnabledStatus();		
}

/************************************************************************/
// The "name" checkbox was clicked
void AdvancedSelectDlg::OnObjectsWithName() 
{
	// Update the enabled status of the controls
	UpdateEnabledStatus();		
}

// A new property type was selected, so we must update the helper text
//
void AdvancedSelectDlg::OnChangePropType() 
{
	UpdateData();

	// Update the formatting text
	GetDlgItem(IDC_PROPVALUE)->SetWindowText(GetFormattingText());

	// Clear the Value string
	//m_sPropValue="";
	//UpdateData(FALSE);

	// Update the enabled status of the controls
	UpdateEnabledStatus();
}

/************************************************************************/
// Returns the formatting text for the selected property type
CString	AdvancedSelectDlg::GetFormattingText()
{
	// Determine the formatting text
	switch (GetPropertyTypeFromIndex(m_nPropType))
	{
	case LT_PT_STRING:
		{
			return "";
		}
	case LT_PT_REAL:
		{
			return "";
		}
	case LT_PT_LONGINT:
		{
			return "";
		}
	case LT_PT_FLAGS:
		{
			return "";
		}
	case LT_PT_VECTOR:
		{
			return "x y z";
		}
	case LT_PT_ROTATION:
		{
			return "yaw pitch roll";
		}
	case LT_PT_COLOR:
		{
			return "red green blue";
		}
	case LT_PT_BOOL:
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
// Returns the property type for a specific index in the combo box
int	AdvancedSelectDlg::GetPropertyTypeFromIndex(int nIndex)
{
	switch (nIndex)
	{
	case 0:
		{
			return LT_PT_BOOL;
		}
	case 1:
		{
			return LT_PT_COLOR;
		}
	case 2:
		{
			return LT_PT_FLAGS;
		}
	case 3:
		{
			return LT_PT_LONGINT;
		}
	case 4:
		{
			return LT_PT_REAL;
		}
	case 5:
		{
			return LT_PT_ROTATION;
		}
	case 6:
		{
			return LT_PT_STRING;
		}
	case 7:
		{
			return LT_PT_VECTOR;
		}
	default:
		{
			ASSERT(FALSE);
			return -1;
		}
	}
}


/************************************************************************/
// This returns TRUE if the value text is valid with the
// selected property type.
bool AdvancedSelectDlg::IsValueStringValid()
{
	// Determine the formatting text
	switch (GetPropertyTypeFromIndex(m_nPropType))
	{
	case LT_PT_STRING:
		{
			// Any string is always valid
			return TRUE;
			break;
		}		
	case LT_PT_REAL:
	case LT_PT_LONGINT:
	case LT_PT_FLAGS:
		{			
			// Check to see if we can get a number from the string
			float fNumber=0;
			if (sscanf(m_sPropValue, "%f", &fNumber) > 0)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
			break;
		}
	case LT_PT_VECTOR:
	case LT_PT_ROTATION:
	case LT_PT_COLOR:
		{
			// Make sure that the vector/rotation/color is in the correct format
			float fX, fY, fZ;
			if (sscanf(m_sPropValue, "%f %f %f", &fX, &fY, &fZ) >= 3)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
			break;
		}
	case LT_PT_BOOL:
		{
			// Convert the string to uppercase for comparison reasons
			CString sUpperValue=m_sPropValue;
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
// This allocates a property object and fills it in with the
// data from the dialog box.  The function returns NULL if
// a valid property cannot be made.
CBaseProp *AdvancedSelectDlg::AllocPropertyFromData()
{
	// Check if we're not specifying value
	if (!m_bMatchValue)
	{
		return new CBaseProp(m_sPropName);
	}

	// Make sure that the value string is valid
	if (!IsValueStringValid())
	{
		return NULL;
	}

	// Build the property 
	CBaseProp *pProperty=NULL;

	switch (GetPropertyTypeFromIndex(m_nPropType))
	{
	case LT_PT_STRING:
		{
			// Allocate the property
			pProperty=new CStringProp(m_sPropName);

			// Set the property data
			((CStringProp *)pProperty)->SetString(m_sPropValue);
			break;
		}
	case LT_PT_REAL:
		{
			// Allocate the property
			pProperty=new CRealProp(m_sPropName);

			// Set the property data						
			((CRealProp *)pProperty)->SetValue(atof(m_sPropValue));
			break;
		}
	case LT_PT_LONGINT:
		{
			// Allocate the property			
			pProperty=new CRealProp(m_sPropName);

			// Set the property data						
			((CRealProp *)pProperty)->SetValue(atof(m_sPropValue));

			// Set the property type
			pProperty->m_Type=LT_PT_LONGINT;
			break;
		}
	case LT_PT_FLAGS:
		{
			// Allocate the property			
			pProperty=new CRealProp(m_sPropName);

			// Set the property data						
			((CRealProp *)pProperty)->SetValue(atof(m_sPropValue));

			// Set the property type
			pProperty->m_Type=LT_PT_FLAGS;
			break;
		}
	case LT_PT_VECTOR:
		{
			// Allocate the property
			pProperty=new CVectorProp(m_sPropName);

			// Set the property data
			CVector v;
			sscanf(m_sPropValue, "%f %f %f", &v.x, &v.y, &v.z);
			((CVectorProp *)pProperty)->SetVector(v);
			break;
		}
	case LT_PT_ROTATION:
		{
			// Allocate the property
			pProperty=new CRotationProp(m_sPropName);

			// Set the property data
			LTVector rotation;
			sscanf(m_sPropValue, "%f %f %f", &rotation.y, &rotation.x, &rotation.z);

			// Convert from degrees to radians...
			VEC_MULSCALAR(rotation, rotation, MATH_CIRCLE / 360.0f);

			((CRotationProp *)pProperty)->SetEulerAngles(rotation);
			break;
		}
	case LT_PT_COLOR:
		{
			// Allocate the property
			pProperty=new CColorProp(m_sPropName);

			// Set the property data
			CVector vColor;
			sscanf(m_sPropValue, "%f %f %f", &vColor.x, &vColor.y, &vColor.z);
			((CVectorProp *)pProperty)->SetVector(vColor);
			break;
		}
	case LT_PT_BOOL:
		{
			// Allocate the property
			pProperty=new CBoolProp(m_sPropName);

			// Set the property data
			CString sUpperValue=m_sPropValue;
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