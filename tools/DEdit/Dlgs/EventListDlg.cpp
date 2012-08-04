#include "bdefs.h"
#include "dedit.h"
#include "resource.h"
#include "eventlistdlg.h"
#include "eventeditordlg.h"

//different column ID's
#define COLUMN_MESSAGE		0
#define COLUMN_TRACK		1
#define COLUMN_TIME			2

//function to handle the sorting of items within columns
static int CALLBACK ListColumnCompare(LPARAM nParam1, LPARAM nParam2, LPARAM nSortOn)
{
	//get the objects
	CObjectEvent* p1 = (CObjectEvent*)nParam1;
	CObjectEvent* p2 = (CObjectEvent*)nParam2;

	//now determine which is less, depending upon which column we are looking at
	switch(nSortOn)
	{
	case COLUMN_MESSAGE:		//the message
		return strcmp(p1->m_sText, p2->m_sText);
		break;
	case COLUMN_TIME:			//the time
		return p1->m_nTrackTime - p2->m_nTrackTime;
		break;
	case COLUMN_TRACK:			//the track
		return p1->m_nTrack - p2->m_nTrack;
		break;
	default:	//unsure
		assert(false);
		return 0;
		break;
	}
}


//--------------------------------------------------------------------------
// CEventListDlg

BEGIN_MESSAGE_MAP(CEventListDlg, CDialog)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_EVENTS, OnEndLabelEdit)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_EVENTS, OnColumnClicked)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST_EVENTS, OnItemActivated)
END_MESSAGE_MAP()

CEventListDlg::CEventListDlg(CObjectEvent* pEvents, CWnd* pParent) :
	CDialog(CEventListDlg::IDD, pParent),
	m_pHeadEvent(pEvents)
{
}

BOOL CEventListDlg::OnInitDialog()
{
	if(!CDialog::OnInitDialog())
		return FALSE;

	//set some advanced options on the list control
	GetListCtrl()->SetExtendedStyle(GetListCtrl()->GetExtendedStyle() | 
										LVS_EX_GRIDLINES | 
										LVS_EX_HEADERDRAGDROP | 
										LVS_EX_FULLROWSELECT);

	//setup the columns
	GetListCtrl()->InsertColumn(COLUMN_MESSAGE, "Message",	LVCFMT_LEFT, 200);
	GetListCtrl()->InsertColumn(COLUMN_TIME,	"Time",		LVCFMT_LEFT, 60);
	GetListCtrl()->InsertColumn(COLUMN_TRACK,	"Track",	LVCFMT_LEFT, 50);

	//now we need to add all of the items
	CObjectEvent* pCurr = m_pHeadEvent;
	uint32 nItemIndex	= 0;

	while(pCurr)
	{
		CString sText;

		//setup all the strings
		int nItem = GetListCtrl()->InsertItem(nItemIndex, pCurr->m_sText);

		sText.Format("%.1fs", pCurr->m_nTrackTime / 1000.0f);
		GetListCtrl()->SetItemText(nItem, COLUMN_TIME, sText);

		if((pCurr->m_nTrack < MAX_TRACKS) && !m_sTrackNames[pCurr->m_nTrack].IsEmpty())
		{
			sText = m_sTrackNames[pCurr->m_nTrack];
		}
		else
		{
			sText.Format("%d", pCurr->m_nTrack + 1);
		}

		GetListCtrl()->SetItemText(nItem, COLUMN_TRACK, sText);

		//save the object reference
		GetListCtrl()->SetItemData(nItem, (DWORD)pCurr);

		//move along in the list
		nItemIndex++;
		pCurr = pCurr->m_pNext;
	}

	return TRUE;
}

void CEventListDlg::OnEndLabelEdit(NMHDR* pNotifyStruct, LRESULT* result)
{
	//figure out which one just got editified
	NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNotifyStruct;

	//get the object that was modified
	CObjectEvent* pObj = (CObjectEvent*)GetListCtrl()->GetItemData(pDispInfo->item.iItem);

	//now save the new text, as long as it isn't NULL, because contrary to what the
	//windows documentation says (surprise), it will be NULL when the user cancels
	if(pDispInfo->item.pszText)
		pObj->m_sText = pDispInfo->item.pszText;

	//allow success
	*result = TRUE;
}

void CEventListDlg::OnColumnClicked(NMHDR* pNotifyStruct, LRESULT* result)
{
	//get the data
	NMLISTVIEW* pListView = (NMLISTVIEW*)pNotifyStruct;

	//identify the column we want to sort on
	DWORD nSortOn = pListView->iSubItem;

	//now sort them
	GetListCtrl()->SortItems(ListColumnCompare, nSortOn);
}

void CEventListDlg::OnItemActivated(NMHDR* pNotifyStruct, LRESULT* result)
{
	NMITEMACTIVATE* pActivate = (NMITEMACTIVATE*)pNotifyStruct;
	GetListCtrl()->EditLabel(pActivate->iItem);

	*result = 0;
}
