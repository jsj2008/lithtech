#ifndef __WINPACKERDLG_H__
#define __WINPACKERDLG_H__

#ifndef __PROPERTYMGR_H__
#	include "propertymgr.h"
#endif

#ifndef __PROPERTYWND_H__
#	include "PropertyWnd.h"
#endif

#ifndef __AUTOTOOLTIPCTRL_H__
#	include "AutoToolTipCtrl.h"
#endif

//forward declarations
class IPackerImpl;

/////////////////////////////////////////////////////////////////////////////
// CWinpackerDlg dialog

class CWinpackerDlg : public CDialog
{
// Construction
public:
	CWinpackerDlg(IPackerImpl* pIPacker, CWnd* pParent = NULL);	// standard constructor

	CPackerPropList*	GetPropList();

	//this function will build up a string that can be saved out to persist the settings
	//for this packer
	CString				BuildSettingsString();

	//the command line parameters the user has specified
	CString				m_sCommandLine;

	//the name of the packer
	CString				m_sPackerName;

	//the file being packed
	CString				m_sFileToPack;

	//determines if this dialog should immediately end
	BOOL				m_bSkipDlg;

// Dialog Data
	//{{AFX_DATA(CWinpackerDlg)
	enum { IDD = IDD_WINPACKER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinpackerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	HICON m_hIcon;
	HICON m_hSaveIcon;
	HICON m_hLoadIcon;

	//to handle tool tips
	CAutoToolTipCtrl	m_ToolTip;

	//the interface that this dialog is to communicate through
	IPackerImpl*		m_pIPacker;

	//the manager for the properties
	CPropertyMgr		m_PropMgr;

	//the property window
	CPropertyWnd		m_PropertyWnd;

	//this will take the command string, break it apart, and use that to override values
	//in the property list
	bool				LoadUserDefaults(const char* pszDefaults);

	CTabCtrl*			GetGroupTabs()	{return (CTabCtrl*)GetDlgItem(IDC_TAB_GROUPS);}

	CComboBox*			GetPresetCombo() {return (CComboBox*)GetDlgItem(IDC_COMBO_PRESETS);}

	//updates the group list to reflect the groups in the propmgr
	void				ResetGroupList();

	//updates the property list when a group is changed
	void				ResetPropertyList();

	//loads the presets in from the registry for this packer
	void				LoadPresets();

	//creates the property window
	void				CreatePropertyWindow();

	//gets the name of the registry string that holds the preset list
	CString				GetPresetListKeyName() const;

	//gets the selected group (NULL if none selected)
	CPropertyGroup*		GetSelectedGroup();

	// Generated message map functions
	//{{AFX_MSG(CWinpackerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnGroupChanged(NMHDR* pNmhdr, LRESULT* pResult);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButtonSave();
	afx_msg void OnButtonLoad();
	afx_msg void OnAddPreset();
	afx_msg void OnRemovePreset();
	afx_msg void OnPresetChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif 
