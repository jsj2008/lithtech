#include "bdefs.h"
#include "levelerrordlg.h"
#include "edithelpers.h"
#include "levelerroroptionsdlg.h"
#include "errorinformationdlg.h"

//fill out the message map

BEGIN_MESSAGE_MAP(CLevelErrorDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_LEVEL_ERROR, OnSelect)
	ON_BN_CLICKED(IDC_BUTTON_LEVEL_ERROR_OPTIONS, OnOptions)
	ON_BN_CLICKED(IDC_BUTTON_SCAN_LEVEL, OnScanLevel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LEVEL_ERROR_LIST, OnSelectionChanged)
	ON_NOTIFY(LVN_GETINFOTIP, IDC_LEVEL_ERROR_LIST, OnListTooltip)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LEVEL_ERROR_LIST, OnSortItems)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LEVEL_ERROR_LIST, OnActivateItem)
END_MESSAGE_MAP()


CLevelErrorDlg::CLevelErrorDlg() :
	CDialog(IDD_LEVEL_ERROR),
	m_pSrcDoc(NULL)
{
}

CLevelErrorDlg::~CLevelErrorDlg()
{
}

//message handlers
void CLevelErrorDlg::OnScanLevel()
{
	//get the active document
	CRegionDoc* pDoc = ::GetActiveRegionDoc();

	//we can't scan a level if there is no level!
	if(pDoc == NULL)
	{
		MessageBox("A level must be opened before this operation can be performed", "Error Scanning Level", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	//show the user a wait cursor
	CWaitCursor WaitCursor;

	//update the load status
	m_ErrorDB.LoadEnabledStatus((COptionsBase*)(GetApp()->GetOptions().GetMiscOptions()));

	//scan the region
	m_ErrorDB.BuildErrorList(pDoc);

	m_pSrcDoc = pDoc;	

	//update the list
	UpdateList();
}

//updates the list with all the items
void CLevelErrorDlg::UpdateList()
{
	//run through all the items in the list and add them to the list control

	//get the list
	CListCtrl* pList = GetList();

	//clear out the list
	pList->DeleteAllItems();

	for(uint32 nCurrError = 0; nCurrError < m_ErrorDB.GetNumErrors(); nCurrError++)
	{
		CLevelError* pError = m_ErrorDB.GetError(nCurrError);

		//add this item to the list and have it hold a pointer to the error
		int nItem = pList->InsertItem(0, pError->GetName());

		//set up the severity
		CString sSeverity;
		switch(pError->GetSeverity())
		{
		case ERRORSEV_CRITICAL:		sSeverity = "Critical";		break;
		case ERRORSEV_HIGH:			sSeverity = "High";			break;
		case ERRORSEV_MEDIUM:		sSeverity = "Medium";		break;
		case ERRORSEV_LOW:			sSeverity = "Low";			break;
		case ERRORSEV_VERYLOW:		sSeverity = "Very Low";		break;
		}

		pList->SetItemText(nItem, 1, sSeverity);

		//see if it has a node
		if(pError->HasNode())
		{
			pList->SetItemText(nItem, 2, pError->GetNodeName());
		}

		pList->SetItemData(nItem, (DWORD)pError);
	}

	//clear the buttons that require a selection (windows doesn't do this appropriately in
	//some cases, like if you clear a list only)
	((CButton*)GetDlgItem(IDC_BUTTON_SELECT_LEVEL_ERROR))->EnableWindow(FALSE);
	
	//setup the counts down below
	CString sText;
	sText.Format("%d Level Errors", m_ErrorDB.GetNumErrors());
	((CStatic*)GetDlgItem(IDC_STATIC_NUM_LEVEL_ERRORS))->SetWindowText(sText);
}

void CLevelErrorDlg::OnOptions()
{
	//bring up the options dialog to modify the enabled status of all the
	//detectors

	CLevelErrorOptionsDlg Dlg(&m_ErrorDB);
	if(Dlg.DoModal() == IDOK)
	{
		//now save the status of the detectors
		m_ErrorDB.SaveEnabledStatus((COptionsBase*)(GetApp()->GetOptions().GetMiscOptions()));
	}
}

void CLevelErrorDlg::OnSelect()
{
	//must have a source doc
	if((m_pSrcDoc == NULL) || (m_pSrcDoc->GetRegion() == NULL))
	{
		return;
	}

	//clear all existing selections
	m_pSrcDoc->GetRegion()->ClearSelections();

	//run through the selection and for each item selected, in the list,
	//we need to select it in the world
	CListCtrl* pList = GetList();

	POSITION Pos = pList->GetFirstSelectedItemPosition();
	while(Pos)
	{
		//get the error object associated with this item
		int nIndex = pList->GetNextSelectedItem(Pos);
		DWORD nItemData = pList->GetItemData(nIndex);
		CLevelError* pError = (CLevelError*)nItemData;

		//now we need to select that node in the world
		if(pError->HasNode())
		{
			CWorldNode* pNode = CLevelError::FindNodeInRegion(m_pSrcDoc, pError->GetNodeID());

			if(pNode)
			{
				m_pSrcDoc->GetRegion()->SelectNode(pNode, TRUE);
			}
		}
	}

	m_pSrcDoc->NotifySelectionChange( );
	m_pSrcDoc->RedrawAllViews( );
}


void CLevelErrorDlg::OnOK()
{
	//hide the window
	ShowWindow(SW_HIDE);
}

void CLevelErrorDlg::OnCancel()
{
	//hide the window
	ShowWindow(SW_HIDE);
}

//retreives the list control
CListCtrl* CLevelErrorDlg::GetList()
{
	return ((CListCtrl*)GetDlgItem(IDC_LEVEL_ERROR_LIST));
}

//notification when the dialog needs to be set up
BOOL CLevelErrorDlg::OnInitDialog()
{
	//setup the detectors for the error database
	m_ErrorDB.BuildDetectorList();

	//set up the columns for the list
	CListCtrl* pList = GetList();

	//set some advanced options on the list control
	pList->SetExtendedStyle(pList->GetExtendedStyle() | 
								LVS_EX_FULLROWSELECT | 
								LVS_EX_GRIDLINES |
								LVS_EX_HEADERDRAGDROP |
								LVS_EX_INFOTIP);

	pList->InsertColumn(0, "Issue",		LVCFMT_LEFT, 130);
	pList->InsertColumn(1, "Severity",	LVCFMT_LEFT, 80);
	pList->InsertColumn(2, "Node",		LVCFMT_LEFT, 80);

	((CButton*)GetDlgItem(IDC_BUTTON_SELECT_LEVEL_ERROR))->EnableWindow(FALSE);

	//update the load status
	m_ErrorDB.LoadEnabledStatus((COptionsBase*)(GetApp()->GetOptions().GetMiscOptions()));

	UpdateList();

	return TRUE;
}

void CLevelErrorDlg::OnSelectionChanged(NMHDR * pNotifyStruct, LRESULT * pResult )
{
	CListCtrl* pList = GetList();

	//see if any can be selected
	BOOL bCanSelect			= FALSE;
	POSITION Pos = pList->GetFirstSelectedItemPosition();
	while(Pos)
	{
		//get the error object associated with this item
		int nIndex = pList->GetNextSelectedItem(Pos);

		CLevelError* pError = (CLevelError*)pList->GetItemData(nIndex);

		if(pError->HasNode())
		{
			bCanSelect = TRUE;
		}
	}

	//enable/disable buttons based upon the count
	((CButton*)GetDlgItem(IDC_BUTTON_SELECT_LEVEL_ERROR))->EnableWindow(bCanSelect);
}

void CLevelErrorDlg::OnListTooltip(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP)pNotifyStruct;

	//see which item is being referred to
	CLevelError* pError = (CLevelError*)GetList()->GetItemData(pGetInfoTip->iItem);

	strncpy(pGetInfoTip->pszText, pError->GetHelp(), pGetInfoTip->cchTextMax);

	//make sure we have an ending to the string
	pGetInfoTip->pszText[pGetInfoTip->cchTextMax - 1] = '\0';
}

int CALLBACK SortErrorList(LPARAM lParam1, LPARAM lParam2, LPARAM nCol)
{
	CLevelError* pError1 = (CLevelError*)lParam1;
	CLevelError* pError2 = (CLevelError*)lParam2;

	//see what we are sorting upon
	switch(nCol)
	{
	case 0:
		//the name
		return strcmp(pError1->GetName(), pError2->GetName());
		break;

	case 1:
		//the severity
		return (pError1->GetSeverity() - pError2->GetSeverity());
		break;

	case 2:
		//the brush name
		{
			if(!pError1->HasNode())
				return -1;
			if(!pError2->HasNode())
				return 1;
			return strcmp(pError1->GetNodeName(), pError2->GetNodeName());
		}
		break;
	default:
		//huh?
		break;
	}

	return 0;
}



void CLevelErrorDlg::OnSortItems(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pNotifyStruct; 

	//figure out which column was clicked
	DWORD nCol = pnmv->iSubItem;

	//now we need to sort the list based upon that column
	GetList()->SortItems(SortErrorList, nCol);
}

void CLevelErrorDlg::OnActivateItem(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	CListCtrl* pList = GetList();

	//find the first item
	POSITION Pos = pList->GetFirstSelectedItemPosition();
	if(Pos)
	{
		//get the error object associated with this item
		int nIndex = pList->GetNextSelectedItem(Pos);

		CLevelError* pError = (CLevelError*)pList->GetItemData(nIndex);
		
		CErrorInformationDlg Dlg(pError, m_pSrcDoc);
		Dlg.DoModal();

		//now save the status of the detectors, in case the user modified them
		m_ErrorDB.SaveEnabledStatus((COptionsBase*)(GetApp()->GetOptions().GetMiscOptions()));
	}
}

//this should be called when a region document is closed so that the dialog can
//potentially clear out its list
void CLevelErrorDlg::NotifyDocumentClosed(CRegionDoc* pDoc)
{
	if(pDoc == m_pSrcDoc)
	{
		//our document is going away
		//clear out the lists
		m_ErrorDB.RemoveAll();
		UpdateList();
		m_pSrcDoc = NULL;
	}
}