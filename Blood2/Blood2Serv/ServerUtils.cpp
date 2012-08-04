/****************************************************************************
;
;	 MODULE:		ServerUtils (.CPP)
;
;	PURPOSE:		Server Utility Functions
;
;	HISTORY:		08/04/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "StdAfx.h"
#include "ServerUtils.h"
#include "WinInet.h"


// Externs...

extern	CString g_sWebRegUrl;
extern	DGUID	GAMEGUID;


// Globals...

HINSTANCE			s_hServerDLL     = NULL;
CreateServerFn		s_fnCreateServer = NULL;
DeleteServerFn		s_fnDeleteServer = NULL;
ServerInterface*	s_pServerMgr     = NULL;


// Functions...

BOOL LoadServerDLL(char *pDLLName)
{
	SI_CREATESTATUS status;

	if (s_pServerMgr && s_hServerDLL)
	{
		return(TRUE);
	}

	s_hServerDLL = LoadLibrary(pDLLName);

	if(!s_hServerDLL)
	{
		return(FALSE);
	}

	s_fnCreateServer = (CreateServerFn)GetProcAddress(s_hServerDLL, "CreateServer");
	s_fnDeleteServer = (DeleteServerFn)GetProcAddress(s_hServerDLL, "DeleteServer");
	
	if (!s_fnCreateServer || !s_fnDeleteServer)
	{
		FreeLibrary(s_hServerDLL);
		s_hServerDLL = NULL;
		return(NULL);
	}

	s_pServerMgr = NULL;

	status = s_fnCreateServer(SI_VERSION, GAMEGUID, &s_pServerMgr);

	if (status != 0)
	{
		return(FALSE);
	}

	return(TRUE);
}

ServerInterface* GetServerInterface()
{
	return(s_pServerMgr);
}

void FreeServerDLL()
{
	if (s_hServerDLL && s_fnDeleteServer)
	{
		s_fnDeleteServer();
		FreeLibrary(s_hServerDLL);
	}

	s_hServerDLL     = NULL;
	s_fnCreateServer = NULL;
	s_fnDeleteServer = NULL;
	s_pServerMgr     = NULL;
}

BOOL SendWebInfo2(const char* sSite, const char* sInfo, BOOL bWait)
{
	// Sanity checks...

	if (!sSite) return(FALSE);
	if (!sInfo) return(FALSE);


	// Open the internet and get a handle to it...

	HINTERNET hNet = InternetOpen("GameServ", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hNet) return(FALSE);


	//  Create the URL we want from the given parameters...

	CString sUrl(sSite);

	sUrl += "?";
	sUrl += sInfo;


	// Convert spaces to pluses...

	int nLen = sUrl.GetLength();

	for (int i = 0; i < nLen; i++)
	{
		if (sUrl.GetAt(i) == ' ') sUrl.SetAt(i, '+');
	}


	//  Open the URL we want...

	HINTERNET hHttp = InternetOpenUrl(hNet, sUrl, NULL, 0, 0, 0);
	if (!hHttp)
	{
		InternetCloseHandle(hNet);
		return(FALSE);
	}


	// Wait until the request is finished...

	if (bWait)
	{
		DWORD dwSize = 0;
		char  sBuf[1024];

		strcpy(sBuf, "");

		if (!InternetReadFile(hHttp, sBuf, 1000, &dwSize))
		{
			InternetCloseHandle(hNet);
			return(FALSE);
		}
	}


	// Clean up...

	InternetCloseHandle(hNet);


	// All done...

	return(TRUE);
}

void RemoveTrailingStuff(char *pStr)
{
	int len;

	len = strlen(pStr);
	if(len > 0 && pStr[len-1] == '\n')
	{
		pStr[len-1] = 0;
	}
}

BOOL SendWebInfo(const char* sInfo, BOOL bWait)
{
	char site[256];
	FILE *fp;

	SendWebInfo2(g_sWebRegUrl, sInfo, bWait);

	// Ignore sSite and get it from the file.
	if(!(fp = fopen("srv_send.txt", "rt")))
		return FALSE;

	while(fgets(site, sizeof(site), fp))
	{
		RemoveTrailingStuff(site);
		SendWebInfo2(site, sInfo, bWait);
	}

	fclose(fp);
	return TRUE;
}


