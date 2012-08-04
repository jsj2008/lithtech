//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// PropertiesDlg.cpp : implementation file
//

#include "bdefs.h"
#include "worldnode.h"
#include "butemgr.h"
#include "dedit.h"
#include "regiondoc.h"

#include "colorwell.h"
#include "vectoredit.h"
#include "regiondoc.h"
#include "mainfrm.h"
#include "vectorbutton.h"
#include "rotationedit.h"
#include "lightattenuation.h"
#include "multilinestringdlg.h"
#include "colorpicker.h"
#include "edithelpers.h"
#include "classdlg.h"
#include "regionview.h"
#include "propertiesdlg.h"
#include "nodeview.h"
#include "classdlg.h"
#include "popupnotedlg.h"
#include "objectbrowserdlg.h"
#include "optionsobjectbrowser.h"
#include "propeditstring.h"
#include "propeditstringlist.h"
#include "deditinternal.h"
#include "eventeditordlg.h"
#include "selecteffectgroupdlg.h"

#define PROPDLG_ID_BASE			45000
#define PROPDLG_STATIC_ID		100000

#define PROPDLG_HORZ_PAD	5

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertiesControls dialog


CPropertiesControls::CPropertiesControls(UINT id, CPropList *pList, CNotifier *pNotifier)
	: CDialog(id, NULL)
{
	InitMe();
	m_pPropList = pList;
	m_pNotifier = pNotifier;
	m_bModal = TRUE;	
}


CPropertiesControls::CPropertiesControls(CWnd* pParent /*=NULL*/)
	: CDialog(CPropertiesControls::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPropertiesControls)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bModal = FALSE;
	InitMe();
}


CPropertiesControls::~CPropertiesControls()
{
	if (m_pFont)
	{
		delete m_pFont;
		m_pFont=NULL;
	}
	if (m_pPopupNoteDlg)
	{
		delete m_pPopupNoteDlg;
		m_pPopupNoteDlg=NULL;
	}
	
	Term( FALSE );
}

BEGIN_MESSAGE_MAP(CPropertiesControls, CDialog)
	//{{AFX_MSG_MAP(CPropertiesControls)
	ON_WM_PAINT()
	ON_WM_KILLFOCUS()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()





/////////////////////////////////////////////////////////////////////////////
// CPropertiesControls interface


void CPropertiesControls::InitMe()
{
	m_pPropList = NULL;
	m_pNotifier = NULL;
		
	m_Controls = NULL;
	m_ppLabelControls = NULL;		

	m_nProperties = 0;
	m_nControls = 0;
	m_iCurPos = 0;
	m_iPage = 0;
	m_iMaxPos = 0;
	m_bReadingControlValues = FALSE;

	m_pFont = new CFont;
	m_pPopupNoteDlg=new CPopupNoteDlg;
	m_pFont->CreatePointFont( 90, "Ariel" );

	m_pCurProp = NULL;
	
	m_bStringModified = false;
}


BOOL CPropertiesControls::Init( CPropList *pList, CNotifier *pNotifier )
{
	CRect		clientRect, rc1, rc2;
	LOGFONT		logFont;
	CWnd		*pItem;

	Term();

	m_pNotifier = pNotifier;

	// Init stuff.
	SetFont( m_pFont, FALSE );

	m_nProperties = pList->m_Props.GetSize();
	m_pPropList = pList;
	m_nControls = 0;
	m_iCurPos = 0;
	m_iPage = 0;
	m_iMaxPos = 0;

	// Get the dimensions of the items.
	GetParent( )->GetClientRect( &clientRect );
	m_pFont->GetLogFont( &logFont );

	m_ItemWidth = (clientRect.Width() - (PROPDLG_HORZ_PAD*2)) / 2;
	// BL 11/5/99 
	// Will need to alter multiplier to 1.8 from 1.3 to accomodate comboboxes.
	m_ItemHeight = (int)(-logFont.lfHeight * 1.8f);

	EnableScrollBar( SB_VERT );
	
	// Create all the controls.
	CreateControls();
	ReadControlValues();

	UpdateScrollBars( );	
	InvalidateRect( NULL, TRUE );
	return TRUE;
}


void CPropertiesControls::Term( BOOL bClear )
{
	DWORD		i;

	// BP 1/3/98
	// This needs to be here, because when the m_Controls are being deleted, somehow
	// a windows WM_COMMAND message gets sent and OnCommand gets called with the items
	// in a bad state.
	m_pNotifier = NULL;
	m_pPropList = NULL;

	m_pCurProp			= NULL;
	m_bStringModified	= false;
		
	// Delete the controls
	if( m_Controls )
	{
		for( i=0; i < m_nProperties; i++ )
			if( m_Controls[i] )
				delete m_Controls[i];

		delete[] m_Controls;
		m_Controls = NULL;
	}

	// Delete the static label controls
	if( m_ppLabelControls )
	{
		for( i=0; i < m_nProperties; i++ )
			if( m_ppLabelControls[i] )
				delete m_ppLabelControls[i];

		delete[] m_ppLabelControls;
		m_ppLabelControls = NULL;
	}	

	// Remove the help for each control
	m_sControlHelpArray.RemoveAll();

	m_nProperties = 0;
	m_nControls = 0;
	
	if( bClear )
		InvalidateRect( NULL, TRUE );
}


void CPropertiesControls::TermIf( CPropList *pList, BOOL bClear )
{
	if( m_pPropList == pList )
		Term( bClear );
}


void CPropertiesControls::NotifyChange( CBaseProp *pProp, BOOL bUpdateView )
{
	if( m_pNotifier )
	{
		if( pProp == NULL )
			m_pNotifier->NotifyPropertiesChange( bUpdateView );
		else
			m_pNotifier->NotifyPropertyChange( pProp, bUpdateView );
	}

	//now we need to tell all the objects themselves that their properties have
	//changed so that they can have a chance to update internal state
	CRegionDoc* pDoc = GetActiveRegionDoc();

	if(pDoc)
	{
		CEditRegion* pRegion = &(pDoc->m_Region);

		//run through all selected objects
		for(uint32 nCurrObj = 0; nCurrObj < pRegion->m_Selections.GetSize(); nCurrObj++)
		{
			//update that this property changed
			pRegion->m_Selections[nCurrObj]->OnPropertyChanged(pProp, true, NULL);
		}
	}
}


void CPropertiesControls::GetLabelRect( int iLabel, CRect &rect )
{
	rect.left = PROPDLG_HORZ_PAD;
	rect.top = ( iLabel - m_iCurPos ) * m_ItemHeight;
	rect.right = (rect.left + m_ItemWidth) - PROPDLG_HORZ_PAD;
	rect.bottom = rect.top + m_ItemHeight;
}


void CPropertiesControls::GetControlRect( int iControl, CRect &rect )
{
	rect.left = (PROPDLG_HORZ_PAD*2)+m_ItemWidth;
	rect.top = ( iControl - m_iCurPos ) * m_ItemHeight;
	rect.right = (rect.left + m_ItemWidth) - PROPDLG_HORZ_PAD;
	rect.bottom = rect.top + m_ItemHeight;
}


void CPropertiesControls::CreateControls()
{
	CBaseProp		*pProp;
	CEdit			*pEdit;
	CButton			*pButton;
	CRect			rect;
	CWnd *pWnd;


	m_Controls = new CWnd*[m_nProperties];
	m_ppLabelControls = new CStatic*[m_nProperties];
	memset(m_ppLabelControls, NULL, m_nProperties*sizeof(CStatic*));

	m_nControls = 0;
	for(DWORD i=0; i < m_nProperties; i++)
	{
		pProp = m_pPropList->m_Props[i];

		if(pProp->m_PropFlags & PF_HIDDEN)
		{
			m_Controls[i] = NULL;
			continue;
		}

		if(!m_bModal)
		{
			if((pProp->m_PropFlags & PF_GROUPMASK) && !(pProp->m_PropFlags & PF_GROUPOWNER))
			{
				m_Controls[i] = NULL;
				continue;
			}
		}
		
		// Create the static label
		CStatic *pStaticCtrl=CreateStaticLabel(pProp, m_nControls);						
		m_ppLabelControls[m_nControls]=pStaticCtrl;

		// Get the help for this control
		m_sControlHelpArray.Add(GetPropHelpString(pProp));
		GetControlRect( m_nControls, rect );

		// Count this property as a control...
		m_nControls++;


		if(pProp->m_PropFlags & PF_COMPOSITETYPE)
		{
			// this property is a composite type.  the property name specifies which custom controls to use
			if( 0 == strcmp( pProp->m_Name, "LightAttenuation" ) )
			{
				pButton = new CButton;
				pButton->Create( "...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect, this, PROPDLG_ID_BASE+i );
				m_Controls[i] = pButton;
			}
			else
			{
				ASSERT( 0 );	// a composite type of unknown type was found, ignoring it
				m_Controls[i] = NULL;
			}
		}
		else if(pProp->m_PropFlags & PF_GROUPOWNER)
		{
			// this button launches a modal dialog for setting a group of properties
			pButton = new CButton;
			pButton->Create( "...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect, this, PROPDLG_ID_BASE+i );
			m_Controls[i] = pButton;
		}
		else
		{
			switch( pProp->m_Type )
			{
				case LT_PT_STRING:
				{
					if ( (pProp->m_PropFlags & PF_STATICLIST) || (pProp->m_PropFlags & PF_DYNAMICLIST) )
					{
						// Create the property edit string list control
						CPropEditStringList *pPropEditStringList;

						pPropEditStringList = new CPropEditStringList;

						DWORD dwComboBoxStyle = (pProp->m_PropFlags & PF_STATICLIST) ? CBS_DROPDOWNLIST | CBS_AUTOHSCROLL : CBS_DROPDOWN | CBS_AUTOHSCROLL;
						pPropEditStringList->Create( dwComboBoxStyle | WS_CHILD | WS_VSCROLL | WS_VISIBLE | WS_BORDER | WS_TABSTOP, rect, this, PROPDLG_ID_BASE+i );
						pPropEditStringList->SetFont( m_pFont );
						pPropEditStringList->LimitText( MAX_STRINGPROP_LEN );

						m_Controls[i] = pPropEditStringList;
						
						// Check to see if this has the PF_FILENAME flag
						if ((pProp->m_PropFlags & PF_FILENAME) || (pProp->m_PropFlags & PF_TEXTUREEFFECT))
						{
							if ( pProp->m_PropFlags & PF_DYNAMICLIST )
							{
								pPropEditStringList->AddButton("B", this, PROPDLG_ID_BASE+i, m_pFont);
							}
							pPropEditStringList->SetButtonWidth(14);
						}
						else if (pProp->m_PropFlags & PF_OBJECTLINK)
						{
							if ( pProp->m_PropFlags & PF_DYNAMICLIST )
							{
								pPropEditStringList->AddButton("B", this, PROPDLG_ID_BASE+i, m_pFont);
							}
							pPropEditStringList->AddButton(">", this, PROPDLG_ID_BASE+i);
							pPropEditStringList->SetButtonWidth(14);
						}
						else
						{
							pPropEditStringList->AddButton(".", this, PROPDLG_ID_BASE+i);
						}					
					}
					else
					{
						// Create the property edit string control
						CPropEditString *pPropEditString;

						pPropEditString = new CPropEditString;
						pPropEditString->Create( ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, rect, this, PROPDLG_ID_BASE+i );
						pPropEditString->SetFont( m_pFont );
						pPropEditString->SetLimitText( MAX_STRINGPROP_LEN );

						m_Controls[i] = pPropEditString;
						
						// Check to see if this has the PF_FILENAME flag
						if ((pProp->m_PropFlags & PF_FILENAME) || (pProp->m_PropFlags & PF_TEXTUREEFFECT))
						{
							pPropEditString->AddButton("B", this, PROPDLG_ID_BASE+i, m_pFont);
							pPropEditString->SetButtonWidth(14);
						}
						else if (pProp->m_PropFlags & PF_OBJECTLINK)
						{
							pPropEditString->AddButton("B", this, PROPDLG_ID_BASE+i, m_pFont);
							pPropEditString->AddButton(">", this, PROPDLG_ID_BASE+i);
							pPropEditString->SetButtonWidth(14);
						}
						else
						{
							pPropEditString->AddButton(".", this, PROPDLG_ID_BASE+i);
						}					
					}
					break;
				}

				case LT_PT_REAL:
				case LT_PT_LONGINT:
				case LT_PT_FLAGS:
				{
					pEdit = new CEdit;
					pEdit->Create( ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, rect, this, PROPDLG_ID_BASE+i );
					pEdit->SetFont( m_pFont );
					pEdit->SetLimitText( MAX_EDITPROP_LEN );

					m_Controls[i] = pEdit;
					break;
				}

				case LT_PT_ROTATION:
				{
					pButton = new CButton;
					pButton->Create( "...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect, this, PROPDLG_ID_BASE+i );

					m_Controls[i] = pButton;
					break;
				}

				case LT_PT_VECTOR:
				{
					pButton = new CVectorButton;
					pButton->Create( "...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect, this, PROPDLG_ID_BASE+i );

					m_Controls[i] = pButton;
					break;
				}

				case LT_PT_COLOR:
				{
					CColorWell *pColor;
					pColor = new CColorWell;
					pColor->Create( "Color", WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect, this, PROPDLG_ID_BASE+i );

					m_Controls[i] = pColor;
					break;
				}

				case LT_PT_BOOL:
				{
					pButton = new CButton;
					pButton->Create("True", WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect, this, PROPDLG_ID_BASE+i);

					m_Controls[i] = pButton;
					break;
				}

				default:
				{
					m_Controls[i] = new CWnd;
					m_Controls[i]->Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect, this, PROPDLG_ID_BASE+i);
					break;
				}
			}		
		}
	}
}

/************************************************************************/
// Creates a static label
CStatic *CPropertiesControls::CreateStaticLabel(CBaseProp *pProp, int nControlIndex)
{
	// Check the parameters
	if (!pProp)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Allocate the control
	CStatic *pStaticCtrl=new CStatic;
	
	// Get the controls rectangle
	CRect rect;
	GetLabelRect(nControlIndex, rect);

	// Position the control
	pStaticCtrl->Create(pProp->m_Name, WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, rect, this, PROPDLG_STATIC_ID);
	
	// Set the controls font
	if (m_pFont)
	{
		pStaticCtrl->SetFont(m_pFont);		
	}

	return pStaticCtrl;
}

void CPropertiesControls::Redraw()
{
	InvalidateRect( NULL, TRUE );

}

/************************************************************************/
// The mouse cursor was moved
void CPropertiesControls::OnMouseMove(UINT nFlags, CPoint point) 
{
	// The default cursor
	HCURSOR hCursor=LoadCursor(NULL, IDC_ARROW);

	// See if the mouse is over a label
	int i;
	for (i=0; i < m_sControlHelpArray.GetSize(); i++)
	{
		// Skip NULL controls
		if (!m_ppLabelControls[i])
		{
			continue;
		}

		// Skip labels that don't have help
		if (m_sControlHelpArray[i].GetLength() == 0)
		{
			continue;
		}

		// Get the screen rectangle for the control
		CRect rcScreen;
		m_ppLabelControls[i]->GetWindowRect(rcScreen);
		ScreenToClient(rcScreen);

		// Expand the rectangle by one pixel so that a point on the bottom edge will
		// lie within the rectangle
		rcScreen.bottom++;

		if (rcScreen.PtInRect(point))
		{
			hCursor=LoadCursor(NULL, IDC_HELP);
			break;
		}		
	}	
	
	// Set the mouse cursor
	if (GetCursor() != hCursor)
	{
		::SetClassLong( m_hWnd, GCL_HCURSOR, (LONG)hCursor);		
	}

	CDialog::OnMouseMove(nFlags, point);
}

/************************************************************************/
// The left mouse button was pressed
void CPropertiesControls::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// This is assigned to the index that the user clicked on
	int nPopupMenu=-1;

	// See if a label was clicked
	int i;
	for (i=0; i < m_sControlHelpArray.GetSize(); i++)
	{
		// Skip NULL controls
		if (!m_ppLabelControls[i])
		{
			continue;
		}

		// Get the screen rectangle for the control
		CRect rcScreen;
		m_ppLabelControls[i]->GetWindowRect(rcScreen);
		ScreenToClient(rcScreen);

		// Expand the rectangle by one pixel so that a point on the bottom edge will
		// lie within the rectangle
		rcScreen.bottom++;

		if (rcScreen.PtInRect(point))
		{
			nPopupMenu=i;
			break;
		}		
	}	

	// Check to see if a label was clicked on
	if (nPopupMenu != -1)
	{
		// Make sure that a help string exists
		m_sPropHelpString=m_sControlHelpArray[nPopupMenu];
		if (m_sPropHelpString.GetLength() != 0)
		{					
			// Display the help note
			if (m_pPopupNoteDlg)
			{		
				m_pPopupNoteDlg->CreateDlg(m_sPropHelpString);
			}
		}
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

/************************************************************************/
// Returns the help string for a specific property
CString CPropertiesControls::GetPropHelpString(CBaseProp *pProp)
{
	// Get the class names
	CStringArray nameArray;
	GetClassNamesFromProperty(pProp, nameArray);	
	if (nameArray.GetSize() == 0)
	{
		return "Information not available.  Please check the classhlp.but file and restart DEdit.";
	}

	// Get the base class names and add them to the property list
	CStringArray baseClassNames;
	GetBaseClassNames(nameArray, baseClassNames);

	// Add the base class names to the name array
	nameArray.Append(baseClassNames);

	// Get the help string by trying each object name
	int i;
	for (i=0; i < nameArray.GetSize(); i++)
	{
		// Check to see if the string exists
		CString sFoundString;
		if (GetApp()->GetHelpString(nameArray[i], pProp->GetName(), sFoundString))
		{
			sFoundString.Replace("\\n", "\n");
			sFoundString.Replace("\\r", "\r");

			// Return the string from ButeMgr;
			return sFoundString;			
		}
	}

	// The string wasn't found
	return "";
}

/************************************************************************/
// Get the base class names for the classes specified in nameArray and
// put them into the baseClassNames array
void CPropertiesControls::GetBaseClassNames(CStringArray &classNameArray, CStringArray &destBaseArray)
{
	// This array contains the classes that are currently being searched
	CStringArray classArray;
	classArray.Copy(classNameArray);

	// This contains the currently found base classes
	CStringArray baseArray;

	// Search through the base classes until the last one has been found
	BOOL bDone=FALSE;
	while (!bDone)
	{
		// Add the base class name for each class
		int i;
		for (i=0; i < classArray.GetSize(); i++)
		{
			CString sBaseClassName;
			if (GetProject()->GetBaseClassName(classArray[i], sBaseClassName))
			{
				if (sBaseClassName != classArray[i])
				{
					baseArray.Add(sBaseClassName);
				}
			}
		}

		if (baseArray.GetSize() == 0)
		{
			// We are done getting the base class names
			bDone=TRUE;
		}
		else
		{
			// Add the base classes to the name array
			destBaseArray.Append(baseArray);

			// Copy the base classes to the class array (to look for their base classes)
			classArray.Copy(baseArray);
			baseArray.RemoveAll();
		}
	}
}

/************************************************************************/
// Returns the class name for a specific property
void CPropertiesControls::GetClassNamesFromProperty(CBaseProp *pProp, CStringArray &destNameArray)
{	
	// Get the document
	CRegionDoc *pDoc=GetActiveRegionDoc();
	if (!pDoc)
	{		
		return;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();
	if (!pRegion)
	{	
		return;
	}

	// Store an array of class names that have already been searched
	CStringArray searchNameArray;
	
	// Search the selected nodes looking for the object that has this property
	int i;
	for (i=0; i < pRegion->GetNumSelections(); i++)
	{
		// Get the node
		CWorldNode *pNode=pRegion->GetSelection(i);

		// Check to see if this class type has already been searched
		BOOL bFound=FALSE;

		int k;
		for (k=0; k < searchNameArray.GetSize(); k++)
		{
			if (searchNameArray[k] == pNode->GetClassName())
			{
				bFound=TRUE;
				break;				
			}
		}
		if (bFound)
		{
			continue;
		}

		// Get the property list
		CPropList *pPropList=pNode->GetPropertyList();

		// Search the propertly list
		CBaseProp *pFoundProp=pPropList->GetProp(pProp->GetName());

		// The object name was found
		if (pFoundProp)
		{
			// Add it to the name array
			destNameArray.Add(pNode->GetClassName());			
		}

		// Add the class name to the array of names that have been searched
		searchNameArray.Add(pNode->GetClassName());
	}
}

void CPropertiesControls::ReadControlValues()
{
	CBaseProp			*pProp;
	CVector				*pVec;
	char				str[256];
	CMoArray<char*>		stringValues;
	DWORD				i, j, iControl;
	CRect rect;
	CBoolProp *pBoolProp;


	m_bReadingControlValues = TRUE;

	for( i=0, iControl = 0; i < m_nProperties; i++ )
	{
		pProp = m_pPropList->m_Props[i];
		
		if( !m_Controls[i] )
			continue;
		
		GetControlRect( iControl, rect );
		iControl++;

		switch( pProp->m_Type )
		{
			case LT_PT_BOOL:
			{
				if(((CBoolProp*)pProp)->m_Value)
					m_Controls[i]->SetWindowText("True");
				else
					m_Controls[i]->SetWindowText("False");
				
				break;
			}

			case LT_PT_STRING:
			{
				if (pProp)
				{
					// { BL 11/08/99 }
					// If it's a list, create the lists, and if it's static, force the matching
					// of the prop's value to a list item. If we can't match something, then set
					// the prop's value to the first thing in the list.
					if ((pProp->m_PropFlags & (PF_STATICLIST | PF_DYNAMICLIST)) != 0)
					{
						if(pProp->m_pPropDef && pProp->m_pPropDef->m_pDEditInternal)
						{
							DEditInternal* pDEditInternal = pProp->m_pPropDef->m_pDEditInternal;
							m_Controls[i]->SetWindowText( ((CStringProp*)pProp)->m_String );
							if ( pProp->m_pPropDef )
							{
								CPropEditStringList* pList = (CPropEditStringList*)m_Controls[i];
								for ( int iString = 0 ; iString < pDEditInternal->m_cStrings ; iString++ )
								{
									pList->AddString(pDEditInternal->m_aszStrings[iString]);
								}

								if ( pProp->m_PropFlags & PF_STATICLIST )
								{
									int iSel = pList->FindStringExact(-1, ((CStringProp*)pProp)->m_String);

 									if( pDEditInternal->m_cStrings <= 0 )
 									{
 										pList->SetCurSel( -1 );
 										((CStringProp*)pProp)->SetString( "" );
		 								NotifyChange( pProp, true );
 									}
 									else if ( iSel == CB_ERR )
									{
										pList->SetCurSel(0);
										_ASSERT(pList->GetLBTextLen(0) < MAX_STRINGPROP_LEN );
										pList->GetLBText(0, ((CStringProp*)pProp)->m_String );
										NotifyChange(pProp, true);
									}
									else
									{
										pList->SetCurSel(iSel);
									}
								}
							}
						}
					}
					else
					{
						m_Controls[i]->SetWindowText( ((CStringProp*)pProp)->m_String );
					}
				}

				break;
			}

			case LT_PT_LONGINT:
			case LT_PT_FLAGS:
			{
				// don't update a group button
				if( !(pProp->m_PropFlags & PF_GROUPOWNER) )
				{
					sprintf(str, "%d", (int)(((CRealProp*)pProp)->m_Value));
					m_Controls[i]->SetWindowText(str);
				}
				break;
			}

			case LT_PT_REAL:
			{
				sprintf( str, "%.3f", ((CRealProp*)pProp)->m_Value );
				m_Controls[i]->SetWindowText( str );
				break;
			}
			
			case LT_PT_VECTOR:
			{
				CVectorButton *pButton;
				pVec = &((CVectorProp*)pProp)->m_Vector;
				pButton = ( CVectorButton * )m_Controls[i];
				pButton->x = pVec->x;
				pButton->y = pVec->y;
				pButton->z = pVec->z;
				break;
			}

			case LT_PT_COLOR:
			{
				pVec = &((CColorProp*)pProp)->m_Vector;
				(( CColorWell * )m_Controls[i] )->m_red = (UINT)pVec->x;
				(( CColorWell * )m_Controls[i] )->m_green = (UINT)pVec->y;
				(( CColorWell * )m_Controls[i] )->m_blue = (UINT)pVec->z;
				break;
			}
			
			default:
			{
				break;
			}
		}
	}

	m_bReadingControlValues = FALSE;
}




/////////////////////////////////////////////////////////////////////////////
// CPropertiesControls message handlers

void CPropertiesControls::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
}

void CPropertiesControls::OnKillFocus(CWnd* pNewWnd) 
{
	CDialog::OnKillFocus(pNewWnd);
}

//handles custom updating for position and rotation props to allow updating to be applied
//on objects as the position changes
static void PosVectorUpdateCallback(const LTVector& vPos, void* pUser)
{
	//get the property name
	CVectorProp* pVecProp = (CVectorProp*)pUser;

	CRegionDoc* pRegionDoc = GetActiveRegionDoc();
	CEditRegion* pRegion = pRegionDoc->GetRegion();

	//run through all the selections and update the positions
	for(uint32 nCurrSel = 0; nCurrSel < pRegion->m_Selections.GetSize(); nCurrSel++)
	{
		CWorldNode* pSel = pRegion->m_Selections[nCurrSel];
		CBaseProp* pProp = pSel->GetPropertyList()->GetProp(pVecProp->GetName());

		if(pProp && (pProp->GetType() == LT_PT_VECTOR))
		{
			((CVectorProp*)pProp)->SetVector(vPos);

			//see if the object should search for its dims when it is rendered
			if((pSel->GetType() == Node_Object) && (pVecProp->GetFlags() & (PF_DIMS | PF_LOCALDIMS)))
			{
				pSel->AsObject()->SetSearchForDims(true);
			}
		}
	}

	//update the selection box
	pRegionDoc->UpdateSelectionBox();

	//now redraw all the views
	pRegionDoc->RedrawAllViews();
}

bool CPropertiesControls::HandlePosVectorProp(CVectorProp* pVecProp)
{
	CVectorEdit vectorDlg;
	vectorDlg.SetVector(pVecProp->m_Vector);
	vectorDlg.SetCallback(PosVectorUpdateCallback, pVecProp);

	CRegionDoc* pRegionDoc = GetActiveRegionDoc();

	if(!pRegionDoc)
		return false;

	CEditRegion* pRegion = pRegionDoc->GetRegion();

	//save all the positions
	CMoArray<LTVector>	OldVal;
	OldVal.SetSize(pRegion->m_Selections.GetSize());

	uint32 nCurrSel;
	for(nCurrSel = 0; nCurrSel < pRegion->m_Selections.GetSize(); nCurrSel++)
	{
		CBaseProp* pProp = pRegion->m_Selections[nCurrSel]->GetPropertyList()->GetProp(pVecProp->GetName());

		if(pProp && (pProp->GetType() == LT_PT_VECTOR))
			OldVal[nCurrSel] = ((CVectorProp*)pProp)->m_Vector;
	}

	bool bOK = ( vectorDlg.DoModal() == IDOK );

	//restore all the values
	for(nCurrSel = 0; nCurrSel < pRegion->m_Selections.GetSize(); nCurrSel++)
	{
		CWorldNode* pSel = pRegion->m_Selections[nCurrSel];
		CBaseProp* pProp = pSel->GetPropertyList()->GetProp(pVecProp->GetName());

		if(pProp && (pProp->GetType() == LT_PT_VECTOR))
			((CVectorProp*)pProp)->SetVector(OldVal[nCurrSel]);

		//see if the object should search for its dims when it is rendered
		if((pSel->GetType() == Node_Object) && (pVecProp->GetFlags() & (PF_DIMS | PF_LOCALDIMS)))
		{
			pSel->AsObject()->SetSearchForDims(true);
		}
	}

	if(bOK)
	{
		pVecProp->m_Vector = vectorDlg.GetVector();
		return true;
	}
	
	//the user cancelled so update and invalidate
	pRegionDoc->UpdateSelectionBox();
	pRegionDoc->RedrawAllViews();

	return false;
}

static void RotationUpdateCallback(const LTVector& vEuler, void* pUser)
{
	//get the property name
	CRotationProp* pRotProp = (CRotationProp*)pUser;

	CRegionDoc* pRegionDoc = GetActiveRegionDoc();
	CEditRegion* pRegion = pRegionDoc->GetRegion();

	//run through all the selections and update the positions
	for(uint32 nCurrSel = 0; nCurrSel < pRegion->m_Selections.GetSize(); nCurrSel++)
	{
		CWorldNode* pSel = pRegion->m_Selections[nCurrSel];
		CBaseProp* pProp = pSel->GetPropertyList()->GetProp(pRotProp->GetName());

		if(pProp && (pProp->GetType() == LT_PT_ROTATION))
		{
			((CRotationProp*)pProp)->SetEulerAngles(vEuler);
		}
	}

	//update the selection box
	pRegionDoc->UpdateSelectionBox();

	//now redraw all the views
	pRegionDoc->RedrawAllViews();
}


bool CPropertiesControls::HandleRotationProp(CRotationProp* pRotProp)
{
	CRotationEdit RotationDlg;
	RotationDlg.m_EulerAngles = pRotProp->GetEulerAngles();
	RotationDlg.SetUserCallback(RotationUpdateCallback, pRotProp);

	CRegionDoc* pRegionDoc = GetActiveRegionDoc();

	if(!pRegionDoc)
		return false;

	CEditRegion* pRegion = pRegionDoc->GetRegion();

	//save all the positions
	CMoArray<LTVector>	OldVal;
	OldVal.SetSize(pRegion->m_Selections.GetSize());

	uint32 nCurrSel;
	for(nCurrSel = 0; nCurrSel < pRegion->m_Selections.GetSize(); nCurrSel++)
	{
		CBaseProp* pProp = pRegion->m_Selections[nCurrSel]->GetPropertyList()->GetProp(pRotProp->GetName());

		if(pProp && (pProp->GetType() == LT_PT_ROTATION))
			OldVal[nCurrSel] = ((CRotationProp*)pProp)->GetEulerAngles();
	}

	bool bOK = ( RotationDlg.DoModal() == IDOK );

	//restore all the values
	for(nCurrSel = 0; nCurrSel < pRegion->m_Selections.GetSize(); nCurrSel++)
	{
		CWorldNode* pSel = pRegion->m_Selections[nCurrSel];
		CBaseProp* pProp = pSel->GetPropertyList()->GetProp(pRotProp->GetName());

		if(pProp && (pProp->GetType() == LT_PT_ROTATION))
			((CRotationProp*)pProp)->SetEulerAngles(OldVal[nCurrSel]);
	}

	if(bOK)
	{
		pRotProp->SetEulerAngles(RotationDlg.m_EulerAngles);
		return true;
	}
	
	//the user cancelled so update and invalidate
	pRegionDoc->UpdateSelectionBox();
	pRegionDoc->RedrawAllViews();

	return false;
}


BOOL CPropertiesControls::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	CColorPicker colorPicker;
	CColorProp *pColorProp;
	CVectorProp *pVectorProp;
	CBoolProp *pBoolProp;
	
	DWORD offset;
	CBaseProp *pProp;
	
	WORD controlID, notifyCode;
	CEdit *pEdit;
	
	int	i, moveBack, len, selStart, selEnd;
	char str[256];

	RECT rect;

	BOOL bVal;
	BOOL bNotify = FALSE;
	BOOL bApply = FALSE;
	CPropList propList;
	DWORD groupMask;
					
	// Get the document
	CRegionDoc *pDoc=GetActiveRegionDoc();
	if (!pDoc)
	{		
		return FALSE;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();
	if (!pRegion)
	{	
		return FALSE;
	}

	if( m_bModal && LOWORD(wParam) == IDOK)
		EndDialog(IDOK);


	if( !m_pPropList )
		return FALSE;

	// Is it one of our controls?
	controlID = LOWORD( wParam );
	notifyCode = HIWORD( wParam );
	
	if( controlID >= PROPDLG_ID_BASE )
	{
		offset = controlID - PROPDLG_ID_BASE;

		if( offset < m_nProperties )
		{
			pProp = m_pPropList->m_Props[offset];
			ASSERT( pProp );

			if(pProp->m_PropFlags & PF_COMPOSITETYPE)
			{
				// this is a composite type, name of prop is the name of the type
				if( 0 == strcmp( pProp->m_Name, "LightAttenuation" ) )
				{
					// this is a light attenuation composite type
					if( notifyCode == BN_CLICKED )
					{
						CLightAttenuationDlg lightAttenuationDlg( m_pPropList );

						lightAttenuationDlg.DoModal();
					}
				}
				else
				{
					ASSERT( 0 );
				}
			}
			else if(pProp->m_PropFlags & PF_EVENT)
			{
				CEventEditorDlg Dlg(this, this);
				Dlg.DoModal();
			}
			else if(pProp->m_PropFlags & PF_GROUPOWNER)
			{
				if(notifyCode == BN_CLICKED)
				{
					// Get all the group properties.
					groupMask = GET_PF_GROUP(pProp->m_PropFlags);
					for(i=0; i < m_pPropList->m_Props; i++)
					{
						if(GET_PF_GROUP(m_pPropList->m_Props[i]->m_PropFlags) == groupMask)
						{
							if(!(m_pPropList->m_Props[i]->m_PropFlags & PF_GROUPOWNER))
							{
								propList.m_Props.Append(m_pPropList->m_Props[i]);
							}
						}
					}

					CPropertiesControls propDlg(IDD_PROPCONTROLSMODAL, &propList, m_pNotifier);
					if (propDlg.DoModal() != IDOK)
					{
						// This dialog is now invalid because the selection has changed
						// from the object browser.
						propList.m_Props.SetSize(0);
						return TRUE;
					}

					propList.m_Props.SetSize(0);
				}				
			}
			else
			{
				switch( pProp->m_Type )
				{
					case LT_PT_BOOL:
					{
						if(notifyCode == BN_CLICKED)
						{
							pBoolProp = (CBoolProp*)pProp;
							pBoolProp->m_Value = !pBoolProp->m_Value;
							bNotify = TRUE;
							m_pCurProp = pProp;

							if(pBoolProp->m_Value)
								m_Controls[offset]->SetWindowText("True");
							else
								m_Controls[offset]->SetWindowText("False");

							SetFocus();
						}
						break;
					}
					
					case LT_PT_STRING:
					{
						if( notifyCode == BN_CLICKED )
						{
							CStringProp *pStringProp;
							pStringProp = (CStringProp *)pProp;

							// If the string is an object link, then handle it accordingly
							if (pProp->m_PropFlags & PF_OBJECTLINK)
							{
								// Use the text from the control to determine if the browse
								// action should occur or if the object should be selected.
								char szBuffer[1024];
								::GetWindowText((HWND)lParam, szBuffer, sizeof(szBuffer)-1);

								// Browse for the selected object								
								if (strcmp(szBuffer, "B") == 0)
								{
									if (BrowseForObject(pStringProp))
									{									
										// Update the window text
										bNotify = TRUE;
										bApply = TRUE;
										m_pCurProp = pProp;
										m_Controls[offset]->SetWindowText(pStringProp->m_String);									
									}
								}
								else
								{
									// Try to find the object
									CBaseEditObj *pObject;
									if (pRegion->FindObjectsByName(pStringProp->m_String, &pObject, 1) > 0)
									{
										if (pObject)
										{
											// Remember if we are modal
											BOOL bModal=m_bModal;

											// Select the object
											pDoc->SelectNode(pObject);

											// Close the window if we are modal
											if (bModal)
											{
												EndDialog(!IDOK);
											}
											return TRUE;
										}
									}
									else
									{
										// Display an error message
										CString sError;
										sError.Format("Error: The %s object cannot be found.", pStringProp->m_String);
										MessageBox(sError, "Error", MB_OK | MB_ICONERROR);
									}
								}
							}
							else if (pProp->m_PropFlags & PF_TEXTUREEFFECT)
							{
								CSelectEffectGroupDlg Dlg(NULL);
								Dlg.m_sEffectGroup = pStringProp->m_String;

								//now do the dialog
								if(Dlg.DoModal())
								{
									bNotify = TRUE;
									strncpy(pStringProp->m_String, Dlg.m_sEffectGroup, 256);
									m_Controls[offset]->SetWindowText(pStringProp->m_String);
								}
							}
							// If the string is a filename, then browse for a new file
							else if (pProp->m_PropFlags & PF_FILENAME)
							{
								if (BrowseForFile((CStringProp *)pProp))
								{
									bNotify = TRUE;
									m_pCurProp = pProp;
									m_Controls[offset]->SetWindowText(pStringProp->m_String);
								}
							}
							// Just create a string editing dialog
							else
							{
								// Create a string editing dialog
								CMultiLineStringDlg dlg;

								CStringProp *pStringProp;
								pStringProp = (CStringProp *)pProp;

								dlg.m_Caption = pStringProp->m_Name;
								dlg.m_String = pStringProp->m_String;
								// { BL 11/08/99 }
								// Make the multiline edit read-only if this is a static list
								dlg.m_bReadOnly = (pProp->m_PropFlags & PF_STATICLIST) ? TRUE : FALSE;
								if( dlg.DoModal( ) == IDOK )
								{
									m_Controls[offset]->SetWindowText( dlg.m_String );
									strncpy(pStringProp->m_String, dlg.m_String, MAX_STRINGPROP_LEN );
									bNotify = TRUE;
									m_pCurProp = pProp;
								}
							}							
						}

						if((notifyCode == CBN_SETFOCUS) && m_pCurProp && (m_pCurProp != pProp))
						{
							m_bStringModified = false;
							//bApply = TRUE;
						}

						if((notifyCode == CBN_KILLFOCUS) && (m_pCurProp == pProp) && m_bStringModified )
						{
							//bApply = TRUE;
							m_bStringModified = false;
						}
						
						if(notifyCode == CBN_EDITCHANGE)
						{
							m_Controls[offset]->GetWindowText( ((CStringProp*)pProp)->m_String, MAX_STRINGPROP_LEN );
							m_pCurProp			= pProp;
							m_bStringModified	= true;
						}					

						if( (notifyCode == CBN_SELENDCANCEL) )
						{
							// Redraw ourselves because the drop down will have smeared itself all over us.

							Redraw();
						}

						if( (notifyCode == CBN_SELENDOK) || (notifyCode == CBN_SELCHANGE) )
						{
							m_bStringModified	= true;
							bApply = TRUE;

							int nCurSel = ((CComboBox*)m_Controls[offset])->GetCurSel();
							_ASSERT(((CComboBox*)m_Controls[offset])->GetLBTextLen(nCurSel) < MAX_STRINGPROP_LEN );
							((CComboBox*)m_Controls[offset])->GetLBText(nCurSel, ((CStringProp*)pProp)->m_String );
							m_pCurProp = pProp;

							// Redraw ourselves because the drop down will have smeared itself all over us.

							Redraw();
						}					

						if(notifyCode == EN_SETFOCUS)
						{
							m_bStringModified = false;
						}

						if((notifyCode == EN_KILLFOCUS) && (m_pCurProp == pProp) && m_bStringModified )
						{
							bApply = TRUE;
							m_bStringModified = false;
						}
						
						if(notifyCode == EN_CHANGE)
						{
							m_Controls[offset]->GetWindowText( ((CStringProp*)pProp)->m_String, MAX_STRINGPROP_LEN );
							m_pCurProp = pProp;
							m_bStringModified = true;
						}					
						break;
					}

					case LT_PT_REAL:
					case LT_PT_LONGINT:
					case LT_PT_FLAGS:
					{
						pEdit = (CEdit*)m_Controls[offset];
						pEdit->GetWindowText( str, 256 );

						if((notifyCode == EN_SETFOCUS) && m_pCurProp && (m_pCurProp != pProp))
						{
							m_bStringModified = false;
							bApply = TRUE;
						}

						if((notifyCode == EN_KILLFOCUS) && (m_pCurProp == pProp) && m_bStringModified )
						{
							bApply = TRUE;
							m_bStringModified = false;
						}

						if( (notifyCode == EN_CHANGE) || (notifyCode == EN_UPDATE) )
						{
							// Remove characters.
							moveBack = 0;
							len = strlen(str);
							for( i=0; i < len; i++ )
							{
								if( ((str[i] < '0') || (str[i] > '9')) && (str[i] != '.') && (str[i] != '-') )
								{
									str[i-moveBack] = str[i];
									++moveBack;
								}
								else if(pProp->m_Type == LT_PT_LONGINT && str[i] == '.')
								{
									str[i-moveBack] = str[i];
									++moveBack;
								}
								else
								{
									str[i-moveBack] = str[i];
								}
							}

							// Reset the text if they entered invalid characters.
							if( moveBack > 0 )
							{
								MessageBeep(0);

								str[len-moveBack] = 0;
								len = strlen(str);

								pEdit->GetSel( selStart, selEnd );
								pEdit->SetWindowText( str );
								pEdit->SetSel( selStart-1, selEnd-1 );
							}
						
							// Store the new number if it is a change
							if(notifyCode == EN_CHANGE)
							{
								((CRealProp*)pProp)->m_Value = (float)atof(str);
								m_pCurProp = pProp;
								m_bStringModified = true;
							}

						}
						else if( notifyCode == EN_KILLFOCUS )
						{
							if( strlen(str) == 0 )
							{
								pEdit->SetWindowText( "0" );
								pEdit->SetSel( 1, 1 );
								((CRealProp*)pProp)->m_Value = 0.0f;
								m_pCurProp = pProp;
								bNotify = TRUE;
							}
						}
						break;
					}

					case LT_PT_COLOR:
					{
						if( notifyCode == BN_CLICKED )
						{
							pColorProp = (CColorProp*)pProp;

							colorPicker.Init();
							colorPicker.SetCurrentColor(RGB((BYTE)pColorProp->m_Vector.x, 									
															(BYTE)pColorProp->m_Vector.y,
															(BYTE)pColorProp->m_Vector.z));
							
							if( colorPicker.DoModal() == IDOK )
							{
								CColorWell *pColor = (( CColorWell * )m_Controls[offset]);

								pColor->m_red = GetRValue( colorPicker.GetColor() );
								pColor->m_green = GetGValue( colorPicker.GetColor() );
								pColor->m_blue = GetBValue( colorPicker.GetColor() );

								pColorProp->m_Vector.x = (float)pColor->m_red;
								pColorProp->m_Vector.y = (float)pColor->m_green;
								pColorProp->m_Vector.z = (float)pColor->m_blue;

								// Update the color display
								pColor->Invalidate(FALSE);

								m_pCurProp = pProp;
								bApply = TRUE;
							}
						}
						break;
					}

					case LT_PT_VECTOR:
					{
						if( notifyCode == BN_CLICKED )
						{
							pVectorProp = (CVectorProp*)pProp;

							if( (stricmp(pVectorProp->GetName(), "Pos") == 0) || 
								(pVectorProp->GetFlags() & (PF_DIMS | PF_LOCALDIMS | PF_ORTHOFRUSTUM)))
							{
								if(HandlePosVectorProp(pVectorProp))
								{
									m_pCurProp = pProp;
									bApply = TRUE;
								}
							}
							else
							{							
								CVectorEdit vectorDlg;
								vectorDlg.SetVector(pVectorProp->m_Vector);
								
								if( vectorDlg.DoModal() == IDOK )
								{
									pVectorProp->m_Vector=vectorDlg.GetVector();
										
									m_pCurProp = pProp;
									bApply = TRUE;
								}
							}
						}
						break;
					}

					case LT_PT_ROTATION:
					{
						if(notifyCode == BN_CLICKED)
						{

							CRotationProp *pRotationProp = (CRotationProp*)pProp;

							if(stricmp(pRotationProp->GetName(), "Rotation") == 0)
							{
								if(HandleRotationProp(pRotationProp))
								{
									m_pCurProp = pProp;
									bApply = TRUE;
								}
							}
							else
							{	
								CRotationEdit rotationDlg;
								rotationDlg.m_EulerAngles = pRotationProp->GetEulerAngles();
								if(rotationDlg.DoModal() == IDOK)
								{
									pRotationProp->SetEulerAngles(rotationDlg.m_EulerAngles);
									m_pCurProp = pProp;
									bApply = TRUE;
								}
							}
						}

						break;
					}

					
					default:
					{
						break;
					}
				}
			}
		}
	}
	
	BOOL bStat =  CDialog::OnCommand(wParam, lParam);

	if(( bNotify || bApply ) && !m_bReadingControlValues && m_pCurProp )
	{
		NotifyChange( m_pCurProp, bApply );
		if( bApply )
		{
			m_pCurProp			= NULL;
			m_bStringModified	= false;
		}
	}

	return bStat;
}


/************************************************************************/
// This uses a file open dialog to browse for a file and place it
// into the string property.
//
// Returns:	TRUE  - The file was succesfully found and copied into the property
//			FALSE - An error occured or cancel was selected on the browse dialog box
BOOL CPropertiesControls::BrowseForFile(CStringProp *pProp)
{
	// The base project path
	CString sBaseProjectPath=GetProject()->m_BaseProjectDir;

	// The filter used for the file open dialog
	CString sFilter("All files (*.*)|*.*|Model (*.lta,*.ltc)|*.lta,*.ltc|Texture (*.dtx)|*.dtx|Wave (*.wav)|*.wav||");

	// The current file
	CString sCurrentFile;
	if (strlen(pProp->m_String) > 0)
	{
		sCurrentFile=sBaseProjectPath+"\\"+pProp->m_String;
	}
	else
	{
		sCurrentFile=sBaseProjectPath+"\\*.*";
	}

	// Create a file open dialog
	CFileDialog fileDlg(TRUE, "*.*", sCurrentFile, NULL, sFilter);

	// Display the dialog
	if (fileDlg.DoModal() != IDOK)
	{
		return FALSE;
	}

	// Get the pathname
	CString sFilePath=fileDlg.GetPathName();

	// The destination string that is copied into the property
	CString sDestString;

	// If a base project path is set, check to see if the filename exists in the base project
	int nBaseProjectLength=sBaseProjectPath.GetLength();
	if ( nBaseProjectLength > 0)
	{
		// Compare the filename to the base project path
		if (sFilePath.GetLength() < nBaseProjectLength ||
			sFilePath.Left(nBaseProjectLength) != sBaseProjectPath)
		{
			// Warn the user that they are chosing a file that is not in their resource path
			MessageBox("Error: The file that you picked is not in the resource path for this project.",
					   "Error", MB_ICONERROR | MB_OK);
			return FALSE;
		}

		// Set the string to the path minus the base path
		sDestString=sFilePath.Right(sFilePath.GetLength()-nBaseProjectLength-1);
	}
	else
	{
		// Set the string to the full path
		sDestString=sFilePath;
	}	

	// Copy the string into the property
	strncpy(pProp->m_String, sDestString, MAX_STRINGPROP_LEN );
	pProp->m_String[MAX_STRINGPROP_LEN-1]='\0';

	return TRUE;
}

/************************************************************************/
// This function is called to browse for an object using the
// object browser.
//
// Returns:  TRUE  - OK was pressed on the browser
//           FALSE - OK was not pressed to close the browser
BOOL CPropertiesControls::BrowseForObject(CStringProp *pStringProp)
{
	// Get the document
	CRegionDoc *pDoc=GetActiveRegionDoc();
	if (!pDoc)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();
	if (!pRegion)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Create the object browsing dialog
	CObjectBrowserDlg objectDlg;
	
	// Set the selected object
	objectDlg.SetSelectedObject(pStringProp->GetString());	

	// Make the "Select" button invisible
	objectDlg.SetControlVisible(IDC_BUTTON_SELECT, FALSE);

	// Set whether or not to group the objects by type
	COptionsObjectBrowser *pOptions=GetApp()->GetOptions().GetObjectBrowserOptions();
	if (pOptions)
	{
		objectDlg.m_bGroupByType=pOptions->IsGroupByType();
	}
		
	// Display the dialog
	if (objectDlg.DoModal() == IDOK)
	{	
		// Get the selected object
		CString sObjName = objectDlg.GetSelectedName();

		// Check to see that we have a valid object
		if (!sObjName.IsEmpty())
		{
			// Update the string property
			strncpy(pStringProp->m_String, sObjName, MAX_STRINGPROP_LEN );		
		}	

		// Save whether or not to group the objects by type		
		if (pOptions)
		{
			pOptions->SetGroupByType(objectDlg.m_bGroupByType);
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CPropertiesControls::OnInitDialog()
{
	CDialog::OnInitDialog( );

	if(m_bModal)
	{
		Init(m_pPropList, m_pNotifier);
	}

	return FALSE;
}


void CPropertiesControls::OnOK()
{
	if( m_pCurProp )
	{
		NotifyChange( m_pCurProp, TRUE );
		m_pCurProp			= NULL;
		m_bStringModified	= false;
	}
}

void CPropertiesControls::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	UpdateScrollBars( );
}


void CPropertiesControls::PlaceControls( )
{
	CRect rectClient, rectControl, rectSplit;
	DWORD i, iControl;
	CBaseProp *pProp;	

	GetClientRect( &rectClient );

	m_ItemWidth = (rectClient.Width() - (PROPDLG_HORZ_PAD*2)) / 2;

	// Move the controls
	for( i=0, iControl = 0; i < m_nProperties; i++ )
	{
		pProp = m_pPropList->m_Props[i];
		
		if( !m_Controls[i] )
			continue;
		
		GetControlRect( iControl, rectControl );
		iControl++;

		// Special case the string controls
		if( pProp->m_Type == LT_PT_STRING )
		{
			if ( (pProp->m_PropFlags & PF_STATICLIST) || (pProp->m_PropFlags & PF_DYNAMICLIST) )
			{
				CPropEditStringList *pPropEditStringList = (CPropEditStringList *)m_Controls[i];
				pPropEditStringList->Position(rectControl);			
			}
			else
			{
				CPropEditString *pPropEditString = (CPropEditString *)m_Controls[i];
				pPropEditString->Position(rectControl);			
			}
		}
		else
		{
			m_Controls[i]->MoveWindow( &rectControl );
		}
	}

	// Move the static controls
	for( i=0, iControl=0; i < m_nProperties; i++ )
	{			
		if( !m_ppLabelControls[i] )
			continue;
		
		GetLabelRect( iControl, rectControl );
		iControl++;
		
		m_ppLabelControls[i]->MoveWindow( &rectControl );
	}

	Invalidate( TRUE );
}

void CPropertiesControls::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int nChange = 0;

	switch( nSBCode )
	{
        case SB_PAGEUP: 
             nChange = MIN(-1, -(int)m_iPage ); 
             break; 

        case SB_PAGEDOWN: 
             nChange = MAX(1, m_iPage ); 
             break; 

        case SB_LINEUP: 
             nChange = -1; 
             break; 

        case SB_LINEDOWN: 
             nChange = 1; 
             break; 

        case SB_THUMBTRACK: 
            nChange = nPos - m_iCurPos; 
            break; 

        default: 
             nChange = 0; 
	}

//    if( nChange = max( -( int )m_iCurPos, min( nChange, ( int )m_iMaxPos - ( int )m_iCurPos ))) 
    if( nChange = MAX( -( int )m_iCurPos, MIN( nChange, m_iMaxPos + 1 - MIN( m_iMaxPos, m_iPage ) - m_iCurPos ))) 
    { 
        m_iCurPos += nChange; 
		UpdateScrollBars( );
    } 

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CPropertiesControls::UpdateScrollBars( )
{
	SCROLLINFO stVertScrollInfo;
	CRect rectClient;

	if( !m_Controls )
		return;

	GetClientRect( &rectClient );
	
    // Determine the maximum vertical scrolling position. 
//	m_iMaxPos = max( 0, m_nControls - ( rectClient.Height( ) / m_ItemHeight ));
	m_iMaxPos = m_nControls - 1;

	// Calculate the page height...
//	m_iPage = min( m_iMaxPos, rectClient.Height( ) / m_ItemHeight );
	m_iPage = rectClient.Height( ) / m_ItemHeight;

    // Make sure the current vertical scrolling position 
    // does not exceed the maximum. 
//    m_iCurPos = min( m_iCurPos, m_iMaxPos );
    m_iCurPos = MIN( m_iCurPos, m_iMaxPos + 1 - MIN( m_iMaxPos, m_iPage ));

	// Setup the new info for vert scrollbar...
	stVertScrollInfo.cbSize = sizeof( SCROLLINFO );
	stVertScrollInfo.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	stVertScrollInfo.nMin = 0;
	stVertScrollInfo.nMax = m_iMaxPos;
	stVertScrollInfo.nPage = m_iPage;
	stVertScrollInfo.nPos = m_iCurPos;
	SetScrollInfo( SB_VERT, &stVertScrollInfo );

	PlaceControls( );
}


/////////////////////////////////////////////////////////////////////////////
// CPropertiesDlg dialog


CPropertiesDlg::CPropertiesDlg()	
{
	//{{AFX_DATA_INIT(CPropertiesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pPropControls = NULL;
	m_pPopupNoteDlg=new CPopupNoteDlg;

	m_pFont = new CFont;
	m_pFont->CreatePointFont( 90, "Courier New" );
	m_dwVertPad = 0;
}


CPropertiesDlg::~CPropertiesDlg()
{
	delete m_pFont;

	if (m_pPopupNoteDlg)
	{
		delete m_pPopupNoteDlg;
		m_pPopupNoteDlg=NULL;
	}

	Term( FALSE );
}


BEGIN_MESSAGE_MAP(CPropertiesDlg, CProjectTabControlBar)
	//{{AFX_MSG_MAP(CPropertiesDlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_CLASS, OnButtonChangeClass)
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CPropertiesDlg interface

// For some reason the controls are gettind disabled even though they have
// command handlers.  This override fixes the problem.
void CPropertiesDlg::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CMRCSizeDialogBar::OnUpdateCmdUI(pTarget, FALSE);
}

BOOL CPropertiesDlg::Init( CPropList *pList, CNotifier *pNotifier, char *pClassName )
{
	CRect		clientRect, rc1, rc2;
	LOGFONT		logFont;
	CWnd		*pItem;
	int			nVertPad;

	Term();

	// Init stuff.
	SetFont( m_pFont, FALSE );

	// Get the dimensions of the items.
	GetClientRect( &clientRect );
	m_pFont->GetLogFont( &logFont );

	pItem = GetDlgItem(IDC_CLASSNAME_TEXT);
	if(pItem)
	{
		if(pClassName)
		{
			m_sClassName=pClassName;
			pItem->SetWindowText(m_sClassName);
		}

		GetWindowRect(&rc1);
		pItem->GetWindowRect(&rc2);
		m_dwVertPad = 10 + rc2.bottom - rc1.top;
	}
	else
		m_dwVertPad = 10;

	clientRect.top += m_dwVertPad;
	m_pPropControls = new CPropertiesControls( );
	m_pPropControls->Create( IDD_PROPCONTROLS, this );
	m_pPropControls->MoveWindow( &clientRect );
	m_pPropControls->Init( pList, pNotifier );
	m_pPropControls->ShowWindow( SW_SHOW );
	
	CString sHelp;
	if(pClassName && GetApp()->GetHelpString(m_sClassName, "ClassDescription", sHelp))
	{
		m_sCurrentClassHelp = sHelp;
		m_sCurrentClassHelp.Replace("\\n", "\n");
		m_sCurrentClassHelp.Replace("\\r", "\r");
	}

	// Disable the "Change Class" button if there aren't any object nodes selected
	if (GetNumObjectNodesSelected() == 0)
	{
		GetDlgItem(IDC_BUTTON_CHANGE_CLASS)->EnableWindow(FALSE);
	}
	else
	{
 		GetDlgItem(IDC_BUTTON_CHANGE_CLASS)->EnableWindow(TRUE);
	}

	InvalidateRect( NULL, TRUE );
	return TRUE;
}

/************************************************************************/
// Termination
void CPropertiesDlg::Term( BOOL bClear )
{
	if( m_pPropControls )
	{
		m_pPropControls->Term( bClear );
		m_pPropControls->DestroyWindow( );
		delete m_pPropControls;
		m_pPropControls = NULL;
	}
}


void CPropertiesDlg::TermIf( CPropList *pList, BOOL bClear )
{
	if( m_pPropControls )
		if( m_pPropControls->GetPropList( ) == pList )
			Term( bClear );
}


void CPropertiesDlg::ReadControlValues()
{
	if(m_pPropControls)
		m_pPropControls->ReadControlValues( ); 
}


void CPropertiesDlg::Redraw()
{
	InvalidateRect( NULL, TRUE );
}



void CPropertiesDlg::OnSize(UINT nType, int cx, int cy) 
{
	CRect clientRect;

	CMRCSizeDialogBar::OnSize(nType, cx, cy);
	
	// Reposition the controls
	RepositionControls();
}

/************************************************************************/
// Repositions the controls
void CPropertiesDlg::RepositionControls()
{
	if( m_pPropControls && IsWindow(m_pPropControls->GetSafeHwnd()))
	{
		// Get the dimensions of the items.
		CRect clientRect;
		GetClientRect( &clientRect );

		clientRect.top += m_dwVertPad;
		m_pPropControls->MoveWindow( &clientRect );
	}
}

/************************************************************************/
void CPropertiesDlg::OnOK()
{
	if( m_pPropControls )
		m_pPropControls->OnOK( );
}

/************************************************************************/
void CPropertiesDlg::OnButtonChangeClass() 
{	
	// Get the document
	CRegionDoc *pDoc=GetActiveRegionDoc();
	if (!pDoc)
	{
		ASSERT(FALSE);
		return;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();
	if (!pRegion)
	{
		ASSERT(FALSE);
		return;
	}

	// Check to see if there are non-object nodes selected
	if (GetNumObjectNodesSelected() != pRegion->GetNumSelections())
	{
		// Ask the user if they want to deselect the nodes and continue
		CString sMessage;
		sMessage.LoadString(IDS_NONOBJECT_NODES_SELECTED);

		if (MessageBox(sMessage, "Change Class", MB_YESNO) == IDYES)
		{
			// Deselect the non-object nodes
			DeselectNonObjectNodes();
		}
		else
		{
			return;
		}
	}

	// Use the template class dialog
	CClassDlg dlg;	
	dlg.SetProject(GetProject());
	dlg.SetTitle("Change Class");
	dlg.SetClass(m_sClassName);

	if( dlg.DoModal() == IDOK )
	{
		CString sClass=dlg.GetSelectedClass();
		if (sClass.GetLength() > 0)
		{
			DoChangeClass(dlg.GetSelectedClass());
		}
	}
}

/************************************************************************/
// Returns the number of object nodes that are selected
int CPropertiesDlg::GetNumObjectNodesSelected()
{
	// Get the document
	CRegionDoc *pDoc=GetActiveRegionDoc();
	if (!pDoc)
	{		
		return 0;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();
	if (!pRegion)
	{		
		return 0;
	}
	
	// The number of selected objects
	int nNumObjects=0;

	// Loop through all of the selections
	int i;
	for (i=0; i < pRegion->GetNumSelections(); i++)
	{
		if (pRegion->GetSelection(i)->GetType() == Node_Object)
		{
			nNumObjects++;			
		}
	}

	return nNumObjects;
}

/************************************************************************/
// Deselects non-object nodes
void CPropertiesDlg::DeselectNonObjectNodes()
{
	// Get the document
	CRegionDoc *pDoc=GetActiveRegionDoc();
	if (!pDoc)
	{
		ASSERT(FALSE);
		return;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();
	if (!pRegion)
	{
		ASSERT(FALSE);
		return;
	}
	
	// Create an array of the nodes that are selected
	int nNumNodes=pRegion->GetNumSelections();
	CWorldNode **pSelArray=new CWorldNode*[nNumNodes];

	int i;
	for (i=0; i < nNumNodes; i++)
	{
		pSelArray[i]=pRegion->GetSelection(i);
	}
	
	// Loop through all of the selections	
	for (i=0; i < nNumNodes; i++)
	{
		if (pSelArray[i]->GetType() != Node_Object)
		{
			pRegion->UnselectNode(pSelArray[i]);
		}
	}

	// Delete the selection array
	delete []pSelArray;

	return;
}

/************************************************************************/
// Changes the class of the selected objects
void CPropertiesDlg::DoChangeClass(CString sClassName)
{
	// Get the document
	CRegionDoc *pDoc=GetActiveRegionDoc();
	if (!pDoc)
	{
		return;
	}

	// Get the region view
	POSITION pos = pDoc->GetFirstViewPosition();
	CRegionView* pView=(CRegionView *)pDoc->GetNextView( pos );
	if(!pView)
	{
		return;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();
	if (!pRegion)
	{
		return;
	}
	
	// Create an array of the nodes that are selected
	int nNumNodes=pRegion->GetNumSelections();
	CWorldNode **pSelArray=new CWorldNode*[nNumNodes];

	int i;
	for (i=0; i < nNumNodes; i++)
	{
		pSelArray[i]=pRegion->GetSelection(i);
	}

	// Get the active parent node
	CWorldNode *pActiveParentNode=pRegion->GetActiveParentNode();

	// This array is used to temporarily store the newly created nodes
	CMoArray<CWorldNode*>	m_newSelectionArray;

	// Create a new object for each object that is selected		
	for (i=0; i < nNumNodes; i++)
	{
		// Get the selected object
		CWorldNode *pSelNode=pSelArray[i];

		// Only convert objects
		if (pSelNode->GetType() == Node_Object)
		{
			// Convert the class
			CWorldNode *pNewNode=pDoc->ConvertObjectClass(pSelNode->AsObject(), sClassName);
			
			// Store the new node in the temporary array
			m_newSelectionArray.Add(pNewNode);

			// Check to see if the selected node is the active parent
			if (pSelNode == pActiveParentNode)
			{
				// Set the active parent node to the newly created node
				pRegion->SetActiveParentNode(pNewNode);
			}
		}			
	}

	// Delete the selected nodes
	pView->DeleteSelectedNodes();

	// Select the newly created nodes
	for (i=0; i < m_newSelectionArray; i++)
	{
		pRegion->SelectNode(m_newSelectionArray[i]);			
	}

	// Notify the document of the selection change
	pDoc->NotifySelectionChange();

	// Delete the selection array
	delete []pSelArray;
}

void CPropertiesDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	
}

void CPropertiesDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// Make sure that there is help for the selected class
	if (m_sCurrentClassHelp.GetLength() != 0)
	{
		// Get the rectangle for the class name control
		CRect rcClassName;
		GetDlgItem(IDC_CLASSNAME_TEXT)->GetWindowRect(rcClassName);
		ScreenToClient(rcClassName);

		// Check to see if the mouse is over the class name control
		if (rcClassName.PtInRect(point))
		{
			// Display the help note			
			if (m_pPopupNoteDlg)
			{		
				m_pPopupNoteDlg->CreateDlg(m_sCurrentClassHelp);
			}
		}	
	}
	
	CMRCSizeDialogBar::OnLButtonDown(nFlags, point);
}

void CPropertiesDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Make sure that there is help for the selected class
	if (m_sCurrentClassHelp.GetLength() == 0)
	{
		SetClassLong( m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
		return;
	}

	// Get the rectangle for the class name control
	CRect rcClassName;
	GetDlgItem(IDC_CLASSNAME_TEXT)->GetWindowRect(rcClassName);
	ScreenToClient(rcClassName);

	// Check to see if the mouse is over the class name control
	HCURSOR hCursor;
	if (rcClassName.PtInRect(point))
	{		
		hCursor=LoadCursor(NULL, IDC_HELP);
	}
	else
	{
		hCursor=LoadCursor(NULL, IDC_ARROW);
	}

	if (GetCursor() != hCursor)
	{
		::SetClassLong( m_hWnd, GCL_HCURSOR, (LONG)hCursor);
	}

	CMRCSizeDialogBar::OnMouseMove(nFlags, point);
}
