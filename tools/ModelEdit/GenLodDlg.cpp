// GenLodDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "genloddlg.h"
#include "lodedit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGenLodDlg dialog


CGenLodDlg::CGenLodDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGenLodDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGenLodDlg)
	m_PieceWeight = 0.0f;
	//}}AFX_DATA_INIT
}


void CGenLodDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGenLodDlg)
	DDX_Control(pDX, IDC_PIECELIST, m_PieceList);
	DDX_Control(pDX, IDC_MINIMUM_NODE_POLIES, m_MinimumNodePolies);
	DDX_Control(pDX, IDC_MAXEDGELENGTH, m_MaxEdgeLength);
	DDX_Control(pDX, IDC_LODS, m_LODs);
	DDX_Text(pDX, IDC_PIECEWEIGHT, m_PieceWeight);
	DDV_MinMaxFloat(pDX, m_PieceWeight, 0.f, 500.f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGenLodDlg, CDialog)
	//{{AFX_MSG_MAP(CGenLodDlg)
	ON_BN_CLICKED(IDC_EDITLOD, OnEditLOD)
	ON_BN_CLICKED(IDC_REMOVELOD, OnRemoveLOD)
	ON_BN_CLICKED(IDC_ADDLOD, OnAddLOD)
	ON_LBN_SELCHANGE(IDC_PIECELIST, OnSelchangePiecelist)
	ON_EN_UPDATE(IDC_PIECEWEIGHT, OnUpdatePieceweight)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CGenLodDlg::FillLODList()
{
	int iSel;
	LODRequestInfo *pLOD;
	char str[512];
	DWORD i;

	iSel = m_LODs.GetCurSel();
	m_LODs.ResetContent();

	for(i=0; i < m_pRequest->m_LODInfos; i++)
	{
		pLOD = &m_pRequest->m_LODInfos[i];

		sprintf(str, "LOD: %d, Dist: %.1f, %% Triangles: %.0f",
			i,
			pLOD->m_Dist,
			((float)pLOD->m_nTris * 100.0f) / m_pRequest->m_pModel->CalcNumTris());

		m_LODs.AddString(str);
	}

	if(iSel == LB_ERR)
		iSel = 0;
		
	m_LODs.SetCurSel(iSel);
}


void CGenLodDlg::SortLODs()
{
	DWORD i;
	BOOL bHappy;
	LODRequestInfo tempLOD;

	if(m_pRequest->m_LODInfos.GetSize() < 2)
		return;
	
	do
	{
		bHappy = TRUE;

		for(i=0; i < m_pRequest->m_LODInfos.GetSize()-1; i++)
		{
			if(m_pRequest->m_LODInfos[i].m_Dist > m_pRequest->m_LODInfos[i+1].m_Dist)
			{
				tempLOD = m_pRequest->m_LODInfos[i];
				m_pRequest->m_LODInfos[i] = m_pRequest->m_LODInfos[i+1];
				m_pRequest->m_LODInfos[i+1] = tempLOD;
				bHappy = FALSE;
			}
		}
	}
	while(!bHappy);
}


/////////////////////////////////////////////////////////////////////////////
// CGenLodDlg message handlers

BOOL CGenLodDlg::OnInitDialog() 
{
	CString theStr;
	DWORD i;

	CDialog::OnInitDialog();

	// Fill the piece list.
	for(i=0; i < m_pRequest->m_pModel->NumPieces(); i++)
	{
		m_PieceList.AddString(m_pRequest->m_pModel->GetPiece(i)->GetName());
	}

	m_PieceList.SetCurSel(0);
	m_PieceWeight = m_pRequest->GetPieceWeight(0);
	UpdateData(FALSE);	
	
	theStr.FormatMessage(IDS_GENERIC_NUM_STRING, 1000);
	m_MaxEdgeLength.SetWindowText((LPCTSTR)theStr);

	theStr.FormatMessage(IDS_GENERIC_NUM_STRING, 8);
	m_MinimumNodePolies.SetWindowText((LPCTSTR)theStr);

	SortLODs();
	FillLODList();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void CGenLodDlg::OnOK() 
{
	TCHAR theText[256];


	m_MaxEdgeLength.GetWindowText(theText, 256);
	m_pRequest->m_MaxEdgeLength = (float)atof(theText);

	m_MinimumNodePolies.GetWindowText(theText, 256);
	m_pRequest->m_nMinPieceTris = atoi(theText);

	CDialog::OnOK();
}


void CGenLodDlg::OnAddLOD() 
{
	LODEdit dlg;
	LODRequestInfo info;

	if(dlg.DoModal() == IDOK)
	{
		dlg.ToLOD(m_pRequest->m_pModel, &info);
		m_pRequest->m_LODInfos.Append(info);
		SortLODs();
		FillLODList();
	}
}


void CGenLodDlg::OnEditLOD() 
{
	DWORD curSel;
	LODEdit dlg;
	LODRequestInfo info;

	curSel = (DWORD)m_LODs.GetCurSel();
	if(curSel < m_pRequest->m_LODInfos.GetSize())
	{
		dlg.FromLOD(m_pRequest->m_pModel, &m_pRequest->m_LODInfos[curSel]);
		if(dlg.DoModal() == IDOK)
		{
			dlg.ToLOD(m_pRequest->m_pModel, &info);
			m_pRequest->m_LODInfos[curSel] = info;
			SortLODs();
			FillLODList();
		}
	}
}

void CGenLodDlg::OnRemoveLOD() 
{
	DWORD curSel;

	curSel = (DWORD)m_LODs.GetCurSel();
	if(curSel < m_pRequest->m_LODInfos.GetSize())
	{
		m_pRequest->m_LODInfos.Remove(curSel);
		FillLODList();
	}
}


void CGenLodDlg::OnSelchangePiecelist() 
{
	m_PieceWeight = m_pRequest->GetPieceWeight((DWORD)m_PieceList.GetCurSel());
	UpdateData(FALSE);
}

void CGenLodDlg::OnUpdatePieceweight() 
{
	DWORD iPiece;


	iPiece = (DWORD)m_PieceList.GetCurSel();
	if(iPiece < m_pRequest->m_PieceWeights)
	{
		UpdateData(TRUE);
		m_pRequest->m_PieceWeights[iPiece] = m_PieceWeight;
	}
}
