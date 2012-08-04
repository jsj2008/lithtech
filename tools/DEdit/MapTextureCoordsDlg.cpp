// MapTextureCoordsDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "maptexturecoordsdlg.h"
#include "regiondoc.h"
#include "regionview.h"
#include "texture.h"
#include "de_world.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapTextureCoordsDlg dialog
CMapTextureCoordsDlg::CMapTextureCoordsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMapTextureCoordsDlg::IDD, NULL /*pParent*/)
{


	//{{AFX_DATA_INIT(CMapTextureCoordsDlg)
	m_UOffset = 0.0f;
	m_VOffset = 0.0f;
	m_UScale = 0.0f;
	m_VScale = 0.0f;
	m_Rotation = 0.0f;
	//}}AFX_DATA_INIT

	m_bAutoApply	= TRUE;
	m_bCreateUndo	= FALSE;
	m_bInitialized	= FALSE;

	m_pView			= (CRegionView*)pParent; 
}


void CMapTextureCoordsDlg::DoDataExchange(CDataExchange* pDX)
{
	float fOldU = m_UOffset, fOldV = m_VOffset;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapTextureCoordsDlg)
	DDX_Text(pDX, IDC_ET_UOFFSET, m_UOffset);
	DDV_MinMaxInt(pDX, (int)m_UOffset, -1000000, 1000000);
	DDX_Text(pDX, IDC_ET_VOFFSET, m_VOffset);
	DDV_MinMaxInt(pDX, (int)m_VOffset, -1000000, 1000000);
	DDX_Text(pDX, IDC_ET_USCALE, m_UScale);
	DDV_MinMaxFloat(pDX, m_UScale, -1000000.0f, 1000000.0f);
	DDX_Text(pDX, IDC_ET_VSCALE, m_VScale);
	DDV_MinMaxFloat(pDX, m_VScale, -1000000.0f, 1000000.0f);
	DDX_Text(pDX, IDC_ET_ROTATION, m_Rotation);
	DDV_MinMaxInt(pDX, (int)m_Rotation, -359, 359);
	DDX_Check(pDX, IDC_CHECK_AUTO_APPLY, m_bAutoApply);
	//}}AFX_DATA_MAP
}

//reads the values from the controls into the internal member variables
//but doesn't use DDX so dialogs will not pop up to prompt for numbers, etc
void CMapTextureCoordsDlg::UpdateDataNoDDX()
{
	float fOldU = m_UOffset, fOldV = m_VOffset;

	CString sText;

	GetDlgItem(IDC_ET_UOFFSET)->GetWindowText(sText);
	m_UOffset = atof(sText);

	GetDlgItem(IDC_ET_VOFFSET)->GetWindowText(sText);
	m_VOffset = atof(sText);

	GetDlgItem(IDC_ET_USCALE)->GetWindowText(sText);
	m_UScale = atof(sText);

	GetDlgItem(IDC_ET_VSCALE)->GetWindowText(sText);
	m_VScale = atof(sText);

	GetDlgItem(IDC_ET_ROTATION)->GetWindowText(sText);
	m_Rotation = atof(sText);
}


BEGIN_MESSAGE_MAP(CMapTextureCoordsDlg, CDialog)
	//{{AFX_MSG_MAP(CMapTextureCoordsDlg)
	ON_COMMAND(IDC_APPLY, OnApply)
	ON_COMMAND(IDC_FLIPX, OnFlipX)
	ON_COMMAND(IDC_FLIPY, OnFlipY)
	ON_BN_CLICKED(IDC_CHECK_AUTO_APPLY, OnAutoApply)
	ON_EN_CHANGE(IDC_ET_UOFFSET, GenericChangeHandler)
	ON_EN_CHANGE(IDC_ET_VOFFSET, GenericChangeHandler)
	ON_EN_CHANGE(IDC_ET_USCALE, GenericChangeHandler)
	ON_EN_CHANGE(IDC_ET_VSCALE, GenericChangeHandler)
	ON_EN_CHANGE(IDC_ET_ROTATION, GenericChangeHandler)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMapTextureCoordsDlg message handlers

void CMapTextureCoordsDlg::DoFlip(DWORD iAxis)
{
	switch(iAxis)
	{
	case 0:
		m_bUMirror = !m_bUMirror;
		break;
	case 1:
		m_bVMirror = !m_bVMirror;
		break;
	}
}


void CMapTextureCoordsDlg::ApplyChanges()
{
	CMoArray<CEditPoly*> polies;
	int32 i;
	CEditPoly *pPoly;
	float uScale, vScale, uOffset, vOffset, rotOffset;
	LTVector rotAxis;
	LTMatrix mRot;
	LTVector O, P, Q;

	
	polies.SetCacheSize(50);
	m_pView->GetSelectedPolies(polies);

	uScale = m_UScale / (float)m_TextureWidth;
	vScale = m_VScale / (float)m_TextureHeight;
	
	if(fabs(uScale) < 0.0001f)
		uScale = 0.0001f;

	if(fabs(vScale) < 0.0001f)
		vScale = 0.0001f;
	
	// Modify the polies.
	for(i=0; i < polies; i++)
	{
		pPoly = polies[i];

		O = m_OriginalOPQs[i * 3 + 0];
		P = m_OriginalOPQs[i * 3 + 1];
		Q = m_OriginalOPQs[i * 3 + 2];

		P.Norm();
		Q.Norm();

		// Apply rotation.
		rotOffset = m_Rotation;
		
		// Radians..
		rotOffset *= (MATH_PI / 180.0f);
		rotAxis = P.Cross(Q);
		rotAxis.Norm();
		Mat_SetupRot(&mRot, &rotAxis, -rotOffset);
		MatVMul_InPlace_H(&mRot, &P);
		MatVMul_InPlace_H(&mRot, &Q);

		// Apply the offset.
		O -= P * (m_UOffset - m_UStart) * uScale;
		O -= Q * (m_VOffset - m_VStart) * vScale;

		// Apply scale.
		P /= uScale;
		Q /= vScale;

		//mirror appropriately
		if(m_bUMirror)
			P = -P;
		if(m_bVMirror)
			Q = -Q;

		pPoly->SetTextureSpace(GetCurrTexture(), O, P, Q);
	}

	m_RefRotation	= m_Rotation;
	m_pView->GetRegionDoc()->RedrawPerspectiveViews();
}


void CMapTextureCoordsDlg::OnApply()
{
	UpdateData();	//capture current data to member variables
	ApplyChanges();
}


void CMapTextureCoordsDlg::OnFlipX()
{
	DoFlip(0);
	GenericChangeHandler();
}


void CMapTextureCoordsDlg::OnFlipY()
{
	DoFlip(1);
	GenericChangeHandler();
}

void CMapTextureCoordsDlg::SetupUndo()
{
	CMoArray<CEditPoly*> polies;
	PreActionList actionList;

	polies.SetCacheSize(50);
	m_pView->GetSelectedPolies(polies);

	// Setup undos.
	for(int32 i=0; i < polies; i++)
	{
		AddToActionListIfNew(&actionList, new CPreAction(ACTION_MODIFYNODE, polies[i]->m_pBrush), TRUE);
	}

	m_pView->GetRegionDoc()->Modify(&actionList, TRUE);
}

void CMapTextureCoordsDlg::GenericChangeHandler()
{

	//only bother processing this update if it is a valid update (meaning we
	//are initialized)
	if(m_bInitialized)
	{
		//something changed, we'll need an undo
		m_bCreateUndo = TRUE;

		//determine if we should automatically update
		if(m_bAutoApply)
		{
			//we need to apply
			UpdateDataNoDDX();
			ApplyChanges();
		}
	}
}

BOOL CMapTextureCoordsDlg::OnInitDialog()
{
	ASSERT(m_pView);

	//init the base dialog
	if(CDialog::OnInitDialog() == FALSE)
		return FALSE;

	//save all the original OPQ vectors
	CMoArray<CEditPoly*> polies;
	polies.SetCacheSize(50);
	m_pView->GetSelectedPolies(polies);

	//set up our array to hold the vectors
	m_OriginalOPQs.SetSize(polies.GetSize() * 3);

	//now save them
	for(uint32 nCurrPoly = 0; nCurrPoly < polies.GetSize(); nCurrPoly++)
	{
		m_OriginalOPQs[nCurrPoly * 3 + 0] = polies[nCurrPoly]->GetTexture(GetCurrTexture()).GetO();
		m_OriginalOPQs[nCurrPoly * 3 + 1] = polies[nCurrPoly]->GetTexture(GetCurrTexture()).GetP();
		m_OriginalOPQs[nCurrPoly * 3 + 2] = polies[nCurrPoly]->GetTexture(GetCurrTexture()).GetQ();
	}

	//setup our spin ranges appropriately
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_TEX_COORD_U))->SetRange(0, m_TextureWidth - 1);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_TEX_COORD_V))->SetRange(0, m_TextureHeight - 1);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_TEX_COORD_U_SCALE))->SetRange(1, 1024);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_TEX_COORD_V_SCALE))->SetRange(1, 1024);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_TEX_COORD_ROTATION))->SetRange(0, 359);

	//move the window to the appropriate location
	CRect DlgArea;
	GetWindowRect(DlgArea);

	//get the screen dimensions
	HDC hScreen = ::GetDC(NULL);
	int nWidth = GetDeviceCaps(hScreen, HORZRES);
	int nHeight = GetDeviceCaps(hScreen, VERTRES);
	::ReleaseDC(NULL, hScreen);

	//make sure that the position won't push it off of the screen
	m_nWindowX = LTMAX(0, LTMIN(nWidth - DlgArea.Width(), m_nWindowX));
	m_nWindowY = LTMAX(0, LTMIN(nHeight - DlgArea.Height(), m_nWindowY));

	MoveWindow(m_nWindowX, m_nWindowY, DlgArea.Width(), DlgArea.Height());

	//setup our internal state
	m_bCreateUndo = FALSE;

	//save the original U and V values
	m_UStart = m_UOffset;
	m_VStart = m_VOffset;

	//no mirrorings as of yet
	m_bUMirror = false;
	m_bVMirror = false;

	//all done now
	m_bInitialized = TRUE;

	return TRUE;
}

void CMapTextureCoordsDlg::OnOK()
{
	//ok, we need to reset all the texture coordinates, setup the undo, and
	//then reset them to this

	//save the window pos
	CRect DlgArea;
	GetWindowRect(DlgArea);
	m_nWindowX = DlgArea.left;
	m_nWindowY = DlgArea.top;

	//first off apply any changes that may have not gotten applied yet
	UpdateData();
	ApplyChanges();

	CMoArray<CEditPoly*> polies;
	polies.SetCacheSize(50);
	m_pView->GetSelectedPolies(polies);

	uint32 nCurrPoly;
	for(nCurrPoly = 0; nCurrPoly < polies.GetSize(); nCurrPoly++)
	{
		//we want to swap the vectors
		LTVector vO, vP, vQ;

		vO = m_OriginalOPQs[nCurrPoly * 3 + 0];
		vP = m_OriginalOPQs[nCurrPoly * 3 + 1];
		vQ = m_OriginalOPQs[nCurrPoly * 3 + 2];

		m_OriginalOPQs[nCurrPoly * 3 + 0] = polies[nCurrPoly]->GetTexture(GetCurrTexture()).GetO();
		m_OriginalOPQs[nCurrPoly * 3 + 1] = polies[nCurrPoly]->GetTexture(GetCurrTexture()).GetP();
		m_OriginalOPQs[nCurrPoly * 3 + 2] = polies[nCurrPoly]->GetTexture(GetCurrTexture()).GetQ();

		polies[nCurrPoly]->SetTextureSpace(GetCurrTexture(), vO, vP, vQ);
	}

	//we have reset to the originals, so now create the undo
	if(m_bCreateUndo)
		SetupUndo();

	//now restore the vectors
	for(nCurrPoly = 0; nCurrPoly < polies.GetSize(); nCurrPoly++)
	{
		polies[nCurrPoly]->SetTextureSpace(GetCurrTexture(),	m_OriginalOPQs[nCurrPoly * 3 + 0],
																m_OriginalOPQs[nCurrPoly * 3 + 1],
																m_OriginalOPQs[nCurrPoly * 3 + 2]);
	}

	//all done
	CDialog::OnOK();

}

void CMapTextureCoordsDlg::OnCancel()
{
	//save the window pos
	CRect DlgArea;
	GetWindowRect(DlgArea);
	m_nWindowX = DlgArea.left;
	m_nWindowY = DlgArea.top;

	//the user cancelled so we want to reset all the texture coordinates
	CMoArray<CEditPoly*> polies;
	polies.SetCacheSize(50);
	m_pView->GetSelectedPolies(polies);

	for(uint32 nCurrPoly = 0; nCurrPoly < polies.GetSize(); nCurrPoly++)
	{
		polies[nCurrPoly]->SetTextureSpace(GetCurrTexture(),	m_OriginalOPQs[nCurrPoly * 3 + 0],
																m_OriginalOPQs[nCurrPoly * 3 + 1],
																m_OriginalOPQs[nCurrPoly * 3 + 2]);
	}

	//all done
	CDialog::OnCancel();
}

void CMapTextureCoordsDlg::OnAutoApply()
{
	//update the boolean
	UpdateData(TRUE);
	GenericChangeHandler();
}

