// RezDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "RezDlg.h"
#include "LinkList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRezDlg dialog


CRezDlg::CRezDlg(CString sExt, CString sInitialChoice, CWnd* pParent /*=NULL*/)
	: CDialog(CRezDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRezDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_sExt			 = sExt;
	m_sInitialChoice = sInitialChoice;
}


void CRezDlg::DoDataExchange(CDataExchange* pDX)
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	CRezMgr *pMgr = pApp->GetRezMgr();

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRezDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		// We are loading the dialog

		CTreeCtrl *pCtrl = (CTreeCtrl *)GetDlgItem(IDC_REZ);
		pCtrl->SetImageList(pApp->GetImageList(), TVSIL_NORMAL);

		if (pCtrl)
		{
			ReadDirectory(pCtrl->GetRootItem(), pMgr->GetRootDir());
		}

		if (m_sInitialChoice.GetLength())
		{
			// Locate and highlight the choice in the resource dialog

			HTREEITEM hItem = pCtrl->GetRootItem();
			hItem = pCtrl->GetChildItem(hItem);

			if (hItem)
			{
				char sTmp[256];
				strcpy(sTmp, m_sInitialChoice.GetBuffer(m_sInitialChoice.GetLength()));

				// Strip off the extension

				char *sCur = sTmp + strlen(sTmp);

				while ((sCur != sTmp) && (*sCur != '.'))
				{
					sCur --;
				}

				*sCur = 0;

				char *sTok = strtok(sTmp, "\\");

				while (sTok)
				{
					// Search through the siblings for a name with this string and
					// proceed if neccessary

					HTREEITEM hSearchItem = hItem;

					BOOL bSearching = TRUE;
					CString sTest = sTok;

					while ((hSearchItem) && (bSearching))
					{
						CString sName = pCtrl->GetItemText(hSearchItem);
						
						if (sTest == sName)
						{
							bSearching = FALSE;
							if (pCtrl->ItemHasChildren(hSearchItem)) hSearchItem = pCtrl->GetChildItem(hSearchItem);
						}
						else
						{
							hSearchItem = pCtrl->GetNextSiblingItem(hSearchItem);
						}
					}

					hItem = hSearchItem;

					sTok = strtok(NULL, "\\");
				}
				
				// Off we go...

				if (hItem) 
				{
					pCtrl->SetFocus();
					pCtrl->EnsureVisible(hItem);
					pCtrl->SelectItem(hItem);
				}

			}
		}
	}
	else
	{
		
	}
}


BEGIN_MESSAGE_MAP(CRezDlg, CDialog)
	//{{AFX_MSG_MAP(CRezDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_REZ, OnDblclkRez)
	ON_BN_CLICKED(IDC_NONE, OnNone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRezDlg message handlers

//------------------------------------------------------------------
//
//   FUNCTION : ReadDirectory()
//
//   PURPOSE  : Reads in a resource directory
//
//------------------------------------------------------------------

void CRezDlg::ReadDirectory(HTREEITEM hItem, CRezDir *pDir)
{
	CTreeCtrl *pCtrl = (CTreeCtrl *)GetDlgItem(IDC_REZ);
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	CRezMgr *pMgr = pApp->GetRezMgr();

	// Add this directory

	HTREEITEM hNewItem = pCtrl->InsertItem("", IM_CLOSED, IM_OPEN, hItem, TVI_FIRST);

	// Add the resources

	CRezTyp* pRezType = pDir->GetFirstType();
	
	while (pRezType)
	{
		char sExt[128];
		pMgr->TypeToStr(pRezType->GetType(), sExt);

		if (!stricmp((char *)(LPCSTR)m_sExt, sExt))
		{
			CRezItm* pRezItm = pDir->GetFirstItem(pRezType);

			while (pRezItm)
			{				
				// Insert alphabetically

				HTREEITEM hRez = NULL;
				hRez = pCtrl->InsertItem(pRezItm->GetName(), IM_RESOURCE, IM_RESOURCE, hNewItem, TVI_SORT);
				pCtrl->SetItemData(hRez, (DWORD)pRezItm);

				pRezItm = pDir->GetNextItem(pRezItm);
			}
		}

		pRezType = pDir->GetNextType(pRezType);
	}

	pCtrl->SetItemText(hNewItem, pDir->GetDirName());

	// Add any sub directories

	CRezDir *pSubDir = pDir->GetFirstSubDir();

	while (pSubDir)
	{		
		ReadDirectory(hNewItem, pSubDir);
		
		pSubDir = pDir->GetNextSubDir(pSubDir);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnOK()
//
//   PURPOSE  : Handles IDOK
//
//------------------------------------------------------------------

void CRezDlg::OnOK() 
{
	CTreeCtrl *pCtrl = (CTreeCtrl *)GetDlgItem(IDC_REZ);
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	CRezMgr *pMgr = pApp->GetRezMgr();
	if (!pCtrl) return;

	HTREEITEM hItem = pCtrl->GetSelectedItem();
	if (!hItem) return;
	
	int iImage, iSelectedImage;
	pCtrl->GetItemImage(hItem, iImage, iSelectedImage);

	if (iImage == IM_RESOURCE)
	{
		CRezItm *pItem = (CRezItm *)pCtrl->GetItemData(hItem);

		char sExt[128];
		pMgr->TypeToStr(pItem->GetType(), sExt);

		CLinkList<CRezDir *> collDirs;

		CRezDir *pDir = pItem->GetParentDir();

		while (pDir)
		{
			collDirs.AddHead(pDir);
			pDir = pDir->GetParentDir();
		}

		CLinkListNode<CRezDir *> *pNode = collDirs.GetHead();
		
		BOOL bFirst = TRUE;

		while (pNode)
		{			
			m_sPath += pNode->m_Data->GetDirName();
			if (!bFirst) m_sPath += "\\";
			bFirst = FALSE;
			
			pNode = pNode->m_pNext;
		}
	
		m_sPath += pItem->GetName();
		m_sPath += ".";
		m_sPath += sExt;
	}
	
	CDialog::OnOK();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDblclkRez()
//
//   PURPOSE  : Handles double clicking on resource tree control
//
//------------------------------------------------------------------

void CRezDlg::OnDblclkRez(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CTreeCtrl *pCtrl = (CTreeCtrl *)GetDlgItem(IDC_REZ);
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	CRezMgr *pMgr = pApp->GetRezMgr();
	if (!pCtrl) return;

	HTREEITEM hItem = pCtrl->GetSelectedItem();
	if (!hItem) return;
	
	int iImage, iSelectedImage;
	pCtrl->GetItemImage(hItem, iImage, iSelectedImage);

	if (iImage == IM_RESOURCE)
	{
		OnOK();
	}
	
	*pResult = 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnInitDialog()
//
//   PURPOSE  : Handles WM_INITDIALOG
//
//------------------------------------------------------------------

BOOL CRezDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the focuse to the resource tree control

	GetDlgItem(IDC_REZ)->SetFocus();

	return FALSE;
}

void CRezDlg::OnNone() 
{
	m_sPath.Format("...", (char *)(LPCSTR)m_sExt);

	CDialog::OnOK();
}
