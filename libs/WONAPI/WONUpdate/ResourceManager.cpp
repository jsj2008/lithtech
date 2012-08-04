//----------------------------------------------------------------------------------
// ResourceManager.cpp
//----------------------------------------------------------------------------------
#include "ResourceManager.h"
#include "PatchUtils.h"
#include "CustomInfo.h"
#include "Resources/SierraUpWRS.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------------------
const GUIString sDispNameKey       = "%DISPLAY_NAME%";
const GUIString sHostNameKey       = "%HOST_NAME%";
const GUIString sPatchNameKey      = "%PATCH_NAME%";
const GUIString sHostTxtKey        = "%HOST_TEXT%";
const GUIString sPatchSizeKey      = "%PATCH_SIZE%";
const GUIString sPatchSizeUnitsKey = "%PATCH_SIZE_UNITS%";
const GUIString sTcpPortKey        = "%TCP_PORT%";
const GUIString sCurVersionKey     = "%CUR_VERSION%";
const GUIString sNewVersionKey     = "%NEW_VERSION%";


//----------------------------------------------------------------------------------
// ResourceManager Constructor.
//----------------------------------------------------------------------------------
ResourceManager::ResourceManager(void)
{
	m_hDefaultMem = NULL;
}

//----------------------------------------------------------------------------------
// ResourceManager Destructor.
//----------------------------------------------------------------------------------
ResourceManager::~ResourceManager(void)
{
	if (m_hDefaultMem)
		FreeResource(m_hDefaultMem);
	m_hDefaultMem = NULL;
}

//----------------------------------------------------------------------------------
// SetDefaultResourceModule: Extracts the resource collection from a windows binary 
// resource.
//----------------------------------------------------------------------------------
void ResourceManager::SetDefaultResourceModule(HMODULE hModule, int nID)
{
	if (m_hDefaultMem)
		FreeResource(m_hDefaultMem);

	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(nID), RT_RCDATA);
	m_hDefaultMem = LoadResource(hModule, hResource);
	LPVOID pMemData = LockResource(m_hDefaultMem);
	m_rcDefault.Open(pMemData, SizeofResource(hModule, hResource));

	// Do not free until after PreLoadResources has been called.
}

//----------------------------------------------------------------------------------
// GetImage: Fetch an image from the appropriate resource collection.
//----------------------------------------------------------------------------------
NativeImagePtr ResourceManager::GetImage(int nId)
{
	// 1) Look for the image in the custom resources.
	NativeImagePtr pImage = m_rcCustom.GetImage(nId);

	// 2) Look for the image in the language resources.
	if (! pImage.get())
		pImage = m_rcLanguage.GetImage(nId);

	// 3) Look for the image in the default resources.
	if (! pImage.get())
		pImage = m_rcDefault.GetImage(nId);

	return pImage;
}

//----------------------------------------------------------------------------------
// GetBackgroundImage: Return the specified dialog's background image.  If that 
// image is not present, return the common dialog background image instead.
//----------------------------------------------------------------------------------
NativeImagePtr ResourceManager::GetBackgroundImage(int nId)
{
	// 1) Look for a specific dialog image in the custom resources.
	NativeImagePtr pImage = m_rcCustom.GetImage(nId);

	// 2) Look for a common dialog image in the custom resources.
	if (! pImage.get())
		pImage = m_rcCustom.GetImage(IDB_COMMON_DLG_BACKGROUND);

	// 3) Look for a specific dialog image in the language resources.
	if (! pImage.get())
		pImage = m_rcLanguage.GetImage(nId);

	// 4) Look for a common dialog image in the language resources.
	if (! pImage.get())
		pImage = m_rcLanguage.GetImage(IDB_COMMON_DLG_BACKGROUND);

	// 5) Look for a specific dialog image in the default resources.
	if (! pImage.get())
		pImage = m_rcDefault.GetImage(nId);

	// 6) Look for a common dialog image in the default resources.
	if (! pImage.get())
		pImage = m_rcDefault.GetImage(IDB_COMMON_DLG_BACKGROUND);

	return pImage;
}

//----------------------------------------------------------------------------------
// GetGameLogoImage: Return the specified dialog's game logo image.  If that 
// image is not present, return the common dialog game logo image instead.
//----------------------------------------------------------------------------------
NativeImagePtr ResourceManager::GetGameLogoImage(int nId)
{
	// 1) Look for a specific dialog image in the custom resources.
	NativeImagePtr pImage = m_rcCustom.GetImage(nId);

	// 2) Look for a common dialog image in the custom resources.
	if (! pImage.get())
		pImage = m_rcCustom.GetImage(IDB_COMMON_DLG_GAME_LOGO);

	// 3) Look for a specific dialog image in the language resources.
	if (! pImage.get())
		pImage = m_rcLanguage.GetImage(nId);

	// 4) Look for a common dialog image in the language resources.
	if (! pImage.get())
		pImage = m_rcLanguage.GetImage(IDB_COMMON_DLG_GAME_LOGO);

	// 5) Look for a specific dialog image in the default resources.
	if (! pImage.get())
		pImage = m_rcDefault.GetImage(nId);

	// 6) Look for a common dialog image in the default resources.
	if (! pImage.get())
		pImage = m_rcDefault.GetImage(IDB_COMMON_DLG_GAME_LOGO);

	return pImage;
}

//----------------------------------------------------------------------------------
// GetMpsLogoImage: Return the specified dialog's MPS logo image.  If that 
// image is not present, return the common dialog MPS logo image instead.
//----------------------------------------------------------------------------------
NativeImagePtr ResourceManager::GetMpsLogoImage(void)
{
	// 1) Look for the image in the custom resources.
	NativeImagePtr pImage = m_rcCustom.GetImage(IDB_COMMON_DLG_MPS_LOGO);

	// 2) Look for the image in the language resources.
	if (! pImage.get())
		pImage = m_rcLanguage.GetImage(IDB_COMMON_DLG_MPS_LOGO);

	// 3) Look for the image in the default resources.
	if (! pImage.get())
		pImage = m_rcDefault.GetImage(IDB_COMMON_DLG_MPS_LOGO);

	return pImage;
}

//----------------------------------------------------------------------------------
// GetString: Fetch a string from the appropriate resource collection, then 
// translate any key/value pairs as needed.
//----------------------------------------------------------------------------------
GUIString ResourceManager::GetString(int nId)
{
	CustomInfo* pCustInfo = GetCustomInfo();

	// 1) Look for the image in the custom resources.
	GUIString sString = m_rcCustom.GetString(nId);

	// 2) Look for the image in the language resources.
	if (sString != "")
		sString = m_rcLanguage.GetString(nId);

	// 3) Look for the image in the default resources.
	if (sString != "")
		sString = m_rcDefault.GetString(nId);

	// Replace Key/Value pairs.
	// ?????

	ReplaceSubString(sString, sDispNameKey, pCustInfo->GetDisplayName());
	ReplaceSubString(sString, sCurVersionKey, pCustInfo->GetCurVersion());
	ReplaceSubString(sString, sNewVersionKey, pCustInfo->GetNewVersion());

//????? Temp code:
ReplaceSubString(sString, sPatchSizeKey, "1,723");
ReplaceSubString(sString, sPatchSizeUnitsKey, "kilobytes");
ReplaceSubString(sString, sHostNameKey, "Sierra.com");

	if (sString.find(sTcpPortKey) != -1)
	{
		// Find all of the ports.
		TCHAR sPort[32];
		TCHAR sPorts[1024] = "21, 15101"; // 21 is the Generic FTP outbound port, 15101 is used by SierraUp to update itself.
		std::list<std::string>* pServers = pCustInfo->GetDirectoryServerNames();
		if (pServers)
		{
			UINT nAddresses = pServers->size();
			USHORT* pPorts = new USHORT[nAddresses];
			memset(pPorts, 0, sizeof(USHORT) * nAddresses);
			std::list<std::string>::iterator Itr = pServers->begin();
			int nPorts = 0;
			pPorts[nPorts++] = 21;
			pPorts[nPorts++] = 15101;

			while (Itr != pServers->end())
			{
				GUIString sAddr = *Itr;
				int nPortOffset = sAddr.find(':');
				if (nPortOffset != -1)
				{
					sAddr.erase(0, nPortOffset + 1);
					USHORT nPort = static_cast<USHORT>(atoi(std::string(sAddr).c_str()));
					bool bFoundPort = false;
					for (int i = 0; i < nPorts; i++)
						if (pPorts[i] == nPort)
							bFoundPort = true;
					if (! bFoundPort)
					{
						pPorts[nPorts++] = nPort;
						if (nPorts == 1)
							wsprintf(sPort, "%hd", nPort);
						else
							wsprintf(sPort, ", %hd", nPort);
						strcat(sPorts, sPort);
					}
				}
				Itr++;
			}
			ReplaceSubString(sString, sTcpPortKey, sPorts);
			delete[] pPorts;
		}
		else
			ReplaceSubString(sString, sTcpPortKey, "<Unknown>"); //$$$
	}

	CPatchData* pSelectedPatch = pCustInfo->GetSelectedPatch();
	if (pSelectedPatch)
	{
		ReplaceSubString(sString, sPatchNameKey, pSelectedPatch->GetPatchName());
		ReplaceSubString(sString, sHostNameKey, pSelectedPatch->GetHostName());
		ReplaceSubString(sString, sHostTxtKey, pSelectedPatch->GetHostTxt());

		if (sString.find(sPatchSizeKey) != -1)
		{
			GUIString sUnits = GetString(IDS_BYTES);
			long nPatchSize = pSelectedPatch->GetPatchSize();
			if (nPatchSize > 1024 * 10)
			{
				sUnits = GetString(IDS_KILOBYTES);
				nPatchSize = (nPatchSize + 1023) / 1024;

				if (nPatchSize > 1024 * 10)
				{
					sUnits = GetString(IDS_MEGABYTES);
					nPatchSize = (nPatchSize + 1023) / 1024;
				}
			}
			GUIString sSize = NumToStrWithCommas(nPatchSize);
			ReplaceSubString(sString, sPatchSizeKey, sSize);
			ReplaceSubString(sString, sPatchSizeUnitsKey, sUnits);
		}
	}



	return sString;
}

//----------------------------------------------------------------------------------
// If the supplied string id contains a valid string, append it to the end of the
// supplied info string.
//----------------------------------------------------------------------------------
void ResourceManager::AppendInfoString(GUIString& sInfo, UINT nId)
{
	GUIString sSep = "\n\n";
	GUIString sLine = GetString(nId);

	if (sLine.length() && sLine != " ")
	{
		if (sInfo.length())
			sInfo.append(sSep);
		sInfo.append(sLine);
	}
}

//----------------------------------------------------------------------------------
// Assemble an information string.
//----------------------------------------------------------------------------------
GUIString ResourceManager::BuildInfoString(UINT nFirstId)
{
	GUIString sInfo = "";

	AppendInfoString(sInfo, nFirstId + 0);
	AppendInfoString(sInfo, nFirstId + 1);
	AppendInfoString(sInfo, nFirstId + 2);
	AppendInfoString(sInfo, nFirstId + 3);
	AppendInfoString(sInfo, nFirstId + 4);
	AppendInfoString(sInfo, nFirstId + 5);
	
	return sInfo;
}
