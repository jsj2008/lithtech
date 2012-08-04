// WeightEditDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "weighteditdlg.h"
#include "modeleditdlg.h"
#include "stringdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// WeightEditDlg dialog


WeightEditDlg::WeightEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(WeightEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(WeightEditDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bUpdateWeights = TRUE;
	m_bChangesMade = FALSE;
}


void WeightEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WeightEditDlg)
	DDX_Control(pDX, IDC_WEIGHT, m_Weight);
	DDX_Control(pDX, IDC_NODES, m_NodeList);
	DDX_Control(pDX, IDC_WEIGHTSETS, m_WeightSets);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(WeightEditDlg, CDialog)
	//{{AFX_MSG_MAP(WeightEditDlg)
	ON_BN_CLICKED(IDC_ADDSET, OnAddSet)
	ON_BN_CLICKED(IDC_REMOVESET, OnRemoveSet)
	ON_EN_UPDATE(IDC_WEIGHT, OnUpdateWeight)
	ON_LBN_SELCHANGE(IDC_NODES, OnSelChangeNodes)
	ON_LBN_SELCHANGE(IDC_WEIGHTSETS, OnSelchangeWeightsets)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void WeightEditDlg::FillWeightSetList(BOOL bPreserveSel)
{
	DWORD i;
	WeightSet *pSet;
	int iCurSel;


	iCurSel = m_WeightSets.GetCurSel();
	m_WeightSets.ResetContent();

	for(i=0; i < m_pModel->NumWeightSets(); i++)
	{
		pSet = m_pModel->GetWeightSet(i);
	
		m_WeightSets.AddString(pSet->GetName());
	}

	if(bPreserveSel)
	{
		if(iCurSel == LB_ERR)
			m_WeightSets.SetCurSel(0);
		else
			m_WeightSets.SetCurSel(iCurSel);
	}
}

void WeightEditDlg::FillNodeList(BOOL bPreserveSel)
{
	DWORD i;
	ModelNode *pNode;
	int iCurSel;

	iCurSel = m_NodeList.GetCurSel();
	m_NodeList.ResetContent();

	for(i=0; i < m_pModel->NumNodes(); i++)
	{
		pNode = m_pModel->GetNode(i);

		m_NodeList.AddString(pNode->GetName());
	}

	if(bPreserveSel)
	{
		if(iCurSel == LB_ERR)
			m_NodeList.SetCurSel(0);
		else
			m_NodeList.SetCurSel(iCurSel);
	}
}


WeightSet* WeightEditDlg::GetCurSelections(
	int *pNodes, DWORD nodeListSizeBytes, int &nNodes)
{
	int iCurSel;

	iCurSel = m_WeightSets.GetCurSel();
	if(iCurSel < 0 || iCurSel >= (int)m_pModel->NumWeightSets())
		return NULL;

	nNodes = m_NodeList.GetSelItems(nodeListSizeBytes/sizeof(int), pNodes);
	if(nNodes <= 0 || nNodes > (int)m_pModel->NumNodes())
		return NULL;

	return m_pModel->GetWeightSet(iCurSel);
}


void WeightEditDlg::HandleSelChange()
{
	WeightSet *pSet;
	char str[256];
	int nNodes, nodes[256];
	
	pSet = GetCurSelections(nodes, sizeof(nodes), nNodes);
	if(!pSet)
		return;

	m_bUpdateWeights = FALSE;
		sprintf(str, "%.3f", pSet->m_Weights[nodes[0]]);
		m_Weight.SetWindowText(str);
	m_bUpdateWeights = TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// WeightEditDlg message handlers

void WeightEditDlg::OnAddSet() 
{
	CStringDlg dlg;
	WeightSet *pSet;

	dlg.m_bAllowLetters = TRUE;
	dlg.m_bAllowNumbers = TRUE;
	dlg.m_bAllowOthers = TRUE;

	if(dlg.DoModal(IDS_NEWWEIGHTSET, IDS_ENTERWEIGHTSETNAME) == IDOK)
	{
		pSet = new WeightSet(m_pModel);
		if(pSet)
		{
			if(!pSet->InitWeights(m_pModel->NumNodes()))
			{
				delete pSet;
				return;
			}

			pSet->SetName(dlg.m_EnteredText);
			m_pModel->AddWeightSet(pSet);
			FillWeightSetList(TRUE);
			m_bChangesMade = TRUE;
		}
	}	
}

void WeightEditDlg::OnRemoveSet() 
{
	int iCurSel;

	iCurSel = m_WeightSets.GetCurSel();
	if(iCurSel < 0 || iCurSel >= (int)m_pModel->NumWeightSets())
		return;

	m_pModel->RemoveWeightSet(iCurSel);
	FillWeightSetList(TRUE);
	m_bChangesMade = TRUE;
}

void WeightEditDlg::OnUpdateWeight() 
{
	WeightSet *pSet;
	int i, nNodes, nodes[256];
	char str[256];
	float theWeight;

	if(!m_bUpdateWeights)
		return;

	pSet = GetCurSelections(nodes, sizeof(nodes), nNodes);
	if(!pSet)
		return;

	m_Weight.GetWindowText(str, sizeof(str));
	theWeight = (float)atof(str);

	for(i=0; i < nNodes; i++)
	{
		pSet->m_Weights[nodes[i]] = theWeight;
	}

	m_bChangesMade = TRUE;
}

BOOL WeightEditDlg::OnInitDialog() 
{
	int i;

	CDialog::OnInitDialog();
	
	FillWeightSetList(TRUE);
	FillNodeList(FALSE);

	for (i = 0; i < m_pDlg->m_NodeList.GetItemCount(); i++)
	{
		if (m_pDlg->m_NodeList.GetItemState(i, LVIS_SELECTED))
			m_NodeList.SetSel(i, TRUE);
	}

	return TRUE;
}

void WeightEditDlg::OnSelChangeNodes() 
{
	HandleSelChange();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelchangeWeightsets()
//
//   PURPOSE  : Handles selection change in IDC_WEIGHTSET
//
//------------------------------------------------------------------

void WeightEditDlg::OnSelchangeWeightsets() 
{
	HandleSelChange();

	// Clear all selections within the node list listbox

	m_NodeList.SelItemRange(FALSE, 0, m_NodeList.GetCount());

	int nCurSel = m_WeightSets.GetCurSel();
	if ((nCurSel != LB_ERR) && (nCurSel < (int)m_pModel->NumWeightSets()))
	{		
		WeightSet *pSet = m_pModel->GetWeightSet(nCurSel);

		// Select all the weighted nodes in this set.
		
		for (int i = 0; i < (int)pSet->m_Weights.GetSize(); i ++)
		{
			if (pSet->m_Weights[i] > 0.0f)
			{
				// Select the node in the nodes list box

				m_NodeList.SetSel(i, TRUE);
			}
		}
	}	
}
