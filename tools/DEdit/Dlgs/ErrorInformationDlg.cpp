#include "bdefs.h"
#include "errorinformationdlg.h"
#include "errordetector.h"
#include "levelerror.h"


//fill out the message map

BEGIN_MESSAGE_MAP(CErrorInformationDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_DETECTOR, OnCheckScanInFuture)
END_MESSAGE_MAP()


CErrorInformationDlg::CErrorInformationDlg(CLevelError* pError, CRegionDoc* pSrcDoc) :
	CDialog(IDD_ERROR_INFORMATION),
	m_pError(pError),
	m_pSrcDoc(pSrcDoc)
{
	ASSERT(pError);
}

CErrorInformationDlg::~CErrorInformationDlg()
{
}


void CErrorInformationDlg::OnCheckScanInFuture()
{
	//get the detector
	CErrorDetector* pDet = m_pError->GetDetector();
	pDet->SetEnabled(((CButton*)GetDlgItem(IDC_CHECK_ENABLE_DETECTOR))->GetCheck() ? true : false);
}

//notification when the dialog needs to be set up
BOOL CErrorInformationDlg::OnInitDialog()
{
	//set up the severity
	CString sSeverity;
	switch(m_pError->GetSeverity())
	{
	case ERRORSEV_CRITICAL:		sSeverity = "Critical";		break;
	case ERRORSEV_HIGH:			sSeverity = "High";			break;
	case ERRORSEV_MEDIUM:		sSeverity = "Medium";		break;
	case ERRORSEV_LOW:			sSeverity = "Low";			break;
	case ERRORSEV_VERYLOW:		sSeverity = "Very Low";		break;
	}

	//first off set up the error information
	((CStatic*)GetDlgItem(IDC_STATIC_ERROR_NAME))->SetWindowText(m_pError->GetName());
	((CEdit*)GetDlgItem(IDC_EDIT_ERROR_HELP_TEXT))->SetWindowText(m_pError->GetHelp());
	((CStatic*)GetDlgItem(IDC_STATIC_ERROR_SEVERITY))->SetWindowText(sSeverity);

	if(m_pError->HasNode() && m_pError->GetNodeName())
	{
		((CStatic*)GetDlgItem(IDC_STATIC_ERROR_NODE_NAME))->SetWindowText(m_pError->GetNodeName());
	}
	else
	{
		((CStatic*)GetDlgItem(IDC_STATIC_ERROR_NODE_NAME))->SetWindowText("N/A");
	}


	//set up the detector information
	CErrorDetector* pDet = m_pError->GetDetector();
	((CStatic*)GetDlgItem(IDC_STATIC_ERROR_DETECTED_BY))->SetWindowText(pDet->GetName());
	((CEdit*)GetDlgItem(IDC_EDIT_DETECTOR_HELP_TEXT))->SetWindowText(pDet->GetHelpText());

	//set the check to the appropriate value
	((CButton*)GetDlgItem(IDC_CHECK_ENABLE_DETECTOR))->SetCheck(pDet->IsEnabled() ? 1 : 0);

	return TRUE;

}

