// SpellResolutionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpellEd.h"
#include "SpellResolutionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellResolution dialog


CSpellResolutionDlg::CSpellResolutionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpellResolutionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpellResolutionDlg)
	//}}AFX_DATA_INIT
}


void CSpellResolutionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellResolutionDlg)
	DDX_Control(pDX, IDC_RATE, m_rate);
	DDX_Control(pDX, IDC_TYPE, m_type);
	DDX_Control(pDX, IDC_INITIALDELAY, m_initialDelay);
	DDX_Control(pDX, IDC_APPLY, m_apply);
	DDX_Control(pDX, IDC_ACTIVATIONPHASE, m_activationPhase);
	DDX_Control(pDX, IDC_EFFECTS, m_effects);
	DDX_Control(pDX, IDC_REMOVE, m_remove);
	DDX_Control(pDX, IDC_ADD, m_add);
	DDX_Control(pDX, IDC_ALLEFFECTS, m_allEffects);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		// Load the dialog

		m_allEffects.AddString("Alter Attributes");

		// Load up the effects....
/*
		CLinkListNode<CBaseEffect *> *pNode = m_pSpell->GetEffects()->GetHead();

		while (pNode)
		{
			CBaseEffect *pEffect = pNode->m_Data;

			switch (pEffect->m_nEffectType)
			{
				case BET_ALTERATTRIBUTES :
				{
					int nIndex = m_effects.AddString("Alter Attributes");
					m_effects.SetItemData(nIndex, (DWORD)pEffect);
				}
				break;
			}

			pNode = pNode->m_pNext;
		}
*/
		// Create the properties dialogs

//		m_alterAttributesDlg.Create(IDD_EF_ALTERATTRIBUTE, this);

		MoveDialogs();

		ActivateProperties(FALSE);
	}
}


BEGIN_MESSAGE_MAP(CSpellResolutionDlg, CDialog)
	//{{AFX_MSG_MAP(CSpellResolutionDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_LBN_SELCHANGE(IDC_EFFECTS, OnSelchangeEffects)
	ON_LBN_KILLFOCUS(IDC_EFFECTS, OnKillfocusEffects)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_APPLY, OnSelchangeApply)
	ON_CBN_SELCHANGE(IDC_ACTIVATIONPHASE, OnSelchangeActivationphase)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellResolution message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnAdd()
//
//   PURPOSE  : Adds a spell effect
//
//------------------------------------------------------------------

void CSpellResolutionDlg::OnAdd() 
{
	int nCurSel = m_allEffects.GetCurSel();
	if (nCurSel == LB_ERR) return;

	CString sTxt;
	m_allEffects.GetText(nCurSel, sTxt);
	int nIndex = m_effects.AddString(sTxt);

	// Create an attribute class
/*
	if (sTxt == "Alter Attributes")
	{
		CAlterAttributes *pAlterAttributes = new CAlterAttributes;
		m_effects.SetItemData(nIndex, (DWORD)pAlterAttributes);

		// Add it to the list of effects....

		m_pSpell->GetEffects()->AddTail(pAlterAttributes);
	}
*/
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRemove()
//
//   PURPOSE  : Removes a spell effect
//
//------------------------------------------------------------------

void CSpellResolutionDlg::OnRemove() 
{
	int nCurSel = m_effects.GetCurSel();
	if (nCurSel == LB_ERR) return;

//	CBaseEffect *pEffect = (CBaseEffect *)m_effects.GetItemData(nCurSel);
//	m_pSpell->GetEffects()->Remove(pEffect);
//	delete pEffect;

	m_effects.DeleteString(nCurSel);
	m_effects.SetCurSel(nCurSel);

	ActivateProperties(FALSE);
}

//------------------------------------------------------------------
//
//   FUNCTION : ActivateProperties()
//
//   PURPOSE  : Activates the properties portion of the dialog
//
//------------------------------------------------------------------

void CSpellResolutionDlg::ActivateProperties(BOOL bActivate)
{
	GetDlgItem(IDC_STATIC1)->EnableWindow(bActivate);
	GetDlgItem(IDC_STATIC2)->EnableWindow(bActivate);
	GetDlgItem(IDC_STATIC3)->EnableWindow(bActivate);
	GetDlgItem(IDC_STATIC4)->EnableWindow(bActivate);
	GetDlgItem(IDC_STATIC5)->EnableWindow(bActivate);
	GetDlgItem(IDC_STATIC6)->EnableWindow(bActivate);
	GetDlgItem(IDC_STATIC7)->EnableWindow(bActivate);
	GetDlgItem(IDC_STATIC8)->EnableWindow(bActivate);

	m_activationPhase.EnableWindow(bActivate);
	m_rate.EnableWindow(bActivate);
	m_initialDelay.EnableWindow(bActivate);
	m_apply.EnableWindow(bActivate);
	m_type.EnableWindow(bActivate);	

	if (bActivate)
	{
		// Retrieve the current selection

		int nCurSel = m_effects.GetCurSel();
		if (nCurSel == LB_ERR) return;

		CString sTxt;
		m_effects.GetText(nCurSel, sTxt);

		ActivatePropertiesDlg(sTxt);
	}
	else
	{
//		m_alterAttributesDlg.ShowWindow(SW_HIDE);
//		m_alterAttributesDlg.EnableWindow(FALSE);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelchangeEffects()
//
//   PURPOSE  : Selects a new effect
//
//------------------------------------------------------------------

void CSpellResolutionDlg::OnSelchangeEffects() 
{
	int nCurSel = m_effects.GetCurSel();
	if (nCurSel == LB_ERR) return;

	ActivateProperties(TRUE);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnKillfocusEffects()
//
//   PURPOSE  : Handles WM_KILLFOCUS for effects list box
//
//------------------------------------------------------------------

void CSpellResolutionDlg::OnKillfocusEffects() 
{

}

//------------------------------------------------------------------
//
//   FUNCTION : ActivatePropertiesDlg()
//
//   PURPOSE  : Activates a specific properties dialog
//
//------------------------------------------------------------------

void CSpellResolutionDlg::ActivatePropertiesDlg(CString sEffect)
{
/*
	int nCurSel = m_effects.GetCurSel();
	if (nCurSel == LB_ERR) return;

	CBaseEffect *pEffect = (CAlterAttributes *)m_effects.GetItemData(nCurSel);

	m_activationPhase.SetCurSel(pEffect->m_nActivationPhase);
	m_apply.SetCurSel(pEffect->m_nType);
	OnSelchangeApply();
	m_rate.SetWindowText(pEffect->m_sRate);
	m_initialDelay.SetWindowText(pEffect->m_sInitialDelay);

	m_alterAttributesDlg.ShowWindow(SW_HIDE);
	m_alterAttributesDlg.EnableWindow(FALSE);

	if (sEffect == "Alter Attributes")
	{		
		m_alterAttributesDlg.SetPtr((CAlterAttributes *)m_effects.GetItemData(nCurSel));
		m_alterAttributesDlg.ShowWindow(SW_SHOW);
		m_alterAttributesDlg.EnableWindow(TRUE);
	}
	else
	{
	}
*/
}

//------------------------------------------------------------------
//
//   FUNCTION : MoveDialogs()
//
//   PURPOSE  : Moves the dialogs to their correct positions
//
//------------------------------------------------------------------

void CSpellResolutionDlg::MoveDialogs()
{
	CRect rcWnd;
	
	CWnd *pWnd = GetDlgItem(IDC_PROPRECT);
	pWnd->GetWindowRect(&rcWnd);
	ScreenToClient(&rcWnd);

//	m_alterAttributesDlg.MoveWindow(rcWnd.left, rcWnd.top, rcWnd.Width(), rcWnd.Height());
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSize()
//
//   PURPOSE  : Handles WM_SIZE
//
//------------------------------------------------------------------

void CSpellResolutionDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);	
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelchangeApply()
//
//   PURPOSE  : Handles selection change for apply combo box
//
//------------------------------------------------------------------

void CSpellResolutionDlg::OnSelchangeApply() 
{
	int nCurSel = m_apply.GetCurSel();
	if (nCurSel == CB_ERR) return;

	if (nCurSel == 0)
	{
		m_rate.EnableWindow(FALSE);
		m_type.EnableWindow(FALSE);
	}
	else
	{
		m_rate.EnableWindow(TRUE);
		m_type.EnableWindow(TRUE);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelchangeActivationphase()
//
//   PURPOSE  : Handles changing activation phase
//
//------------------------------------------------------------------

void CSpellResolutionDlg::OnSelchangeActivationphase() 
{
	int nCurSel = m_effects.GetCurSel();
	if (nCurSel == LB_ERR) return;

//	CBaseEffect *pEffect = (CBaseEffect *)m_effects.GetItemData(nCurSel);
//	if (!pEffect) return;

//	pEffect->m_nActivationPhase = m_activationPhase.GetCurSel();
}
