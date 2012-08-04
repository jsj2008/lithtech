#ifndef __ERRORINFORMATIONDLG_H__
#define __ERRORINFORMATIONDLG_H__

class CLevelError;
class CRegionDoc;

class CErrorInformationDlg : public CDialog
{
public:

	CErrorInformationDlg(CLevelError* pError, CRegionDoc* pSrcDoc);
	~CErrorInformationDlg();

	//message handlers
	afx_msg void	OnCheckScanInFuture();

	//notification when the dialog needs to be set up
	BOOL OnInitDialog();

private:

	//the error that we are giving information about
	CLevelError*	m_pError;

	//source document
	CRegionDoc*		m_pSrcDoc;

	//windows message map
	DECLARE_MESSAGE_MAP()
};

#endif

