//----------------------------------------------------------------------------------
// CustomInfo.h
//----------------------------------------------------------------------------------
#ifndef __CustomInfo_H__
#define __CustomInfo_H__

#pragma warning(disable : 4786)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <list>
#include "WONAPI/WONServer/ServerContext.h"
#include "WONGUI/GUIString.h"
#include "WONGUI/ResourceCollection.h"
#include "Resources/SierraUpWRS.h"
#include "PatchData.h"
#include "ResourceManager.h"


namespace WONAPI
{

//----------------------------------------------------------------------------------
// SierraUpHelpCallbackProc.
//----------------------------------------------------------------------------------
extern "C" 
{
	typedef void (*SierraUpHelpCallbackProc)(void); //lint !e761
}


//----------------------------------------------------------------------------------
// CustomInfo.
//----------------------------------------------------------------------------------
class CustomInfo
{
private:
	// Primary application information.
	GUIString                m_sDisplayName;           // Name to present to the user.
	std::string              m_sProductName;           // Name to identify the application.
	std::string              m_sExtraConfig;           // Extra Config for the patch.
	std::string              m_sPatchFolder;           // Folder to save the patch to. 
	std::string              m_sPatchTypes;            // Tells us which types of patches we want.
///	std::string              m_sPatchDescExe;          // Exe used to show patch info files.

	// Version infomation.
	std::string              m_sCurVersion;            // Present version of the application.
	std::string              m_sNewVersion;            // Patch (to be) version of the application.

	// Directory Servers.
	std::list<std::string>*  m_pDirSrvrNames;          // List or Directory Server Names (queue names separately for Win95 socket bug).
	WONAPI::ServerContextPtr m_pDirSrvrs;              // List of Directory Servers.

	// Custom Resource Access.
	ResourceManager          m_ResourceManager;        // Resource Management.

	// Returned Patch Information.
	CPatchDataList           m_PatchDataList;          // List of availible patches.
	CPatchData*              m_pSelectedPatch;         // Patch that the user selected.
	bool                     m_bMixedPatches;          // Are some patches manual and some automatic?
	bool                     m_bOptionalUpgrade;       // Is there an optional upgrade avilible for this version?

	// User Selected, previously downloaded, patch. 
	std::string              m_sPatchFile;             // File (with folder) of the pending patch.

	// Help button callbacks.
	SierraUpHelpCallbackProc m_pWelcomeHelpProc;       // Callback to invoke if the help button is pushed.
	SierraUpHelpCallbackProc m_pConfigProxyHelpProc;   // Callback to invoke if the help button is pushed.
	SierraUpHelpCallbackProc m_pMotdHelpProc;          // Callback to invoke if the help button is pushed.
	SierraUpHelpCallbackProc m_pOptionalPatchHelpProc; // Callback to invoke if the help button is pushed.
	SierraUpHelpCallbackProc m_pPatchDetailsHelpProc;  // Callback to invoke if the help button is pushed.
	SierraUpHelpCallbackProc m_pSelectHostHelpProc;    // Callback to invoke if the help button is pushed.
	SierraUpHelpCallbackProc m_pDownloadHelpProc;      // Callback to invoke if the help button is pushed.
	SierraUpHelpCallbackProc m_pWebLaunchHelpProc;     // Callback to invoke if the help button is pushed.

	// Timeouts.
  	DWORD                    m_tMotdTimeout;           // How long to wait before giving up on the MotD.
	DWORD                    m_tVersionTimeout;        // How long to wait for the Current Version.
	DWORD                    m_tPatchTimeout;          // How long to wait for the Patch.
	DWORD                    m_tPatchDescTimeout;      // How long to wait for the Patch Descriptions.

	// Remote Debugging.
	bool                     m_bDebug;                 // Do we want to enable debug tracing?

public:
	CustomInfo(void);

	inline void SetDisplayName(const GUIString& sName)                       { m_sDisplayName = sName; }
	inline GUIString GetDisplayName(void) const                              { return m_sDisplayName; }

	inline void SetProductName(const std::string& sName)                     { m_sProductName = sName; }
	inline std::string GetProductName(void) const                            { return m_sProductName; }

	inline void SetExtraConfig(const std::string& sExtra)                    { m_sExtraConfig = sExtra; }
	inline std::string GetExtraConfig(void) const                            { return m_sExtraConfig; }

	void SetPatchFolder(const std::string& sDir);
	std::string GetPatchFolder(void);

	inline void SetPatchTypes(const std::string& sTypes)                     { m_sPatchTypes = sTypes; }
	inline std::string GetPatchTypes(void) const                             { return m_sPatchTypes; }

///	inline void SetPatchDescriptionExe(const std::stirng& sExePath)          { m_sPatchDescExe = sExePath; }
///	inline std::string GetPatchDescriptionExe(void) const                    { return m_sPatchDescExe; }

	inline void SetCurVersion(const std::string& sVersion)                   { m_sCurVersion = sVersion; }
	inline std::string GetCurVersion(void) const                             { return m_sCurVersion; }

	inline void SetNewVersion(const std::string& sVersion)                   { m_sNewVersion = sVersion; }
	inline std::string GetNewVersion(void) const                             { return m_sNewVersion; }

	inline void ClearDirectoryServers(void)                                  { m_pDirSrvrNames->clear(); m_pDirSrvrs->Clear(); }
	inline void AddDirectoryServer(const std::string& sDirSrvr)              { m_pDirSrvrNames->push_back(sDirSrvr); }
	inline std::list<std::string>* GetDirectoryServerNames(void) const       { return m_pDirSrvrNames; }
	WONAPI::ServerContextPtr GetDirectoryServers(void);

	inline ResourceManager* GetResourceManager(void)                         { return &m_ResourceManager; }

	void ClearPatchList(void);
	inline void AddPatch(CPatchData* pPatch)                                 { m_PatchDataList.push_back(pPatch); }
	inline CPatchDataList* GetPatchList(void)                                { return &m_PatchDataList; } //lint !e1536

	inline void SetSelectedPatch(CPatchData* pPatch)                         { m_pSelectedPatch = pPatch; }
	inline CPatchData* GetSelectedPatch(void) const                          { return m_pSelectedPatch; }

	inline void SetMixedPatches(bool bMixed)                                 { m_bMixedPatches = bMixed; }
	inline bool GetMixedPatches(void) const                                  { return m_bMixedPatches; }

	inline void SetOptionalUpgrade(bool bOptional)                           { m_bOptionalUpgrade = bOptional; }
	inline bool GetOptionalUpgrade(void) const                               { return m_bOptionalUpgrade; }

	inline void SetPatchFile(std::string sFile)                              { m_sPatchFile = sFile; }
	inline std::string GetPatchFile(void) const                              { return m_sPatchFile; }

	inline void SetWelcomeHelpCallback(SierraUpHelpCallbackProc pProc)       { m_pWelcomeHelpProc = pProc; }
	inline SierraUpHelpCallbackProc GetWelcomeHelpCallback(void) const       { return m_pWelcomeHelpProc; }
	inline void InvokeWelcomeHelpCallback(void)                              { if (m_pWelcomeHelpProc) m_pWelcomeHelpProc(); }

	inline void SetConfigProxyHelpCallback(SierraUpHelpCallbackProc pProc)   { m_pConfigProxyHelpProc = pProc; }
	inline SierraUpHelpCallbackProc GetConfigProxyHelpCallback(void) const   { return m_pConfigProxyHelpProc; }
	inline void InvokeConfigProxyHelpCallback(void)                          { if (m_pConfigProxyHelpProc) m_pConfigProxyHelpProc(); }

	inline void SetMotdHelpCallback(SierraUpHelpCallbackProc pProc)          { m_pMotdHelpProc = pProc; }
	inline SierraUpHelpCallbackProc GetMotdHelpCallback(void) const          { return m_pMotdHelpProc; }
	inline void InvokeMotdHelpCallback(void)                                 { if (m_pMotdHelpProc) m_pMotdHelpProc(); }

	inline void SetOptionalPatchHelpCallback(SierraUpHelpCallbackProc pProc) { m_pOptionalPatchHelpProc = pProc; }
	inline SierraUpHelpCallbackProc GetOptionalPatchHelpCallback(void) const { return m_pOptionalPatchHelpProc; }
	inline void InvokeOptionalPatchHelpCallback(void)                        { if (m_pOptionalPatchHelpProc) m_pOptionalPatchHelpProc(); }

	inline void SetPatchDetailsHelpCallback(SierraUpHelpCallbackProc pProc)  { m_pPatchDetailsHelpProc = pProc; }
	inline SierraUpHelpCallbackProc GetPatchDetailsHelpCallback(void) const  { return m_pPatchDetailsHelpProc; }
	inline void InvokePatchDetailsHelpCallback(void)                         { if (m_pPatchDetailsHelpProc) m_pPatchDetailsHelpProc(); }

	inline void SetSelectHostHelpCallback(SierraUpHelpCallbackProc pProc)    { m_pSelectHostHelpProc = pProc; }
	inline SierraUpHelpCallbackProc GetSelectHostHelpCallback(void) const    { return m_pSelectHostHelpProc; }
	inline void InvokeSelectHostHelpCallback(void)                           { if (m_pSelectHostHelpProc) m_pSelectHostHelpProc(); }

	inline void SetDownloadHelpCallback(SierraUpHelpCallbackProc pProc)      { m_pDownloadHelpProc = pProc; }
	inline SierraUpHelpCallbackProc GetDownloadHelpCallback(void) const      { return m_pDownloadHelpProc; }
	inline void InvokeDownloadHelpCallback(void)                             { if (m_pDownloadHelpProc) m_pDownloadHelpProc(); }

	inline void SetWebLaunchHelpCallback(SierraUpHelpCallbackProc pProc)     { m_pWebLaunchHelpProc = pProc; }
	inline SierraUpHelpCallbackProc GetWebLaunchHelpCallback(void) const     { return m_pWebLaunchHelpProc; }
	inline void InvokeWebLaunchHelpCallback(void)                            { if (m_pWebLaunchHelpProc) m_pWebLaunchHelpProc(); }

	inline void SetMotdTimeout(DWORD tTime)                                  { m_tMotdTimeout = tTime; }
	inline DWORD GetMotdTimeout(void) const                                  { return m_tMotdTimeout; }

	inline void SetVersionTimeout(DWORD tTime)                               { m_tVersionTimeout = tTime; }
	inline DWORD GetVersionTimeout(void) const                               { return m_tVersionTimeout; }

	inline void SetPatchTimeout(DWORD tTime)                                 { m_tPatchTimeout = tTime; }
	inline DWORD GetPatchTimeout(void) const                                 { return m_tPatchTimeout; }

	inline void SetPatchDescTimeout(DWORD tTime)                             { m_tPatchDescTimeout = tTime; }
	inline DWORD GetPatchDescTimeout(void) const                             { return m_tPatchDescTimeout; }

	inline void SetDebug(bool bEnable = true)                                { m_bDebug = bEnable; }
	inline bool GetDebug(void) const                                         { return m_bDebug; }
};


//----------------------------------------------------------------------------------
// Prototypes.
//----------------------------------------------------------------------------------
extern "C" CustomInfo* GetCustomInfo(void);

} // namespace

#endif

