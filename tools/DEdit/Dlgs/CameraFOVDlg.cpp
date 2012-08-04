#include "bdefs.h"
#include "dedit.h"
#include "resource.h"
#include "camerafovdlg.h"
#include "viewdef.h"
#include "OptionsDisplay.h"
#include "RegionDoc.h"
#include "RegionView.h"

BEGIN_MESSAGE_MAP(CCameraFOVDlg, CDialog)
	ON_EN_CHANGE(IDC_EDIT_VERTICALFOV, OnFOVChanged)
	ON_BN_CLICKED(IDC_MAINTAINASPECT, OnAspectClicked)
END_MESSAGE_MAP()


CCameraFOVDlg::CCameraFOVDlg() :
	CDialog(IDD_CAMERAFOV),
	m_bInitialized(false)
{
}

CCameraFOVDlg::~CCameraFOVDlg()
{
}

BOOL CCameraFOVDlg::OnInitDialog()
{
	//we need to save the originals in case the user cancel's out
	m_fOrigVertFOV		= GetApp()->GetOptions().GetDisplayOptions()->GetViewAngle();
	m_bOrigUseAspect	= GetApp()->GetOptions().GetDisplayOptions()->IsUseAspectRatio();

	//set the ranges for the spin controls
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_VERTICALFOV))  ->SetRange(1, 179);

	//init the text in the edit controls converting to degrees
	CString sVal;

	sVal.Format("%d", (int)(m_fOrigVertFOV * 180 / MATH_PI + 0.5f));
	GetDlgItem(IDC_EDIT_VERTICALFOV)->SetWindowText(sVal);

	//setup the check box
	((CButton*)GetDlgItem(IDC_MAINTAINASPECT))->SetCheck(m_bOrigUseAspect ? TRUE : FALSE);

	//init the enabled status for the controls
	EnableControls();

	m_bInitialized = true;

	return TRUE;
}

void CCameraFOVDlg::OnOK()
{
	CDialog::OnOK();
}

void CCameraFOVDlg::OnCancel()
{
	//save the view angle to the registry data
	GetApp()->GetOptions().GetDisplayOptions()->SetViewAngle(m_fOrigVertFOV);

	//we need to undo the changes we have made
	UpdateViewAngles(m_fOrigVertFOV, m_bOrigUseAspect);

	CDialog::OnCancel();
}

void CCameraFOVDlg::OnFOVChanged()
{
	if(!m_bInitialized)
		return;

	//read in the values from the controls, setup the view, and redraw

	//read in the values from the controls (converting them to radians)
	CString sVal;

	GetDlgItem(IDC_EDIT_VERTICALFOV)->GetWindowText(sVal);
	float fVert = atof(sVal) * MATH_PI / 180.0f;

	bool bUseAspect = ((CButton*)GetDlgItem(IDC_MAINTAINASPECT))->GetCheck() ? true : false;

	//save the view angle to the registry data
	GetApp()->GetOptions().GetDisplayOptions()->SetViewAngle(fVert);
	GetApp()->GetOptions().GetDisplayOptions()->SetUseAspectRatio(bUseAspect);

	//now setup the camera accordingly
	UpdateViewAngles(fVert, bUseAspect);
}

void CCameraFOVDlg::OnAspectClicked()
{
	//they are toggling whether or not to use the aspect ratio, so enable controls
	//appropriately and then adjust the FOV
	EnableControls();
	OnFOVChanged();
}

void CCameraFOVDlg::EnableControls()
{
}

//updates all views to reflect the new view angle
void CCameraFOVDlg::UpdateViewAngles(float fAngle, bool bUseAspect)
{
	CDocTemplate		*pTemplate = GetApp()->m_pWorldTemplate;
	POSITION			Pos;

	//go through all documents...
	for( Pos = pTemplate->GetFirstDocPosition(); Pos; )
	{
		CDocument* pDoc = pTemplate->GetNextDoc(Pos);

		//go through all views...
		POSITION ViewPos;
		for(ViewPos = pDoc->GetFirstViewPosition(); ViewPos; )
		{
			CRegionView* pView = (CRegionView*)pDoc->GetNextView(ViewPos);

			//update the camera controls
			pView->m_PerspectiveView.SetupCamera(fAngle, bUseAspect);

			//now redraw it if it is in a perspective view
			if(pView->IsPerspectiveViewType())
			{
				pView->DrawRect();
			}
		}
	}

}