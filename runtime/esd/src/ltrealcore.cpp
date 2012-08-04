/****************************************************************************
;
;	MODULE:		LTRealCore (.CPP)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	10-20-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrealcore.h"
#include "ltrconout.h"
#include "ltrealfileobject.h"

//-----------------------------------------------------------------------------
// CLTRealCore member functions
//-----------------------------------------------------------------------------
CLTRealCore::CLTRealCore()
{
	m_bInitialized = LTFALSE;

	m_bRealPlayerInstalled = LTFALSE;
	m_bRealPlayerPlugInInstalled = LTFALSE;

	m_hDll = LTNULL;
	m_hFileSystemDll = LTNULL;
		
	m_fpCreateEngine = LTNULL;
	m_fpCloseEngine = LTNULL;

	m_pRMAEngine = LTNULL;
	m_pClientEngineSetup = LTNULL;

	m_pAudioDevice = LTNULL;
	m_pRMAAudioDeviceManager = LTNULL;
	m_pRMAAudioPushdown = LTNULL;

	// Real core handles
	m_hVSRC3260 = LTNULL;
	m_hPNXR3260 = LTNULL;
	m_hSMPL3260 = LTNULL;
	m_hAUTH3260 = LTNULL;
	m_hMETA3260 = LTNULL;
	m_hMP3F3260 = LTNULL;
	m_hMP3R3260 = LTNULL;

	// Real core handles (video specific)
	m_hRNCO3260 = LTNULL;
	m_hRV303260 = LTNULL;
	m_hDRV33260 = LTNULL;
	m_hRV203260 = LTNULL;
	m_hDRV23260 = LTNULL;
}

//-----------------------------------------------------------------------------
CLTRealCore::~CLTRealCore()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealCore::Init()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealCore::Init()");

	if (LTTRUE == m_bInitialized)
		return LT_ALREADYINITIALIZED;

	// Sanity Check!
	LTBOOL bInstalled = LTFALSE;
	if (LT_OK != IsRealPlayerInstalled(&bInstalled))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "IsRealPlayerInstalled() failed.");
		return LT_ERROR;
	}
	if (!bInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "RealPlayer is not installed.");
		return LT_ERROR;
	}
	if (LT_OK != IsRealPlayerPlugInInstalled(&bInstalled))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "IsRealPlayerPlugInInstalled() failed.");
		return LT_ERROR;
	}
	if (!bInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "RealPlayer plugin is not installed.");
		return LT_ERROR;
	}

	HKEY hKey;
	char szPathName[_MAX_PATH];
	char szFileName[_MAX_PATH];
	DWORD bufSize;
	PN_RESULT hRes;

//-----------------------------------------------------------------------------
	// Load the file system library
	szPathName[0] = '\0';
	szFileName[0] = '\0';
	bufSize = sizeof(szPathName) - 1;

	// Get the location of the Lithtech filesystem plugin from the windows registry
	if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,
	 "Software\\RealNetworks\\Preferences\\DT_Plugins", &hKey)) 
	{ 
		// Get the engine path
		hRes = RegQueryValue(hKey, "", szPathName, (long *)&bufSize); 
		RegCloseKey(hKey); 
	}
	else
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to find RA plugin directory.");
		return LT_ERROR;
	}

	// Add the filename
	strcpy(szFileName, szPathName);
	strcat(szFileName, LTREALCOREPLUGINFILENAME);

	// Load the engine core
	ASSERT(!m_hFileSystemDll);
	if (!(m_hFileSystemDll = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load lithrez plugin library.");
		return LT_ERROR;
	}

	m_bRealPlayerInstalled = LTTRUE;

	// Pass in function pointers for binding
	FPBindFunctions	pBindFunctionsFunc = (FPBindFunctions)GetProcAddress(m_hFileSystemDll, "BindFunctions");
	if (pBindFunctionsFunc)
	{
		FPBindBlock MyBindBlock;
		MyBindBlock.m_FPAdvise = &(CLTRealFileObject().Advise);
		MyBindBlock.m_FPClose = &(CLTRealFileObject().Close);
		MyBindBlock.m_FPGetFilename = &(CLTRealFileObject().GetFilename);
		MyBindBlock.m_FPGetRequest = &(CLTRealFileObject().GetRequest);
		MyBindBlock.m_FPInit = &(CLTRealFileObject().Init);
		MyBindBlock.m_FPRead = &(CLTRealFileObject().Read);
		MyBindBlock.m_FPSeek = &(CLTRealFileObject().Seek);
		MyBindBlock.m_FPSetRequest = &(CLTRealFileObject().SetRequest);
		MyBindBlock.m_FPStat = &(CLTRealFileObject().Stat);
		MyBindBlock.m_FPWrite = &(CLTRealFileObject().Write);
		pBindFunctionsFunc(&MyBindBlock);
	}
	else
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to bind function block.");
		return LT_ERROR;
	}
//-----------------------------------------------------------------------------

	// Load the RMA core libraries
	szPathName[0] = '\0';
	szFileName[0] = '\0';
	bufSize = sizeof(szPathName) - 1;

	// Get the location of the RMA core from the windows registry
	if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,
	 "Software\\RealNetworks\\Preferences\\DT_Common", &hKey)) 
	{ 
		// Get the engine path
		hRes = RegQueryValue(hKey, "", szPathName, (long *)&bufSize); 
		RegCloseKey(hKey); 
	}
	else
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to find RA core directory.");
		return LT_ERROR;
	}

	// Add the filename
	strcpy(szFileName, szPathName);
	strcat(szFileName, LTREALCOREENGINEFILENAME);

	// Load the engine core
	if (!m_hDll)
	{
		if (!(m_hDll = LoadLibrary(szFileName)))
		{
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
			return LT_ERROR;
		}
	}
/*
	// Real core handles
	szPathName[0] = '\0';
	szFileName[0] = '\0';
	bufSize = sizeof(szPathName) - 1;

	if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,
	 "Software\\RealNetworks\\Preferences\\DT_Plugins", &hKey)) 
	{ 
		// Get the engine path
		hRes = RegQueryValue(hKey, "", szPathName, (long *)&bufSize); 
		RegCloseKey(hKey); 
	}
	else
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to find RA core directory.");
		return LT_ERROR;
	}

	// Real core handles
	strcpy(szFileName, szPathName);
	strcat(szFileName, "VSRC3260.DLL");
	ASSERT(!m_hVSRC3260);
	if (!(m_hVSRC3260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
	strcpy(szFileName, szPathName);
	strcat(szFileName, "PNXR3260.DLL");
	ASSERT(!m_hPNXR3260);
	if (!(m_hPNXR3260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
	strcpy(szFileName, szPathName);
	strcat(szFileName, "SMPL3260.DLL");
	ASSERT(!m_hSMPL3260);
	if (!(m_hSMPL3260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
	strcpy(szFileName, szPathName);
	strcat(szFileName, "AUTH3260.DLL");
	ASSERT(!m_hAUTH3260);
	if (!(m_hAUTH3260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
	strcpy(szFileName, szPathName);
	strcat(szFileName, "META3260.DLL");
	ASSERT(!m_hMETA3260);
	if (!(m_hMETA3260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
	strcpy(szFileName, szPathName);
	strcat(szFileName, "MP3F3260.DLL");
	ASSERT(!m_hMP3F3260);
	if (!(m_hMP3F3260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
	strcpy(szFileName, szPathName);
	strcat(szFileName, "MP3R3260.DLL");
	ASSERT(!m_hMP3R3260);
	if (!(m_hMP3R3260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
*/
//-----------------------------------------------------------------------------
	szPathName[0] = '\0';
	szFileName[0] = '\0';
	bufSize = sizeof(szPathName) - 1;

	if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,
	 "Software\\RealNetworks\\Preferences\\DT_Codecs", &hKey)) 
	{ 
		// Get the engine path
		hRes = RegQueryValue(hKey, "", szPathName, (long *)&bufSize); 
		RegCloseKey(hKey); 
	}
	else
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to find RA core directory.");
		return LT_ERROR;
	}

	// Real core handles (video)
	strcpy(szFileName, szPathName);
	strcat(szFileName, "RNCO3260.DLL");
	ASSERT(!m_hRNCO3260);
	if (!(m_hRNCO3260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
	strcpy(szFileName, szPathName);
	strcat(szFileName, "RV303260.DLL");
	ASSERT(!m_hRV303260);
	if (!(m_hRV303260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
	strcpy(szFileName, szPathName);
	strcat(szFileName, "DRV33260.DLL");
	ASSERT(!m_hDRV33260);
	if (!(m_hDRV33260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
	strcpy(szFileName, szPathName);
	strcat(szFileName, "RV203260.DLL");
	ASSERT(!m_hRV203260);
	if (!(m_hRV203260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
	strcpy(szFileName, szPathName);
	strcat(szFileName, "DRV23260.DLL");
	ASSERT(!m_hDRV23260);
	if (!(m_hDRV23260 = LoadLibrary(szFileName)))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load RA core library.");
		return LT_ERROR;
	}
//-----------------------------------------------------------------------------

	// Assign function pointers
	if (!m_fpCreateEngine)
		m_fpCreateEngine = (FPRMCREATEENGINE)GetProcAddress(m_hDll, "CreateEngine");
	if (!m_fpCloseEngine)
		m_fpCloseEngine = (FPRMCLOSEENGINE)GetProcAddress(m_hDll, "CloseEngine");
	if (!m_fpCreateEngine || !m_fpCloseEngine)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to get RA core functions.");
		return LT_ERROR;
	}

	// Create the engine
	if (!m_pRMAEngine)
	{
		if (PNR_OK != m_fpCreateEngine((IRMAClientEngine**)&m_pRMAEngine))
		{
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to create RA engine.");
			return LT_ERROR;
		}
	}

	ReplaceAudioDevice();

	// Query interface on the client engine to get the client engine setup
	if (PNR_OK != m_pRMAEngine->QueryInterface(IID_IRMAClientEngineSetup, (void**)&m_pClientEngineSetup))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to create client engine setup.");
		return LT_ERROR;
	}
	else
		m_pClientEngineSetup->AddRef();

	// We're all done
	m_bInitialized = LTTRUE;
	m_bRealPlayerInstalled = LTTRUE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealCore::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealCore::Term()");

	if (LTTRUE != m_bInitialized)
		return LT_NOTINITIALIZED;

	// Clean up the audio device manager
	if (m_pRMAAudioDeviceManager)
	{
		if (m_pAudioDevice)
			m_pRMAAudioDeviceManager->Remove(m_pAudioDevice);

		m_pRMAAudioDeviceManager->Release();
		m_pRMAAudioDeviceManager = NULL;
	}

	// Clean up the audio device
	if (m_pAudioDevice)
	{
		m_pAudioDevice->Release();
		m_pAudioDevice = NULL;
	}

	// Clean up the engine setup object
	if (m_pClientEngineSetup)
	{
		m_pClientEngineSetup->Release();
		m_pClientEngineSetup = NULL;
	}

	// Clean up the engine
	if (m_pRMAEngine)
	{
		ASSERT(m_fpCloseEngine);
		m_fpCloseEngine(m_pRMAEngine);
		m_pRMAEngine = NULL;
	}

	// Clean up Real core handles
	if (m_hVSRC3260)
	{
		FreeLibrary(m_hVSRC3260);
		m_hVSRC3260 = LTNULL;
	}
	if (m_hPNXR3260)
	{
		FreeLibrary(m_hPNXR3260);
		m_hPNXR3260 = LTNULL;
	}
	if (m_hSMPL3260)
	{
		FreeLibrary(m_hSMPL3260);
		m_hSMPL3260 = LTNULL;
	}
	if (m_hAUTH3260)
	{
		FreeLibrary(m_hAUTH3260);
		m_hAUTH3260 = LTNULL;
	}
	if (m_hMETA3260)
	{
		FreeLibrary(m_hMETA3260);
		m_hMETA3260 = LTNULL;
	}
	if (m_hMP3F3260)
	{
		FreeLibrary(m_hMP3F3260);
		m_hMP3F3260 = LTNULL;
	}
	if (m_hMP3R3260)
	{
		FreeLibrary(m_hMP3R3260);
		m_hMP3R3260 = LTNULL;
	}

	// Clean up Real core handles
	if (m_hRNCO3260)
	{
		FreeLibrary(m_hRNCO3260);
		m_hRNCO3260 = LTNULL;
	}
	if (m_hRV303260)
	{
		FreeLibrary(m_hRV303260);
		m_hRV303260 = LTNULL;
	}
	if (m_hDRV33260)
	{
		FreeLibrary(m_hDRV33260);
		m_hDRV33260 = LTNULL;
	}
	if (m_hRV203260)
	{
		FreeLibrary(m_hRV203260);
		m_hRV203260 = LTNULL;
	}
	if (m_hDRV23260)
	{
		FreeLibrary(m_hDRV23260);
		m_hDRV23260 = LTNULL;
	}

	// Clean up the linked library
	if (m_hFileSystemDll)
	{
		FreeLibrary(m_hFileSystemDll);
		m_hFileSystemDll = NULL;
	}

	// Clean up the linked library
	if (m_hDll)
	{
		FreeLibrary(m_hDll);
		m_hDll = NULL;
	}

	m_bInitialized = LTFALSE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealCore::IsRealPlayerInstalled(bool* pbIsInstalled)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealCore::IsRealPlayerInstalled()");

	// If we've already determined that it's installed (via ::Init()) then
	// simply return
	if (m_bRealPlayerInstalled)
	{
		*pbIsInstalled = LTTRUE;
		return LT_OK;
	}

	// Otherwise find the RMA core libraries
	HKEY hKey = LTNULL;
	char szPathName[_MAX_PATH];
	char szFileName[_MAX_PATH];
	DWORD bufSize = 0;
	PN_RESULT hRes = LTNULL;
	FILE* pFile = LTNULL;

	szPathName[0] = '\0';
	szFileName[0] = '\0';
	bufSize = sizeof(szPathName) - 1;

	// Get the location of the RMA core from the windows registry
	if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,
	 "Software\\RealNetworks\\Preferences\\DT_Common", &hKey)) 
	{ 
		// Get the engine path
		hRes = RegQueryValue(hKey, "", szPathName, (long *)&bufSize); 
		RegCloseKey(hKey); 
	}
	else
	{
		m_bRealPlayerInstalled = LTFALSE;
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to find RA core directory.");
		return LT_ERROR;
	}

	// Add the filename
	strcpy(szFileName, szPathName);
	strcat(szFileName, LTREALCOREENGINEFILENAME);
	pFile = fopen(szFileName, "r");
	if (pFile)
	{
		m_bRealPlayerInstalled = LTTRUE;
		fclose(pFile);
	}
	else
		m_bRealPlayerInstalled = LTFALSE;

	*pbIsInstalled = m_bRealPlayerInstalled;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealCore::IsRealPlayerPlugInInstalled(bool* pbIsInstalled)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealCore::IsRealPlayerPlugInInstalled()");

	// If we've already determined that it's installed (via ::Init()) then
	// simply return
	if (m_bRealPlayerPlugInInstalled)
	{
		*pbIsInstalled = LTTRUE;
		return LT_OK;
	}

	// Otherwise find the RMA core libraries
	HKEY hKey = LTNULL;
	char szPathName[_MAX_PATH];
	char szFileName[_MAX_PATH];
	DWORD bufSize = 0;
	PN_RESULT hRes = LTNULL;
	FILE* pFile = LTNULL;

	szPathName[0] = '\0';
	szFileName[0] = '\0';
	bufSize = sizeof(szPathName) - 1;

	// Get the location of the RMA core from the windows registry
	if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,
	 "Software\\RealNetworks\\Preferences\\DT_Plugins", &hKey)) 
	{ 
		// Get the engine path
		hRes = RegQueryValue(hKey, "", szPathName, (long *)&bufSize); 
		RegCloseKey(hKey); 
	}
	else
	{
		m_bRealPlayerPlugInInstalled = LTFALSE;
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to load lithrez plugin library.");
		return LT_ERROR;
	}

	// Add the filename
	strcpy(szFileName, szPathName);
	strcat(szFileName, LTREALCOREPLUGINFILENAME);
	pFile = fopen(szFileName, "r");
	if (pFile)
	{
		m_bRealPlayerPlugInInstalled = LTTRUE;
		fclose(pFile);
	}
	else
		m_bRealPlayerPlugInInstalled = LTFALSE;

	*pbIsInstalled = m_bRealPlayerPlugInInstalled;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealCore::ReplaceAudioDevice()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealCore::ReplaceAudioDevice()");

	// Clean up the audio device manager
	if (m_pRMAAudioDeviceManager)
	{
		if (m_pAudioDevice)
			m_pRMAAudioDeviceManager->Remove(m_pAudioDevice);
	}

	if (!m_pAudioDevice)
	{
		LT_MEM_TRACK_ALLOC(m_pAudioDevice = new CLTAudioDevice,LT_MEM_TYPE_SOUND);
		ASSERT(m_pAudioDevice);
		m_pAudioDevice->Init();
		m_pAudioDevice->AddRef();
	}
	if (!m_pRMAAudioDeviceManager)
	{
		if (PNR_OK != m_pRMAEngine->QueryInterface(IID_IRMAAudioDeviceManager,
											(void**)&m_pRMAAudioDeviceManager))
		{
			delete m_pAudioDevice;
			m_pAudioDevice = LTNULL;
			return LT_ERROR;
		}
	}
	HRESULT hResult = m_pRMAAudioDeviceManager->Replace(m_pAudioDevice);
	if (PNR_OK != hResult)
	{
#ifdef _DEBUG
		CLTRealCore().DumpRealErrorCode(hResult);
#endif // _DEBUG
		delete m_pAudioDevice;
		m_pAudioDevice = LTNULL;
		return LT_ERROR;
	}
	if (PNR_OK == m_pRMAAudioDeviceManager->QueryInterface(IID_IRMAAudioPushdown,
										(void**)&m_pRMAAudioPushdown))
	{
		m_pRMAAudioPushdown->SetAudioPushdown(0);
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealCore::CreatePlayer(IRMAPlayer** ppRMAPlayer)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealCore::CreatePlayer()");

	if (LTTRUE != m_bInitialized)
		return LT_NOTINITIALIZED;

	ASSERT(m_pRMAEngine);
	ASSERT(LTNULL == *ppRMAPlayer);

	// Create the player
	if (PNR_OK != m_pRMAEngine->CreatePlayer(*ppRMAPlayer))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "Unable to create RA player.");
		return LT_ERROR;
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealCore::DestroyPlayer(IRMAPlayer** ppRMAPlayer)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealCore::DestroyPlayer()");

	if (LTTRUE != m_bInitialized)
		return LT_NOTINITIALIZED;

	ASSERT(m_pRMAEngine);
	ASSERT(LTNULL != *ppRMAPlayer);
	((IRMAPlayer*)*ppRMAPlayer)->Stop();
	m_pRMAEngine->ClosePlayer(*ppRMAPlayer);
	((IRMAPlayer*)*ppRMAPlayer)->Release();
	*ppRMAPlayer = NULL;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealCore::DumpRealErrorCode(HRESULT hResult)
{
	switch(hResult)
	{
		case PNR_NOTIMPL:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NOTIMPL");
			break;
		case PNR_OUTOFMEMORY:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_OUTOFMEMORY");
			break;
		case PNR_INVALID_PARAMETER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_PARAMETER");
			break;
		case PNR_NOINTERFACE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NOINTERFACE");
			break;
		case PNR_POINTER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_POINTER");
			break;
		case PNR_HANDLE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_HANDLE");
			break;
		case PNR_ABORT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ABORT");
			break;
		case PNR_FAIL:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_FAIL");
			break;
		case PNR_ACCESSDENIED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ACCESSDENIED");
			break;
		case PNR_IGNORE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_IGNORE");
			break;
		case PNR_OK:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_OK");
			break;
		case PNR_INVALID_OPERATION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_OPERATION");
			break;
		case PNR_INVALID_VERSION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_VERSION");
			break;
		case PNR_INVALID_REVISION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_REVISION");
			break;
		case PNR_NOT_INITIALIZED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NOT_INITIALIZED");
			break;
		case PNR_DOC_MISSING:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_DOC_MISSING");
			break;
		case PNR_UNEXPECTED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_UNEXPECTED");
			break;
		case PNR_INCOMPLETE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INCOMPLETE");
			break;
		case PNR_BUFFERTOOSMALL:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BUFFERTOOSMALL");
			break;
		case PNR_UNSUPPORTED_VIDEO:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_UNSUPPORTED_VIDEO");
			break;
		case PNR_UNSUPPORTED_AUDIO:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_UNSUPPORTED_AUDIO");
			break;
		case PNR_INVALID_BANDWIDTH:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_BANDWIDTH");
			break;
/*		case PNR_NO_RENDERER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NO_RENDERER");
			break;
		case PNR_NO_FILEFORMAT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NO_FILEFORMAT");
			break;
		case PNR_MISSING_COMPONENTS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_MISSING_COMPONENTS");
			break;*/
		case PNR_NO_RENDERER:  // these defs all have the same value...
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NO_RENDERER");
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NO_FILEFORMAT");
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_MISSING_COMPONENTS");
			break;
		case PNR_ELEMENT_NOT_FOUND:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ELEMENT_NOT_FOUND");
			break;
		case PNR_NOCLASS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NOCLASS");
			break;
		case PNR_CLASS_NOAGGREGATION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_CLASS_NOAGGREGATION");
			break;
		case PNR_NOT_LICENSED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NOT_LICENSED");
			break;
		case PNR_NO_FILESYSTEM:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NO_FILESYSTEM");
			break;
		case PNR_BUFFERING:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BUFFERING");
			break;
		case PNR_PAUSED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PAUSED");
			break;
		case PNR_NO_DATA:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NO_DATA");
			break;
		case PNR_STREAM_DONE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_STREAM_DONE");
			break;
		case PNR_NET_SOCKET_INVALID:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NET_SOCKET_INVALID");
			break;
		case PNR_NET_CONNECT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NET_CONNECT");
			break;
		case PNR_BIND:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BIND");
			break;
		case PNR_SOCKET_CREATE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_SOCKET_CREATE");
			break;
		case PNR_INVALID_HOST:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_HOST");
			break;
		case PNR_NET_READ:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NET_READ");
			break;
		case PNR_NET_WRITE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NET_WRITE");
			break;
		case PNR_NET_UDP:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NET_UDP");
			break;
		case PNR_RETRY:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RETRY");
			break;
		case PNR_SERVER_TIMEOUT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_SERVER_TIMEOUT");
			break;
		case PNR_SERVER_DISCONNECTED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_SERVER_DISCONNECTED");
			break;
		case PNR_WOULD_BLOCK:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_WOULD_BLOCK");
			break;
		case PNR_GENERAL_NONET:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_GENERAL_NONET");
			break;
		case PNR_BLOCK_CANCELED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BLOCK_CANCELED");
			break;
		case PNR_MULTICAST_JOIN:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_MULTICAST_JOIN");
			break;
		case PNR_GENERAL_MULTICAST:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_GENERAL_MULTICAST");
			break;
		case PNR_MULTICAST_UDP:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_MULTICAST_UDP");
			break;
		case PNR_AT_INTERRUPT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_AT_INTERRUPT");
			break;
		case PNR_MSG_TOOLARGE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_MSG_TOOLARGE");
			break;
		case PNR_NET_TCP:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NET_TCP");
			break;
		case PNR_TRY_AUTOCONFIG:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_TRY_AUTOCONFIG");
			break;
		case PNR_NOTENOUGH_BANDWIDTH:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NOTENOUGH_BANDWIDTH");
			break;
		case PNR_HTTP_CONNECT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_HTTP_CONNECT");
			break;
		case PNR_PORT_IN_USE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PORT_IN_USE");
			break;
		case PNR_AT_END:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_AT_END");
			break;
		case PNR_INVALID_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_FILE");
			break;
		case PNR_INVALID_PATH:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_PATH");
			break;
		case PNR_RECORD:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RECORD");
			break;
		case PNR_RECORD_WRITE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RECORD_WRITE");
			break;
		case PNR_TEMP_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_TEMP_FILE");
			break;
		case PNR_ALREADY_OPEN:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ALREADY_OPEN");
			break;
		case PNR_SEEK_PENDING:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_SEEK_PENDING");
			break;
		case PNR_CANCELLED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_CANCELLED");
			break;
		case PNR_FILE_NOT_FOUND:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_FILE_NOT_FOUND");
			break;
		case PNR_WRITE_ERROR:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_WRITE_ERROR");
			break;
		case PNR_FILE_EXISTS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_FILE_EXISTS");
			break;
		case PNR_FILE_NOT_OPEN:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_FILE_NOT_OPEN");
			break;
		case PNR_ADVISE_PREFER_LINEAR:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ADVISE_PREFER_LINEAR");
			break;
		case PNR_PARSE_ERROR:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PARSE_ERROR");
			break;
		case PNR_BAD_SERVER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BAD_SERVER");
			break;
		case PNR_ADVANCED_SERVER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ADVANCED_SERVER");
			break;
		case PNR_OLD_SERVER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_OLD_SERVER");
			break;
		case PNR_REDIRECTION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_REDIRECTION");
			break;
		case PNR_SERVER_ALERT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_SERVER_ALERT");
			break;
		case PNR_PROXY:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PROXY");
			break;
		case PNR_PROXY_RESPONSE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PROXY_RESPONSE");
			break;
		case PNR_ADVANCED_PROXY:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ADVANCED_PROXY");
			break;
		case PNR_OLD_PROXY:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_OLD_PROXY");
			break;
		case PNR_INVALID_PROTOCOL:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_PROTOCOL");
			break;
		case PNR_INVALID_URL_OPTION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_URL_OPTION");
			break;
		case PNR_INVALID_URL_HOST:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_URL_HOST");
			break;
		case PNR_INVALID_URL_PATH:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_URL_PATH");
			break;
		case PNR_HTTP_CONTENT_NOT_FOUND:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_HTTP_CONTENT_NOT_FOUND");
			break;
		case PNR_NOT_AUTHORIZED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NOT_AUTHORIZED");
			break;
		case PNR_UNEXPECTED_MSG:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_UNEXPECTED_MSG");
			break;
		case PNR_BAD_TRANSPORT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BAD_TRANSPORT");
			break;
		case PNR_NO_SESSION_ID:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NO_SESSION_ID");
			break;
		case PNR_PROXY_DNR:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PROXY_DNR");
			break;
		case PNR_PROXY_NET_CONNECT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PROXY_NET_CONNECT");
			break;
		case PNR_AUDIO_DRIVER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_AUDIO_DRIVER");
			break;
		case PNR_LATE_PACKET:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_LATE_PACKET");
			break;
		case PNR_OVERLAPPED_PACKET:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_OVERLAPPED_PACKET");
			break;
		case PNR_OUTOFORDER_PACKET:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_OUTOFORDER_PACKET");
			break;
		case PNR_NONCONTIGUOUS_PACKET:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NONCONTIGUOUS_PACKET");
			break;
		case PNR_OPEN_NOT_PROCESSED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_OPEN_NOT_PROCESSED");
			break;
		case PNR_EXPIRED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_EXPIRED");
			break;
		case PNR_INVALID_INTERLEAVER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_INTERLEAVER");
			break;
		case PNR_BAD_FORMAT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BAD_FORMAT");
			break;
		case PNR_CHUNK_MISSING:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_CHUNK_MISSING");
			break;
		case PNR_INVALID_STREAM:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_STREAM");
			break;
		case PNR_DNR:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_DNR");
			break;
		case PNR_OPEN_DRIVER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_OPEN_DRIVER");
			break;
		case PNR_UPGRADE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_UPGRADE");
			break;
		case PNR_NOTIFICATION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NOTIFICATION");
			break;
		case PNR_NOT_NOTIFIED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NOT_NOTIFIED");
			break;
		case PNR_STOPPED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_STOPPED");
			break;
		case PNR_CLOSED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_CLOSED");
			break;
		case PNR_INVALID_WAV_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_WAV_FILE");
			break;
		case PNR_NO_SEEK:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NO_SEEK");
			break;
		case PNR_DEC_INITED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_DEC_INITED");
			break;
		case PNR_DEC_NOT_FOUND:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_DEC_NOT_FOUND");
			break;
		case PNR_DEC_INVALID:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_DEC_INVALID");
			break;
		case PNR_DEC_TYPE_MISMATCH:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_DEC_TYPE_MISMATCH");
			break;
		case PNR_DEC_INIT_FAILED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_DEC_INIT_FAILED");
			break;
		case PNR_DEC_NOT_INITED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_DEC_NOT_INITED");
			break;
		case PNR_DEC_DECOMPRESS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_DEC_DECOMPRESS");
			break;
		case PNR_OBSOLETE_VERSION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_OBSOLETE_VERSION");
			break;
		case PNR_ENC_FILE_TOO_SMALL:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_FILE_TOO_SMALL");
			break;
		case PNR_ENC_UNKNOWN_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_UNKNOWN_FILE");
			break;
		case PNR_ENC_BAD_CHANNELS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_BAD_CHANNELS");
			break;
		case PNR_ENC_BAD_SAMPSIZE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_BAD_SAMPSIZE");
			break;
		case PNR_ENC_BAD_SAMPRATE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_BAD_SAMPRATE");
			break;
		case PNR_ENC_INVALID:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_INVALID");
			break;
		case PNR_ENC_NO_OUTPUT_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_NO_OUTPUT_FILE");
			break;
		case PNR_ENC_NO_INPUT_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_NO_INPUT_FILE");
			break;
		case PNR_ENC_NO_OUTPUT_PERMISSIONS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_NO_OUTPUT_PERMISSIONS");
			break;
		case PNR_ENC_BAD_FILETYPE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_BAD_FILETYPE");
			break;
		case PNR_ENC_INVALID_VIDEO:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_INVALID_VIDEO");
			break;
		case PNR_ENC_INVALID_AUDIO:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_INVALID_AUDIO");
			break;
		case PNR_ENC_NO_VIDEO_CAPTURE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_NO_VIDEO_CAPTURE");
			break;
		case PNR_ENC_INVALID_VIDEO_CAPTURE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_INVALID_VIDEO_CAPTURE");
			break;
		case PNR_ENC_NO_AUDIO_CAPTURE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_NO_AUDIO_CAPTURE");
			break;
		case PNR_ENC_INVALID_AUDIO_CAPTURE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_INVALID_AUDIO_CAPTURE");
			break;
		case PNR_ENC_TOO_SLOW_FOR_LIVE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_TOO_SLOW_FOR_LIVE");
			break;
		case PNR_ENC_ENGINE_NOT_INITIALIZED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_ENGINE_NOT_INITIALIZED");
			break;
		case PNR_ENC_CODEC_NOT_FOUND:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_CODEC_NOT_FOUND");
			break;
		case PNR_ENC_CODEC_NOT_INITIALIZED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_CODEC_NOT_INITIALIZED");
			break;
		case PNR_ENC_INVALID_INPUT_DIMENSIONS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_INVALID_INPUT_DIMENSIONS");
			break;
		case PNR_ENC_MESSAGE_IGNORED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_MESSAGE_IGNORED");
			break;
		case PNR_ENC_NO_SETTINGS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_NO_SETTINGS");
			break;
		case PNR_ENC_NO_OUTPUT_TYPES:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_NO_OUTPUT_TYPES");
			break;
		case PNR_ENC_IMPROPER_STATE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_IMPROPER_STATE");
			break;
		case PNR_ENC_INVALID_SERVER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_INVALID_SERVER");
			break;
		case PNR_ENC_INVALID_TEMP_PATH:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_INVALID_TEMP_PATH");
			break;
		case PNR_ENC_MERGE_FAIL:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_MERGE_FAIL");
			break;
		case PNR_BIN_DATA_NOT_FOUND:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BIN_DATA_NOT_FOUND");
			break;
		case PNR_BIN_END_OF_DATA:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BIN_END_OF_DATA");
			break;
		case PNR_BIN_DATA_PURGED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BIN_DATA_PURGED");
			break;
		case PNR_BIN_FULL:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BIN_FULL");
			break;
		case PNR_BIN_OFFSET_PAST_END:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BIN_OFFSET_PAST_END");
			break;
		case PNR_ENC_NO_ENCODED_DATA:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_NO_ENCODED_DATA");
			break;
		case PNR_ENC_INVALID_DLL:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_INVALID_DLL");
			break;
		case PNR_NOT_INDEXABLE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NOT_INDEXABLE");
			break;
		case PNR_ENC_NO_BROWSER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_NO_BROWSER");
			break;
		case PNR_ENC_NO_FILE_TO_SERVER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_NO_FILE_TO_SERVER");
			break;
		case PNR_ENC_INSUFFICIENT_DISK_SPACE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_ENC_INSUFFICIENT_DISK_SPACE");
			break;
		case PNR_RMT_USAGE_ERROR:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_USAGE_ERROR");
			break;
		case PNR_RMT_INVALID_ENDTIME:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_INVALID_ENDTIME");
			break;
		case PNR_RMT_MISSING_INPUT_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_MISSING_INPUT_FILE");
			break;
		case PNR_RMT_MISSING_OUTPUT_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_MISSING_OUTPUT_FILE");
			break;
		case PNR_RMT_INPUT_EQUALS_OUTPUT_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_INPUT_EQUALS_OUTPUT_FILE");
			break;
		case PNR_RMT_UNSUPPORTED_AUDIO_VERSION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_UNSUPPORTED_AUDIO_VERSION");
			break;
		case PNR_RMT_DIFFERENT_AUDIO:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_DIFFERENT_AUDIO");
			break;
		case PNR_RMT_DIFFERENT_VIDEO:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_DIFFERENT_VIDEO");
			break;
		case PNR_RMT_PASTE_MISSING_STREAM:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_PASTE_MISSING_STREAM");
			break;
		case PNR_RMT_END_OF_STREAM:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_END_OF_STREAM");
			break;
		case PNR_RMT_IMAGE_MAP_PARSE_ERROR:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_IMAGE_MAP_PARSE_ERROR");
			break;
		case PNR_RMT_INVALID_IMAGEMAP_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_INVALID_IMAGEMAP_FILE");
			break;
		case PNR_RMT_EVENT_PARSE_ERROR:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_EVENT_PARSE_ERROR");
			break;
		case PNR_RMT_INVALID_EVENT_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_INVALID_EVENT_FILE");
			break;
		case PNR_RMT_INVALID_OUTPUT_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_INVALID_OUTPUT_FILE");
			break;
		case PNR_RMT_INVALID_DURATION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_INVALID_DURATION");
			break;
		case PNR_RMT_NO_DUMP_FILES:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_NO_DUMP_FILES");
			break;
		case PNR_RMT_NO_EVENT_DUMP_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_NO_EVENT_DUMP_FILE");
			break;
		case PNR_RMT_NO_IMAP_DUMP_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_NO_IMAP_DUMP_FILE");
			break;
		case PNR_RMT_NO_DATA:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_NO_DATA");
			break;
		case PNR_RMT_EMPTY_STREAM:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_EMPTY_STREAM");
			break;
		case PNR_RMT_READ_ONLY_FILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RMT_READ_ONLY_FILE");
			break;
		case PNR_PROP_NOT_FOUND:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PROP_NOT_FOUND");
			break;
		case PNR_PROP_NOT_COMPOSITE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PROP_NOT_COMPOSITE");
			break;
		case PNR_PROP_DUPLICATE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PROP_DUPLICATE");
			break;
		case PNR_PROP_TYPE_MISMATCH:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PROP_TYPE_MISMATCH");
			break;
		case PNR_PROP_ACTIVE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PROP_ACTIVE");
			break;
		case PNR_PROP_INACTIVE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PROP_INACTIVE");
			break;
		case PNR_COULDNOTINITCORE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_COULDNOTINITCORE");
			break;
		case PNR_PERFECTPLAY_NOT_SUPPORTED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PERFECTPLAY_NOT_SUPPORTED");
			break;
		case PNR_NO_LIVE_PERFECTPLAY:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NO_LIVE_PERFECTPLAY");
			break;
		case PNR_PERFECTPLAY_NOT_ALLOWED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PERFECTPLAY_NOT_ALLOWED");
			break;
		case PNR_NO_CODECS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_NO_CODECS");
			break;
		case PNR_SLOW_MACHINE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_SLOW_MACHINE");
			break;
		case PNR_FORCE_PERFECTPLAY:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_FORCE_PERFECTPLAY");
			break;
		case PNR_INVALID_HTTP_PROXY_HOST:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_HTTP_PROXY_HOST");
			break;
		case PNR_INVALID_METAFILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_INVALID_METAFILE");
			break;
		case PNR_BROWSER_LAUNCH:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_BROWSER_LAUNCH");
			break;
		case PNR_VIEW_SOURCE_NOCLIP:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_VIEW_SOURCE_NOCLIP");
			break;
		case PNR_VIEW_SOURCE_DISSABLED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_VIEW_SOURCE_DISSABLED");
			break;
		case PNR_RESOURCE_NOT_CACHED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RESOURCE_NOT_CACHED");
			break;
		case PNR_RESOURCE_NOT_FOUND:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RESOURCE_NOT_FOUND");
			break;
		case PNR_RESOURCE_CLOSE_FILE_FIRST:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RESOURCE_CLOSE_FILE_FIRST");
			break;
		case PNR_RESOURCE_NODATA:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RESOURCE_NODATA");
			break;
		case PNR_RESOURCE_BADFILE:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RESOURCE_BADFILE");
			break;
		case PNR_RESOURCE_PARTIALCOPY:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_RESOURCE_PARTIALCOPY");
			break;
		case PNR_PPV_NO_USER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PPV_NO_USER");
			break;
		case PNR_PPV_GUID_READ_ONLY:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PPV_GUID_READ_ONLY");
			break;
		case PNR_PPV_GUID_COLLISION:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PPV_GUID_COLLISION");
			break;
		case PNR_REGISTER_GUID_EXISTS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_REGISTER_GUID_EXISTS");
			break;
		case PNR_PPV_AUTHORIZATION_FAILED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PPV_AUTHORIZATION_FAILED");
			break;
		case PNR_PPV_OLD_PLAYER:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PPV_OLD_PLAYER");
			break;
		case PNR_PPV_ACCOUNT_LOCKED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PPV_ACCOUNT_LOCKED");
			break;
		case PNR_PPV_DBACCESS_ERROR:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PPV_DBACCESS_ERROR");
			break;
		case PNR_PPV_USER_ALREADY_EXISTS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_PPV_USER_ALREADY_EXISTS");
			break;
		case PNR_UPG_AUTH_FAILED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_UPG_AUTH_FAILED");
			break;
		case PNR_UPG_CERT_AUTH_FAILED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_UPG_CERT_AUTH_FAILED");
			break;
		case PNR_UPG_CERT_EXPIRED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_UPG_CERT_EXPIRED");
			break;
		case PNR_UPG_CERT_REVOKED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_UPG_CERT_REVOKED");
			break;
		case PNR_UPG_RUP_BAD:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_UPG_RUP_BAD");
			break;
		case PNR_AUTOCFG_SUCCESS:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_AUTOCFG_SUCCESS");
			break;
		case PNR_AUTOCFG_FAILED:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_AUTOCFG_FAILED");
			break;
		case PNR_AUTOCFG_ABORT:
			LTRConsoleOutput(LTRA_CONOUT_ERROR, "PNR_AUTOCFG_ABORT");
			break;
		default:
			ASSERT(0);
	}

	return LT_OK;
}

#endif // LITHTECH_ESD
