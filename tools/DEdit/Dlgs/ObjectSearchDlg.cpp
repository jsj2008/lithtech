#include "bdefs.h"
#include "objectsearchdlg.h"
#include "resource.h"
#include "edithelpers.h"
#include "editprojectmgr.h"
#include "dirdialog.h"
#include "filepalette.h"
#include "texture.h"
#include "regiondoc.h"
#include "worldnode.h"
#include "ProjectBar.h"
#include "levelerror.h"
#include "multilinestringdlg.h"

#if _MSC_VER >= 1300
#include <fstream>
#else
#include <fstream.h>
#endif

#define INVALID_RESULT		-1

//----------------------------------------------------------------------------
// CObjectSearchDlg


BEGIN_MESSAGE_MAP (CObjectSearchDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnButtonSearch)	

	ON_EN_CHANGE(IDC_EDIT_SEARCH_TEXT, OnSearchTextChanged)

	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST_SEARCH_RESULTS, OnActivateItem)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_SEARCH_RESULTS, OnSelectionChange)

END_MESSAGE_MAP()

CObjectSearchDlg::CObjectSearchDlg() :	
	CDialog(IDD_OBJECTSEARCH),
	m_pSrcDoc(NULL)
{
}

CObjectSearchDlg::~CObjectSearchDlg()
{
}

//---------------------------------------------------------------------------------------
// User Interface
//
// Functions for handling the user interface of the dialog
//
//---------------------------------------------------------------------------------------

//standard button handlers
void CObjectSearchDlg::OnOK()
{
	//don't call the base OK....
	ShowWindow(SW_HIDE);
}

void CObjectSearchDlg::OnCancel()
{
	//don't call the base cancel, but hide ourselves...
	ShowWindow(SW_HIDE);
}

//handle initialization and loading of icons
BOOL CObjectSearchDlg::OnInitDialog()
{
	if(!CDialog::OnInitDialog())
		return FALSE;

	//make it so the user can select any part of that row. Makes it much easier to use
	GetResultsList()->SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP );
	
	//update the enabled status
	UpdateEnabled();

	//add our columns to the stat list
	GetResultsList()->InsertColumn(0, "Property", LVCFMT_LEFT, 150);
	GetResultsList()->InsertColumn(1, "Value", LVCFMT_LEFT, 440);
	GetResultsList()->InsertColumn(2, "Type", LVCFMT_LEFT, 75);

	return TRUE;
}

//handles updating the various controls
void CObjectSearchDlg::UpdateEnabled()
{
	//get the number of items selected in the list
	CString sSearchText;
	GetDlgItem(IDC_EDIT_SEARCH_TEXT)->GetWindowText(sSearchText);

	//remove spaces so we don't accidentally report a blank field as valid
	sSearchText.TrimLeft();
	
	GetDlgItem(IDC_BUTTON_SEARCH)->EnableWindow(!sSearchText.IsEmpty());
}

//handle when the selection changes
void CObjectSearchDlg::OnSelectionChange(NMHDR* pmnh, LRESULT* pResult)
{
	//see if this is actually a change
	NMLISTVIEW* pLVHdr = (NMLISTVIEW*)pmnh;

	if(pLVHdr->uChanged & LVIF_STATE)
	{
		//nothing for now
	}
}

//given a property, this will return a string equivilant form of it
static CString	PropToString(CBaseProp* pProp)
{
	CString sRV;

	switch(pProp->GetType())
	{
	case LT_PT_STRING:
		sRV = ((CStringProp*)pProp)->GetString();
		break;
	case LT_PT_LONGINT:
		sRV.Format("%d", (int32)(((CRealProp*)pProp)->m_Value));
		break;
	case LT_PT_REAL:
		sRV.Format("%.2f", ((CRealProp*)pProp)->m_Value);
		break;
	case LT_PT_VECTOR:
		sRV.Format("%.2f %.2f %.2f", VEC_EXPAND(((CVectorProp*)pProp)->m_Vector));
		break;
	case LT_PT_COLOR:
		sRV.Format("%.0f %.0f %.0f", VEC_EXPAND(((CVectorProp*)pProp)->m_Vector));
		break;
	case LT_PT_ROTATION:
		sRV.Format("%.2f %.2f %.2f", VEC_EXPAND(((CRotationProp*)pProp)->GetEulerAngles()));
		break;
	case LT_PT_BOOL:
		sRV = (((CBoolProp*)pProp)->m_Value) ? "True" : "False";
		break;
	case LT_PT_FLAGS:
		sRV.Format("%d", (int32)(((CRealProp*)pProp)->m_Value));
		break;
	default:
		assert(!"Unrecognized property type found");
		break;
	}

	return sRV;
}

//given a property, this will return the string name of the type
static const char*	PropTypeToString(CBaseProp* pProp)
{
	CString sRV;

	switch(pProp->GetType())
	{
	case LT_PT_STRING:
		return "String";
		break;
	case LT_PT_LONGINT:
		return "Integer";
		break;	
	case LT_PT_REAL:
		return "Real";
		break;	
	case LT_PT_VECTOR:
		return "Vector";
		break;	
	case LT_PT_COLOR:
		return "Color";
		break;	
	case LT_PT_ROTATION:
		return "Rotation";
		break;	
	case LT_PT_BOOL:
		return "Bool";
		break;	
	case LT_PT_FLAGS:
		return "Flags";
		break;	
	default:
		return "Unknown";
		break;
	}
}

static bool IsSeparatorChar(char ch)
{
	//allow space separators
	if(isspace(ch))
		return true;

	//also allow certain assorted characters
	if(	(ch == '(') ||
		(ch == ')') ||
		(ch == ';') ||
		(ch == '\"') ||
		(ch == '\''))
	{
		return true;
	}

	return false;
}

//this will search a property based upon the specified parameters and return whether or not it should be included
bool CObjectSearchDlg::SearchProperty(CBaseProp* pProp, const CString& sSearchText, bool bMatchWholeWord, bool bCaseSensitive)
{
	//first off get the string form of the property
	CString sProp = PropToString(pProp);

	if(!bCaseSensitive)
	{
		sProp.MakeUpper();
	}

	//alright, now try and find that string in there
	int nSearchPos = 0;
	int nSearchRes = -1;

	while((nSearchRes = sProp.Find(sSearchText, nSearchPos)) != -1)
	{
		//we have a match, handle whole word matching
		if(!bMatchWholeWord)
		{
			//we don't care about matching the word, success
			return true;
		}

		//the ending location of the found string
		int nEndRes = nSearchRes + sSearchText.GetLength();

		//see if left and right is a valid character
		if( ((nSearchRes == 0) || IsSeparatorChar(sProp[nSearchRes - 1])) && 
			((nEndRes >= sProp.GetLength()) || IsSeparatorChar(sProp[nEndRes])))
		{
			//this is valid
			return true;
		}

		nSearchPos = nSearchRes + 1;
	}

	//no such luck
	return false;
}


void CObjectSearchDlg::InsertObject(const CString& sObjectName, CWorldNode* pNode, const char* pszType )
{
	SSearchResult Result;
	Result.m_eType			= SSearchResult::eLineType_ObjectName;
	Result.m_nNodeID		= pNode->GetUniqueID();
	Result.m_sName			= sObjectName;
	Result.m_bCanEditProps	= false;
	Result.m_sType			= pszType;

	uint32 nIndex			= m_Results.GetSize();

	m_Results.Append(Result);

	int nAddedAt = GetResultsList()->InsertItem(GetResultsList()->GetItemCount(), "Object:");
	GetResultsList()->SetItemText(nAddedAt, 1, sObjectName);
	GetResultsList()->SetItemText(nAddedAt, 2, pszType);
	GetResultsList()->SetItemData(nAddedAt, nIndex);
}

void CObjectSearchDlg::InsertProperty(const CString& sPropertyName, const CString& sPropertyVal, CWorldNode* pNode, bool bCanEditProps, const char* pszType)
{
	SSearchResult Result;
	Result.m_eType			= SSearchResult::eLineType_Property;
	Result.m_nNodeID		= pNode->GetUniqueID();
	Result.m_sName			= sPropertyName;
	Result.m_sPropertyValue = sPropertyVal;
	Result.m_bCanEditProps	= bCanEditProps;
	Result.m_sType			= pszType;

	uint32 nIndex			= m_Results.GetSize();

	m_Results.Append(Result);

	int nAddedAt = GetResultsList()->InsertItem(GetResultsList()->GetItemCount(), sPropertyName);
	GetResultsList()->SetItemText(nAddedAt, 1, sPropertyVal);
	GetResultsList()->SetItemText(nAddedAt, 2, pszType);
	GetResultsList()->SetItemData(nAddedAt, nIndex);
}

void CObjectSearchDlg::InsertBlankLine()
{
	int nAddedAt = GetResultsList()->InsertItem(GetResultsList()->GetItemCount(), "");
	GetResultsList()->SetItemData(nAddedAt, INVALID_RESULT);
}

//This function does the actual searching, and will recurse through the node tree finding
//and adding selections
void CObjectSearchDlg::ApplySearchR(CWorldNode* pNode, const CString& sSearchText, const CString& sPrefabPrefix, CWorldNode* pPrefabRoot, 
									bool bMatchWholeWord, bool bCaseSensitive, bool bSearchSelection, bool bSearchPrefabs,
									bool bIgnoreHidden, bool bIgnoreFrozen)
{
	bool bIgnore = false;

	//handle exclusion criteria
	if(bSearchSelection && !pNode->IsFlagSet(NODEFLAG_SELECTED))
		bIgnore = true;

	if(bIgnoreHidden && pNode->IsFlagSet(NODEFLAG_HIDDEN))
		bIgnore = true;

	if(bIgnoreFrozen && pNode->IsFlagSet(NODEFLAG_FROZEN))
		bIgnore = true;

	//first off, see if we need to process this object
	if(!bIgnore)
	{
		bool bAddedProps = false;

		//we need to search this object's properties
		for(uint32 nCurrProp = 0; nCurrProp < pNode->GetPropertyList()->GetSize(); nCurrProp++)
		{
			CBaseProp* pProp = pNode->GetPropertyList()->GetAt(nCurrProp);

			if(SearchProperty(pProp, sSearchText, bMatchWholeWord, bCaseSensitive))
			{
				//this is a match, we need to add this to our list
				CWorldNode* pObjectNode = (pPrefabRoot) ? pPrefabRoot : pNode;

				//first add our object line though if this is the first prop
				if(!bAddedProps)
				{
					bAddedProps = true;
					InsertObject(sPrefabPrefix + pNode->GetName(), pObjectNode, pNode->GetClassName());
				}

				//add our line
				InsertProperty(pProp->GetName(), PropToString(pProp), pObjectNode, !pPrefabRoot, PropTypeToString(pProp));
			}
		}

		//add the spacing line at the bottom if we added props
		if(bAddedProps)
		{
			InsertBlankLine();
		}
	}

	//alright, now see if we need to search through prefab trees
	if(bSearchPrefabs && (pNode->GetType() == Node_PrefabRef))
	{
		//append our prefab name data
		CString sNewPrefabPrefix = sPrefabPrefix + pNode->GetName() + '.';

		//figure out the new parent
		CWorldNode* pFinalRoot = (pPrefabRoot) ? pPrefabRoot : pNode;

		//recurse into the prefabs world
		ApplySearchR(const_cast<CWorldNode*>(((CPrefabRef*)pNode)->GetPrefabTree()), sSearchText, sNewPrefabPrefix, pFinalRoot, bMatchWholeWord, bCaseSensitive, bSearchSelection, bSearchPrefabs, bIgnoreHidden, bIgnoreFrozen);
	}

	//now we need to go through all children
	for( GPOS Pos = pNode->m_Children; Pos; )
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);
		ApplySearchR(pChild, sSearchText, sPrefabPrefix, pPrefabRoot, bMatchWholeWord, bCaseSensitive, bSearchSelection, bSearchPrefabs, bIgnoreHidden, bIgnoreFrozen);
	}
}


//clears out the list control of all items
void CObjectSearchDlg::ClearResultsList()
{
	GetResultsList()->DeleteAllItems();
	m_Results.SetSize(0);
}

//handle the button for updating the texture list
void CObjectSearchDlg::OnButtonSearch()
{
	//get the active document
	CRegionDoc* pDoc = ::GetActiveRegionDoc();

	//Bail if we aren't setup with an active level
	if(pDoc == NULL)
	{
		MessageBox("Unable to perform search since no level is open", "Error searching", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//this is now our source document
	m_pSrcDoc = pDoc;

	CWaitCursor WaitCursor;

	//clear out the old
	ClearResultsList();

	//we need to read in all the properties
	bool bMatchWholeWord	= GetCheck(IDC_CHECK_MATCH_WHOLE_WORD);
	bool bCaseSensitive		= GetCheck(IDC_CHECK_CASE_SENSITIVE);
	bool bSearchSelection	= GetCheck(IDC_CHECK_SEARCH_SELECTION_ONLY);
	bool bSearchPrefabs		= GetCheck(IDC_CHECK_SEARCH_PREFABS);
	bool bIgnoreHidden		= GetCheck(IDC_CHECK_IGNORE_HIDDEN);
	bool bIgnoreFrozen		= GetCheck(IDC_CHECK_IGNORE_FROZEN);
	
	//get the string we need to search for
	CString sSearchText;
	GetDlgItem(IDC_EDIT_SEARCH_TEXT)->GetWindowText(sSearchText);

	if(!bCaseSensitive)
		sSearchText.MakeUpper();

	//alright, now start at the root node and try and find our selections
	ApplySearchR(pDoc->GetRegion()->GetRootNode(), sSearchText, "", NULL, bMatchWholeWord, bCaseSensitive, bSearchSelection, bSearchPrefabs, bIgnoreHidden, bIgnoreFrozen);
}

//given a proprty, this will handle in place editing of the property if applicable
bool CObjectSearchDlg::EditProperty(int nIndex, CBaseProp* pProp, SSearchResult* pResult)
{
	//see if this property is a string
	if(pProp->GetType() == LT_PT_STRING)
	{
		CStringProp* pStringProp = (CStringProp*)pProp;


		//it is, we can do in place editing
		CMultiLineStringDlg TextDlg;

		TextDlg.m_Caption	= "Enter Property Text";
		TextDlg.m_String	= pStringProp->GetString();

		if( TextDlg.DoModal() == IDOK )
		{
			strncpy(pStringProp->m_String, TextDlg.m_String, MAX_STRINGPROP_LEN);
			pStringProp->m_String[MAX_STRINGPROP_LEN] = '\0';

			//update our text in the list
			GetResultsList()->SetItemText(nIndex, 1, pStringProp->m_String);
			return true;
		}
	}

	return false;
}

//handle a double click on a texture
void CObjectSearchDlg::OnActivateItem(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	//Bail if we aren't setup with an active level
	if(m_pSrcDoc == NULL)
	{
		return;
	}

	CListCtrl* pList = GetResultsList();

	//find the first item
	POSITION Pos = pList->GetFirstSelectedItemPosition();
	if(Pos)
	{
		NMITEMACTIVATE* pActiveInfo = (NMITEMACTIVATE*)pNotifyStruct;

		//get the error object associated with this item
		int nIndex = pActiveInfo->iItem;

		//figure out the result index
		int nResultIndex = pList->GetItemData(nIndex);

		//see if it is a blank line
		if(nResultIndex == INVALID_RESULT)
			return;

		//get the texture so we can grab the name
		SSearchResult* pResult = &m_Results[nResultIndex];

		//see if we can find this node
		CWorldNode* pNode = m_pSrcDoc->GetRegion()->FindNodeByID(pResult->m_nNodeID);

		if(!pNode)
			return;

		if(pResult->m_eType == SSearchResult::eLineType_Property)
		{
			//this is a property, we should try and edit it
			CBaseProp* pProp = pNode->GetPropertyList()->GetProp(pResult->m_sName);

			if(pProp && pResult->m_bCanEditProps)
			{
				if(EditProperty(nIndex, pProp, pResult))
				{
					//save the result back into the item
					pResult->m_sPropertyValue = PropToString(pProp);

					//and notify of the property change
					pNode->OnPropertyChanged(pProp, true, NULL);
				}
			}
		}

		//now select this object
		m_pSrcDoc->SelectNode(pNode);

		//select it in the node tree
		GetNodeView()->m_NodeViewTree.Select(pNode->GetItem(), TVGN_CARET);

		//notify the region of the selection
		m_pSrcDoc->NotifySelectionChange();
	}
}

//handle the user changing the search text field
void CObjectSearchDlg::OnSearchTextChanged()
{
	UpdateEnabled();
}

//this should be called when a region document is closed so that the dialog can
//potentially clear out its list
void CObjectSearchDlg::NotifyDocumentClosed(CRegionDoc* pDoc)
{
	if(pDoc == m_pSrcDoc)
	{
		//our document is going away
		//clear out the lists
		m_Results.SetSize(0);
		GetResultsList()->DeleteAllItems();
		m_pSrcDoc = NULL;
	}
}
