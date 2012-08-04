//////////////////////////////////////////////////////////////////////////////
// WON testing stuff

#include "stdafx.h"

#include "iltclient.h"

#pragma warning (disable:4503)
#include "wonapi.h"
#include "wonauth/authcontext.h"
#include "wondir/getdirop.h"
#include "wondir/addentityop.h"
#include "wondir/renewentityop.h"
#include "wondir/removeentityop.h"
#include "wonauth/getcertop.h"
#include "wonmisc/getmotdop.h"
#include "wonmisc/checkvalidversionop.h"

#include <malloc.h>

extern ILTCSBase* g_pLTCSBase;
extern ILTClient* g_pLTClient;

using namespace WONAPI;

WONAPICore *g_pWONCore = 0;

std::wstring g_sWONCommunity = L"NOLF2";
std::string g_sWONProduct = "NOLF2";
std::wstring g_sWONCurPath = L"/NOLF2";
typedef std::list<std::string> TWONServerList;
TWONServerList g_lWONServerList;
std::string g_sWONCDKey = "";

static std::wstring Make_wstring(const char *pCharString)
{
	uint32 nStrLen = strlen(pCharString);
	wchar_t *pTempBuffer = (wchar_t*)alloca(sizeof(wchar_t) * (nStrLen + 1));

	// Convert it into our temporary buffer
	const char *pInFinger = pCharString;
	wchar_t *pOutFinger = pTempBuffer;
	wchar_t *pOutEnd = pTempBuffer + nStrLen;
	while (pOutFinger != pOutEnd)
	{
		*pOutFinger = btowc(*pInFinger);
		++pOutFinger;
		++pInFinger;
	}

	*pOutFinger = 0;

	// Save it
	std::wstring sResult = pTempBuffer;
	return sResult;
}

static std::string Make_string(const wchar_t *pCharString)
{
	uint32 nStrLen = wcslen(pCharString);
	char *pTempBuffer = (char*)alloca(sizeof(char) * (nStrLen + 1));

	// Convert it into our temporary buffer
	const wchar_t *pInFinger = pCharString;
	char *pOutFinger = pTempBuffer;
	char *pOutEnd = pTempBuffer + nStrLen;
	while (pOutFinger != pOutEnd)
	{
		*pOutFinger = wctob(*pInFinger);
		++pOutFinger;
		++pInFinger;
	}

	*pOutFinger = 0;

	// Save it
	std::string sResult = pTempBuffer;
	return sResult;
}

ServerContextPtr WONGetCurrentServerContext(const TWONServerList &lServers)
{
	// Get the server list
	ServerContextPtr cDirContext = new ServerContext;
	TWONServerList::const_iterator iCurDirServer = lServers.begin();
	for (; iCurDirServer != g_lWONServerList.end(); ++iCurDirServer)
	{
		cDirContext->AddAddress(IPAddr(*iCurDirServer));
	}

	return cDirContext;
}

void WONQueryDir(const char *pPath, bool bRecurse = false)
{
	GetDirOpPtr cGetDir = new GetDirOp(WONGetCurrentServerContext(g_lWONServerList));

	cGetDir->SetPath(g_sWONCurPath + Make_wstring("/") + Make_wstring(pPath));
	cGetDir->SetFlags(
		DIR_GF_DECOMPSERVICES |
		DIR_GF_DECOMPSUBDIRS |
		((bRecurse) ? DIR_GF_DECOMPRECURSIVE : 0) |
		DIR_GF_ADDTYPE |
		DIR_GF_ADDDISPLAYNAME |
		DIR_GF_DIRADDNAME |
		DIR_GF_DIRADDPATH |
		DIR_GF_SERVADDPATH |
		DIR_GF_SERVADDNAME);
	cGetDir->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE);
	if (cGetDir->GetStatus() != WS_Success)
	{
		g_pLTCSBase->CPrint("Directory not found (%s)", WONStatusToString(cGetDir->GetStatus()));
		return;
	}

	// Display everything
	const DirEntityList &cFullDir = cGetDir->GetDirEntityList();
	DirEntityList::const_iterator iFullDirEntry = cFullDir.begin();
	for (; iFullDirEntry != cFullDir.end(); ++iFullDirEntry)
	{
		const wchar_t *pName = (*iFullDirEntry)->mDisplayName.empty() ? (*iFullDirEntry)->mName.c_str() : (*iFullDirEntry)->mDisplayName.c_str();
		g_pLTCSBase->CPrint("  %ls%s%ls (%c)", (*iFullDirEntry)->mPath.c_str(), (*iFullDirEntry)->mPath.length() == 1 ? "" : "/", pName, (*iFullDirEntry)->mType);
	}
	g_pLTCSBase->CPrint("  <END>");

	return;
}

void WONQueryServices(const char *pPath)
{
	GetDirOpPtr cGetDir = new GetDirOp(WONGetCurrentServerContext(g_lWONServerList));

	cGetDir->SetPath(g_sWONCurPath + Make_wstring("/") + Make_wstring(pPath));
	cGetDir->SetFlags(
		DIR_GF_DECOMPSERVICES |
		DIR_GF_DECOMPRECURSIVE |
		DIR_GF_ADDTYPE |
		DIR_GF_ADDDISPLAYNAME |
		DIR_GF_ADDCREATED |
		DIR_GF_ADDTOUCHED |
		DIR_GF_ADDLIFESPAN |
		DIR_GF_SERVADDPATH |
		DIR_GF_SERVADDNETADDR |
		DIR_GF_SERVADDNAME);
	cGetDir->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE);
	if (cGetDir->GetStatus() != WS_Success)
	{
		g_pLTCSBase->CPrint("Directory not found");
		return;
	}

	// Display everything
	const DirEntityList &cFullDir = cGetDir->GetDirEntityList();
	DirEntityList::const_iterator iFullDirEntry = cFullDir.begin();
	for (; iFullDirEntry != cFullDir.end(); ++iFullDirEntry)
	{
		const wchar_t *pName = (*iFullDirEntry)->mDisplayName.empty() ? (*iFullDirEntry)->mName.c_str() : (*iFullDirEntry)->mDisplayName.c_str();
		g_pLTCSBase->CPrint("  %ls%s%ls", (*iFullDirEntry)->mPath.c_str(), (*iFullDirEntry)->mPath.length() == 1 ? "" : "/", pName);
		g_pLTCSBase->CPrint("    Type : %s", (*iFullDirEntry)->IsService() ? "Service" : "Directory");
		g_pLTCSBase->CPrint("    Display Name : %ls", (*iFullDirEntry)->mDisplayName.c_str());
		g_pLTCSBase->CPrint("    Created : %d", (*iFullDirEntry)->mCreated);
		g_pLTCSBase->CPrint("    Touched : %d", (*iFullDirEntry)->mTouched);
		g_pLTCSBase->CPrint("    Lifespan : %d", (*iFullDirEntry)->mLifespan);
		g_pLTCSBase->CPrint("    Path : %ls", (*iFullDirEntry)->mPath.c_str());
		g_pLTCSBase->CPrint("    Net Addr : %s", (*iFullDirEntry)->GetNetAddrAsIP().GetHostAndPortString().c_str());
		g_pLTCSBase->CPrint("    Name : %ls", (*iFullDirEntry)->mName.c_str());
	}
	g_pLTCSBase->CPrint("  <END>");

	return;
}

AuthContextPtr WONAuthorize()
{
	bool bSuccess = true;
	WONStatus nResult;

	CDKey cTheKey(g_sWONProduct);
	if (g_sWONCDKey.empty())
		cTheKey.LoadFromRegistry();
	else
		cTheKey.Init(g_sWONCDKey);
	if (!cTheKey.IsValid())
	{
		g_pLTCSBase->CPrint("Warning! Invalid CD key : %s", g_sWONCDKey.c_str());
		//return AuthContextPtr(0);
	}

	// Get the authentication server
	GetDirOpPtr cGetDir = new GetDirOp(WONGetCurrentServerContext(g_lWONServerList));
	cGetDir->SetPath(L"/TitanServers/Auth");
	cGetDir->SetFlags(
		DIR_GF_DECOMPSERVICES |
		DIR_GF_SERVADDNETADDR);
	nResult = cGetDir->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE);
	bSuccess &= (nResult == WS_Success);
	if (bSuccess)
	{
		bSuccess &= !cGetDir->GetDirEntityList().empty();
	}
	if (!bSuccess)
	{
		g_pLTCSBase->CPrint("Unable to query authentication server: %s", WONStatusToString(nResult));
		return AuthContextPtr(0);
	}

	// Authenticate
	AuthContextPtr cAuthorization = new AuthContext;
	HashFileList cFileCheckList;
	/*
	cFileCheckList.push_back("lithtech.exe");
	cAuthorization->SetHashFileList(g_sWONCommunity, cFileCheckList);
	*/
	// Temporarily provide a user id and password
	// NYI
	cAuthorization->SetUserName(L"kevintest");
	cAuthorization->SetPassword(L"kevintest");
	cAuthorization->SetCommunity(g_sWONCommunity);
	cAuthorization->SetCDKey(g_sWONCommunity, cTheKey);
	cAuthorization->AddAddressesFromDir(cGetDir->GetDirEntityList());
	GetCertOpPtr cCertOp = new GetCertOp(cAuthorization);
	nResult = cCertOp->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE);
	bSuccess &= (nResult == WS_Success);
	if (!bSuccess)
	{
		g_pLTCSBase->CPrint("Unable to retrieve certification: %s", WONStatusToString(nResult));
		return AuthContextPtr(0);
	}

	return cAuthorization;

}

void WONPublishService(const char *pName, const char *pDisplayName)
{
	if (WONAuthorize() == 0)
		return;

	bool bSuccess = true;
	WONStatus nResult;

	// Add the service
	AddServiceOpPtr cAddService = new AddServiceOp(WONGetCurrentServerContext(g_lWONServerList));
	cAddService->SetNetAddr(IPAddr(IPAddr::GetLocalAddr().GetHost(), 27888));
	cAddService->SetName(Make_wstring(pName));
	cAddService->SetDisplayName(Make_wstring(pDisplayName));
	cAddService->SetPath(g_sWONCurPath);
	cAddService->SetLifespan(300);
	nResult = cAddService->RunBlock(OP_TIMEOUT_INFINITE) ? WS_Success : cAddService->GetStatus();
	bSuccess &= (nResult == WS_Success);
	if (!bSuccess)
	{
		g_pLTCSBase->CPrint("Unable to create service: %s", WONStatusToString(nResult));
		return;
	}

	g_pLTCSBase->CPrint("Service %s created successfully", pName);
}

void WONRemoveService(const char *pName)
{
	if (WONAuthorize() == 0)
		return;

	bool bSuccess = true;

	// Go find the directory's IP address
	std::wstring sName = Make_wstring(pName);
	IPAddr cServiceAddr;
	GetDirOpPtr cGetDir = new GetDirOp(WONGetCurrentServerContext(g_lWONServerList));
	cGetDir->SetPath(g_sWONCurPath);
	cGetDir->SetFlags(
		DIR_GF_DECOMPSERVICES |
		DIR_GF_SERVADDNAME |
		DIR_GF_SERVADDNETADDR);
	bSuccess &= (cGetDir->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE) == WS_Success);
	if (bSuccess)
	{
		bSuccess = false;
		DirEntityList::const_iterator iCurEntity = cGetDir->GetDirEntityList().begin();
		for (; !bSuccess && iCurEntity != cGetDir->GetDirEntityList().end(); ++iCurEntity)
		{
			bSuccess |= ((*iCurEntity)->mName == sName);
			if (bSuccess)
				cServiceAddr = (*iCurEntity)->GetNetAddrAsIP();
		}
	}
	if (!bSuccess)
	{
		g_pLTCSBase->CPrint("Unable to find service");
		return;
	}

	// Remove the service
	RemoveServiceOpPtr cRemoveService = new RemoveServiceOp(WONGetCurrentServerContext(g_lWONServerList));
	cRemoveService->SetNetAddr(cServiceAddr);
	cRemoveService->SetName(sName);
	cRemoveService->SetPath(g_sWONCurPath);
	WONStatus nResult = cRemoveService->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE);
	bSuccess &= (nResult == WS_Success);
	if (!bSuccess)
	{
		g_pLTCSBase->CPrint("Unable to remove service: %s", WONStatusToString(nResult));
		return;
	}

	g_pLTCSBase->CPrint("Service %s removed successfully", pName);
}

void WONRemoveServiceByIP(const char *pIPAddr)
{
	if (WONAuthorize() == 0)
		return;

	bool bSuccess = true;

	// Go find the directory's name
	std::wstring sName;
	IPAddr cServiceAddr(pIPAddr);
	GetDirOpPtr cGetDir = new GetDirOp(WONGetCurrentServerContext(g_lWONServerList));
	cGetDir->SetPath(g_sWONCurPath);
	cGetDir->SetFlags(
		DIR_GF_DECOMPSERVICES |
		DIR_GF_SERVADDNAME |
		DIR_GF_SERVADDNETADDR);
	bSuccess &= (cGetDir->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE) == WS_Success);
	if (bSuccess)
	{
		bSuccess = false;
		DirEntityList::const_iterator iCurEntity = cGetDir->GetDirEntityList().begin();
		for (; !bSuccess && iCurEntity != cGetDir->GetDirEntityList().end(); ++iCurEntity)
		{
			bSuccess |= ((*iCurEntity)->GetNetAddrAsIP() == cServiceAddr);
			if (bSuccess)
				sName = (*iCurEntity)->mName;
		}
	}
	if (!bSuccess)
	{
		g_pLTCSBase->CPrint("Unable to find service");
		return;
	}

	// Remove the service
	RemoveServiceOpPtr cRemoveService = new RemoveServiceOp(WONGetCurrentServerContext(g_lWONServerList));
	cRemoveService->SetNetAddr(cServiceAddr);
	cRemoveService->SetPath(g_sWONCurPath);
	cRemoveService->SetName(sName);
	bSuccess &= (cRemoveService->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE) == WS_Success);
	if (!bSuccess)
	{
		g_pLTCSBase->CPrint("Unable to remove service");
		return;
	}

	g_pLTCSBase->CPrint("Service %s removed successfully", pIPAddr);
}

void WONQueryMOTD(const char *pGameName)
{
	GetMOTDOpPtr cGetMOTD = new GetMOTDOp(std::string((pGameName) ? pGameName : ""));
	if (!cGetMOTD->RunBlock(OP_TIMEOUT_INFINITE) != WS_Success)
	{
		if (!cGetMOTD->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE) != WS_Success)
		{
			g_pLTCSBase->CPrint("Failed querying message of the day");
			return;
		}
		else
		{
			g_pLTCSBase->CPrint("RunBlock failed, Run succeeded");
		}
	}

	// Get the right MOTD and truncate it so CPrint doesn't crash...
	std::string sMOTD;
	sMOTD.assign((pGameName) ? cGetMOTD->GetGameMOTD()->data() : cGetMOTD->GetSysMOTD()->data(), 490);
	g_pLTCSBase->CPrint("MOTD : %s", sMOTD.c_str());
}

void WONVerifyCDKey(const char *pCDKey)
{
	bool bSuccess = true;
	WONStatus nResult;

	// Check it locally
	CDKey cTheKey(g_sWONProduct);
	cTheKey.Init(pCDKey);
	if (!cTheKey.IsValid())
	{
		g_pLTCSBase->CPrint("That CD key is not valid");
		return;
	}

	// Get the authentication server
	GetDirOpPtr cGetDir = new GetDirOp(WONGetCurrentServerContext(g_lWONServerList));
	cGetDir->SetPath(L"/TitanServers/Auth");
	cGetDir->SetFlags(
		DIR_GF_DECOMPSERVICES |
		DIR_GF_SERVADDNETADDR);
	nResult = cGetDir->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE);
	bSuccess &= (nResult == WS_Success);
	if (bSuccess)
	{
		bSuccess &= !cGetDir->GetDirEntityList().empty();
	}
	if (!bSuccess)
	{
		g_pLTCSBase->CPrint("Unable to query authentication server: %s", WONStatusToString(nResult));
		return;
	}

	// Authenticate
	AuthContextPtr cAuthorization = new AuthContext;
	HashFileList cFileCheckList;
	cAuthorization->SetCommunity(g_sWONCommunity);
	cAuthorization->SetCDKey(g_sWONCommunity, cTheKey);
	cAuthorization->AddAddressesFromDir(cGetDir->GetDirEntityList());
	GetCertOpPtr cCertOp = new GetCertOp(cAuthorization);
	nResult = cCertOp->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE);
	bSuccess &= (nResult == WS_Success);
	if (!bSuccess)
	{
		g_pLTCSBase->CPrint("Unable to retrieve certification: %s", WONStatusToString(nResult));
		return;
	}

	g_pLTCSBase->CPrint("CD key valid");

	return;

}

void WONVerifyVersion(const char *pVersion, const char *pRegion)
{
	if (!pVersion || !pVersion[0])
		g_pLTCSBase->CPrint("You need to provide a version number.");

	bool bSuccess = true;
	WONStatus nResult;

	GetDirOpPtr cGetDir = new GetDirOp(WONGetCurrentServerContext(g_lWONServerList));
	cGetDir->SetPath(L"/" + g_sWONCommunity + L"/Patch");
	cGetDir->SetFlags(
		DIR_GF_DECOMPSERVICES |
		DIR_GF_SERVADDNETADDR);
	nResult = cGetDir->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE);
	bSuccess &= (nResult == WS_Success);
	if (bSuccess)
	{
		bSuccess &= !cGetDir->GetDirEntityList().empty();
	}
	if (!bSuccess)
	{
		g_pLTCSBase->CPrint("Unable to query patch server: %s", WONStatusToString(nResult));
		return;
	}

	ServerContextPtr cServerContext = new ServerContext;
	cServerContext->AddAddressesFromDir(cGetDir->GetDirEntityList());

	CheckValidVersionOpPtr pVersionOp = new CheckValidVersionOp(g_sWONProduct, cServerContext);

	pVersionOp->SetVersion(pVersion);
	pVersionOp->SetConfigName(pRegion);
	pVersionOp->SetGetPatchList(false);

	nResult = pVersionOp->Run(OP_MODE_BLOCK, OP_TIMEOUT_INFINITE);
	bSuccess &= (nResult == WS_Success);
	if (!bSuccess)
	{
		switch (nResult)
		{
			case WS_DBProxyServ_OutOfDate :
			case WS_DBProxyServ_ValidNotLatest :
				g_pLTCSBase->CPrint("Version is out of date, and there's an update available: %s", WONStatusToString(nResult));
				break;
			case WS_DBProxyServ_OutOfDateNoUpdate :
				g_pLTCSBase->CPrint("Version is out of date, no updates available: %s", WONStatusToString(nResult));
				break;
			default :
				g_pLTCSBase->CPrint("Unable to retrieve version info: %s", WONStatusToString(nResult));
				break;
		}
		return;
	}

	g_pLTCSBase->CPrint("That version is the current, valid version");

	return;

}

void WONDisplayHelp()
{
	g_pLTCSBase->CPrint("Unknown command.  Try one of these:");
	g_pLTCSBase->CPrint("  AddServer - Add a server to the directory servers list.");
	g_pLTCSBase->CPrint("  RemoveServer - Add a server to the directory servers list.");
	g_pLTCSBase->CPrint("  ListServers - List the current directory servers.");
	g_pLTCSBase->CPrint("  SetCommunity - Set the current community.");
	g_pLTCSBase->CPrint("  GetCommunity - Display the current community.");
	g_pLTCSBase->CPrint("  SetProduct - Set the current product.");
	g_pLTCSBase->CPrint("  GetProduct - Display the current product.");
	g_pLTCSBase->CPrint("  SetCDKey - Set the current CDKey.");
	g_pLTCSBase->CPrint("  GetCDKey - Display the current CDKey.");
	g_pLTCSBase->CPrint("  SetPath - Set the current path.");
	g_pLTCSBase->CPrint("  GetPath - Display the current path.");
	g_pLTCSBase->CPrint("  QueryFullDirTree - List the entire WON directory tree.");
	g_pLTCSBase->CPrint("  QueryDir - List the contents of a specific directory.");
	g_pLTCSBase->CPrint("  QueryServices - List details of the services in a specific directory.");
	g_pLTCSBase->CPrint("  QueryMOTD - Get the MOTD.");
	g_pLTCSBase->CPrint("  Publish - Add a service associated with this network ID.  (Publish [name] [display name])");
	g_pLTCSBase->CPrint("  Remove - Remove a service by name.  (Remove <name>)");
	g_pLTCSBase->CPrint("  RemoveIP - Remove a service by IP address.  (Remove <IP:Port>)");
	g_pLTCSBase->CPrint("  VerifyCDKey - Verify a CD key (VerifyCDKey <CDKey>)");
	g_pLTCSBase->CPrint("  VerifyVersion - Verify a version number (VerifyVersion <version> [region])");
}

void WONConsoleHook(int argc, char **argv)
{
	if (argc <= 0)
	{
		WONDisplayHelp();
		return;
	}

	if (!stricmp(argv[0], "AddServer"))
	{
		if (argc < 2)
		{
			g_pLTCSBase->CPrint("Please provide a server to add");
			return;
		}
		g_lWONServerList.push_back(std::string(argv[1]));
		g_pLTCSBase->CPrint("Added server %s", argv[1]);
	}
	else if (!stricmp(argv[0], "RemoveServer"))
	{
		if (argc < 2)
		{
			g_pLTCSBase->CPrint("Please provide a server to remove");
			return;
		}
		const char *pStar = strchr(argv[1], '*');
		uint32 nWildcardPos = pStar - argv[1];
		bool bWildcard = pStar != 0;
		TWONServerList::iterator iCurServer = g_lWONServerList.begin();
		uint32 nRemovalCount = 0;
		while (iCurServer != g_lWONServerList.end())
		{
			bool bMatch;
			if (bWildcard)
				bMatch = strnicmp(iCurServer->c_str(), argv[1], nWildcardPos) == 0;
			else
				bMatch = stricmp(iCurServer->c_str(), argv[1]) == 0;
			if (bMatch)
			{
				iCurServer = g_lWONServerList.erase(iCurServer);
				++nRemovalCount;
			}
			else
				++iCurServer;
		}
		if (g_lWONServerList.empty())
			g_pLTCSBase->CPrint("Server list cleared");
		else
			g_pLTCSBase->CPrint("%d servers removed.  %d servers remaining", nRemovalCount, g_lWONServerList.size());
	}
	else if (!stricmp(argv[0], "ListServers"))
	{
		g_pLTCSBase->CPrint("Server list:");
		TWONServerList::const_iterator iCurServer = g_lWONServerList.begin();
		for (; iCurServer != g_lWONServerList.end(); ++iCurServer)
		{
			g_pLTCSBase->CPrint("  %s", iCurServer->c_str());
		}
		g_pLTCSBase->CPrint("  <END>");
	}
	else if (!stricmp(argv[0], "SetCommunity"))
	{
		g_sWONCommunity = Make_wstring((argc > 1) ? argv[1] : "");
		g_pLTCSBase->CPrint("Set current community to : %s", argv[1]);
	}
	else if (!stricmp(argv[0], "GetCommunity"))
	{
		g_pLTCSBase->CPrint("Current community : %ls", g_sWONCommunity.c_str());
	}
	else if (!stricmp(argv[0], "SetProduct"))
	{
		g_sWONProduct = (argc > 1) ? argv[1] : "";
		g_pLTCSBase->CPrint("Set current Product to : %s", argv[1]);
	}
	else if (!stricmp(argv[0], "GetProduct"))
	{
		g_pLTCSBase->CPrint("Current Product : %s", g_sWONProduct.c_str());
	}
	else if (!stricmp(argv[0], "SetCDKey"))
	{
		g_sWONCDKey = (argc > 1) ? argv[1] : "";
		g_pLTCSBase->CPrint("Set current CDKey to : %s", argv[1]);
	}
	else if (!stricmp(argv[0], "GetCDKey"))
	{
		g_pLTCSBase->CPrint("Current CDKey : %s", g_sWONCDKey.c_str());
	}
	else if (!stricmp(argv[0], "SetPath"))
	{
		g_sWONCurPath = Make_wstring((argc > 1) ? argv[1] : "");
		g_pLTCSBase->CPrint("Set current path to : %ls", g_sWONCurPath.c_str());
	}
	else if (!stricmp(argv[0], "GetPath"))
	{
		g_pLTCSBase->CPrint("Current path : %ls", g_sWONCurPath.c_str());
	}
	else if (!stricmp(argv[0], "Publish"))
	{
		std::string sDisplayName("");
		for (int nRebuildDisplayName = 2; nRebuildDisplayName < argc; ++nRebuildDisplayName)
			sDisplayName = sDisplayName + argv[nRebuildDisplayName] + " ";
		if (!sDisplayName.empty())
			sDisplayName.erase(sDisplayName.end() - 1);
		WONPublishService((argc > 1) ? argv[1] : "", sDisplayName.c_str());
	}
	else if (!stricmp(argv[0], "Remove"))
	{
		if (argc < 2)
		{
			g_pLTCSBase->CPrint("Please provide the server name.");
			g_pLTCSBase->CPrint("  WON Remove <name>");
		}
		else
		{
			WONRemoveService(argv[1]);
		}
	}
	else if (!stricmp(argv[0], "RemoveIP"))
	{
		if (argc < 2)
		{
			g_pLTCSBase->CPrint("Please provide the server address.");
			g_pLTCSBase->CPrint("  WON RemoveIP <IP:Port>");
		}
		else
		{
			WONRemoveServiceByIP(argv[1]);
		}
	}
	else if (!stricmp(argv[0], "QueryFullDirTree"))
	{
		g_pLTCSBase->CPrint("Full directory tree:");
		std::wstring sTemp;
		sTemp.swap(g_sWONCurPath);
		WONQueryDir("/", true);
		sTemp.swap(g_sWONCurPath);
	}
	else if (!stricmp(argv[0], "QueryDir"))
	{
		WONQueryDir((argc > 1) ? argv[1] : "");
	}
	else if (!stricmp(argv[0], "QueryServices"))
	{
		WONQueryServices((argc > 1) ? argv[1] : "");
	}
	else if (!stricmp(argv[0], "QueryMOTD"))
	{
		WONQueryMOTD((argc > 1) ? argv[1] : 0);
	}
	else if (!stricmp(argv[0], "VerifyCDKey"))
	{
		WONVerifyCDKey((argc > 1) ? argv[1] : "");
	}
	else if (!stricmp(argv[0], "VerifyVersion"))
	{
		WONVerifyVersion((argc > 1) ? argv[1] : "", (argc > 2) ? argv[2] : "EN");
	}
	else
	{
		WONDisplayHelp();
	}
}

void TitanTest_Init()
{
	if (g_pLTClient)
	{
		g_pWONCore = WONAPICore::GetInstance();

		g_lWONServerList.clear();
		g_lWONServerList.push_back("nolf2.m1.sierra.com:15101");
		g_lWONServerList.push_back("nolf2.m2.sierra.com:15101");
		g_lWONServerList.push_back("nolf2.m3.sierra.com:15101");

		g_pLTClient->RegisterConsoleProgram("WON", WONConsoleHook);
	}
}

void TitanTest_Term()
{
	if (g_pLTClient)
	{
		g_pLTClient->UnregisterConsoleProgram("WON");

		g_pWONCore = 0;
	}
}
