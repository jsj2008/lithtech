#ifndef __SELECTEFFECTGROUPDLG_H__
#define __SELECTEFFECTGROUPDLG_H__

class CSelectEffectGroupDlg : public CDialog
{
public:
	enum { IDD = IDD_SELECTEFFECTGROUP };

	CSelectEffectGroupDlg(CWnd* pParent);
	~CSelectEffectGroupDlg();

	//message callback
	BOOL OnInitDialog();
	void OnOK();
	void OnButtonNew();
	void OnButtonEdit();
	void OnSelectionChanged();

	//fills up the combo box
	void FillComboBox();

	//updates the enabled state of the controls
	void UpdateEnabled();

	//the effect group name
	CString m_sEffectGroup;

	DECLARE_MESSAGE_MAP()
	
private:
};


#endif

