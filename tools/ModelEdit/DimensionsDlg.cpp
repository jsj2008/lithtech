// DimensionsDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "dimensionsdlg.h"
#include "model_ops.h"
#include "gl_modelrender.h"
#include "modeleditdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDimensionsDlg dialog


CDimensionsDlg::CDimensionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDimensionsDlg::IDD, pParent),
	m_pModel(NULL)
{
	//{{AFX_DATA_INIT(CDimensionsDlg)
	//}}AFX_DATA_INIT
}


void CDimensionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDimensionsDlg)
	DDX_Control(pDX, IDC_ZDIM, m_ZDim);
	DDX_Control(pDX, IDC_YDIM, m_YDim);
	DDX_Control(pDX, IDC_XDIM, m_XDim);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDimensionsDlg, CDialog)
	//{{AFX_MSG_MAP(CDimensionsDlg)
	ON_BN_CLICKED(ID_APPLY, OnApply)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(ID_DONE, OnDone)
	ON_BN_CLICKED(ID_USEANIMDIMS, OnUseCurAnim)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDimensionsDlg message handlers

BOOL CDimensionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	UpdateControls();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CDimensionsDlg::PostNcDestroy() 
{
	CDialog::PostNcDestroy();

	//delete this modeless dialog to prevent a memory lead
	delete this;

}

void CDimensionsDlg::OnApply() 
{
	SetDimensions();
}

void CDimensionsDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	//need to reset these when the window is hid or shown so that
	//when the user hits enter it won't hide the dialog -JohnO
	GotoDlgCtrl( GetDlgItem(ID_APPLY) );
	SetDefID(ID_APPLY);

	UpdateControls();
}

void CDimensionsDlg::OnDone() 
{
	SetDimensions();
	ShowWindow(SW_HIDE);	
}

void CDimensionsDlg::UpdateControls( void )
{
	CString str;
	AnimInfo *pAnim;

	ASSERT(m_pModel);
	if( !m_pModel )
		return;

	if(m_Anims[0] < (int)m_pModel->m_Anims.GetSize())
	{
		pAnim = &m_pModel->m_Anims[m_Anims[0]];

		str.Format( "%f", pAnim->m_vDims.x );
		m_XDim.SetWindowText( str );
		str.Format( "%f", pAnim->m_vDims.y );
		m_YDim.SetWindowText( str );
		str.Format( "%f", pAnim->m_vDims.z );
		m_ZDim.SetWindowText( str );
	}
}

void CDimensionsDlg::SetDimensions( void )
{
	CString str;
	int i, iAnim;
	AnimInfo *pAnim;

	ASSERT( m_pModel );
	if( !m_pModel )
		return;

	for(i=0; i < m_nAnims; i++)
	{
		iAnim = m_Anims[i];
		
		if(iAnim >= (int)m_pModel->m_Anims.GetSize())
			continue;

		pAnim = &m_pModel->m_Anims[iAnim];

		m_XDim.GetWindowText( str );
		pAnim->m_vDims.x = ( float )atof(( LPCTSTR )str );
		m_YDim.GetWindowText( str );
		pAnim->m_vDims.y = ( float )atof(( LPCTSTR )str );
		m_ZDim.GetWindowText( str );
		pAnim->m_vDims.z = ( float )atof(( LPCTSTR )str );
	}
}

//given an animation and a keyframe, this will find the dims that encompass the
//model
bool CDimensionsDlg::FindAnimDims(Model* pModel, uint32 nAnim, uint32 nKeyFrame, LTVector& vDims)
{
	//clear out the dims to start out with in case we fail
	vDims.Init();

	AnimTracker tracker, *pTracker;
	tracker.m_TimeRef.Init(pModel, nAnim, nKeyFrame, nAnim, nKeyFrame, 0.0f);

	AnimInfo *pAnim = &pModel->m_Anims[nAnim];

	pTracker = &tracker;//pAnim->m_pAnim;

	static CMoArray<TVert> tVerts;

	// Use the model code to setup the vertices.

	int nTrackers = 1;
	nTrackers = DMIN(nTrackers, MAX_GVP_ANIMS);

	GVPStruct gvp;
	
	gvp.m_nAnims = 0;
	for(int i = 0; i < nTrackers; i++)
	{
		gvp.m_Anims[i] = pTracker[i].m_TimeRef;
		gvp.m_nAnims++;
	}

	LTMatrix m;
	m.Identity();

	int nWantedVerts = pModel->GetTotalNumVerts() * 2;
	if(tVerts.GetSize() < nWantedVerts)
	{
		if(!tVerts.SetSize(nWantedVerts))
			return false;
	}

	gvp.m_VertexStride = sizeof(TVert);
	gvp.m_Vertices = tVerts.GetArray();
	gvp.m_BaseTransform = m;
	gvp.m_CurrentLODDist = 0;
	
	if (AlternateGetVertexPositions(pModel, &gvp, true, false, false, false))
	{
		LTVector vMax(0, 0, 0);

		for (i = 0; i < pModel->GetTotalNumVerts(); i ++)
		{
			TVert v = tVerts[i];

			if (fabs(v.m_vPos.x) > vMax.x) vMax.x = (float)fabs(v.m_vPos.x);
			if (fabs(v.m_vPos.y) > vMax.y) vMax.y = (float)fabs(v.m_vPos.y);
			if (fabs(v.m_vPos.z) > vMax.z) vMax.z = (float)fabs(v.m_vPos.z);
		}

		// Setup the new dims....

		//round max up to the .1 decimal place
		vMax.x = (float)(ceil(vMax.x * 10.0) / 10.0);
		vMax.y = (float)(ceil(vMax.y * 10.0) / 10.0);
		vMax.z = (float)(ceil(vMax.z * 10.0) / 10.0);

		vDims = vMax;

		return true;
	}

	//failure
	return false;
}

void CDimensionsDlg::OnUseCurAnim()
{
	
	DWORD dwAnimIndex;
	DWORD dwKeyframeIndex;

	CModelEditDlg *pDlg;
	pDlg = (CModelEditDlg *)AfxGetApp()->GetMainWnd();

	if (!pDlg->GetEditAnimInfo(&dwAnimIndex, &dwKeyframeIndex, false)) return;

	LTVector vMax;
	if(FindAnimDims(m_pModel, dwAnimIndex, dwKeyframeIndex, vMax))
	{
		AnimInfo *pAnim = &m_pModel->m_Anims[dwAnimIndex];
		pAnim->m_vDims = vMax;

		CString sTxt;

		sTxt.Format("%f", pAnim->m_vDims.x);
		m_XDim.SetWindowText(sTxt);

		sTxt.Format("%f", pAnim->m_vDims.y);
		m_YDim.SetWindowText(sTxt);

		sTxt.Format("%f", pAnim->m_vDims.z);
		m_ZDim.SetWindowText(sTxt);
	} 
}
