// FxPropDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "FxPropDlg.h"
#include "RezButton.h"
#include "VectorCombo.h"
#include "FloatSpinCtrl.h"
#include "IntSpinCtrl.h"
#include "VectorButton.h"
#include "tdguard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FXDLG_FIRSTID				5000

/////////////////////////////////////////////////////////////////////////////
// CFxPropDlg dialog


CFxPropDlg::CFxPropDlg(CKey *pKey, CFastList<FX_PROP> *pList, CWnd* pParent /*=NULL*/)
	: CDialog(CFxPropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFxPropDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pKey		 = pKey;
	m_pCollProps = pList;
	m_nCurScrollPos = 0;
}


void CFxPropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFxPropDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{	
		CDC *pDC = GetDC();

		DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP;

		// We are loading the dialog, go through the list
		// of fx properties and add them to the dialog
		
		CFastListNode<FX_PROP> *pNode = m_pCollProps->GetHead();

		CString sDlgName = "FX Properties for ";
		sDlgName += m_pKey->GetFxRef()->m_sName;
		
		if( m_pKey->GetCustomName()[0] )
		{
			sDlgName += " - ";
			sDlgName += m_pKey->GetCustomName();
		}

		SetWindowText(sDlgName);

		// Get the outside rectangle

		CWnd *pWnd = GetDlgItem(IDC_PROPERTIESRECT);
		CRect rcStatic;

		pWnd->GetClientRect(&rcStatic);
		pWnd->ClientToScreen(&rcStatic);
		ScreenToClient(&rcStatic);

		int xTxtStart  = rcStatic.left;
		int xTxtEnd    = rcStatic.left + (rcStatic.Width() / 3);
		int xCtrlStart = xTxtEnd + 10;
		int xCtrlEnd   = rcStatic.right;
		int yCur	   = rcStatic.top;

		DWORD dwID = FXDLG_FIRSTID;
		
		while (pNode)
		{
			FX_DLGPROP *pFxDlgProp = new FX_DLGPROP;

			CString sName = pNode->m_Data.m_sName;
			sName += " :";
			CSize szTxt = pDC->GetTextExtent(sName);
			CRect rcTxt(xTxtStart, yCur, xTxtEnd, yCur + 20);
			CRect rcCtrl(xCtrlStart, yCur, xCtrlEnd, yCur + 20);

			pFxDlgProp->m_fxProp = pNode->m_Data;

			pFxDlgProp->m_pStatic = new CStatic;
			pFxDlgProp->m_pStatic->Create(sName, WS_VISIBLE | SS_RIGHT, rcTxt, this, 0);

			// Based upon the type of variable, create a control
			
			switch (pNode->m_Data.m_nType)
			{
				case FX_PROP::INTEGER :
				{
					char sTmp[256];
					rcCtrl.right = rcCtrl.left + (rcCtrl.Width() / 3);
					CEdit *pEdit = new CEdit;
					pEdit->Create(dwStyle, rcCtrl, this, dwID);

					// Set the initial value

					pEdit->SetWindowText(CString(itoa(pNode->m_Data.m_data.m_nVal, sTmp, 10)));

					// Create the spin control

					CIntSpinCtrl *pControl = new CIntSpinCtrl(pEdit);
					pControl->Create(WS_VISIBLE, CRect(rcCtrl.right, rcCtrl.top, rcCtrl.right + 40, rcCtrl.bottom), this, 0);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pControl);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pEdit);
				}
				break;

				case FX_PROP::FLOAT :
				{
					rcCtrl.right = rcCtrl.left + (rcCtrl.Width() / 3);
					CEdit *pEdit = new CEdit;
					pEdit->Create(dwStyle, rcCtrl, this, dwID);

					// Set the initial value

					char sTmp[256];
					sprintf(sTmp, FLOAT_PRECISION, pNode->m_Data.m_data.m_fVal);
					pEdit->SetWindowText(CString(sTmp));

					// Create the spin control

					CFloatSpinCtrl *pControl = new CFloatSpinCtrl(pEdit);
					pControl->Create(WS_VISIBLE, CRect(rcCtrl.right, rcCtrl.top, rcCtrl.right + 40, rcCtrl.bottom), this, 0);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pControl);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pEdit);
				}
				break;

				case FX_PROP::STRING :
				{
					CEdit *pEdit = new CEdit;
					pEdit->Create(dwStyle, rcCtrl, this, dwID);
					pEdit->SetWindowText(CString(pNode->m_Data.m_data.m_sVal));
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pEdit);
				}
				break;

				case FX_PROP::COMBO :
				{
					char sTmp[256];
					char *sTok;

					strcpy(sTmp, pNode->m_Data.m_data.m_sVal);
					sTok = strtok(sTmp, ",");
					sTok = strtok(NULL, ",");

					// Figure out the widest string

					int nMaxStringLen = 0;

					while (sTok)
					{
						CSize szText = pDC->GetTextExtent(sTok);
						if (szText.cx > nMaxStringLen) nMaxStringLen = szText.cx;
						
						sTok = strtok(NULL, ",");
					}
				
					DWORD dwComboStyle = WS_VSCROLL | CBS_HASSTRINGS | CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL;

					int nCtrlHeight = rcCtrl.Height();

					rcCtrl.bottom = rcCtrl.top + rcCtrl.Height() + 100;
					rcCtrl.right  = rcCtrl.left + nMaxStringLen + 30;
					CComboBox *pBox = new CComboBox;
					pBox->Create(dwStyle | dwComboStyle, rcCtrl, this, dwID);
					pBox->SetItemHeight(-1, nCtrlHeight - 4);

					// Parse out the string

					strcpy(sTmp, pNode->m_Data.m_data.m_sVal);
					sTok = strtok(sTmp, ",");

					// This is our value of the combo box...

					int nSel = atoi(sTok);

					// And these are the actual combo box items

					while (sTok)
					{
						sTok = strtok(NULL, ",");

						if (sTok)
						{
							pBox->AddString(sTok);
						}
					}

					pBox->SetCurSel(nSel);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pBox);
				}
				break;

				case FX_PROP::VECTOR :
				{
					char sTmp[256];

					// Create three edit controls

					int xCur = xCtrlStart;

					CDC *pDC = GetDC();

					CEdit *pEdit;
					CStatic *pStatic;
					CSize szOrd = pDC->GetTextExtent("X:");

					int xWidth = 70;
					int xSpacer = 10;
					
					pEdit = new CEdit;
					pStatic = new CStatic;
					CRect rcStatic(xCur, rcCtrl.top, xCur + szOrd.cx, rcCtrl.bottom);
					pStatic->Create("X:", WS_VISIBLE, rcStatic, this, 0);
					xCur += szOrd.cx + 5;
					CRect rcVec(xCur, rcCtrl.top, xCur + xWidth, rcCtrl.bottom);
					pEdit->Create(dwStyle, rcVec, this, dwID);
					sprintf(sTmp, VECTOR_PRECISION, pNode->m_Data.m_data.m_fVec[0]);
					pEdit->SetWindowText(sTmp);
					dwID ++;
					xCur += xWidth + xSpacer;
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pStatic);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pEdit);


					pEdit = new CEdit;
					pStatic = new CStatic;
					rcStatic = CRect(xCur, rcCtrl.top, xCur + szOrd.cx, rcCtrl.bottom);
					pStatic->Create("Y:", WS_VISIBLE, rcStatic, this, 0);
					xCur += szOrd.cx + 5;
					rcVec = CRect(xCur, rcCtrl.top, xCur + xWidth, rcCtrl.bottom);
					pEdit->Create(dwStyle, rcVec, this, dwID);
					sprintf(sTmp, VECTOR_PRECISION, pNode->m_Data.m_data.m_fVec[1]);
					pEdit->SetWindowText(sTmp);
					dwID ++;
					xCur += xWidth + xSpacer;
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pStatic);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pEdit);


					pEdit = new CEdit;
					pStatic = new CStatic;
					rcStatic = CRect(xCur, rcCtrl.top, xCur + szOrd.cx, rcCtrl.bottom);
					pStatic->Create("Z:", WS_VISIBLE, rcStatic, this, 0);
					xCur += szOrd.cx + 5;
					rcVec = CRect(xCur, rcCtrl.top, xCur + xWidth, rcCtrl.bottom);
					pEdit->Create(dwStyle, rcVec, this, dwID);
					sprintf(sTmp, VECTOR_PRECISION, pNode->m_Data.m_data.m_fVec[2]);
					pEdit->SetWindowText(sTmp);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pStatic);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pEdit);
					xCur += xWidth + xSpacer;

					// Create the helper button

					CVectorButton *pButton = new CVectorButton(dwID);
					pButton->Create("Edit Vector", WS_VISIBLE, CRect(xCur, rcCtrl.top - 2, rcCtrl.right, rcCtrl.bottom), this, 0);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pButton);

//					CVectorCombo *pCombo = new CVectorCombo(dwID);
//					pCombo->Create(WS_VISIBLE | CBS_DROPDOWNLIST, CRect(xCur, rcCtrl.top - 2, rcCtrl.right, rcCtrl.bottom + 200), this, 0);
//					pFxDlgProp->m_collWnds.AddTail((CWnd *)pCombo);

					ReleaseDC(pDC);
				}
				break;

				case FX_PROP::PATH :
				{
					char sTmp[256];
					strcpy(sTmp, pNode->m_Data.m_data.m_sVal);
					// Get the extension

					char *sExt = strtok(sTmp, "|");
					char *sVal = strtok(NULL, "|");
					
					// Create a button....

					CRezButton *pRezButton = new CRezButton(sExt, sVal);
					CRect rcButton = rcCtrl;
					rcButton.top	-= 2;
					rcButton.bottom += 2;
					pRezButton->Create(sVal, dwStyle & ~WS_BORDER, rcButton, this, dwID);
					pFxDlgProp->m_collWnds.AddTail((CWnd *)pRezButton);
				}
				break;
			}
			
			// Add this property to the list of properties

			m_collDlgProps.AddTail(pFxDlgProp);
			
			// Check to see if this property is off the page...

			CWnd *pPropWnd = GetDlgItem(IDC_PROPERTIESRECT);
			CRect rcPropRect;

			pPropWnd->GetClientRect(&rcPropRect);
			pPropWnd->ClientToScreen(&rcPropRect);
			ScreenToClient(&rcPropRect);

			if( yCur >= rcPropRect.Height() )
			{
				// We're gonna need the scroll bar

				SCROLLINFO	ScrollInfo;
				CScrollBar	*pScrollBar = (CScrollBar*)GetDlgItem(IDC_PROPSCROLL);

				ScrollInfo.cbSize	= sizeof( SCROLLINFO );
				ScrollInfo.fMask	= SIF_ALL | SIF_DISABLENOSCROLL;
				ScrollInfo.nMax		= m_collDlgProps.GetSize() - 1;
				ScrollInfo.nMin		= 0;
				ScrollInfo.nPage	= rcPropRect.Height() / 25;
				ScrollInfo.nPos		= 0;

				pScrollBar->SetScrollInfo( &ScrollInfo );
				pScrollBar->EnableScrollBar( ESB_ENABLE_BOTH );
				pScrollBar->EnableScrollBar( ESB_DISABLE_LTUP );

				CWnd *pWnd = pFxDlgProp->m_pStatic;

				// Hide the property (the static label and all child windows)...
				
				pWnd->ShowWindow( false );

				CLinkListNode<CWnd*> *pNode = pFxDlgProp->m_collWnds.GetHead();
				while( pNode )
				{
					pWnd = pNode->m_Data;
					pWnd->ShowWindow( false );

					pNode = pNode->m_pNext;
				}
			
			}

			pNode = pNode->m_pNext;

			yCur += 25;
			dwID ++;
		}

		ReleaseDC(pDC);

		InvalidateRect(NULL, TRUE);
	}
	else
	{
		// We are unloading the dialog

		DWORD dwID = FXDLG_FIRSTID;

		CLinkListNode<FX_DLGPROP *> *pNode = m_collDlgProps.GetHead();
		CFastListNode<FX_PROP> *pFxPropNode = m_pCollProps->GetHead();

		while (pNode)
		{
			FX_DLGPROP *pFxDlgProp = pNode->m_Data;

			switch (pNode->m_Data->m_fxProp.m_nType)
			{
				case FX_PROP::INTEGER :
				{
					CEdit *pEdit = (CEdit *)GetDlgItem(dwID);

					CString sInt;
					pEdit->GetWindowText(sInt);

					int val = atoi(sInt);

					// Set the value

					pFxPropNode->m_Data.m_data.m_nVal = val;
				}
				break;

				case FX_PROP::FLOAT :
				{
					CEdit *pEdit = (CEdit *)GetDlgItem(dwID);

					CString sInt;
					pEdit->GetWindowText(sInt);

					float val = (float)atof(sInt);

					// Set the value

					pFxPropNode->m_Data.m_data.m_fVal = val;
				}
				break;

				case FX_PROP::STRING :
				{
					CEdit *pEdit = (CEdit *)GetDlgItem(dwID);

					CString sInt;
					pEdit->GetWindowText(sInt);

					// Set the value

					strcpy(pFxPropNode->m_Data.m_data.m_sVal, (char *)(LPCSTR)sInt);
				}
				break;

				case FX_PROP::COMBO :
				{
					CString sTxt;
					CComboBox *pCombo = (CComboBox *)GetDlgItem(dwID);

					int nSel = pCombo->GetCurSel();
					if (nSel == CB_ERR) nSel = 0;

					// Retrieve the text items and build the string

					sTxt.Format("%d,", nSel);

					for (int i = 0; i < pCombo->GetCount(); i ++)
					{
						CString sLBText;

						pCombo->GetLBText(i, sLBText);

						sTxt += sLBText;
						if (i != pCombo->GetCount() - 1) sTxt += ",";
					}

					// Set the value

					strcpy(pFxPropNode->m_Data.m_data.m_sVal, (char *)(LPCSTR)sTxt);
				}
				break;

				case FX_PROP::VECTOR :
				{
					float val;
					CEdit *pEdit;
					CString sInt;

					// Read in the vectors

					pEdit = (CEdit *)GetDlgItem(dwID);					
					pEdit->GetWindowText(sInt);
					val = (float)atof(sInt);
					pFxPropNode->m_Data.m_data.m_fVec[0] = val;
					dwID ++;

					pEdit = (CEdit *)GetDlgItem(dwID);					
					pEdit->GetWindowText(sInt);
					val = (float)atof(sInt);
					pFxPropNode->m_Data.m_data.m_fVec[1] = val;
					dwID ++;

					pEdit = (CEdit *)GetDlgItem(dwID);					
					pEdit->GetWindowText(sInt);
					val = (float)atof(sInt);
					pFxPropNode->m_Data.m_data.m_fVec[2] = val;
				}
				break;

				case FX_PROP::PATH :
				{
					CString sExt = strtok(pFxPropNode->m_Data.m_data.m_sVal, "|");
					CRezButton *pRezButton = (CRezButton *)GetDlgItem(dwID);

					CString sTxt;
					pRezButton->GetWindowText(sTxt);

					sExt += "|";
					sExt += sTxt;

					strcpy(pFxPropNode->m_Data.m_data.m_sVal, sExt);
				}
				break;
			}

			dwID ++;

			pNode = pNode->m_pNext;
			pFxPropNode = pFxPropNode->m_pNext;
		}
	}
}


BEGIN_MESSAGE_MAP(CFxPropDlg, CDialog)
	//{{AFX_MSG_MAP(CFxPropDlg)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFxPropDlg message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnInitDialog()
//
//   PURPOSE  : Handles WM_INITDIALOG
//
//------------------------------------------------------------------

BOOL CFxPropDlg::OnInitDialog() 
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}

	CDialog::OnInitDialog();
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//------------------------------------------------------------------
//
//   FUNCTION : OnCommand()
//
//   PURPOSE  : Handles WM_COMMAND
//
//------------------------------------------------------------------

BOOL CFxPropDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD wNotify = HIWORD(wParam);
	WORD wID     = LOWORD(lParam);

	switch (wNotify)
	{
		case BN_CLICKED :
		{
			CButton *pButton = (CButton *)GetDlgItem(lParam);
		}
		break;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

//------------------------------------------------------------------
//
//   FUNCTION : DestroyWindow()
//
//   PURPOSE  : Called when window is destroyed
//
//------------------------------------------------------------------

BOOL CFxPropDlg::DestroyWindow() 
{
	CLinkListNode<FX_DLGPROP *> *pNode = m_collDlgProps.GetHead();
	
	// Nuke all the windows

	pNode = m_collDlgProps.GetHead();

	while (pNode)
	{
		if (pNode->m_Data->m_pStatic) delete pNode->m_Data->m_pStatic;
		if (pNode->m_Data->m_pWnd) delete pNode->m_Data->m_pWnd;
		
		CLinkListNode<CWnd *> *pWndNode = pNode->m_Data->m_collWnds.GetHead();

		while (pWndNode)
		{
			delete pWndNode->m_Data;

			pWndNode = pWndNode->m_pNext;
		}

		pNode->m_Data->m_collWnds.RemoveAll();

		delete pNode->m_Data;
		
		pNode = pNode->m_pNext;
	}

	m_collDlgProps.RemoveAll();
	
	return CDialog::DestroyWindow();
}

void CFxPropDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default

	CLinkListNode<FX_DLGPROP*>	*pFxDlgProp = m_collDlgProps.GetHead();
	CWnd	*pWnd;
	CRect	rcProp, rcStatic;
	int		nShow;

	GetDlgItem( IDC_PROPERTIESRECT )->GetClientRect( &rcStatic );
	GetDlgItem( IDC_PROPERTIESRECT )->ClientToScreen( &rcStatic );
	ScreenToClient( &rcStatic );

	int nChange = 0;
	switch( nSBCode )
	{
        case SB_PAGEUP:
			nChange = 25;
            break; 

        case SB_PAGEDOWN: 
            nChange = -25; 
            break; 

        case SB_LINEUP: 
            nChange = 25; 
            break; 

        case SB_LINEDOWN: 
            nChange = -25; 
            break; 

        case SB_THUMBTRACK: 
            nChange = (nPos - m_nCurScrollPos) * -25; 
            break; 

        default: 
            CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
			return;
	}

	m_nCurScrollPos += nChange / -25;

	SCROLLINFO	ScrollInfo;
	ScrollInfo.cbSize	= sizeof( SCROLLINFO );
	ScrollInfo.fMask	= SIF_ALL | SIF_DISABLENOSCROLL;
	ScrollInfo.nMin		= 0;
	ScrollInfo.nMax		= m_collDlgProps.GetSize() - 1;
	ScrollInfo.nPage	= rcStatic.Height() / 25;
	ScrollInfo.nPos		= m_nCurScrollPos;

	pScrollBar->SetScrollInfo( &ScrollInfo );

	if( m_nCurScrollPos <= 0 )
	{
		pScrollBar->EnableScrollBar( ESB_DISABLE_LTUP );
	}
	else if( m_nCurScrollPos >= (int)(m_collDlgProps.GetSize() - ScrollInfo.nPage) )
	{
		pScrollBar->EnableScrollBar( ESB_DISABLE_RTDN );
	}
	
	while( pFxDlgProp )
	{
		nShow = SW_SHOW;

		// Move the static label...

		pWnd = pFxDlgProp->m_Data->m_pStatic;
		
		pWnd->GetClientRect( &rcProp );
		pWnd->ClientToScreen(&rcProp);
		ScreenToClient(&rcProp);

		if( (rcProp.top + nChange < rcStatic.top) || (rcProp.bottom + nChange > rcStatic.bottom) )
			nShow = SW_HIDE;

		pWnd->ShowWindow( nShow );
		pWnd->MoveWindow( rcProp.left, rcProp.top + nChange, rcProp.Width(), rcProp.Height(), FALSE );
		
		CLinkListNode<CWnd*> *pNode = pFxDlgProp->m_Data->m_collWnds.GetHead();
		while( pNode )
		{
			pWnd = pNode->m_Data;
			
			pWnd->GetClientRect( &rcProp );
			pWnd->ClientToScreen(&rcProp);
			ScreenToClient(&rcProp);

			pWnd->ShowWindow( nShow );
			pWnd->MoveWindow( rcProp.left, rcProp.top + nChange, rcProp.Width(), rcProp.Height(), FALSE );

			pNode = pNode->m_pNext;
		}

		pFxDlgProp = pFxDlgProp->m_pNext;
	}

	pFxDlgProp = m_collDlgProps.GetHead();
	while( pFxDlgProp )
	{
		pWnd = pFxDlgProp->m_Data->m_pStatic;
		pWnd->Invalidate();
		
		CLinkListNode<CWnd*> *pNode = pFxDlgProp->m_Data->m_collWnds.GetHead();
		while( pNode )
		{
			pWnd = pNode->m_Data;
			pWnd->Invalidate();

			pNode = pNode->m_pNext;
		}

		pFxDlgProp = pFxDlgProp->m_pNext;
	}
	Invalidate();

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}
