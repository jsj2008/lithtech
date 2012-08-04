#include "bdefs.h"
#include "leveltexturesoptionsdlg.h"
#include "resource.h"

#include "levelerrordb.h"

BEGIN_MESSAGE_MAP(CLevelTexturesOptionsDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_RESET_ALL_DETECTORS, OnResetAll)
END_MESSAGE_MAP()


CLevelTexturesOptionsDlg::CLevelTexturesOptionsDlg(CLevelTexturesColumn* pColumns, uint32 nCount) :
	CDialog(IDD_LEVEL_TEXTURES_OPTIONS),
	m_pColumns(pColumns),
	m_nNumColumns(nCount)
{
	//there MUST be a database passed
	ASSERT(pColumns);
}

CLevelTexturesOptionsDlg::~CLevelTexturesOptionsDlg()
{
}

BOOL CLevelTexturesOptionsDlg::OnInitDialog()
{
	//get the list
	CListCtrl*	pList = ((CListCtrl*)GetDlgItem(IDC_LIST_TEXTURE_COLUMNS));

	//setup some list styles
	pList->SetExtendedStyle(	pList->GetExtendedStyle() |
								LVS_EX_CHECKBOXES |
								LVS_EX_FULLROWSELECT |
								LVS_EX_INFOTIP );

	//run through the various detectors and add them to the list
	for(uint32 nCurrCol = 0; nCurrCol < m_nNumColumns; nCurrCol++)
	{
		//get the current detector
		CLevelTexturesColumn* pCol = &m_pColumns[nCurrCol];

		int nItem = pList->InsertItem(0, pCol->m_sName);
		pList->SetCheck(nItem, pCol->m_bEnabled);
		pList->SetItemData(nItem, (DWORD)pCol);
	}

	return TRUE;
}

void CLevelTexturesOptionsDlg::OnOK()
{
	//get the list
	CListCtrl*	pList = ((CListCtrl*)GetDlgItem(IDC_LIST_TEXTURE_COLUMNS));

	//run through the various detectors and add them to the list
	for(uint32 nCurrDet = 0; nCurrDet < pList->GetItemCount(); nCurrDet++)
	{
		//get the current detector
		CLevelTexturesColumn* pCol = (CLevelTexturesColumn*)pList->GetItemData(nCurrDet);

		//update the enabled status
		pCol->m_bEnabled = pList->GetCheck(nCurrDet);
	}

	CDialog::OnOK();
}

void CLevelTexturesOptionsDlg::OnCancel()
{
	//do nothing
	CDialog::OnCancel();
}

void CLevelTexturesOptionsDlg::OnResetAll()
{
	//run through all list items and turn the check on
	//get the list
	CListCtrl*	pList = ((CListCtrl*)GetDlgItem(IDC_LIST_TEXTURE_COLUMNS));

	//run through the various detectors and add them to the list
	for(uint32 nCurrDet = 0; nCurrDet < pList->GetItemCount(); nCurrDet++)
	{
		CLevelTexturesColumn* pCol = (CLevelTexturesColumn*)pList->GetItemData(nCurrDet);
		pList->SetCheck(nCurrDet, pCol->m_bDefaultEnabled);
	}
}



