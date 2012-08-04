#ifndef __LEVELERROROPTIONSDLG_H__
#define __LEVELERROROPTIONSDLG_H__

class CLevelErrorDB;

class CLevelErrorOptionsDlg : public CDialog
{
public:

	CLevelErrorOptionsDlg(CLevelErrorDB* pDB);
	~CLevelErrorOptionsDlg();

	BOOL OnInitDialog();
	void OnOK();
	void OnCancel();

	//for getting the tooltip help
	afx_msg void	OnListTooltip(NMHDR * pNotifyStruct, LRESULT * pResult);
	afx_msg void	OnResetAll();

private:

	CLevelErrorDB*		m_pDB;

	//windows message map
	DECLARE_MESSAGE_MAP()
};

#endif