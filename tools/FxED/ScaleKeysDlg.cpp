// ScaleKeysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "ScaleKeysDlg.h"
#include "StringDlg.h"
#include "ScaleAnimDlg.h"
#include "FloatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScaleKeysDlg dialog


CScaleKeysDlg::CScaleKeysDlg(CKey *pKey, CWnd* pParent /*=NULL*/)
	: CDialog(CScaleKeysDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScaleKeysDlg)
	//}}AFX_DATA_INIT

	m_pKey = pKey;
}

//------------------------------------------------------------------
//
//   FUNCTION : DoDataExchange()
//
//   PURPOSE  : Dialog data exchange mechanism
//
//------------------------------------------------------------------

void CScaleKeysDlg::DoDataExchange(CDataExchange* pDX)
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScaleKeysDlg)
	DDX_Control(pDX, IDC_MINSCALE, m_minScale);
	DDX_Control(pDX, IDC_MAXSCALE, m_maxScale);
	DDX_Control(pDX, IDC_SCALEKEYS, m_scaleKeyCtrl);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		// We are loading the dialog

		if (!pApp->GetScaleFavourites()->GetSize()) GetDlgItem(IDC_CHOOSEFAVOURITE)->EnableWindow(FALSE);

		m_scaleKeyCtrl.SetKey(m_pKey);
		
		CLinkListNode<SCALEKEY> *pNode = m_pKey->GetScaleKeys()->GetHead();

		m_minScale.SetValue(m_pKey->GetMinScalePtr());
		m_maxScale.SetValue(m_pKey->GetMaxScalePtr());

		float minScale = m_pKey->GetMinScale();
		float maxScale = m_pKey->GetMaxScale();
		float scaleDist = maxScale - minScale;
		m_fLastScale = maxScale;
		
		while (pNode)
		{
			KEY k;
			k.m_tmAnchor  = pNode->m_Data.m_tmKey;
			k.m_valAnchor = (pNode->m_Data.m_scale - minScale) / scaleDist;
	
			m_scaleKeyCtrl.GetKeys()->AddTail(k);

			pNode = pNode->m_pNext;
		}		
	}
	else
	{
		// We are unloading the dialog

		char sTmp[256];

		m_minScale.GetWindowText(sTmp, 256);
		m_pKey->SetMinScale((float)atof(sTmp));

		m_maxScale.GetWindowText(sTmp, 256);
		m_pKey->SetMaxScale((float)atof(sTmp));

		CLinkListNode<KEY> *pNode = m_scaleKeyCtrl.GetKeys()->GetHead();

		// Delete all the existing keys

		m_pKey->GetScaleKeys()->RemoveAll();

		float minScale = m_pKey->GetMinScale();
		float maxScale = m_pKey->GetMaxScale();
		float scaleDist = maxScale - minScale;

		while (pNode)
		{
			SCALEKEY s;
			s.m_tmKey = pNode->m_Data.m_tmAnchor;
			s.m_scale = minScale + (pNode->m_Data.m_valAnchor * scaleDist);

			m_pKey->GetScaleKeys()->AddTail(s);
			
			pNode = pNode->m_pNext;
		}
	}
}


BEGIN_MESSAGE_MAP(CScaleKeysDlg, CDialog)
	//{{AFX_MSG_MAP(CScaleKeysDlg)
	ON_BN_CLICKED(IDC_ADDTOFAVOURITES, OnAddToFavourites)
	ON_BN_CLICKED(IDC_CHOOSEFAVOURITE, OnChooseFavourite)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_SETALLSCALE, OnSetAllScale)
//	ON_MESSAGE(WM_VALUEUPDATE, OnValueUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScaleKeysDlg message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnAddToFavourites()
//
//   PURPOSE  : Adds to the scale favourites list
//
//------------------------------------------------------------------

void CScaleKeysDlg::OnAddToFavourites() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	CStringDlg dlg("Choose Name");

	if (dlg.DoModal() == IDOK)
	{
		SK_FAVOURITE *pNewFavourite = new SK_FAVOURITE;

		pNewFavourite->m_sName = dlg.m_sText;

		// Copy the current keys

		CLinkListNode<KEY> *pNode = m_scaleKeyCtrl.GetKeys()->GetHead();

		float minScale = m_pKey->GetMinScale();
		float maxScale = m_pKey->GetMaxScale();
		float scaleDist = maxScale - minScale;

		pNewFavourite->m_minScale = minScale;
		pNewFavourite->m_maxScale = maxScale;

		while (pNode)
		{
			SCALEKEY s;
			s.m_tmKey = pNode->m_Data.m_tmAnchor;
			s.m_scale = minScale + (pNode->m_Data.m_valAnchor * scaleDist);

			pNewFavourite->m_collKeys.AddTail(s);
			
			pNode = pNode->m_pNext;
		}

		// Add it

		pApp->GetScaleFavourites()->AddTail(pNewFavourite);
	}	

	if (pApp->GetScaleFavourites()->GetSize())
	{
		GetDlgItem(IDC_CHOOSEFAVOURITE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CHOOSEFAVOURITE)->EnableWindow(FALSE);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnChooseFavourite()
//
//   PURPOSE  : Chooses a favourite scale animation
//
//------------------------------------------------------------------

void CScaleKeysDlg::OnChooseFavourite() 
{
	CScaleAnimDlg dlg;

	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_pFavourite)
		{
			m_pKey->GetScaleKeys()->RemoveAll();

			CLinkListNode<SCALEKEY> *pNode = dlg.m_pFavourite->m_collKeys.GetHead();

			while (pNode)
			{
				m_pKey->GetScaleKeys()->AddTail(pNode->m_Data);

				pNode = pNode->m_pNext;
			}

			m_pKey->SetMinScale(dlg.m_pFavourite->m_minScale);
			m_pKey->SetMaxScale(dlg.m_pFavourite->m_maxScale);

			Refresh();
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : Refresh()
//
//   PURPOSE  : Refreshes the dialog
//
//------------------------------------------------------------------

void CScaleKeysDlg::Refresh()
{
	char sTmp[256];

	sprintf(sTmp, FLOAT_PRECISION, m_pKey->GetMinScale());
	m_minScale.SetWindowText(sTmp);

	sprintf(sTmp, FLOAT_PRECISION, m_pKey->GetMaxScale());
	m_maxScale.SetWindowText(sTmp);

	m_scaleKeyCtrl.GetKeys()->RemoveAll();
	
	float minScale = m_pKey->GetMinScale();
	float maxScale = m_pKey->GetMaxScale();
	float scaleDist = maxScale - minScale;
	
	CLinkListNode<SCALEKEY> *pNode = m_pKey->GetScaleKeys()->GetHead();
		
	while (pNode)
	{
		KEY k;
		k.m_tmAnchor  = pNode->m_Data.m_tmKey;
		k.m_valAnchor = (pNode->m_Data.m_scale - minScale) / scaleDist;

		m_scaleKeyCtrl.GetKeys()->AddTail(k);

		pNode = pNode->m_pNext;
	}

	m_scaleKeyCtrl.Invalidate();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnReset()
//
//   PURPOSE  : Resets the scale dialog
//
//------------------------------------------------------------------

void CScaleKeysDlg::OnReset() 
{
	int ret = AfxMessageBox("Are you sure you want to reset ?", MB_ICONEXCLAMATION | MB_YESNO);

	if (ret == IDYES)
	{
		m_scaleKeyCtrl.GetKeys()->RemoveAll();
		m_pKey->GetScaleKeys()->RemoveAll();

		SCALEKEY s;

		s.m_tmKey = 0.0f;
		s.m_scale = m_pKey->GetMaxScale() / 10.0f;
		m_pKey->GetScaleKeys()->AddTail(s);

		s.m_tmKey = 1.0f;
		s.m_scale = m_pKey->GetMaxScale() / 10.0f;
		m_pKey->GetScaleKeys()->AddTail(s);
		
		float minScale = m_pKey->GetMinScale();
		float maxScale = m_pKey->GetMaxScale();
		float scaleDist = maxScale - minScale;
		
		CLinkListNode<SCALEKEY> *pNode = m_pKey->GetScaleKeys()->GetHead();
			
		while (pNode)
		{
			KEY k;
			k.m_tmAnchor  = pNode->m_Data.m_tmKey;
			k.m_valAnchor = (pNode->m_Data.m_scale - minScale) / scaleDist;

			m_scaleKeyCtrl.GetKeys()->AddTail(k);

			pNode = pNode->m_pNext;
		}

		m_scaleKeyCtrl.Invalidate();
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSetAllScale()
//
//   PURPOSE  : Sets all keys to one value
//
//------------------------------------------------------------------

void CScaleKeysDlg::OnSetAllScale() 
{
	CFloatDlg dlg(0.0f, "Choose single scale");

	if (dlg.DoModal() == IDOK)
	{
		CLinkListNode<KEY> *pNode = m_scaleKeyCtrl.GetKeys()->GetHead();

		float maxScale = m_pKey->GetMaxScale();
		float minScale = m_pKey->GetMinScale();

		float fNewScale = dlg.m_float;
		if (fNewScale > maxScale) fNewScale = maxScale;
		if (fNewScale < minScale) fNewScale = minScale;
		
		if (fNewScale != 0.0f)
		{
			fNewScale = fNewScale / (maxScale - minScale);
		}
			
		while (pNode)
		{
			pNode->m_Data.m_valAnchor = fNewScale;
			
			pNode = pNode->m_pNext;
		}

		m_scaleKeyCtrl.Invalidate();
	}
}

void CScaleKeysDlg::OnValueUpdate()
{
	// Scale all the keys

	float fMul = m_fLastScale / m_pKey->GetMaxScale();
	m_fLastScale = m_pKey->GetMaxScale();

	CLinkListNode<KEY> *pNode = m_scaleKeyCtrl.GetKeys()->GetHead();
		
	while (pNode)
	{
		pNode->m_Data.m_valAnchor *= fMul;

		pNode = pNode->m_pNext;
	}

	m_scaleKeyCtrl.Invalidate();
}

LRESULT CScaleKeysDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_VALUEUPDATE)
	{
		OnValueUpdate();
	}
	
	return CDialog::DefWindowProc(message, wParam, lParam);
}
