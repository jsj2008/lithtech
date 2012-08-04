// LODEdit.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "lodedit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// LODEdit dialog


LODEdit::LODEdit(CWnd* pParent /*=NULL*/)
	: CDialog(LODEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(LODEdit)
	m_fDistance = 0.0f;
	m_fReduction = 0.0f;
	//}}AFX_DATA_INIT
}


void LODEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LODEdit)
	DDX_Text(pDX, IDC_DISTANCE, m_fDistance);
	DDV_MinMaxFloat(pDX, m_fDistance, 0.f, 1.e+006f);
	DDX_Text(pDX, IDC_REDUCTION, m_fReduction);
	DDV_MinMaxFloat(pDX, m_fReduction, 0.f, 100.f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(LODEdit, CDialog)
	//{{AFX_MSG_MAP(LODEdit)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void LODEdit::ToLOD(Model *pModel, LODRequestInfo *pInfo)
{
	pInfo->m_Dist = m_fDistance;
	pInfo->m_nTris = (DWORD)((float)pModel->CalcNumTris() * (m_fReduction/100.0f));
}

void LODEdit::FromLOD(Model *pModel, LODRequestInfo *pInfo)
{
	m_fDistance = pInfo->m_Dist;
	m_fReduction = ((float)pInfo->m_nTris * 100.0f) / pModel->CalcNumTris();
}


/////////////////////////////////////////////////////////////////////////////
// LODEdit message handlers
