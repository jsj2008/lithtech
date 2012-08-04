#ifndef __CAMERAFOVDLG_H__
#define __CAMERAFOVDLG_H__

class CPerspectiveViewDef;

class CCameraFOVDlg : 
	public CDialog
{
public:

	CCameraFOVDlg();
	~CCameraFOVDlg();

	BOOL OnInitDialog();

	void OnOK();
	void OnCancel();

private:

	//updates all views to reflect the new view angle
	void			UpdateViewAngles(float fAngle, bool bUseAspect);

	//updates the enabled status of the controls
	void			EnableControls();

	//the original settings (in case the user hits cancel)
	CReal			m_fOrigVertFOV;
	bool			m_bOrigUseAspect;

	//determine if this is valid to update
	bool			m_bInitialized;

	afx_msg void	OnFOVChanged();
	afx_msg void	OnAspectClicked();

	//windows message map
	DECLARE_MESSAGE_MAP()
};

#endif