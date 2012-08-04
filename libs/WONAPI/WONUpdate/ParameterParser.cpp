//----------------------------------------------------------------------------------
// ParameterParser.cpp
//----------------------------------------------------------------------------------
#include "ParameterParser.h"
#include "CustomInfo.h"
#include "PatchUtils.h"


using namespace WONAPI;


//----------------------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------------------
const int MAX_PARAM_LEN  = 80000; // Should be massive overkill, but someone will push it.
LPCTSTR sRestartExe      = "SierraUpRestarter.exe";
LPCTSTR sPsapiDll        = "SierraUpPsapi.dll";
LPCTSTR sRestartIniFile  = ".\\SierraUpRestarter.ini";
LPCTSTR sSelfPatchFolder = ".\\_SierraUpdatePatch";
LPCTSTR sRestartWaitTime = "1000";


//----------------------------------------------------------------------------------
// Globals.
//----------------------------------------------------------------------------------
static GUIString g_sDefaultConfigFile      = "SierraUp.cfg";
static GUIString g_sConfigFileKey          = "ConfigFile";
static GUIString g_sWorkingFolderKey       = "WorkingFolder";
static GUIString g_sProductNameKey         = "ProductName";
static GUIString g_sDisplayNameKey         = "DisplayName";
static GUIString g_sPatchFolderKey         = "PatchFolder";
static GUIString g_sCurrentVerKey          = "CurrentVersion";
static GUIString g_sUpdateVersionKey       = "UpdateVersion";
static GUIString g_sPatchTypesKey          = "PatchTypes";
static GUIString g_sDirServerKey           = "DirectoryServer";
static GUIString g_sWelcomeDlgHlpFileKey   = "WelcomeDlgHelpFile";
static GUIString g_sOptPatchDlgHlpFileKey  = "OptionalPatchDlgHelpFile";
static GUIString g_sSelectDlgHlpFileKey    = "SelectDlgHelpFile";
static GUIString g_sDownloadDlgHlpFileKey  = "DownloadDlgHelpFile";
static GUIString g_sProxyDlgHlpFileKey     = "ProxyDlgHelpFile";
static GUIString g_sVisitHostDlgHlpFileKey = "VisitHostDlgHelpFile";
static GUIString g_sHelpExeKey             = "HelpExe";
static GUIString g_sResourceDllFileKey     = "ResourceDllFile";
static GUIString g_sLanguageDllFileKey     = "LanguageDllFile";
static GUIString g_sExtraConfigKey         = "ExtraConfig";
static GUIString g_sAutoStartKey           = "AutoStart";
static GUIString g_sMonitorPatchKey        = "MonitorPatch";
static GUIString g_sPatchSucceededValKey   = "PatchSucceededValue";
static GUIString g_sVersionExeKey          = "VersionExe";
static GUIString g_sLaunchExeKey           = "LaunchExe";
static GUIString g_sLaunchParamsKey        = "LaunchParams";
static GUIString g_sMotdTimeoutKey         = "MotdTimeout";
static GUIString g_sVersionTimeoutKey      = "VersionTimeout";
static GUIString g_sPatchTimeoutKey        = "PatchTimeout";
static GUIString g_sPatchDescTimeoutKey    = "PatchDescriptionTimeout";
static GUIString g_sPatchDescExeKey        = "PatchDescriptionExe";
static GUIString g_sDebugKey               = "Debug";
static GUIString g_sValidateVerFileKey     = "ValidateVersionFile";
static GUIString g_sNoPatchUrlKey          = "NoPatchURL";

static GUIString g_sNoPatchUrlTag          = "%NO_PATCH_URL%";

/*
//----------------------------------------------------------------------------------
// Static member variables.
//----------------------------------------------------------------------------------
CString CSierraUpDlg::m_sWelcomeDlgHelpFile;
CString CSierraUpDlg::m_sOptPatchDlgHelpFile;
CString CSierraUpDlg::m_sSelectDlgHelpFile;
CString CSierraUpDlg::m_sProxyDlgHelpFile;
CString CSierraUpDlg::m_sDownloadDlgHelpFile;
CString CSierraUpDlg::m_sVisitHostDlgHelpFile;
CString CSierraUpDlg::m_sHelpExe;
CString CSierraUpDlg::m_sExtraConfig;
CString CSierraUpDlg::m_sLanguageDllFile;
CString CSierraUpDlg::m_sResourceDllFile;
*/


//----------------------------------------------------------------------------------
// IsStartOfLine: ???
//----------------------------------------------------------------------------------
bool ParameterParser::IsStartOfLine(const GUIString& sFileContents, int nOffset)
{
	if (nOffset == 0)
		return true;

	return (sFileContents.at(nOffset - 1) == '\r' || sFileContents.at(nOffset - 1) == '\n');
}

//----------------------------------------------------------------------------------
// FindNextComment: ???
//----------------------------------------------------------------------------------
int ParameterParser::FindNextComment(const GUIString& sFileContents, int nStartPos)
{
	// Semi colens and double slashes start comments.
	int nComment = -1;
	int nSemiColen = sFileContents.find(';', nStartPos);
	int nDblSlash = sFileContents.find("//", nStartPos);

	if (nSemiColen != -1)
		while (! IsStartOfLine(sFileContents, nSemiColen))
			nSemiColen = sFileContents.find(';', nSemiColen + 1);

	if (nDblSlash != -1)
		while (! IsStartOfLine(sFileContents, nDblSlash))
			nDblSlash = sFileContents.find("//", nDblSlash + 1);

	if (nSemiColen != -1 && nDblSlash != -1)
		return min(nSemiColen, nDblSlash);
	else if (nSemiColen != -1)
		return nSemiColen;
	else 
		return nDblSlash;
}

//----------------------------------------------------------------------------------
// FindNextLine: ???
//----------------------------------------------------------------------------------
int ParameterParser::FindNextLine(const GUIString& sFileContents, int nStartPos)
{
	// Semi colens and double slashes start comments.
	int nComment = -1;
	int nCR = sFileContents.find('\r', nStartPos);
	int nNL = sFileContents.find('\n', nStartPos);

	if (nCR != -1 && nNL != -1)
		return min(nNL, nCR) + 1;
	else if (nCR != -1)
		return nCR + 1;
	else if (nNL != -1)
		return nNL + 1;
	else
		return -1;
}

//----------------------------------------------------------------------------------
// CopyLine: ???
//----------------------------------------------------------------------------------
GUIString ParameterParser::CopyLine(const GUIString& sFileContents, int nStart)
{
	GUIString sResult;

	while(sFileContents.at(nStart) && sFileContents.at(nStart) != '\r' && sFileContents.at(nStart) != '\n')
		sResult.append(sFileContents.at(nStart++));

	return sResult;
}

//----------------------------------------------------------------------------------
// RemoveComments: Strip comments from the config file contents.
//----------------------------------------------------------------------------------
void ParameterParser::RemoveComments(GUIString& sFileContents)
{
	int nNextComment = FindNextComment(sFileContents, 0);

	while (nNextComment != -1)
	{
		int nEOL = nNextComment;
		while (sFileContents.at(nEOL) && (sFileContents.at(nEOL) != '\r' && sFileContents.at(nEOL) != '\n'))
			++nEOL;
		if (sFileContents.at(nEOL)) // skip the cr/lf.
			++nEOL;

		sFileContents.erase(nNextComment, nEOL - nNextComment);
		nNextComment = FindNextComment(sFileContents, nNextComment);
	}
}

//----------------------------------------------------------------------------------
// RemoveBlankLines: Strip blank lines from the config file contents.
//----------------------------------------------------------------------------------
void ParameterParser::RemoveBlankLines(GUIString& sFileContents)
{
	int nOffset = 0;
	int nReturn = FindNextLine(sFileContents, 0);

	while (nReturn != -1)
	{
		if (nReturn == nOffset)
			sFileContents.erase(nReturn, 1);
		else
			nOffset = nReturn;

		nReturn = FindNextLine(sFileContents, nReturn);
	}
}

//----------------------------------------------------------------------------------
// RemoveExtraIniInfo: Handle the case where we are embeded within a standard .ini 
// file.  Half-life requested this to stay backward compatible and to avoid having 
// to keep multiple installer files up to date.  It seemed reasonable, so I added 
// it.
//----------------------------------------------------------------------------------
void ParameterParser::RemoveExtraIniInfo(GUIString& sFileContents)
{
	const GUIString sSierraUpSec = "[SierraUp]";

	bool bDiscardingSection = false;

	int nOffset = 0;
	int nLine = nOffset;

	while (nLine != -1)
	{
		if (nLine < sFileContents.length() && sFileContents.at(nLine) == '[')
		{
			GUIString sLine = CopyLine(sFileContents, nLine);

			if (sLine.find(sSierraUpSec) == 0)
			{
				if (bDiscardingSection)
				{
					// Remove everything up to and including the marker.
					nLine = FindNextLine(sFileContents, nLine); // Consume the marker.
					sFileContents.erase(nLine, nLine - nOffset);
					bDiscardingSection = false;
				}
			}
			else if (! bDiscardingSection)
			{
				nOffset = nLine;
				bDiscardingSection = true;
			}
		}

		nLine = FindNextLine(sFileContents, nLine);
	}

	// Discard everything after the SierraUp Section.
	if (bDiscardingSection)
		sFileContents.erase(nOffset, sFileContents.length() - nOffset);
}

//----------------------------------------------------------------------------------
// SetLastError: Set the last error (so we can display it later).
//----------------------------------------------------------------------------------
bool ParameterParser::SetLastError(const GUIString& sFormat, ...)
{
	TCHAR sError[MAX_PARAM_LEN] = "";

	// Initialize variable arguments.
	va_list ArgList;
	va_start(ArgList, sFormat); //lint !e1924

	// Format the string.
	wvsprintf(sError, std::string(sFormat).c_str(), ArgList);

	// Reset variable arguments.
	va_end(ArgList); //lint !e1924

	m_sLastError = sError;

	return m_sLastError.length() == 0;
}

//----------------------------------------------------------------------------------
// LoadConfigFile: Load the configuration file, and read all parameters from it.
//----------------------------------------------------------------------------------
bool ParameterParser::LoadConfigFile(const std::string& sFile)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

	FILE* pFile = fopen(sFile.c_str(), "rt+");
	if (pFile)
	{
		TCHAR sRawContents[MAX_PARAM_LEN] = "";
		UINT nRead = static_cast<UINT>(fread(sRawContents, 1, sizeof(sRawContents) - 1, pFile));
		if (nRead == sizeof(sRawContents) - 1)
		{
			TCHAR sRead[32];
			wsprintf(sRead, "%d", nRead);
			GUIString sFormat = pResMgr->GetString(IDS_CFG_FILE_TOO_BIG);

			ReplaceSubString(sFormat, "%FILE%", sFile);
			ReplaceSubString(sFormat, "%SIZE%", sRead);

			SetLastError(sFormat);
			fclose(pFile);
			return false;
		}
		if (nRead > 0)
		{
			sRawContents[nRead] = 0;
			GUIString sFileContents = sRawContents;
			RemoveComments(sFileContents);
			ExtractParameters(sFileContents);
			fclose(pFile);
			return m_sLastError.length() == 0;
		}

		SetLastError(pResMgr->GetString(IDS_NO_READ_CFG_FILE), sFile);
		fclose(pFile);
		return false;
	}
	else
	{
		SetLastError(pResMgr->GetString(IDS_NO_OPEN_CFG_FILE), sFile);
		return false;
	}
}

/*
//----------------------------------------------------------------------------------
// UpdateConfigFileCurrentVersion: Update the current version number in the 
// supplied config file.
//
// Note: If the current version is not set in the file, it will NOT be set by the
// call to ReplaceParameter.
//----------------------------------------------------------------------------------
bool ParameterParser::UpdateConfigFileCurrentVersion(const std::string& sFile, const GUIString& sVersion)
{
//	DebugLog("Updating version in config file to %s.\n", sVersion);

	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

	FILE* pFile = fopen(sFile.c_str(), "rt+");
	if (pFile)
	{
		TCHAR sRawContents[MAX_PARAM_LEN] = "";
		UINT nRead = static_cast<UINT>(fread(sRawContents, 1, sizeof(sRawContents) - 1, pFile));
		if (nRead == sizeof(sRawContents) - 1)
		{
			TCHAR sRead[32];
			wsprintf(sRead, "%d", nRead);
			GUIString sFormat = pResMgr->GetString(IDS_CFG_FILE_TOO_BIG);

			ReplaceSubString(sFormat, "%FILE%", sFile);
			ReplaceSubString(sFormat, "%SIZE%", sRead);

			SetLastError(sFormat);
			fclose(pFile);
			return false;
		}
		if (nRead > 0)
		{
			sRawContents[nRead] = 0;
			GUIString sFileContents = sRawContents;
			fclose(pFile);

			// Replace the existing version.
			if (! ReplaceParameter(sFileContents, g_sCurrentVerKey, sVersion))
			{
				// We did not find it (to replace it), so add it to the end.
				GUIString sVersionLine = TEXT("\n") + g_sCurrentVerKey + TEXT(" ") + sVersion; //lint !e55 !e56

				if (nRead + sVersionLine.length() >= MAX_PARAM_LEN)
				{
					DebugLog("Config file too big, unable to append version.\n");
					return false;
				}
				sFileContents += sVersionLine;
				DebugLog("Appended version %s to config file.\n", sVersion);
			}

			hFile = _open(sFile, _O_WRONLY | _O_TEXT);
			if (hFile != -1)
			{
				_write(hFile, sFileContents, sFileContents.length());
				fclose(pFile);
				return true;
			}
			else
			{
				// Could not write to the file...
				DebugLog("Unable to write config file (to update version).  File may be locked.\n");
				return false;
			}
		}

		DebugLog("Unable to read config file (to update version).\n");
		SetLastError(pResMgr->GetString(IDS_NO_READ_CFG_FILE), sFile);
		fclose(pFile);
		return false;
	}
	else
	{
		DebugLog("Unable to open config file (to update version).\n");
		SetLastError(pResMgr->GetString(IDS_NO_OPEN_CFG_FILE), sFile);
		return false;
	}
}
*/

//----------------------------------------------------------------------------------
// ExtractConfigFile: Extract the configuration file, and read all parameters from 
// it.
//----------------------------------------------------------------------------------
bool ParameterParser::ExtractConfigFile(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sConfigFileKey, "");
	if (sVal.length())
		return LoadConfigFile(sVal);

	return true; // No file specified, we are fine.
}

//----------------------------------------------------------------------------------
// ExtractWorkingFolder: Extract the Working Folder from the command line/config 
// file.
//----------------------------------------------------------------------------------
bool ParameterParser::ExtractWorkingFolder(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sWorkingFolderKey, "");
	if (sVal.length())
		return SetCurrentDirectory(std::string(sVal).c_str()) == TRUE;
	return true;
}

//----------------------------------------------------------------------------------
// ExtractProductName: Extract the Product Name (ID) from the command line/config 
// file.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractProductName(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sProductNameKey, "");
	if (sVal.length())
		GetCustomInfo()->SetProductName(sVal);
}

//----------------------------------------------------------------------------------
// ExtractDisplayName: Extract the Display Name (for GUI) from the command line/
// config file.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractDisplayName(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sDisplayNameKey, "");
	if (sVal.length())
		GetCustomInfo()->SetDisplayName(sVal);
}

//----------------------------------------------------------------------------------
// ExtractPatchFolder: Extract the Patch Folder from the command line/config file.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractPatchFolder(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sPatchFolderKey, "");
	if (sVal.length())
		GetCustomInfo()->SetPatchFolder(sVal);
}

//----------------------------------------------------------------------------------
// ExtractUpdateVersion: Extract the update version from the command line (n/a to 
// config file).
//----------------------------------------------------------------------------------
void ParameterParser::ExtractUpdateVersion(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sUpdateVersionKey, "");
	if (sVal.length())
		GetCustomInfo()->SetUpdateVersion(sVal);
}

//----------------------------------------------------------------------------------
// ExtractPatchTypes: Extract the patch types string.  This string is passed 
// diorectly to the GetVerOp, so we don't parse it ourself.  If this string 
// contains all of the types for a given patch, that ppatch will be considered.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractPatchTypes(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sPatchTypesKey, "");
	if (sVal.length())
		GetCustomInfo()->SetPatchTypes(sVal);
}

//----------------------------------------------------------------------------------
// ExtractCurrentVersion: Extract the current version from the command line/config 
// file.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractCurrentVersion(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sCurrentVerKey, "");
	if (sVal.length())
		GetCustomInfo()->SetCurVersion(sVal);
}

//----------------------------------------------------------------------------------
// ExtractDirectoryServers: Extract the Directory Servers from the command line/
// config file.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractDirectoryServers(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sDirServerKey, "");
	while (sVal.length())
	{
//		m_DirectoryServers.push_back(sVal);
		GetCustomInfo()->AddDirectoryServer(sVal);
		sVal = ExtractParam(sParams, g_sDirServerKey, "");
	}
}

//----------------------------------------------------------------------------------
// ExtractHelpInfo: Extract the Select Dialog Help File from the command line/
// config file.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractHelpInfo(GUIString& sParams)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

	GUIString sVal = ExtractParam(sParams, g_sWelcomeDlgHlpFileKey, "");
	if (sVal.length())
	{
		// Do range checking.
		if (! FileExists(sVal))
			SetLastError(pResMgr->GetString(IDS_BAD_HELP_FILE), sVal);
//		else
//			m_sWelcomeDlgHelpFile = sVal;
	}

	sVal = ExtractParam(sParams, g_sOptPatchDlgHlpFileKey, "");
	if (sVal.length())
	{
		// Do range checking.
		if (! FileExists(sVal))
			SetLastError(pResMgr->GetString(IDS_BAD_HELP_FILE), sVal);
//		else
//			m_sOptPatchDlgHelpFile = sVal;
	}

	sVal = ExtractParam(sParams, g_sSelectDlgHlpFileKey, "");
	if (sVal.length())
	{
		// Do range checking.
		if (! FileExists(sVal))
			SetLastError(pResMgr->GetString(IDS_BAD_HELP_FILE), sVal);
//		else
//			m_sSelectDlgHelpFile = sVal;
	}

	sVal = ExtractParam(sParams, g_sProxyDlgHlpFileKey, "");
	if (sVal.length())
	{
		// Do range checking.
		if (! FileExists(sVal))
			SetLastError(pResMgr->GetString(IDS_BAD_HELP_FILE), sVal);
//		else
//			m_sProxyDlgHelpFile = sVal;
	}

	sVal = ExtractParam(sParams, g_sDownloadDlgHlpFileKey, "");
	if (sVal.length())
	{
		// Do range checking.
		if (! FileExists(sVal))
			SetLastError(pResMgr->GetString(IDS_BAD_HELP_FILE), sVal);
//		else
//			m_sDownloadDlgHelpFile = sVal;
	}

	sVal = ExtractParam(sParams, g_sVisitHostDlgHlpFileKey, "");
	if (sVal.length())
	{
		// Do range checking.
		if (! FileExists(sVal))
			SetLastError(pResMgr->GetString(IDS_BAD_HELP_FILE), sVal);
//		else
//			m_sVisitHostDlgHelpFile = sVal;
	}

	sVal = ExtractParam(sParams, g_sHelpExeKey, "");
	if (sVal.length())
	{
////		// Do range checking.
////		if (! FileExists(sVal))
////			SetLastError(pResMgr->GetString(IDS_BAD_HELP_FILE), sVal);
////		else
//			m_sHelpExe = sVal;
	}
}

//----------------------------------------------------------------------------------
// ExtractExtraConfig: Extract the language that the application has been 
// localaized to.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractExtraConfig(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sExtraConfigKey, "");
	if (sVal.length())
	{
		GetCustomInfo()->SetExtraConfig(sVal);
		return;
	}

	// Historic - for original versions.
	sVal = ExtractParam(sParams, "Language", "");
	if (sVal.length())
	{
//		DebugLog("Old parameter 'Language' specified, should be changed to 'ExtraConfig'\n");
		GetCustomInfo()->SetExtraConfig(sVal);
	}
}

//----------------------------------------------------------------------------------
// ExtractAutoStart: Extract the switch telling us to auto start the application.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractAutoStart(GUIString& sParams)
{
	if (ExtractValuelessParam(sParams, g_sAutoStartKey))
		GetCustomInfo()->SetAutoStart(true);
}

//----------------------------------------------------------------------------------
// ExtractMonitorPatch: Extract the switch telling us to moinitor the patch (don't 
// shut down).
//----------------------------------------------------------------------------------
void ParameterParser::ExtractMonitorPatch(GUIString& sParams)
{
	if (ExtractValuelessParam(sParams, g_sMonitorPatchKey))
		GetCustomInfo()->SetMonitorPatch(true);
}

//----------------------------------------------------------------------------------
// ExtractPatchSucceededValue: Extract the switch telling us to moinitor the patch 
// (don't shut down).
//----------------------------------------------------------------------------------
void ParameterParser::ExtractPatchSucceededValue(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sPatchSucceededValKey, "");
	if (sVal.length())
		GetCustomInfo()->SetPatchSuccessValue(atoi(std::string(sVal).c_str()));
}

//----------------------------------------------------------------------------------
// ExtractVersionExe: Extract the executble to extract the version from if a patch 
// is applied, and we are monitoring the patch.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractVersionExe(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sVersionExeKey, "");
	if (sVal.length())
		GetCustomInfo()->SetVersionExe(sVal);
}

//----------------------------------------------------------------------------------
// ExtractLaunchExe: Extract the executble to run if the patch is up to date.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractLaunchExe(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sLaunchExeKey, "");
	if (sVal.length())
		GetCustomInfo()->SetLaunchExe(sVal);

	// These are cumulative: from the cmd line and cfg file (so keep adding them).
	sVal = ExtractParam(sParams, g_sLaunchParamsKey, "");
	if (sVal.length())
		GetCustomInfo()->AppendLaunchParameters(sVal);
}

//----------------------------------------------------------------------------------
// ExtractLanguageResourceDllFile: Extract the DLL that contains internationalized 
// resources.
//
// Note: This method is depricated.  It will translate the DLL into a WRS file and 
// update the config file to invoke the WRF file instead.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractLanguageResourceDllFile(GUIString& sParams)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

	GUIString sVal = ExtractParam(sParams, g_sLanguageDllFileKey, "");
	if (sVal.length())
	{
		if (! FileExists(sVal))
			SetLastError(pResMgr->GetString(IDS_BAD_RES_FILE), sVal);
		else
		{
//			m_sLanguageDllFile = sVal;
//			gApp.GetAppInfo().SetLanguageResourceDll(m_sLanguageDllFile);

//			HICON hIcon = gApp.GetAppInfo().GetIcon(IDI_APP);
//			if (hIcon)
//			{
//				DestroyIcon(m_hIcon);
//				m_hIcon = hIcon;
//				SetIcon(m_hIcon, TRUE);  // Set big icon.
//				SetIcon(m_hIcon, FALSE); // Set small icon.
//			}
		}
	}
}

//----------------------------------------------------------------------------------
// ExtractCustomResourceDllFile: Extract the DLL that contains customized 
// resources.
//
// Note: This method is depricated.  It will translate the DLL into a WRS file and 
// update the config file to invoke the WRF file instead.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractCustomResourceDllFile(GUIString& sParams)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

	GUIString sVal = ExtractParam(sParams, g_sResourceDllFileKey, "");
	if (sVal.length())
	{
		if (! FileExists(sVal))
			SetLastError(pResMgr->GetString(IDS_BAD_RES_FILE), sVal);
		else
		{
//			m_sResourceDllFile = sVal;
//			pResMgr->SetCustomResourceFile(m_sResourceDllFile); //????? was a dll - need to translate it.

//			HICON hIcon = gApp.GetAppInfo().GetIcon(IDI_APP);
//			if (hIcon && m_hWnd)
//			{
//				DestroyIcon(m_hIcon);
//				m_hIcon = hIcon;
//				SetIcon(m_hIcon, TRUE);  // Set big icon.
//				SetIcon(m_hIcon, FALSE); // Set small icon.
//			}
		}
	}
}

//----------------------------------------------------------------------------------
// ExtractResourceFiles: ???
//----------------------------------------------------------------------------------
void ParameterParser::ExtractResourceFiles(GUIString& sParams)
{
	// New style (WONGUI) resource files.
//	ExtractLanguageResourceFile(sParams);
//	ExtractCustomResourceFile(sParams);  // Must follow Language to over-ride the icon.

	// These will be converted to WONGUI resources.
	ExtractLanguageResourceDllFile(sParams);
	ExtractCustomResourceDllFile(sParams);  // Must follow Language to over-ride the icon.
}

//----------------------------------------------------------------------------------
// ExtractValidateVersionFile: Do we need to do an extra version check in a text 
// file?
//
// Tribes2 uses this mechanism to make sure their patches completed.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractValidateVersionFile(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sValidateVerFileKey, "");
	if (sVal.length())
		GetCustomInfo()->SetValidateVersionFile(sVal);
}

//----------------------------------------------------------------------------------
// ExtractNoPatchURL: ???
//----------------------------------------------------------------------------------
void ParameterParser::ExtractNoPatchURL(GUIString& sParams)
{
	GUIString sVal = ExtractParam(sParams, g_sNoPatchUrlKey, "");
	if (sVal.length())
		GetCustomInfo()->SetNoPatchURL(sVal);
}

//----------------------------------------------------------------------------------
// ExtractTimeouts: Extract the operation timeout values.
//----------------------------------------------------------------------------------
void ParameterParser::ExtractTimeouts(GUIString& sParams)
{
	CustomInfo* pCustInfo = GetCustomInfo();

	GUIString sVal = ExtractParam(sParams, g_sMotdTimeoutKey, "");
	if (sVal.length())
		pCustInfo->SetMotdTimeout(static_cast<DWORD>(atoi(std::string(sVal).c_str())));

	sVal = ExtractParam(sParams, g_sVersionTimeoutKey, "");
	if (sVal.length())
		pCustInfo->SetVersionTimeout(static_cast<DWORD>(atoi(std::string(sVal).c_str())));

	sVal = ExtractParam(sParams, g_sPatchTimeoutKey, "");
	if (sVal.length())
		pCustInfo->SetPatchTimeout(static_cast<DWORD>(atoi(std::string(sVal).c_str())));

	sVal = ExtractParam(sParams, g_sPatchDescTimeoutKey, "");
	if (sVal.length())
		pCustInfo->SetPatchDescTimeout(static_cast<DWORD>(atoi(std::string(sVal).c_str())));
}

/////----------------------------------------------------------------------------------
///// ExtractPatchDescriptionExe: Extract the executable to use to display the patch 
///// description files.
/////----------------------------------------------------------------------------------
///void ParameterParser::ExtractPatchDescriptionExe(GUIString& sParams)
///{
///	GUIString sVal = ExtractParam(sParams, g_sPatchDescExeKey, "");
///	if (sVal.length())
///		GetCustomInfo()->SetPatchDescriptionExe(sVal);
///}

//----------------------------------------------------------------------------------
// ExtractDebug: Extract the debug flag
//----------------------------------------------------------------------------------
void ParameterParser::ExtractDebug(GUIString& sParams)
{
	if (ExtractValuelessParam(sParams, g_sDebugKey))
		GetCustomInfo()->SetDebug(true);
}

//----------------------------------------------------------------------------------
// ExtractParameters: Extract all parameters (except config file).
//----------------------------------------------------------------------------------
void ParameterParser::ExtractParameters(GUIString& sParams)
{
	// Handle the case where we are embeded within a standard .ini file.  Half-life 
	// requested this to stay backward compatible and to avoid having to keep 
	// multiple installer files up to date.
	RemoveExtraIniInfo(sParams);

	// Extract the debug flag first.
	ExtractDebug(sParams);

	// If specified, extract parameters from a config file (this can cause recursion).
	ExtractConfigFile(sParams);

	// Extract the various parameters (override the config file if appropriate).
	ExtractResourceFiles(sParams);
	ExtractProductName(sParams);
	ExtractDisplayName(sParams);
	ExtractPatchFolder(sParams);
	ExtractPatchTypes(sParams);
	ExtractCurrentVersion(sParams);
	ExtractDirectoryServers(sParams);
	ExtractHelpInfo(sParams);
	ExtractExtraConfig(sParams);
	ExtractAutoStart(sParams);
	ExtractMonitorPatch(sParams);
	ExtractPatchSucceededValue(sParams);
	ExtractVersionExe(sParams);
	ExtractLaunchExe(sParams);
	ExtractValidateVersionFile(sParams);
	ExtractNoPatchURL(sParams);
	ExtractTimeouts(sParams);
///	ExtractPatchDescriptionExe(sParams);

	// Strip end-of-line comments.
	RemoveComments(sParams);
	RemoveBlankLines(sParams);

	// Look for leftovers.
	if (m_sLastError.length() == 0)
	{
		int nPos = sParams.findOneOf(TEXT(" \t\r\n"));
		while (nPos != -1)
		{
			sParams.erase(nPos, 1);
			nPos = sParams.findOneOf(TEXT(" \t\r\n"));
		}

//		if (sParams.length())
//			DebugLog(GetCustomInfo()->GetResourceManager()->GetString(IDS_UNKNOWN_PARAM), sParams);
	}
}

//----------------------------------------------------------------------------------
// ParseCommandLine: Parse the command line.
//----------------------------------------------------------------------------------
bool ParameterParser::ParseCommandLine(const GUIString& sCmdLine)
{
	GUIString sParams = sCmdLine;

	SetLastError("");

	// Remove the first parameter - it is our exe name and should not be considered.
	TCHAR cBreak = ' ';
	if (sParams.at(0) == '\"')
	{
		sParams.erase(0, 1);
		cBreak = '\"';
	}
	int nStop = sParams.find(cBreak);
	if (nStop != -1)
		nStop = sParams.length();

	sParams.erase(0, nStop);

	// Now parse what is left.
	ExtractParameters(sParams);

	return m_sLastError.length() == 0;
}

//----------------------------------------------------------------------------------
// ValidateAppInfo: Make sure we received all required fields.
//----------------------------------------------------------------------------------
bool ParameterParser::ValidateAppInfo(void)
{
	CustomInfo* pCustInfo = GetCustomInfo();
	ResourceManager* pResMgr = pCustInfo->GetResourceManager();

	if (pCustInfo->GetProductName().length() == 0)
		return SetLastError(pResMgr->GetString(IDS_IP_NO_PROD_NAME));

	if (pCustInfo->GetDisplayName().length() == 0)
		return SetLastError(pResMgr->GetString(IDS_IP_NO_DISP_NAME));

	if (pCustInfo->GetPatchFolder().length() == 0)
		return SetLastError(pResMgr->GetString(IDS_IP_NO_PATCH_FOLDER));

	if (pCustInfo->GetCurVersion().length() == 0)
		return SetLastError(pResMgr->GetString(IDS_IP_NO_VERSION));

	if (pCustInfo->GetDirectoryServerNames()->size() == 0)
		return SetLastError(pResMgr->GetString(IDS_IP_NO_DIR_SERVER));

	// The other fields are optional.

	return true;
}
