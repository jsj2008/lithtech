//----------------------------------------------------------------------------------
// CustomInfo.cpp
//----------------------------------------------------------------------------------
#include "CustomInfo.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// Global Variables;
//----------------------------------------------------------------------------------
CustomInfo g_CustomInfo;


//----------------------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------------------
const std::string sPatchSettingsFile = "PatchSettings.cfg";

const char sPatchListSec[]           = "PatchList";
const char sDefHostKey[]             = "DefaultHost";

const char sMonitoredSec[]           = "Monitored";
const char sLastVersionKey[]         = "LastVersion";

const char sFtpPatchInfoSec[]        = "FtpPatchInto";

///const int         DEF_TIMEOUT        = 120000; // 2 minutes is plenty long enough.
///const std::string DEF_PATCH_DESC_EXE = TEXT("notepad.exe");


//----------------------------------------------------------------------------------
// Accessor Fuctions.
//----------------------------------------------------------------------------------
extern "C" CustomInfo* GetCustomInfo(void)
{
	return &g_CustomInfo;
}


//----------------------------------------------------------------------------------
// CustomInfo Constructor.
//----------------------------------------------------------------------------------
CustomInfo::CustomInfo(void)
{
	m_sDisplayName           = "";
	m_sProductName           = "";
	m_sExtraConfig           = "";

	m_sPatchFolder           = "";
	m_sPatchTypes            = "";
///	m_sPatchDescExe          = "";
	m_sCurVersion            = "";
	m_sNewVersion            = "";

	m_pDirSrvrNames          = NULL;
	m_pDirSrvrs              = NULL;

	m_pSelectedPatch         = NULL;
	m_bMixedPatches          = false;
	m_bOptionalUpgrade       = false;

	m_sPatchFile             = "";

	m_pWelcomeHelpProc       = NULL;
	m_pConfigProxyHelpProc   = NULL;
	m_pMotdHelpProc          = NULL;
	m_pOptionalPatchHelpProc = NULL;
	m_pPatchDetailsHelpProc  = NULL;
	m_pSelectHostHelpProc    = NULL;
	m_pDownloadHelpProc      = NULL;
	m_pWebLaunchHelpProc     = NULL;

	m_tMotdTimeout           =  60000; // 1 minute.
	m_tVersionTimeout        = 120000; // 2 minutes.
	m_tPatchTimeout          =  60000; // 1 minute.
	m_tPatchDescTimeout      =  60000; // 1 minute.

	m_bDebug                 = false;

	// Temp code ?????
	m_sDisplayName           = "Empire Earth";
	m_sProductName           = "Tribes2";
	m_sPatchFolder           = ".\\patch";
	m_sPatchTypes            = "opt";
	m_sCurVersion            = "1.0.0.0";
	m_sNewVersion            = "1.0.4.0";
	m_tMotdTimeout           = 30000;
	m_tVersionTimeout        = 45000;
	m_tPatchTimeout          = 45000;
	m_tPatchDescTimeout      = 30000;
}

/*
//----------------------------------------------------------------------------------
// SaveDefaultSelection: Save the specified host (so we can select it next time)
//----------------------------------------------------------------------------------
void CustomInfo::SaveDefaultSelection(const GUIString& sName)
{
	std::string sFile = m_sPatchFolder + sPatchSettingsFile;
	std::string sPatch = sName;
	WritePrivateProfileString(sPatchListSec, sDefHostKey, sPatch.c_str(), sFile.c_str());
}

//----------------------------------------------------------------------------------
// LoadDefaultSelection: Load the default (if present) host.
//----------------------------------------------------------------------------------
GUIString CustomInfo::LoadDefaultSelection(void)
{
	char sName[MAX_PATH];
	std::string sFile = m_sPatchFolder + sPatchSettingsFile;
	GetPrivateProfileString(sPatchListSec, sDefHostKey, TEXT(""), sName, MAX_PATH, sFile.c_str());
	return sName;
}

//----------------------------------------------------------------------------------
// TagFtpDownload: Write the Settings associated with a particular FTP file 
// download.  We use this info to determine if we should attempt a resumed download.
//----------------------------------------------------------------------------------
void CustomInfo::TagFtpDownload(const std::string& sPatchFile, 
								const std::string& sVersion, 
								long nSize, long nCheckSum)
{
	std::string sFile = m_sPatchFolder + sPatchSettingsFile;

	char sInfo[MAX_PATH];
	wsprintf(sInfo, "%s - %d - %d", sVersion.c_str(), nSize, nCheckSum);

	WritePrivateProfileString(sFtpPatchInfoSec, sPatchFile.c_str(), sInfo.c_str(), sFile.c_str());
}

//----------------------------------------------------------------------------------
// MatchFtpDownloadTag: Check to see if the patch info for the specified file 
// matches the existing patch info.  If so, we will resume the download.
//----------------------------------------------------------------------------------
bool CustomInfo::MatchFtpDownloadTag(const std::string& sPatchFile, 
									 const std::string& sVersion, 
									 long nSize, long nCheckSum)
{
	std::string sFile = m_sPatchFolder + sPatchSettingsFile;

	char sInfo[MAX_PATH];
	char sCheck[MAX_PATH];
	wsprintf(sCheck, "%s - %d - %d", sVersion.c_str(), nSize, nCheckSum);

	GetPrivateProfileString(sFtpPatchInfoSec, sPatchFile.c_str(), "", sInfo, sizeof(sInfo), sFile.c_str());

	return lstrcmp(sInfo, sCheck) == 0;
}

//----------------------------------------------------------------------------------
// SaveLastMonitoredVersion: Write the last monitored patch version number.
//----------------------------------------------------------------------------------
void CustomInfo::SaveLastMonitoredVersion(const std::string& sVersion)
{
	std::string sFile = m_sPatchFolder + sPatchSettingsFile;
	WritePrivateProfileString(sMonitoredSec, sLastVersionKey, sVersion.c_str(), sFile.c_str());
}

//----------------------------------------------------------------------------------
// GetLastMonitoredVersion: Fetch the last monitored patch version number.
//----------------------------------------------------------------------------------
std::string CustomInfo::GetLastMonitoredVersion(void)
{
	char sVersion[MAX_PATH];
	std::string sFile = m_sPatchFolder + sPatchSettingsFile;
	GetPrivateProfileString(sMonitoredSec, sLastVersionKey, "0.0.0.0", sVersion, MAX_PATH, sFile.c_str());
	return sVersion;
}
*/

//----------------------------------------------------------------------------------
// ClearPatchList: Empty the patch data list.
//----------------------------------------------------------------------------------
void CustomInfo::ClearPatchList(void)
{
	CPatchDataList::iterator Itr = m_PatchDataList.begin();

	while (Itr != m_PatchDataList.end())
	{
		delete *Itr;
		m_PatchDataList.erase(Itr);
		Itr = m_PatchDataList.begin();
	}
}

//----------------------------------------------------------------------------------
// SetPatchFolder: Set the folder that the patch will be saved into.
//----------------------------------------------------------------------------------
void CustomInfo::SetPatchFolder(const std::string& sFolder)
{
	m_sPatchFolder = sFolder;
	TCHAR cBackSlash = '\\';

	// Make sure it ends with a backslash.
	int nLen = m_sPatchFolder.length();
	int nLastSlash = m_sPatchFolder.rfind(cBackSlash);
	
	if (nLen && nLastSlash != nLen - 1)
		m_sPatchFolder += cBackSlash;
}

//----------------------------------------------------------------------------------
// GetPatchFolder: Fetch the folder that the patch will be saved into.
//----------------------------------------------------------------------------------
std::string CustomInfo::GetPatchFolder(void)
{
	// If no path was specified, use the temp directory.
	if (! m_sPatchFolder.length())
	{
		char sTempDir[MAX_PATH];
		GetTempPath(MAX_PATH, sTempDir);
		SetPatchFolder(sTempDir);
	}

	return m_sPatchFolder;
}

//----------------------------------------------------------------------------------
// GetDirectoryServers: Assemble and return the list od directory servers.
// Note: On some versions of Win95, this will cause a three minute timeout (if the 
// user is not online), so we package this here so we can put in in a thread.
//----------------------------------------------------------------------------------
WONAPI::ServerContextPtr CustomInfo::GetDirectoryServers(void)
{
	m_pDirSrvrs->Clear();

	std::list<std::string>::iterator Itr = m_pDirSrvrNames->begin();

	while (Itr != m_pDirSrvrNames->end())
	{
		std::string sDirSrvr = *Itr;
		m_pDirSrvrs->AddAddress(WONAPI::IPAddr(sDirSrvr));
		++Itr;
	}

	return m_pDirSrvrs;
}
