#ifndef __OBJECTSELFILTERDLG_H__
#define __OBJECTSELFILTERDLG_H__

class CBaseEditObj;

class CObjectSelFilterDlg : public CDialog
{
public:

	CObjectSelFilterDlg();
	~CObjectSelFilterDlg();

	BOOL OnInitDialog();

	void OnOK();
	void OnCancel();

	//updates the class listing based upon the existing list of classes as held by the project
	bool			UpdateClassListing();

	//clears the class listing
	bool			ClearClassListing();

	//querying whether or not an object passes the criteria for selection
	bool			CanSelectObject(CWorldNode* pObj);

	afx_msg void	OnUpdateData();
	afx_msg void	OnUpdateEnabled();
	afx_msg void	OnClear();
	afx_msg void	OnReset();
	afx_msg void	OnInvert();
	afx_msg void	OnAddPreset();
	afx_msg void	OnRemovePreset();
	afx_msg void	OnPresetChanged();
	afx_msg void	OnItemStateChanged(NMHDR * pNotifyStruct, LRESULT * pResult);

private:

	bool		FillPresetList();

	bool		InsertClass(const char* pszName, uint32& nMaxExtent);

	CListCtrl*	GetClassList()			{ return (CListCtrl*)GetDlgItem(IDC_LIST_CLASSES); }
	CEdit*		GetObjNameEdit()		{ return (CEdit*)GetDlgItem(IDC_EDIT_OBJECTNAME); }
	CButton*	GetUseNameButton()		{ return (CButton*)GetDlgItem(IDC_CHECK_MATCH_NAME); }
	CButton*	GetUseClassButton()		{ return (CButton*)GetDlgItem(IDC_CHECK_MATCH_CLASS); }
	CButton*	GetResetClassButton()	{ return (CButton*)GetDlgItem(IDC_BUTTON_SELALL); }
	CButton*	GetInvertClassButton()	{ return (CButton*)GetDlgItem(IDC_BUTTON_SELINVERT); }
	CButton*	GetClearClassButton()	{ return (CButton*)GetDlgItem(IDC_BUTTON_SELNONE); }
	CButton*	GetAddPresetButton()	{ return (CButton*)GetDlgItem(IDC_BUTTON_ADDPRESET); }
	CButton*	GetRemovePresetButton()	{ return (CButton*)GetDlgItem(IDC_BUTTON_REMOVEPRESET); }
	CComboBox*	GetPresetCombo()		{ return (CComboBox*)GetDlgItem(IDC_COMBO_PRESETS); }

	//safeguard to prevent infinite recursion on checkboxes, avoids processing them
	bool		m_bIgnoreCheckChanges;

	//frequently checked cached values
	BOOL		m_bCheckName;
	BOOL		m_bCheckClass;
	CString		m_sNameFilter;

	//windows message map
	DECLARE_MESSAGE_MAP()
};

#endif