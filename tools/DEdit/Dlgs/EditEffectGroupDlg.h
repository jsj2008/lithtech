#ifndef __EDITEFFECTGROUPDLG_H__
#define __EDITEFFECTGROUPDLG_H__

class CEditEffectGroupDlg :
	public CDialog
{
public:
	enum { IDD = IDD_EDITEFFECTGROUP };

	//structors
	CEditEffectGroupDlg(CWnd* pParent);
	~CEditEffectGroupDlg();

	//string to be filled out with the default file name that is to be edited,
	//if it is blank it assumes a new effect is being created
	CString	m_sLoadEffect;

	//handles the OnOk, which involves saving out the file specified by the name
	//into the appropriate directory
	void OnOK();

	//hanle setting up the dialog
	BOOL OnInitDialog();

	//handle the browse buttons
	void OnBrowseScript1();
	void OnBrowseScript2();

	//handle the combo boxes
	void OnChannelChanged1();
	void OnChannelChanged2();
	void OnOverrideChanged1();
	void OnOverrideChanged2();


	//utility function for finding the directory where these files should be saved
	static CString GetEffectGroupDir();	

private:

	//gets the ID of the channel given the specified stage
	uint32 GetChannelID(uint32 nStage);

	//called to setup the text located within a script edit box
	void SetupScriptText(uint32 nStage, const char* pszFilename);

	//called when the channel is changed
	void ChannelChanged(uint32 nStage);

	//called when the override is changed
	void OverrideChanged(uint32 nStage);

	//hides all the variables of the specified stage
	void HideVariables(uint32 nStage);

	//internal handler for browsing a script
	void BrowseScript(uint32 nStage);

	//updates the enabled status of the controls
	void UpdateEnabled();

	//Loads a script into the specified stage and updates the text on the
	//controls appropriately
	void LoadScript(const char* pszName, uint32 nStage);

	//loads the file specified in m_sLoadEffect
	bool LoadEffectGroup();

	//disables the specified stage
	void EnableStage(uint32 nStage, bool bEnable);

	//determines if a stage is disabled
	bool IsDisabled(uint32 nStage);

	//determines what stage is overriding this one, returns a value greater than 
	//the number of possible stages if it is not overridden
	uint32 GetOverrideStage(uint32 nStage);

	//helper functions for getting the appropriate controls
	CStatic*	GetVarStatic(uint32 nStage, uint32 nVar);
	CEdit*		GetVarEdit(uint32 nStage, uint32 nVar);
	CEdit*		GetScriptName(uint32 nStage);
	CButton*	GetBrowseScript(uint32 nStage);
	CComboBox*	GetChannel(uint32 nStage);
	CComboBox*	GetOverride(uint32 nStage);
	CEdit*		GetName();

	//saves the file out
	bool SaveEffectGroup();

	DECLARE_MESSAGE_MAP()
};

#endif
