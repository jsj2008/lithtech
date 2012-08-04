#include "bdefs.h"
#include "levelerroroptionsdlg.h"
#include "resource.h"

#include "levelerrordb.h"

BEGIN_MESSAGE_MAP(CLevelErrorOptionsDlg, CDialog)
	ON_NOTIFY(LVN_GETINFOTIP, IDC_LIST_ERROR_DETECTORS, OnListTooltip)
	ON_BN_CLICKED(IDC_BUTTON_RESET_ALL_DETECTORS, OnResetAll)
END_MESSAGE_MAP()


CLevelErrorOptionsDlg::CLevelErrorOptionsDlg(CLevelErrorDB* pDB) :
	CDialog(IDD_LEVEL_ERROR_OPTIONS),
	m_pDB(pDB)
{
	//there MUST be a database passed
	ASSERT(pDB);
}

CLevelErrorOptionsDlg::~CLevelErrorOptionsDlg()
{
}

BOOL CLevelErrorOptionsDlg::OnInitDialog()
{
	ASSERT(m_pDB);

	//get the list
	CListCtrl*	pList = ((CListCtrl*)GetDlgItem(IDC_LIST_ERROR_DETECTORS));

	//setup some list styles
	pList->SetExtendedStyle(	pList->GetExtendedStyle() |
								LVS_EX_CHECKBOXES |
								LVS_EX_FULLROWSELECT |
								LVS_EX_INFOTIP );

	//run through the various detectors and add them to the list
	for(uint32 nCurrDet = 0; nCurrDet < m_pDB->GetNumDetectors(); nCurrDet++)
	{
		//get the current detector
		CErrorDetector* pDet = m_pDB->GetDetector(nCurrDet);

		int nItem = pList->InsertItem(0, pDet->GetName());
		pList->SetCheck(nItem, pDet->IsEnabled());
		pList->SetItemData(nItem, (DWORD)pDet);
	}

	return TRUE;
}

void CLevelErrorOptionsDlg::OnOK()
{
	//we need to read the status from the list back into all the detectors
	ASSERT(m_pDB);

	//get the list
	CListCtrl*	pList = ((CListCtrl*)GetDlgItem(IDC_LIST_ERROR_DETECTORS));

	//run through the various detectors and add them to the list
	for(uint32 nCurrDet = 0; nCurrDet < pList->GetItemCount(); nCurrDet++)
	{
		//get the current detector
		CErrorDetector* pDet = (CErrorDetector*)pList->GetItemData(nCurrDet);

		//update the enabled status
		pDet->SetEnabled(pList->GetCheck(nCurrDet));
	}

	CDialog::OnOK();
}

void CLevelErrorOptionsDlg::OnCancel()
{
	//do nothing
	CDialog::OnCancel();
}

void CLevelErrorOptionsDlg::OnListTooltip(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	//get the list
	CListCtrl*	pList = ((CListCtrl*)GetDlgItem(IDC_LIST_ERROR_DETECTORS));

	LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP)pNotifyStruct;

	//see which item is being referred to
	CErrorDetector* pDet = (CErrorDetector*)pList->GetItemData(pGetInfoTip->iItem);

	if(pDet->GetHelpText())
	{
		strncpy(pGetInfoTip->pszText, pDet->GetHelpText(), pGetInfoTip->cchTextMax);
		
		//make sure we have an ending to the string
		pGetInfoTip->pszText[pGetInfoTip->cchTextMax - 1] = '\0';
	}
}

void CLevelErrorOptionsDlg::OnResetAll()
{
	//run through all list items and turn the check on
	//get the list
	CListCtrl*	pList = ((CListCtrl*)GetDlgItem(IDC_LIST_ERROR_DETECTORS));

	//run through the various detectors and add them to the list
	for(uint32 nCurrDet = 0; nCurrDet < pList->GetItemCount(); nCurrDet++)
	{
		pList->SetCheck(nCurrDet, TRUE);
	}
}



