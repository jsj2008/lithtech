// PropertyWnd.cpp : implementation file
//

#include "stdafx.h"
#include "winpacker.h"
#include "PropertyWnd.h"
#include "PackerProperty.h"
#include "IPackerImpl.h"
#include "PropertyMgr.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//first ID for all the controls
#define BASE_CONTROL_ID				20000
#define PROPERTY_CONTROL_HEIGHT		20
#define PROPERTY_SPACING			2
#define PROPERTY_HEIGHT				(PROPERTY_CONTROL_HEIGHT + PROPERTY_SPACING)
#define BROWSE_BUTTON_WIDTH			20

BEGIN_MESSAGE_MAP(CPropertyWnd, CWnd)
	//{{AFX_MSG_MAP(CPropertyWnd)
	ON_WM_VSCROLL()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//---------------------------------------------------------------------------
//Utility functions

//given a string it will property format it to be a name
static CString	FormatStaticText(const char* pszText)
{
	CString rv;
	rv.Format("%s: ", pszText);
	return rv;
}

//given a property it will determine if a static control should be created for it
static bool		CreateStaticName(const CPackerProperty* pProp)
{
	if(pProp->GetType() == PROPERTY_INTERFACE)
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyWnd

CPropertyWnd::CPropertyWnd(IPackerImpl* pIPacker, CPropertyMgr* pPropMgr) :
	m_pGroup(NULL),
	m_pIPacker(pIPacker),
	m_pPropMgr(pPropMgr)
{
	ASSERT(m_pIPacker);
	ASSERT(m_pPropMgr);

	SetAllControlsToNull();

}

CPropertyWnd::~CPropertyWnd()
{
	EmptyPropertyList();
}


//sets all the control pointers to NULL
void CPropertyWnd::SetAllControlsToNull()
{
	for(uint32 nCurrProp = 0; nCurrProp < MAX_PROPS_PER_GROUP; nCurrProp++)
	{
		m_pPropName[nCurrProp] = NULL;

		for(uint32 nCurrCtrl = 0; nCurrCtrl < MAX_CONTROLS_PER_PROP; nCurrCtrl++)
		{
			m_pPropControl[nCurrProp][nCurrCtrl] = NULL;
		}
	}
}

//given a group it will run through the group and find the maximum width 
//of the names in pixels
uint32 CPropertyWnd::FindMaxNameWidth(CPropertyGroup* pGroup)
{
	//get the DC and install the font
	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(&m_Font);

	//now find the max width
	uint32 nMaxWidth = 0;

	for(uint32 nCurrProp = 0; nCurrProp < pGroup->GetNumProperties(); nCurrProp++)
	{
		//cache the pointer
		CPackerProperty* pProp = pGroup->GetProperty(nCurrProp);

		//get the pixel width
		CSize size = pDC->GetTextExtent(FormatStaticText(pProp->GetName()));

		//update the max
		nMaxWidth = max((uint32)size.cx, nMaxWidth);
	}

	//reset the font
	pDC->SelectObject(pOldFont);

	return nMaxWidth;
}

//called when the properties need to be recreated most likely because of a group
//change
BOOL CPropertyWnd::CreatePropertyList(CPropertyGroup* pGroup)
{
	//sanity check
	ASSERT(pGroup);

	//clean up the old stuff
	if(EmptyPropertyList() == FALSE)
	{
		return FALSE;
	}

	//now we need to do dimension info gathering for laying out the properties
	CRect rClient;
	GetClientRect(rClient);

	//get info for the scrollbars
	uint32 nNumProps = pGroup->GetNumProperties();
	uint32 nPropHeight		= PROPERTY_HEIGHT;

	CRect rPadding(5, 5, 5, 5);

	//we can now setup the scrollbar information since we now can know the height
	SCROLLINFO	ScrollInfo;
	ScrollInfo.cbSize		= sizeof(ScrollInfo);
	ScrollInfo.fMask		= SIF_ALL;
	ScrollInfo.nMin			= 0;
	ScrollInfo.nMax			= PROPERTY_HEIGHT * nNumProps + rPadding.top + rPadding.bottom;
	ScrollInfo.nPos			= 0;
	ScrollInfo.nPage		= rClient.Height();
	SetScrollInfo(SB_VERT, &ScrollInfo);

	//one other thing we need to do is run through and get the dimensions of all the static
	//controls
	uint32 nMaxNameWidth	= FindMaxNameWidth(pGroup);
	
	//now we need to reget the dimensions since that call could've taken away or added
	//the scrollbar
	GetClientRect(rClient);

	//shrink the rectangle a bit
	rClient.DeflateRect(rPadding);

	//find the dimensions for the name along the X
	uint32 nNameStartX		= rPadding.left;
	uint32 nNameEndX		= rPadding.left + nMaxNameWidth;

	//find the dimensions for the control area on the X
	uint32 nControlStartX	= nNameEndX;
	uint32 nControlEndX		= rPadding.left + rClient.Width();

	//now find the height for the property
	uint32 nCurrY			= rPadding.top;

	//the current ID
	uint32 nCurrID			= BASE_CONTROL_ID;

	//find the height of the font so we can center the names of the properties
	uint32 nFontHeight		= GetFontHeight();

	//now we need to allocate all of the names and appropriate controls
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		CPackerProperty* pProp = pGroup->GetProperty(nCurrProp);
		ASSERT(pProp);

		uint32 nLeftX = nControlStartX;

		//skip over UI properties so they can create custom controls only
		if(CreateStaticName(pProp))
		{
			//allocate the name for this item
			m_pPropName[nCurrProp] = new CStatic;

			//check the allocation
			if(m_pPropName[nCurrProp] == NULL)
			{
				EmptyPropertyList();
				return FALSE;
			}

			//the Y position, (need to center it)
			uint32 nTextY = nCurrY + (PROPERTY_CONTROL_HEIGHT - nFontHeight) / 2;

			//now setup the name
			CString sName = FormatStaticText(pProp->GetName());

			m_pPropName[nCurrProp]->Create(sName, WS_CHILD | WS_VISIBLE | SS_RIGHT, 
											CRect(nNameStartX, nTextY, nNameEndX, nCurrY + nPropHeight),
											this);

			m_pPropName[nCurrProp]->SetFont(&m_Font);
		}
		else
		{
			//don't create a name control for it
			m_pPropName[nCurrProp]	= NULL;
			nLeftX					= rPadding.left;
		}

		//now create the other controls based upon the property type
		CreatePropertyControls(	pProp, nCurrProp, nCurrID, 
								CRect(nLeftX, nCurrY, nControlEndX, nCurrY + nPropHeight - PROPERTY_SPACING));

		//update the offsets
		nCurrY  += nPropHeight;
		nCurrID += MAX_CONTROLS_PER_PROP;
	}

	//save this group
	m_pGroup = pGroup;

	return PropertyModified(NULL);
}

//frees all the properties from this window and erases all attachements to the group
BOOL CPropertyWnd::EmptyPropertyList()
{
	//remove the tooltips first
	if(m_pGroup)
	{
		RemoveTooltips();
	}

	//run through our list of controls and free each one
	for(uint32 nCurrProp = 0; nCurrProp < MAX_PROPS_PER_GROUP; nCurrProp++)
	{
		delete m_pPropName[nCurrProp];
		m_pPropName[nCurrProp] = NULL;

		for(uint32 nCurrCtrl = 0; nCurrCtrl < MAX_CONTROLS_PER_PROP; nCurrCtrl++)
		{
			delete m_pPropControl[nCurrProp][nCurrCtrl];
			m_pPropControl[nCurrProp][nCurrCtrl] = NULL;
		}
	}

	m_pGroup = NULL;

	return TRUE;
}

//gets the group associated with the list
CPropertyGroup* CPropertyWnd::GetAssociatedGroup()
{
	return m_pGroup;
}

//called to update the values inside of each of the controls
BOOL CPropertyWnd::UpdateControlValues()
{
	//sanity check
	ASSERT(m_pGroup);

	uint32 nNumProps = m_pGroup->GetNumProperties();

	//run through all the controls, and for each one we need to determine if the value
	//has changed, and if it has, need to update the control
	for(uint32 nPropIndex = 0; nPropIndex < nNumProps; nPropIndex++)
	{
		CPackerProperty* pProp = m_pGroup->GetProperty(nPropIndex);

		switch(pProp->GetType())
		{
		case PROPERTY_REAL:
			{
				//get the value of the edit control
				CString sVal;
				m_pPropControl[nPropIndex][0]->GetWindowText(sVal);
				float fVal = (float)atof(sVal);

				//see if it is different
				if(fabs(fVal - ((CPackerRealProperty*)pProp)->GetValue()) > 0.009)
				{
					//it is different, lets reset the value
					CPackerRealProperty* pReal = (CPackerRealProperty*)pProp;
					sVal.Format((pReal->IsInteger()) ? "%.0f" : "%.2f", pReal->GetValue());
					m_pPropControl[nPropIndex][0]->SetWindowText(sVal);
				}
			}
			break;
		case PROPERTY_STRING:
			{
				//get the value of the edit control
				CString sVal;
				m_pPropControl[nPropIndex][0]->GetWindowText(sVal);

				//see if it matches
				if(sVal.Compare(((CPackerStringProperty*)pProp)->GetValue()) != 0)
				{
					//different. Reset it
					m_pPropControl[nPropIndex][0]->SetWindowText(((CPackerStringProperty*)pProp)->GetValue());
				}
			}
			break;
		case PROPERTY_ENUM:
			{
				CComboBox* pCombo = (CComboBox*)m_pPropControl[nPropIndex][0];
				if(pCombo->GetCurSel() != (int)((CPackerEnumProperty*)pProp)->GetSelection())
				{
					pCombo->SetCurSel(((CPackerEnumProperty*)pProp)->GetSelection());
				}
			}
			break;
		case PROPERTY_BOOL:
			{
				CButton* pButton = (CButton*)m_pPropControl[nPropIndex][0];
				if((!!pButton->GetCheck()) != ((CPackerBoolProperty*)pProp)->GetValue())
				{
					pButton->SetCheck(((CPackerBoolProperty*)pProp)->GetValue() ? 1 : 0);
				}
			}
			break;
		case PROPERTY_INTERFACE:
			{
			}
			break;
		default:
			//need to add code for new property
			ASSERT(false);
			break;
		}
	}

	return TRUE;
}


//called to update the enabled status of the controls
BOOL CPropertyWnd::UpdateControlEnabledStatus()
{
	//make sure that we have already been set up
	ASSERT(m_pGroup);

	uint32 nNumProps = m_pGroup->GetNumProperties();

	//run through all the controls, make and update the enabled
	//run through our list of controls and free each one
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		//determine the enabled status of this item
		BOOL bEnabled = m_pGroup->GetProperty(nCurrProp)->IsEnabled() ? TRUE : FALSE;

		for(uint32 nCurrCtrl = 0; nCurrCtrl < MAX_CONTROLS_PER_PROP; nCurrCtrl++)
		{
			CWnd* pWnd = m_pPropControl[nCurrProp][nCurrCtrl];

			//bail if it is not present
			if(pWnd == NULL)
				continue;

			//it is present, so change the status
			pWnd->EnableWindow(bEnabled);
		}
	}

	return TRUE;
}

void CPropertyWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CRect rClient;
	GetClientRect(rClient);

	int nDelta = 0;

	int nOldPos = GetScrollPos(SB_VERT);

	int nMaxSize = PROPERTY_HEIGHT * MAX_PROPS_PER_GROUP;

	switch(nSBCode)
	{
	case SB_TOP:				nDelta = -nMaxSize;					break;
	case SB_BOTTOM:				nDelta = nMaxSize;					break;
	case SB_PAGEUP:				nDelta = -rClient.Height();			break;
	case SB_PAGEDOWN:			nDelta = rClient.Height();			break;
	case SB_LINEUP:				nDelta = -PROPERTY_HEIGHT;			break;
	case SB_LINEDOWN:			nDelta = PROPERTY_HEIGHT;			break;
	case SB_THUMBPOSITION:		nDelta = nPos - nOldPos;			break;
	case SB_THUMBTRACK:			nDelta = nPos - nOldPos;			break;
	default:
		break;
	}

	//get the bar range
	int nMinRng, nMaxRng;
	GetScrollRange(SB_VERT, &nMinRng, &nMaxRng);

	//now we need to find the final position and clamp it
	int nNewPos = max(nMinRng, min(nMaxRng - rClient.Height(), nOldPos + nDelta));

	//recalculate the delta (now clamped) and pixel based
	nDelta = nNewPos - nOldPos;

	//see if we even moved
	if(nDelta == 0)
		return;

	SetScrollPos(SB_VERT, nNewPos);


	//scroll the window
	ScrollWindow(0, -nDelta);

	//draw the invalidated rectangle
	UpdateWindow();
	
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

//creates the appropriate controls for a property
void CPropertyWnd::CreatePropertyControls(CPackerProperty* pProp, uint32 nProp, uint32 nID, const CRect& rOrigArea)
{
	ASSERT(pProp);

	//flags for all controls
	DWORD nBaseFlags = WS_CHILD | WS_VISIBLE | WS_TABSTOP;

	//a working area rectangle
	CRect rArea(rOrigArea);

	switch(pProp->GetType())
	{
	case PROPERTY_REAL:
		{
			//create the edit box for editing the number
			CEdit* pNewEdit = new CEdit;
			if(pNewEdit)
			{
				pNewEdit->CreateEx(WS_EX_CLIENTEDGE, "Edit", "", nBaseFlags | ES_AUTOHSCROLL, 
									rArea.left, rArea.top, rArea.Width(), rArea.Height(),
									GetSafeHwnd(), (HMENU)nID);

				CPackerRealProperty* pReal = (CPackerRealProperty*)pProp;

				//set the default
				CString sText;
				sText.Format((pReal->IsInteger()) ? "%.0f" : "%.2f", pReal->GetValue());
				pNewEdit->SetWindowText(sText);

				//save it in the list
				m_pPropControl[nProp][0] = pNewEdit;

				//setup the tooltip
				m_ToolTip.AddWindowTool(pNewEdit, pProp->GetHelp());
			}
		}
		break;
	case PROPERTY_STRING:
		{
			CPackerStringProperty* pString = (CPackerStringProperty*)pProp;

			//rectangle for the edit control
			CRect rEditArea(rArea);

			//see if this is going to be a filename
			if(pString->IsFilename())
			{
				rEditArea.DeflateRect(0, 0, BROWSE_BUTTON_WIDTH, 0);
			}

			//create the edit box for editing the string
			CEdit* pNewEdit = new CEdit;
			if(pNewEdit)
			{
				pNewEdit->CreateEx(WS_EX_CLIENTEDGE, "Edit", "", nBaseFlags | ES_AUTOHSCROLL, 
									rEditArea.left, rEditArea.top, 
									rEditArea.Width(), rEditArea.Height(),
									GetSafeHwnd(), (HMENU)nID);

				//set the default
				pNewEdit->SetWindowText(pString->GetValue());

				//save it in the list
				m_pPropControl[nProp][0] = pNewEdit;
			}

			//setup the tooltip
			m_ToolTip.AddWindowTool(pNewEdit, pProp->GetHelp());

			//create the browse button if needed
			if(pString->IsFilename())
			{
				CButton* pNewButton = new CButton;
				if(pNewButton)
				{
					pNewButton->Create("...", nBaseFlags, CRect(rEditArea.right, rArea.top, rArea.right, rArea.bottom), this, nID + 1);
					m_pPropControl[nProp][1] = pNewButton;

					//setup the button's tooltip
					m_ToolTip.AddWindowTool(pNewButton, IDS_TOOLTIP_BROWSE_FOR_FILE);
				}					
			}
		}
		break;
	case PROPERTY_ENUM:
		{
			//create the combo box for the drop down of selections
			CComboBox* pNewCombo = new CComboBox;
			if(pNewCombo)
			{
				CPackerEnumProperty* pEnum = (CPackerEnumProperty*)pProp;

				CRect rFullArea(rArea);
				rFullArea.InflateRect(0, 0, 0, PROPERTY_HEIGHT * min(3, pEnum->GetNumItems()));

				pNewCombo->Create(nBaseFlags | CBS_DROPDOWNLIST, rFullArea, this, nID);

				//add the items
				for(uint32 nCurrItem = 0; nCurrItem < pEnum->GetNumItems(); nCurrItem++)
				{
					pNewCombo->InsertString(nCurrItem, pEnum->GetItem(nCurrItem));
				}

				//select the item
				pNewCombo->SetCurSel(pEnum->GetSelection());

				//save it in the list
				m_pPropControl[nProp][0] = pNewCombo;

				//setup the tooltip
				m_ToolTip.AddWindowTool(pNewCombo, pProp->GetHelp());
			}
		}
		break;
	case PROPERTY_BOOL:
		{
			//create a check box for checking/unchecking items
			CButton* pNewButton = new CButton;
			if(pNewButton)
			{
				pNewButton->Create("", nBaseFlags | BS_AUTOCHECKBOX, rArea, this, nID); 
				m_pPropControl[nProp][0] = pNewButton;

				//init the default value
				CPackerBoolProperty* pBoolProp = (CPackerBoolProperty*)pProp;
				pNewButton->SetCheck(pBoolProp->GetValue() ? 1 : 0);
		
				//setup the tooltip
				m_ToolTip.AddWindowTool(pNewButton, pProp->GetHelp());
			}
		}
		break;
	case PROPERTY_INTERFACE:
		{
			CPackerInterfaceProperty* pIntf = (CPackerInterfaceProperty*)pProp;

			DWORD nTextStyle	= 0;
			bool  bDisplayName	= true;

			switch(pIntf->GetInterfaceType())
			{
			case CPackerInterfaceProperty::TEXT_LEFT:
				nTextStyle = SS_LEFT;
				break;
			case CPackerInterfaceProperty::TEXT_RIGHT:
				nTextStyle = SS_RIGHT;
				break;
			case CPackerInterfaceProperty::TEXT_CENTER:
				nTextStyle = SS_CENTER;
				break;
			case CPackerInterfaceProperty::SEPARATOR:
				nTextStyle = SS_GRAYFRAME;
				bDisplayName = false;
				rArea.bottom -= rArea.Height() * 3 / 4;
				break;
			default:
				break;
			}

			if(pIntf->GetInterfaceType() != CPackerInterfaceProperty::BLANK)
			{
				CStatic* pStatic = new CStatic;
				if(pStatic)
				{
					if(bDisplayName)
					{
						rArea.top = rArea.bottom - GetAbsoluteFontHeight();
					}

					pStatic->Create(bDisplayName ? pProp->GetName() : "", nBaseFlags | nTextStyle, rArea, this, nID); 
					m_pPropControl[nProp][0] = pStatic;
				}
			}
		}
		break;
	default:
		ASSERT(FALSE);
		break;
	}

	//setup the fonts for all the new properties
	for(uint32 nCurrCtrl = 0; nCurrCtrl < MAX_CONTROLS_PER_PROP; nCurrCtrl++)
	{
		if(m_pPropControl[nProp][nCurrCtrl])
		{
			m_pPropControl[nProp][nCurrCtrl]->SetFont(&m_Font);
		}
	}
}

void CPropertyWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Do not call CWnd::OnPaint() for painting messages
}

BOOL CPropertyWnd::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	//determine if this is a control that we have created. If it is we need to identify
	//the type and handle the saving of the data, and notifying the library so that
	//it can handle enabling/disabling of different items

	DWORD nControlID = LOWORD( wParam );

	//if this isn't one of our controls, let windows handle it
	if(	(m_pGroup == NULL) || (nControlID < BASE_CONTROL_ID) || 
		(nControlID >= m_pGroup->GetNumProperties() * MAX_CONTROLS_PER_PROP + BASE_CONTROL_ID))
	{
		//not for our eyes...
		return CWnd::OnCommand(wParam, lParam);
	}

	//this is one of our controls, let find out which property it maps to
	uint32 nPropIndex = (nControlID - BASE_CONTROL_ID) / MAX_CONTROLS_PER_PROP;

	//find the index to the control
	uint32 nControlIndex = (nControlID - BASE_CONTROL_ID) % MAX_CONTROLS_PER_PROP;

	//get the property
	CPackerProperty* pProp = m_pGroup->GetProperty(nPropIndex);

	DWORD nNotifyCode = HIWORD( wParam );

	bool bModified = false;

	//now handle the different properties
	switch(pProp->GetType())
	{
	case PROPERTY_STRING:
		{
			CPackerStringProperty* pString = (CPackerStringProperty*)pProp;

			//check for the modification of the edit
			if((nControlIndex == 0) && (nNotifyCode == EN_CHANGE))
			{
				CString sValue;
				m_pPropControl[nPropIndex][0]->GetWindowText(sValue);
				pString->SetValue(sValue);
				bModified = true;
			}
			else if((nControlIndex == 1) && (nNotifyCode == BN_CLICKED))
			{
				//they clicked on the browse button, so we need to provide them
				//with the file open dialog
				ASSERT(pString->IsFilename());

				//the flags for the dialog
				DWORD nFlags = (pString->IsFileLoad()) ? OFN_FILEMUSTEXIST : OFN_HIDEREADONLY;

				CFileDialog Dlg(pString->IsFileLoad() ? TRUE : FALSE, NULL, NULL, nFlags, pString->GetFileFilter());
				if(Dlg.DoModal() == IDOK)
				{
					//see if we need to strip the filename off of the path
					CString sFilename = Dlg.GetPathName();

					if(pString->ShouldStripFilename())
					{
						int nSlashPos = max(sFilename.ReverseFind('\\'), sFilename.ReverseFind('/'));

						if(nSlashPos != -1)
							sFilename = sFilename.Left(nSlashPos + 1);
					}

					//we set the string to the filename
					m_pPropControl[nPropIndex][0]->SetWindowText(sFilename);
					bModified = true;
				}
			}
		}
		break;
	case PROPERTY_ENUM:
		{
			if((nControlIndex == 0) && (nNotifyCode == CBN_SELCHANGE))
			{
				CPackerEnumProperty* pEnum = (CPackerEnumProperty*)pProp;
				pEnum->SetSelection(((CComboBox*)m_pPropControl[nPropIndex][0])->GetCurSel());
				bModified = true;
			}
		}
		break;
	case PROPERTY_BOOL:
		{
			if((nControlIndex == 0) && (nNotifyCode == BN_CLICKED))
			{
				CPackerBoolProperty* pBool = (CPackerBoolProperty*)pProp;
				pBool->SetValue(((CButton*)m_pPropControl[nPropIndex][0])->GetCheck() ? true : false);
				bModified = true;
			}
		}
		break;
	case PROPERTY_REAL:
		{
			if((nControlIndex == 0) && (nNotifyCode == EN_CHANGE))
			{
				CString sText;
				m_pPropControl[nPropIndex][0]->GetWindowText(sText);
				((CPackerRealProperty*)pProp)->SetValue((float)atof(sText));
				bModified = true;
			}
		}
		break;
	case PROPERTY_INTERFACE:
		{
		}
		break;
	default:
		ASSERT(FALSE);
		break;
	}

	//if any properties have been modified, we need to let the library know so that it can
	//update the enabled status of the properties
	if(bModified)
	{
		PropertyModified(pProp);
	}
	
	return CWnd::OnCommand(wParam, lParam);
}

//determines the height of the m_Font object
uint32 CPropertyWnd::GetFontHeight()
{
	CDC* pDC = GetDC();
	//install our font
	CFont* pOldFont = pDC->SelectObject(&m_Font);

	//get the struct
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);

	uint32 nHeight = tm.tmAscent;

	//clean up
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	return nHeight;
}

//determines the absolute height (this includes the descending area)
uint32 CPropertyWnd::GetAbsoluteFontHeight()
{
	CDC* pDC = GetDC();
	//install our font
	CFont* pOldFont = pDC->SelectObject(&m_Font);

	//get the struct
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);

	uint32 nHeight = tm.tmAscent;

	//clean up
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	return nHeight + tm.tmDescent;
}


int CPropertyWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//just need to initialize our tool tip control
	m_ToolTip.Create(this);
	
	return 0;
}

//removes all the controls from the tool tip mgr
void CPropertyWnd::RemoveTooltips()
{
	//make sure that we have already been set up
	ASSERT(m_pGroup);

	uint32 nNumProps = m_pGroup->GetNumProperties();

	//run through all the controls, make and update the enabled
	//run through our list of controls and free each one
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		for(uint32 nCurrCtrl = 0; nCurrCtrl < MAX_CONTROLS_PER_PROP; nCurrCtrl++)
		{
			//see if this control exists
			if(m_pPropControl[nCurrProp][nCurrCtrl])
			{
				m_ToolTip.DelTool(m_pPropControl[nCurrProp][nCurrCtrl]);
			}
		}
	}
}

//called when a property has been modified from the outside
BOOL CPropertyWnd::PropertyModified(CPackerProperty* pProp)
{
	//first off notify the packer
	m_pIPacker->PropertyChanged(pProp, &m_pPropMgr->GetPropList(), m_pPropMgr);

	//now the enabled status could have changed, so refresh it
	UpdateControlEnabledStatus();

	//also update the values
	UpdateControlValues();

	return TRUE;
}

BOOL CPropertyWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	//fake a scroll message
	SendMessage(WM_VSCROLL, (zDelta > 0) ? SB_LINEUP : SB_LINEDOWN, NULL);
		
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CPropertyWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//we just want to grab the focus when this occurs
	SetFocus();
	
	CWnd::OnLButtonDown(nFlags, point);
}
