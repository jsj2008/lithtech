#include "bdefs.h"
#include "resource.h"
#include "EditEffectGroupDlg.h"
#include "editprojectmgr.h"
#include "edithelpers.h"

#if _MSC_VER >= 1300
#include <fstream>
#else
#include <fstream.h>
#endif

#include <direct.h>

//defines
#define NUM_STAGES				2
#define NUM_VARS				6
#define EFFECTGROUP_VERSION		1
#define EFFECTSCRIPT_VERSION	1

//various stage types possible
#define STAGETYPE_DISABLED					0
#define STAGETYPE_OVERRIDDEN				1
#define STAGETYPE_EVALUATED					2

//the different channels that the transforms can be installed on
enum  {	TSChannel_Null,
		TSChannel_Base,
		TSChannel_Detail,
		TSChannel_EnvMap,
		TSChannel_LightMap,
		TSChannel_DualTexture,

		//must come last
		TSChannel_Count
	};

//message map
BEGIN_MESSAGE_MAP(CEditEffectGroupDlg, CDialog)
	//{{AFX_MSG_MAP(CSpherePrimitiveDlg)
	ON_EN_CHANGE(IDC_EDIT_NAME, UpdateEnabled)
	ON_BN_CLICKED(IDC_BUTTON_BROWSESCRIPT1, OnBrowseScript1)
	ON_BN_CLICKED(IDC_BUTTON_BROWSESCRIPT2, OnBrowseScript2)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNEL1, OnChannelChanged1)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNEL2, OnChannelChanged2)
	ON_CBN_SELCHANGE(IDC_COMBO_OVERRIDE1, OnOverrideChanged1)
	ON_CBN_SELCHANGE(IDC_COMBO_OVERRIDE2, OnOverrideChanged2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//structors
CEditEffectGroupDlg::CEditEffectGroupDlg(CWnd* pParent) :
	CDialog(CEditEffectGroupDlg::IDD, pParent)
{
}

CEditEffectGroupDlg::~CEditEffectGroupDlg()
{
}

//handles the OnOk, which involves saving out the file specified by the name
//into the appropriate directory
void CEditEffectGroupDlg::OnOK()
{
	//try and save the effect group
	if(!SaveEffectGroup())
		return;

	//success, so bail
	CDialog::OnOK();
}

//hanle setting up the dialog
BOOL CEditEffectGroupDlg::OnInitDialog()
{
	if(!CDialog::OnInitDialog())
		return FALSE;

	//hide all the variables
	for(uint32 nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
	{
		GetOverride(nCurrStage)->SelectString(-1, "None");
		GetChannel(nCurrStage)->SelectString(-1, "Disabled");
		EnableStage(nCurrStage, false);
		HideVariables(nCurrStage);
	}

	//see if this is a new one, or if we are opening an old one
	if(!m_sLoadEffect.IsEmpty())
	{
		//this is not a new effect, try and open up the old one
		if(!LoadEffectGroup())
		{
			CString sError;
			sError.Format("Unable to open the file %s", m_sLoadEffect);
			MessageBox(sError, "Error Opening File", MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}

		//make the name read only and set the text
		GetName()->EnableWindow(FALSE);
		GetName()->SetWindowText(m_sLoadEffect);
	}

	//update enabled status
	UpdateEnabled();

	return TRUE;
}

//handle the browse buttons
void CEditEffectGroupDlg::OnBrowseScript1()
{
	BrowseScript(0);
}

void CEditEffectGroupDlg::OnBrowseScript2()
{
	BrowseScript(1);
}

//handle the combo boxes
void CEditEffectGroupDlg::OnChannelChanged1()
{
	ChannelChanged(0);
}

void CEditEffectGroupDlg::OnChannelChanged2()
{
	ChannelChanged(1);
}

void CEditEffectGroupDlg::OnOverrideChanged1()
{
	OverrideChanged(0);
}

void CEditEffectGroupDlg::OnOverrideChanged2()
{
	OverrideChanged(1);
}

//internal handler for browsing a script
void CEditEffectGroupDlg::BrowseScript(uint32 nStage)
{
	//open up a file dialog to find the script
	CFileDialog Dlg(TRUE, "*.tfs", NULL, OFN_FILEMUSTEXIST, "Texture Effect Scripts (*.tfs)|*.tfs|All Files (*.*)|*.*||");

	if(Dlg.DoModal() == IDOK)
	{
		//we need to load the texture script into that stage
		LoadScript(Dlg.GetPathName(), nStage);
		SetupScriptText(nStage, Dlg.GetPathName());
	}
}

//updates the enabled status of the controls
void CEditEffectGroupDlg::UpdateEnabled()
{
	//only enable the OK button if there is a name
	CEdit* pEditName = (CEdit*)GetDlgItem(IDC_EDIT_NAME);

	CString sName;
	pEditName->GetWindowText(sName);

	GetDlgItem(IDOK)->EnableWindow(!sName.IsEmpty());
}

//Loads a script into the specified stage and updates the text on the
//controls appropriately
void CEditEffectGroupDlg::LoadScript(const char* pszName, uint32 nStage)
{
	//first off, make sure that all the variables are hidden (they will be unhidden
	//as appropriate)
	HideVariables(nStage);

	//ok, try and open up this file
#if _MSC_VER >= 1300
	std::ifstream InFile(pszName, std::ios::in | std::ios::binary);
#else
	ifstream InFile(pszName, ios::nocreate | ios::in | ios::binary);
#endif

	//make sure it worked
	if(!InFile)
		return;

	//ok, we have the file, load in the version
	uint16 nVersion;
	InFile.read((char*)&nVersion, sizeof(nVersion));

	//make sure the version matches
	if(nVersion != EFFECTSCRIPT_VERSION)
		return;

	//read the size of the user variable block
	uint32 nUserVarSize;
	InFile.read((char*)&nUserVarSize, sizeof(nUserVarSize));

	//read in the number of user variables
	uint8 nNumVars;
	InFile.read((char*)&nNumVars, sizeof(nNumVars));

	//load in each variable
	CString sVar;
	for(uint32 nCurrVar = 0; nCurrVar < (uint32)nNumVars; nCurrVar++)
	{
		char ch;

		do
		{
			//read in the character
			InFile.read((char*)&ch, sizeof(ch));

			//add it if it isn't the end of string
			if(ch != '\0')
			{
				sVar += ch;
			}
			else
			{
				//it is the end of the string, we need to install this string
				//in the appropriate slot
				CStatic* pStatic = GetVarStatic(nStage, nCurrVar);

				if(pStatic)
				{
					pStatic->ShowWindow(SW_SHOW);
					pStatic->SetWindowText(sVar);
				}

				//show the edit field
				CEdit* pEdit = GetVarEdit(nStage, nCurrVar);

				if(pEdit)
				{
					pEdit->ShowWindow(SW_SHOW);
				}

				//clear out the string
				sVar = "";
			}

		}
		while(ch != '\0');
	}

	//all done.
	return;
}

//hides all the variables of the specified stage
void CEditEffectGroupDlg::HideVariables(uint32 nStage)
{
	for(uint32 nCurrVar = 0; nCurrVar < NUM_VARS; nCurrVar++)
	{
		//hide the text control
		CStatic* pStatic = GetVarStatic(nStage, nCurrVar);
		if(pStatic)
			pStatic->ShowWindow(SW_HIDE);

		//hide the edit control
		CEdit* pEdit = GetVarEdit(nStage, nCurrVar);
		if(pEdit)
			pEdit->ShowWindow(SW_HIDE);
	}
}

//loads a file in from the file specified in the member string
bool CEditEffectGroupDlg::LoadEffectGroup()
{
	//try and open up the file
#if _MSC_VER >= 1300
	std::ifstream InFile(GetEffectGroupDir() + m_sLoadEffect, std::ios::binary | std::ios::in );
#else
	ifstream InFile(GetEffectGroupDir() + m_sLoadEffect, ios::binary | ios::in | ios::nocreate);
#endif

	if(!InFile)
		return false;

	//load the file now
	
	//now read in the file version
	uint32 nVersion;
	InFile.read((char*)&nVersion, sizeof(nVersion));

	//make sure that the file version matches
	if(nVersion != EFFECTGROUP_VERSION)
	{
		return false;
	}

	//ok, now we need to read in how many stages are listed in the file
	uint32 nNumStages;
	InFile.read((char*)&nNumStages, sizeof(nNumStages));

	//clamp it to the maximum number of stages
	nNumStages = LTMIN(nNumStages, NUM_STAGES);

	//now read in each stage
	for(uint32 nCurrStage = 0; nCurrStage < nNumStages; nCurrStage++)
	{
		//read in the type of the stage
		uint32 nStageType;
		InFile.read((char*)&nStageType, sizeof(nNumStages));

		//just continue if this stage is disabled
		if(nStageType == STAGETYPE_DISABLED)
		{
			continue;
		}

		//read in the channel that this stage maps to
		uint32 nChannel;
		InFile.read((char*)&nChannel, sizeof(nChannel));

		//select it from the drop down
		switch(nChannel)
		{
		case TSChannel_Base:		GetChannel(nCurrStage)->SelectString(-1, "Base Texture"); break;
		case TSChannel_Detail:		GetChannel(nCurrStage)->SelectString(-1, "Detail Texture"); break;
		case TSChannel_EnvMap:		GetChannel(nCurrStage)->SelectString(-1, "Environment map"); break;
		case TSChannel_LightMap:	GetChannel(nCurrStage)->SelectString(-1, "Light Map"); break;
		case TSChannel_DualTexture:	GetChannel(nCurrStage)->SelectString(-1, "Dual Texture"); break;
		}
		ChannelChanged(nCurrStage);

		if(nStageType == STAGETYPE_OVERRIDDEN)
		{
			//this stage is overridded, load in the stage that is overriding
			uint32 nOverride;
			InFile.read((char*)&nOverride, sizeof(nOverride));

			CString sVal;
			sVal.Format("%d", nOverride + 1);
			GetOverride(nCurrStage)->SelectString(-1, sVal);

			OverrideChanged(nCurrStage);
		}
		else if(nStageType == STAGETYPE_EVALUATED)
		{
			//this stage is evaluated, make sure to load in the name of the script
			//and its default values
			char pszScript[MAX_PATH];

			//read the length of the script name
			uint16 nStrLen;
			InFile.read((char*)&nStrLen, sizeof(nStrLen));

			CString sScript;
			for(uint32 nCurrChar = 0; nCurrChar < nStrLen; nCurrChar++)
			{
				char ch;
				InFile.read((char*)&ch, sizeof(ch));
				sScript += ch;
			}
			
			//now read in the defaults
			uint32 nNumDefaults;
			InFile.read((char*)&nNumDefaults, sizeof(nNumDefaults));

			for(uint32 nCurrDefault = 0; nCurrDefault < nNumDefaults; nCurrDefault++)
			{
				float fVar;
				InFile.read((char*)&fVar, sizeof(fVar));
				
				if(nCurrDefault < NUM_VARS)
				{
					CEdit* pEdit = GetVarEdit(nCurrStage, nCurrDefault);
					CString sVal;
					sVal.Format("%f", fVar);
					pEdit->SetWindowText(sVal);
				}
			}

			//now load our script
			sScript = GetProject()->m_BaseProjectDir + sScript;

			LoadScript(sScript, nCurrStage);
			SetupScriptText(nCurrStage, sScript);
		}
	}

	return true;
}

//called to setup the text located within a script edit box
void CEditEffectGroupDlg::SetupScriptText(uint32 nStage, const char* pszFilename)
{
	//create a string version
	CString sFilename(pszFilename);

	//get the project directory
	CString sRoot = GetProject()->m_BaseProjectDir;

	//get the length
	uint32 nLen = sRoot.GetLength();

	//now make sure it matches
	if(nLen <= sFilename.GetLength())
	{
		if(sFilename.Left(nLen).CompareNoCase(sRoot) == 0)
		{
			//it matches, go ahead and remove it
			sFilename = sFilename.Mid(nLen);
		}
	}

	//now set it up in the field
	GetScriptName(nStage)->SetWindowText(sFilename);
}

//saves the file out
bool CEditEffectGroupDlg::SaveEffectGroup()
{
	//first off, make sure that this configuration is valid
	uint32 nCurrStage;

	for(nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
	{
		//nothing to check if it is disabled
		if(IsDisabled(nCurrStage))
			continue;

		//see if this is overridden
		uint32 nOverride = GetOverrideStage(nCurrStage);

		if(nOverride < NUM_STAGES)
		{
			//this is overridden, make sure that the stage it is pointing to
			//is not overridden
			if(GetOverrideStage(nOverride) < NUM_STAGES)
			{
				CString sError;
				sError.Format("Stage %d is overridden and points to a stage that is also overridden. Point this stage to the correct stage", nCurrStage);
				MessageBox(sError, "Incorrect Overriding", MB_ICONEXCLAMATION | MB_OK);
				return false;
			}
		}
		else
		{
			//not overridden, make sure that the script is specified
			CString sScript;
			GetScriptName(nCurrStage)->GetWindowText(sScript);

			if(sScript.IsEmpty())
			{
				CString sError;
				sError.Format("Stage %d does not have a script specified", nCurrStage);
				MessageBox(sError, "Missing Script", MB_ICONEXCLAMATION | MB_OK);
				return false;
			}
		}
	}

	//get the name
	CString sName;
	GetName()->GetWindowText(sName);
	sName = GetEffectGroupDir() + sName;

	if((sName.GetLength() < 4) || (sName.Right(4).CompareNoCase(".tfg") != 0))
		sName += ".tfg";

	//make sure that the directory exists
	_mkdir(GetEffectGroupDir());

	//try and open up the file
#if _MSC_VER >= 1300
	std::ofstream OutFile(sName, std::ios::binary | std::ios::out);
#else
	ofstream OutFile(sName, ios::binary | ios::out);
#endif

	if(!OutFile)
		return false;

	//write the file now
	
	//now write out the file version
	uint32 nTemp;
	nTemp = EFFECTGROUP_VERSION;
	OutFile.write((char*)&nTemp, sizeof(nTemp));

	//ok, now we need to write out how many stages are listed in the file
	nTemp = NUM_STAGES;
	OutFile.write((char*)&nTemp, sizeof(nTemp));

	//now read in each stage
	for(nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
	{
		if(IsDisabled(nCurrStage))
		{
			nTemp = STAGETYPE_DISABLED;
			OutFile.write((char*)&nTemp, sizeof(nTemp));
			continue;
		}

		//write out the stage type
		uint32 nOverride = GetOverrideStage(nCurrStage);
		nTemp = (nOverride < NUM_STAGES) ? STAGETYPE_OVERRIDDEN : STAGETYPE_EVALUATED;
		OutFile.write((char*)&nTemp, sizeof(nTemp));

		//determine the channel
		uint32 nChannel = GetChannelID(nCurrStage);
		OutFile.write((char*)&nChannel, sizeof(nChannel));

		if(nOverride < NUM_STAGES)
		{
			//this stage is overridded, load in the stage that is overriding
			nTemp = GetOverrideStage(nCurrStage);
			OutFile.write((char*)&nTemp, sizeof(nTemp));
		}
		else
		{
			CString sScript;
			GetScriptName(nCurrStage)->GetWindowText(sScript);

			//write the length of the script name
			uint16 nTemp16 = (uint16)sScript.GetLength();
			OutFile.write((char*)&nTemp16, sizeof(nTemp16));
			OutFile.write((const char*)sScript, nTemp16 * sizeof(char));
			
			//now write out the defaults
			nTemp = NUM_VARS;
			OutFile.write((char*)&nTemp, sizeof(nTemp));

			for(uint32 nCurrDefault = 0; nCurrDefault < NUM_VARS; nCurrDefault++)
			{
				CString sVal;
				GetVarEdit(nCurrStage, nCurrDefault)->GetWindowText(sVal);

				float fVar = atof(sVal);
				OutFile.write((char*)&fVar, sizeof(fVar));
			}
		}
	}

	return true;
}

//gets the ID of the channel given the specified stage
uint32 CEditEffectGroupDlg::GetChannelID(uint32 nStage)
{
	//get the name of this channel
	CString sVal;
	GetChannel(nStage)->GetLBText(GetChannel(nStage)->GetCurSel(), sVal);

	//now classify it
	if(sVal.CompareNoCase("Base Texture") == 0)
		return TSChannel_Base;
	else if(sVal.CompareNoCase("Detail Texture") == 0)
		return TSChannel_Detail;
	else if(sVal.CompareNoCase("Environment map") == 0)
		return TSChannel_EnvMap;
	else if(sVal.CompareNoCase("Light Map") == 0)
		return TSChannel_LightMap;
	else if(sVal.CompareNoCase("Dual Texture") == 0)
		return TSChannel_DualTexture;

	return TSChannel_Null;
}

//utility function for finding the directory where these files should be saved
CString CEditEffectGroupDlg::GetEffectGroupDir()
{
	//get the root directory
	CString sDir = GetProject()->m_BaseProjectDir;

	//add on our subdirectory and wildcard
	sDir.TrimRight("\\/");
	sDir += "\\TextureEffectGroups\\";

	return sDir;
}

//disables the specified stage
void CEditEffectGroupDlg::EnableStage(uint32 nStage, bool bEnable)
{
	bool bOverridden = GetOverrideStage(nStage) < NUM_STAGES;

	//now disable all the other controls
	GetOverride(nStage)->EnableWindow(bEnable);
	GetScriptName(nStage)->EnableWindow(bEnable && !bOverridden);
	GetBrowseScript(nStage)->EnableWindow(bEnable && !bOverridden);

	for(uint32 nCurrVar = 0; nCurrVar < NUM_VARS; nCurrVar++)
	{
		GetVarEdit(nStage, nCurrVar)->EnableWindow(bEnable && !bOverridden);
	}
}

//determines if a stage is disabled
bool CEditEffectGroupDlg::IsDisabled(uint32 nStage)
{
	//get the combo for the stage
	CComboBox* pChannel = GetChannel(nStage);

	CString sVal;
	pChannel->GetLBText(pChannel->GetCurSel(), sVal);

	if(sVal.CompareNoCase("Disabled") == 0)
		return true;

	return false;
}

//determines what stage is overriding this one, returns a value greater than 
//the number of possible stages if it is not overridden
uint32 CEditEffectGroupDlg::GetOverrideStage(uint32 nStage)
{
	//get the combo for the stage
	CComboBox* pOverride = GetOverride(nStage);

	//get the text
	CString sStage;
	pOverride->GetLBText(pOverride->GetCurSel(), sStage);

	//see if this is valid
	if(sStage.CompareNoCase("None") == 0)
		return (uint32)-1;

	return (uint32)atoi(sStage) - 1;
}

//called when the channel is changed
void CEditEffectGroupDlg::ChannelChanged(uint32 nStage)
{
	EnableStage(nStage, !IsDisabled(nStage));
}

//called when the override is changed
void CEditEffectGroupDlg::OverrideChanged(uint32 nStage)
{
	EnableStage(nStage, !IsDisabled(nStage));
}



//--------------------------------------------------------------------------------------
// Control utilities.
// These are kind of ugly, but relying on the resource manager having subsequent numbers
// is even uglier and far more error prone. Basically these are just mappings from
// stages to controls.
//--------------------------------------------------------------------------------------

//helper function to get the static ID of the specified stage and variable
CStatic* CEditEffectGroupDlg::GetVarStatic(uint32 nStage, uint32 nVar)
{
	uint32 nID = 0;

	switch(nStage)
	{
	case 0:
		{
			switch(nVar)
			{
			case 0:		nID = IDC_STATIC_S1P0; break;
			case 1:		nID = IDC_STATIC_S1P1; break;
			case 2:		nID = IDC_STATIC_S1P2; break;
			case 3:		nID = IDC_STATIC_S1P3; break;
			case 4:		nID = IDC_STATIC_S1P4; break;
			case 5:		nID = IDC_STATIC_S1P5; break;
			}
		}
		break;
	case 1:
		{
			switch(nVar)
			{
			case 0:		nID = IDC_STATIC_S2P0; break;
			case 1:		nID = IDC_STATIC_S2P1; break;
			case 2:		nID = IDC_STATIC_S2P2; break;
			case 3:		nID = IDC_STATIC_S2P3; break;
			case 4:		nID = IDC_STATIC_S2P4; break;
			case 5:		nID = IDC_STATIC_S2P5; break;
			}
		}
		break;
	}

	if(nID == 0)
		return NULL;

	return (CStatic*)GetDlgItem(nID);
}

//helper function to get the edit ID of the specified stage and variable
CEdit* CEditEffectGroupDlg::GetVarEdit(uint32 nStage, uint32 nVar)
{
	uint32 nID = 0;

	switch(nStage)
	{
	case 0:
		{
			switch(nVar)
			{
			case 0:		nID = IDC_EDIT_S1P0; break;
			case 1:		nID = IDC_EDIT_S1P1; break;
			case 2:		nID = IDC_EDIT_S1P2; break;
			case 3:		nID = IDC_EDIT_S1P3; break;
			case 4:		nID = IDC_EDIT_S1P4; break;
			case 5:		nID = IDC_EDIT_S1P5; break;
			}
		}
		break;
	case 1:
		{
			switch(nVar)
			{
			case 0:		nID = IDC_EDIT_S2P0; break;
			case 1:		nID = IDC_EDIT_S2P1; break;
			case 2:		nID = IDC_EDIT_S2P2; break;
			case 3:		nID = IDC_EDIT_S2P3; break;
			case 4:		nID = IDC_EDIT_S2P4; break;
			case 5:		nID = IDC_EDIT_S2P5; break;
			}
		}
		break;
	}

	if(nID == 0)
		return NULL;

	return (CEdit*)GetDlgItem(nID);
}


CEdit* CEditEffectGroupDlg::GetScriptName(uint32 nStage)
{
	switch(nStage)
	{
	case 0: return (CEdit*)GetDlgItem(IDC_EDIT_SCRIPT1); break;
	case 1: return (CEdit*)GetDlgItem(IDC_EDIT_SCRIPT2); break;
	}

	return NULL;
}

CButton* CEditEffectGroupDlg::GetBrowseScript(uint32 nStage)
{
	switch(nStage)
	{
	case 0: return (CButton*)GetDlgItem(IDC_BUTTON_BROWSESCRIPT1); break;
	case 1: return (CButton*)GetDlgItem(IDC_BUTTON_BROWSESCRIPT2); break;
	}

	return NULL;
}

CComboBox* CEditEffectGroupDlg::GetChannel(uint32 nStage)
{
	switch(nStage)
	{
	case 0: return (CComboBox*)GetDlgItem(IDC_COMBO_CHANNEL1); break;
	case 1: return (CComboBox*)GetDlgItem(IDC_COMBO_CHANNEL2); break;
	}

	return NULL;
}


CComboBox* CEditEffectGroupDlg::GetOverride(uint32 nStage)
{
	switch(nStage)
	{
	case 0: return (CComboBox*)GetDlgItem(IDC_COMBO_OVERRIDE1); break;
	case 1: return (CComboBox*)GetDlgItem(IDC_COMBO_OVERRIDE2); break;
	}

	return NULL;
}

CEdit* CEditEffectGroupDlg::GetName()
{
	return (CEdit*)GetDlgItem(IDC_EDIT_NAME);
}
