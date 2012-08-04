// SpellBaseParmDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpellEd.h"
#include "SpellBaseParmDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellBaseParmDlg dialog


CSpellBaseParmDlg::CSpellBaseParmDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpellBaseParmDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpellBaseParmDlg)
	//}}AFX_DATA_INIT
}


void CSpellBaseParmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellBaseParmDlg)
	DDX_Control(pDX, IDC_COST, m_cost);
	DDX_Control(pDX, IDC_DESCRIPTION, m_desc);
	DDX_Control(pDX, IDC_CASTHOWOFTEN, m_castHowOften);
	DDX_Control(pDX, IDC_CASTPTTYPE, m_castPtType);
	DDX_Control(pDX, IDC_TARGET, m_targetType);
	DDX_Control(pDX, IDC_TYPE, m_type);
	DDX_Control(pDX, IDC_RADIUS, m_radius);
	DDX_Control(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		// We are loading the dialog

		m_name.SetWindowText(m_pSpell->GetName());
		m_name.EnableWindow(FALSE);

		m_desc.SetWindowText(m_pSpell->GetDesc());
		m_radius.SetWindowText(m_pSpell->GetRadius());
		m_cost.SetWindowText(m_pSpell->GetCastCost());
		
		CComboBox *pBox;

		pBox = (CComboBox *)GetDlgItem(IDC_TOTEMS);
		pBox->SetCurSel(0);

		pBox = (CComboBox *)GetDlgItem(IDC_LEVELS);
		pBox->SetCurSel(0);

		pBox = (CComboBox *)GetDlgItem(IDC_TYPE);
		pBox->SetCurSel(m_pSpell->GetType());

		pBox = (CComboBox *)GetDlgItem(IDC_TARGET);
		pBox->SetCurSel(m_pSpell->GetTargetType());

		pBox = (CComboBox *)GetDlgItem(IDC_CASTPTTYPE);
		pBox->SetCurSel(m_pSpell->GetCastPtType());

		pBox = (CComboBox *)GetDlgItem(IDC_CASTHOWOFTEN);
		pBox->SetCurSel(m_pSpell->GetHowOften());

		pBox = (CComboBox *)GetDlgItem(IDC_ANIMSPEED);
		pBox->SetCurSel(m_pSpell->GetCastSpeed());

		CListBox *pList;
		pList = (CListBox *)GetDlgItem(IDC_REQUIRESTOTEM);

		CLinkListNode<TOTEM_REQUIREMENT> *pNode = m_pSpell->GetTotemRequirements()->GetHead();

		while (pNode)
		{
			pList->AddString(pNode->m_Data.m_sRequirement);

			pNode = pNode->m_pNext;
		}
	}
}


BEGIN_MESSAGE_MAP(CSpellBaseParmDlg, CDialog)
	//{{AFX_MSG_MAP(CSpellBaseParmDlg)
	ON_BN_CLICKED(IDC_ADDTOTEM, OnAddTotem)
	ON_BN_CLICKED(IDC_REMOVETOTEM, OnRemoveTotem)
	ON_CBN_SELCHANGE(IDC_CASTHOWOFTEN, OnSelChangeCastHowOften)
	ON_CBN_SELCHANGE(IDC_CASTPTTYPE, OnSelChangeCastPtType)
	ON_CBN_SELCHANGE(IDC_TARGET, OnSelChangeTargetType)
	ON_CBN_SELCHANGE(IDC_TYPE, OnSelChangeSpellType)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_DESCRIPTION, OnChangeDescription)
	ON_CBN_SELCHANGE(IDC_ANIMSPEED, OnSelchangeAnimspeed)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellBaseParmDlg message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnAddTotem()
//
//   PURPOSE  : Adds a totemic requirement
//
//------------------------------------------------------------------

void CSpellBaseParmDlg::OnAddTotem() 
{
	CString sTotem;
	CString sLevel;	
	CString sRequires;
	CComboBox *pBox;
	CListBox *pList;

	pBox   = (CComboBox *)GetDlgItem(IDC_TOTEMS);
	pBox->GetWindowText(sTotem);

	pBox   = (CComboBox *)GetDlgItem(IDC_LEVELS);
	pBox->GetWindowText(sLevel);

	pList = (CListBox *)GetDlgItem(IDC_REQUIRESTOTEM);

	if (sLevel == "Any Level")
	{
		sRequires = sTotem;
		sRequires += " of any level";
	}
	else
	{
		sRequires = sTotem;
		sRequires += " level ";
		sRequires += sLevel;
	}

	// Check before we add this totem

	for (int i = 0; i < pList->GetCount(); i ++)
	{
		CString sListString;
		pList->GetText(i, sListString);

		CString sString = sListString;

		BOOL bOkayToAdd = TRUE;
		
		if (sRequires == sString)
		{
			AfxMessageBox("Totem level requirement already exists !!!", MB_ICONEXCLAMATION);
			return;
		}

		CString sCheckTotem = strtok((char *)(LPCSTR)sString, " ");

		if (sCheckTotem == sTotem)
		{
			AfxMessageBox("Totem level requirement already exists !!!", MB_ICONEXCLAMATION);
			return;
		}
	}

	pList->AddString(sRequires);
	
	TOTEM_REQUIREMENT tr;
	tr.m_sRequirement = sRequires;

	m_pSpell->GetTotemRequirements()->AddTail(tr);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRemoveTotem()
//
//   PURPOSE  : Removes a totemic requirement
//
//------------------------------------------------------------------

void CSpellBaseParmDlg::OnRemoveTotem() 
{
	CListBox *pList;
	pList = (CListBox *)GetDlgItem(IDC_REQUIRESTOTEM);

	int nCurSel = pList->GetCurSel();
	if (nCurSel != LB_ERR)

	pList->DeleteString(nCurSel);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelChangeCastHowOften()
//
//   PURPOSE  : Sets up casting requirement
//
//------------------------------------------------------------------

void CSpellBaseParmDlg::OnSelChangeCastHowOften() 
{
	m_pSpell->SetHowOften(m_castHowOften.GetCurSel());
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelChangeCastPtType()
//
//   PURPOSE  : Changes what attribute is drained when spell is cast
//
//------------------------------------------------------------------

void CSpellBaseParmDlg::OnSelChangeCastPtType() 
{
	m_pSpell->SetCastPtType(m_castPtType.GetCurSel());
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelChangeTargetType()
//
//   PURPOSE  : Changes the types of targets for this spell
//
//------------------------------------------------------------------

void CSpellBaseParmDlg::OnSelChangeTargetType() 
{
	m_pSpell->SetTargetType(m_targetType.GetCurSel());
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDestroy()
//
//   PURPOSE  : Handles WM_DESTROY
//
//------------------------------------------------------------------

void CSpellBaseParmDlg::OnDestroy() 
{
	// Retrieve all the pertinent information right here !!!

	CString sTxt;
	m_desc.GetWindowText(sTxt);
	m_pSpell->SetDesc(sTxt.GetBuffer(sTxt.GetLength()));
	
	m_cost.GetWindowText(sTxt);
	m_pSpell->SetCastCost(sTxt);

	m_radius.GetWindowText(sTxt);
	m_pSpell->SetRadius(sTxt);	

	CDialog::OnDestroy();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnChangeDescription()
//
//   PURPOSE  : Updates spell description
//
//------------------------------------------------------------------

void CSpellBaseParmDlg::OnChangeDescription() 
{
	CString sDesc;
	m_desc.GetWindowText(sDesc);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelchangeAnimspeed()
//
//   PURPOSE  : Changes the animation length
//
//------------------------------------------------------------------

void CSpellBaseParmDlg::OnSelchangeAnimspeed() 
{
	CComboBox *pBox = (CComboBox *)GetDlgItem(IDC_ANIMSPEED);
	int nCurSel = pBox->GetCurSel();
	if (nCurSel == CB_ERR) return;

	m_pSpell->SetCastSpeed(nCurSel);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelChangeSpellType()
//
//   PURPOSE  : Changes the type of spell
//
//------------------------------------------------------------------

void CSpellBaseParmDlg::OnSelChangeSpellType() 
{
	CComboBox *pBox = (CComboBox *)GetDlgItem(IDC_TYPE);
	int nCurSel = pBox->GetCurSel();
	if (nCurSel == CB_ERR) return;

	m_pSpell->SetType(nCurSel);
}
