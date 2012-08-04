// ButeEditView.cpp : implementation of the CButeEditView class
//

#include "stdafx.h"
#include "ButeEdit.h"

#include "ButeEditDoc.h"
#include "ButeEditView.h"
#include "BEStringFunc.h"
#include "MainFrm.h"

#include "ModifyBoolDlg.h"
#include "ModifyByteDlg.h"
#include "ModifyDoubleDlg.h"
#include "ModifyDWordDlg.h"
#include "ModifyFloatDlg.h"
#include "ModifyIntegerDlg.h"
#include "ModifyPointDlg.h"
#include "ModifyRangeDlg.h"
#include "ModifyRectangleDlg.h"
#include "ModifyStringDlg.h"
#include "ModifyVectorDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_KEY		0
#define COLUMN_TYPE		1
#define COLUMN_DATA		2

/////////////////////////////////////////////////////////////////////////////
// CButeEditView

IMPLEMENT_DYNCREATE(CButeEditView, CListView)

BEGIN_MESSAGE_MAP(CButeEditView, CListView)
	ON_WM_CONTEXTMENU()
	//{{AFX_MSG_MAP(CButeEditView)
	ON_COMMAND(ID_EDIT_MODIFY, OnEditModify)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MODIFY, OnUpdateEditModify)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CListView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CButeEditView construction/destruction

CButeEditView::CButeEditView()
{
	// Indicates if the columns have been created
	m_bColumnsCreated=FALSE;
}

CButeEditView::~CButeEditView()
{
}

BOOL CButeEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CButeEditView drawing

void CButeEditView::OnDraw(CDC* pDC)
{
	CButeEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);	
}

void CButeEditView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	// Make sure that we are in the report style
	ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
	ModifyStyle(NULL, LVS_SINGLESEL);	

	// Add the columns to the list control
	if (!m_bColumnsCreated)
	{
		GetListCtrl().InsertColumn(COLUMN_KEY, "Key    ");
		GetListCtrl().InsertColumn(COLUMN_TYPE, "Type    ");
		GetListCtrl().InsertColumn(COLUMN_DATA, "Data");		
		m_bColumnsCreated=TRUE;
	}

	BuildListControl();
}

/////////////////////////////////////////////////////////////////////////////
// CButeEditView printing

BOOL CButeEditView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CButeEditView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CButeEditView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CButeEditView diagnostics

#ifdef _DEBUG
void CButeEditView::AssertValid() const
{
	CListView::AssertValid();
}

void CButeEditView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CButeEditDoc* CButeEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CButeEditDoc)));
	return (CButeEditDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CButeEditView message handlers
void CButeEditView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO: add code to react to the user changing the view style of your window
}

void CButeEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	BuildListControl();	
}

/************************************************************************/
// Builds the list control
void CButeEditView::BuildListControl()
{
	// Get the document
	CButeEditDoc* pDoc = GetDocument();
	
	// Clear the key array
	m_keyArray.RemoveAll();

	// The list control
	CListCtrl &listControl=GetListCtrl();

	// Clear the list control
	listControl.SetRedraw(FALSE);
	listControl.DeleteAllItems();

	// Get the selected tag
	CString sSelectedTag=pDoc->GetSelectedTag();
	if (sSelectedTag.GetLength() > 0)
	{
		// Get the bute keys
		pDoc->GetButeMgr()->GetKeys(sSelectedTag, GetKeysCallback, (void *)&m_keyArray);

		int i;
		for (i=0; i < m_keyArray.GetSize(); i++)
		{
			// Add the key
			listControl.InsertItem(i, m_keyArray[i]);

			// Add the type for the key
			listControl.SetItemText(i, COLUMN_TYPE, GetTypeString(sSelectedTag, m_keyArray[i]));

			// Add the data for the key
			listControl.SetItemText(i, COLUMN_DATA, GetDataString(sSelectedTag, m_keyArray[i]));

			// Set the data for the item to its index in the key array
			listControl.SetItemData(i, (DWORD)i);
		}
	}

	// Set the column widths
	listControl.SetColumnWidth(COLUMN_KEY, LVSCW_AUTOSIZE_USEHEADER);
	listControl.SetColumnWidth(COLUMN_TYPE, LVSCW_AUTOSIZE_USEHEADER);
	listControl.SetColumnWidth(COLUMN_DATA, LVSCW_AUTOSIZE_USEHEADER);

	// Sort the list control
	listControl.SortItems(ListSortCompareFn, (DWORD)&m_keyArray);

	// Enable the drawing of the control
	listControl.SetRedraw(TRUE);
}

/************************************************************************/
// This is called by the listbox for sorting
int CALLBACK CButeEditView::ListSortCompareFn(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// Get the key array
	CStringArray *pKeyArray=(CStringArray *)lParamSort;

	// Compare the strings
	return pKeyArray->GetAt(lParam1).Compare(pKeyArray->GetAt(lParam2));
}

/************************************************************************/
// Returns a string that represents the type for a tag and key
CString	CButeEditView::GetTypeString(CString sTag, CString sKey)
{
	// Get the document
	CButeEditDoc* pDoc = GetDocument();

	// Get the bute mgr
	CButeMgr *pButeMgr=pDoc->GetButeMgr();

	// The return string
	CString sReturn;

	// Get the type for this tag/key	
	switch (pButeMgr->GetType(sTag, sKey))
	{
	case CButeMgr::IntType:		sReturn="int";			break;		
	case CButeMgr::DwordType:	sReturn="DWORD";		break;
	case CButeMgr::ByteType:	sReturn="byte";			break;
	case CButeMgr::BoolType:	sReturn="bool";			break;
	case CButeMgr::DoubleType:	sReturn="double";		break;
	case CButeMgr::FloatType:	sReturn="float";		break;
	case CButeMgr::StringType:	sReturn="string";		break;
	case CButeMgr::RectType:	sReturn="rectangle";	break;
	case CButeMgr::PointType:	sReturn="point";		break;
	case CButeMgr::VectorType:	sReturn="vector";		break;
	case CButeMgr::RangeType:	sReturn="range";		break;
	}

	// Return the string
	return sReturn;
}

/************************************************************************/
// Returns a string that represents the data for a tag and key
CString	CButeEditView::GetDataString(CString sTag, CString sKey)
{
	// Get the document
	CButeEditDoc* pDoc = GetDocument();

	// Get the bute mgr
	CButeMgr *pButeMgr=pDoc->GetButeMgr();

	// The return string
	CString sReturn;

	// Get the type for this tag/key
	CButeMgr::SymTypes symType=pButeMgr->GetType(sTag, sKey);
	switch (symType)
	{
	case CButeMgr::IntType:
		{
			// Format the integer type
			sReturn.Format("%d", pButeMgr->GetInt(sTag, sKey));
			break;
		}
	case CButeMgr::DwordType:
		{
			// Format the DWORD type
			sReturn.Format("%u", pButeMgr->GetDword(sTag, sKey));
			break;
		}
	case CButeMgr::ByteType:
		{
			// Format the BYTE type
			sReturn.Format("%d", pButeMgr->GetByte(sTag, sKey));
			break;
		}
	case CButeMgr::BoolType:
		{	
			// Format the BOOL type
			if (pButeMgr->GetBool(sTag, sKey))
			{
				sReturn="TRUE";
			}			
			else
			{
				sReturn="FALSE";
			}			
			break;
		}
	case CButeMgr::DoubleType:
		{
			// Format the double type
			sReturn.Format("%f", pButeMgr->GetDouble(sTag, sKey));
			CBEStringFunc::TrimZeros(sReturn);
			break;
		}
	case CButeMgr::FloatType:
		{
			// Format the float type
			sReturn.Format("%f", pButeMgr->GetFloat(sTag, sKey));
			CBEStringFunc::TrimZeros(sReturn);
			break;
		}
	case CButeMgr::StringType:
		{
			// Format the string type
			sReturn=pButeMgr->GetString(sTag, sKey);

			// Remove the double quotes from the string so that it displays correctly
			sReturn.Replace("\"\"", "\"");
			break;
		}
	case CButeMgr::RectType:
		{
			// Format the rectangle type
			CRect rc=pButeMgr->GetRect(sTag, sKey);
			sReturn.Format("(%d, %d, %d, %d)", rc.left, rc.top, rc.right, rc.bottom);
			break;
		}
	case CButeMgr::PointType:
		{
			// Format the point type
			CPoint point=pButeMgr->GetPoint(sTag, sKey);
			sReturn.Format("(%d, %d)", point.x, point.y);
			break;
		}
	case CButeMgr::VectorType:
		{
			// Format the vector type
			CAVector vector=pButeMgr->GetVector(sTag, sKey);
			
			// Create the vector string
			CString x;
			x.Format("%.8f", vector.Geti());
			CBEStringFunc::TrimZeros(x);

			CString y;
			y.Format("%.8f", vector.Getj());
			CBEStringFunc::TrimZeros(y);

			CString z;
			z.Format("%.8f", vector.Getk());
			CBEStringFunc::TrimZeros(z);

			// Format the 3 values into the vector string
			sReturn.Format("<%s, %s, %s>", x, y, z);
			break;
		}
	case CButeMgr::RangeType:
		{
			// Format the range type
			CARange range=pButeMgr->GetRange(sTag, sKey);

			CString sMin;
			sMin.Format("%f", range.GetMin());
			CBEStringFunc::TrimZeros(sMin);

			CString sMax;
			sMax.Format("%f", range.GetMax());
			CBEStringFunc::TrimZeros(sMax);

			sReturn.Format("[%s, %s]", sMin, sMax);
			break;
		}	
	default:
		{
			// The switch statement should have been handled in one of the cases above!
			ASSERT(FALSE);
		}
	}

	// Return the string
	return sReturn;
}

/************************************************************************/
// Call this to modify the currently selected key
// Returns: TRUE if the key is modified
BOOL CButeEditView::ModifySelectedKey()
{	
	// Get the currently selected tag
	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	if (pos)
	{
		// Modify the tag
		return ModifyKey(GetListCtrl().GetNextSelectedItem(pos));
	}
	else
	{
		return FALSE;
	}
}

/************************************************************************/
// Call this to modify a key at a specific index
// Returns: TRUE if the key is modified
BOOL CButeEditView::ModifyKey(int nIndex)
{
	// Get the document
	CButeEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	// Get the selected tag
	CString sSelectedTag=pDoc->GetSelectedTag();

	// Get the specified key
	CString sSelectedKey=GetListCtrl().GetItemText(nIndex, 0);
	
	// Modify the key
	if (ModifyKey(sSelectedTag, sSelectedKey))
	{
		// Update the text		
		GetListCtrl().SetItemText(nIndex, COLUMN_DATA, GetDataString(sSelectedTag, sSelectedKey));
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/************************************************************************/
// Call this to modify a specific tag/key
// Returns: TRUE if the key is modified
BOOL CButeEditView::ModifyKey(CString sTag, CString sKey)
{
	// Get the document
	CButeEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Get the bute manager
	CButeMgr *pButeMgr=pDoc->GetButeMgr();

	// Indicates if the key has been modified
	BOOL bModified=FALSE;

	// Modify the key depending on the type
	switch (pButeMgr->GetType(sTag, sKey))
	{
	case CButeMgr::IntType:
		{
			// Modify the integer type
			CModifyIntegerDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.m_nValue=pButeMgr->GetInt(sTag, sKey);
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();

			ASSERT(pButeMgr->Success());

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::IntType, (void *)&modifyDlg.m_nValue);
				}
				else
				{
					pButeMgr->SetInt(sTag, sKey, modifyDlg.m_nValue);
				}

				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	case CButeMgr::DwordType:
		{
			// Modify the DWORD type
			CModifyDWordDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.m_dwValue=pButeMgr->GetDword(sTag, sKey);
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();
			ASSERT(pButeMgr->Success());

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::DwordType, (void *)&modifyDlg.m_dwValue);
				}
				else
				{
					pButeMgr->SetDword(sTag, sKey, modifyDlg.m_dwValue);
				}

				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	case CButeMgr::ByteType:
		{
			// Modify the BYTE type
			CModifyByteDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.m_nValue=pButeMgr->GetByte(sTag, sKey);
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();
			ASSERT(pButeMgr->Success());

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::ByteType, (void *)&modifyDlg.m_nValue);
				}
				else
				{
					pButeMgr->SetByte(sTag, sKey, modifyDlg.m_nValue);
				}
				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	case CButeMgr::BoolType:
		{	
			// Modify the BOOL type
			CModifyBoolDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.SetValue(pButeMgr->GetBool(sTag, sKey));			
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();
			ASSERT(pButeMgr->Success());

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					bool bValue=modifyDlg.GetValue();
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::BoolType, (void *)&bValue);
				}
				else
				{
					pButeMgr->SetBool(sTag, sKey, modifyDlg.GetValue());
				}
				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	case CButeMgr::DoubleType:
		{
			// Modify the double type
			CModifyDoubleDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.m_fValue=pButeMgr->GetDouble(sTag, sKey);
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();
			ASSERT(pButeMgr->Success());

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::DoubleType, (void *)&modifyDlg.m_fValue);
				}
				else
				{
					pButeMgr->SetDouble(sTag, sKey, modifyDlg.m_fValue);
				}
				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	case CButeMgr::FloatType:
		{
			// Modify the float type
			CModifyFloatDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.m_fValue=pButeMgr->GetFloat(sTag, sKey);
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();
			ASSERT(pButeMgr->Success());

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::FloatType, (void *)&modifyDlg.m_fValue);
				}
				else
				{
					pButeMgr->SetFloat(sTag, sKey, modifyDlg.m_fValue);
				}
				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	case CButeMgr::StringType:
		{
			// Modify the string type
			CModifyStringDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.m_sValue=pButeMgr->GetString(sTag, sKey);
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();
			ASSERT(pButeMgr->Success());

			// Convert \\r and \\n to line feeds and carriage returns						
			modifyDlg.m_sValue.Replace("\\r", "\r");
			modifyDlg.m_sValue.Replace("\\n", "\n");

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Convert \r and \n to \\r and \\n
				modifyDlg.m_sValue.Replace("\r", "\\r");
				modifyDlg.m_sValue.Replace("\n", "\\n");
				
				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::StringType, (void *)&modifyDlg.m_sValue);
				}
				else
				{
					pButeMgr->SetString(sTag, sKey, modifyDlg.m_sValue);
				}
				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	case CButeMgr::RectType:
		{
			// Modify the rectangle type
			CModifyRectangleDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.SetRect(pButeMgr->GetRect(sTag, sKey));
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();
			ASSERT(pButeMgr->Success());

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					CRect rcValue=modifyDlg.GetRect();
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::RectType, (void *)&rcValue);
				}
				else
				{
					pButeMgr->SetRect(sTag, sKey, modifyDlg.GetRect());
				}
				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	case CButeMgr::PointType:
		{
			// Modify the point type				
			CModifyPointDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.SetPoint(pButeMgr->GetPoint(sTag, sKey));
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();
			ASSERT(pButeMgr->Success());

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					CPoint pointValue=modifyDlg.GetPoint();
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::PointType, (void *)&pointValue);
				}
				else
				{
					pButeMgr->SetPoint(sTag, sKey, modifyDlg.GetPoint());
				}
				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	case CButeMgr::VectorType:
		{
			// Modify the vector type			
			CModifyVectorDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.SetVector(pButeMgr->GetVector(sTag, sKey));
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();
			ASSERT(pButeMgr->Success());

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					CAVector vValue=modifyDlg.GetVector();
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::VectorType, (void *)&vValue);
				}
				else
				{
					pButeMgr->SetVector(sTag, sKey, modifyDlg.GetVector());
				}
				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	case CButeMgr::RangeType:
		{
			// Modify the range type
			CModifyRangeDlg modifyDlg;
			modifyDlg.m_sName=sKey;
			modifyDlg.SetRange(pButeMgr->GetRange(sTag, sKey));
			modifyDlg.m_bReplaceAll=GetReplaceAllKeys();
			ASSERT(pButeMgr->Success());

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Save the replace all value
				SetReplaceAllKeys(modifyDlg.m_bReplaceAll);

				// Check to see if all of the keys should be replaced
				if (modifyDlg.m_bReplaceAll)
				{
					CARange rangeValue=modifyDlg.GetRange();
					pDoc->ReplaceAllKeyValues(sKey, CButeMgr::RangeType, (void *)&rangeValue);
				}
				else
				{
					pButeMgr->SetRange(sTag, sKey, modifyDlg.GetRange());
				}
				ASSERT(pButeMgr->Success());
				bModified=TRUE;
			}
			break;
		}
	default:
		{
			// The switch statement should have been handled in one of the cases above!
			ASSERT(FALSE);
		}
	}

	// Set the modified flag
	if (bModified)
	{
		pDoc->SetModifiedFlag();
	}
	return bModified;
}

/************************************************************************/
// The callback function to retrieve the keys in the bute file
bool CButeEditView::GetKeysCallback(const char* szTag, CButeMgr::CSymTabItem *pItem, void* pAux)
{
	// Get the string array
	CStringArray *pStringArray=(CStringArray *)pAux;
	ASSERT(pStringArray);

	// Add the string
	if (pStringArray)
	{
		pStringArray->Add(szTag);
	}

	return true;
}

/************************************************************************/
// The modify menu option was chosen
void CButeEditView::OnEditModify() 
{
	// Modify the current selected key
	ModifySelectedKey();
}

/************************************************************************/
// The command update handler for the "modify" option
void CButeEditView::OnUpdateEditModify(CCmdUI* pCmdUI) 
{
	// Make sure that there is on item selected
	if (GetListCtrl().GetSelectedCount() == 1)
	{
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CButeEditView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Modify the current selected key
	ModifySelectedKey();
	
	*pResult = 0;
}

void CButeEditView::OnContextMenu(CWnd*, CPoint point)
{

	// CG: This block was added by the Pop-up Menu component
	{
		if (point.x == -1 && point.y == -1){
			//keystroke invocation
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);

			point = rect.TopLeft();
			point.Offset(5, 5);
		}

		CMenu menu;
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_BUTE_EDIT_VIEW));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		CWnd* pWndPopupOwner = this;

		while (pWndPopupOwner->GetStyle() & WS_CHILD)
			pWndPopupOwner = pWndPopupOwner->GetParent();

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
			pWndPopupOwner);
	}
}

/***************************************************************/
// Gets whether the "Replace all keys" should be
// enabled by default.
BOOL CButeEditView::GetReplaceAllKeys()
{
	return AfxGetApp()->GetProfileInt("ModifyDlg", "ReplaceAllValues", (int)FALSE);
}

/***************************************************************/
// Sets whether the "Replace all keys" should be
// enabled by default.
void CButeEditView::SetReplaceAllKeys(BOOL bSet)
{
	AfxGetApp()->WriteProfileInt("ModifyDlg", "ReplaceAllValues", (int)bSet);
}

/************************************************************************/
// The user pressed return
void CButeEditView::OnReturn(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Modify the current selected key
	ModifySelectedKey();
	
	*pResult = 0;
}

/************************************************************************/
// The user clicked on an item
void CButeEditView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Get the main window
	CMainFrame *pFrame=(CMainFrame *)AfxGetMainWnd();

	// Update the status bar text
	pFrame->UpdateStatusBarText();

	*pResult = 0;
}
