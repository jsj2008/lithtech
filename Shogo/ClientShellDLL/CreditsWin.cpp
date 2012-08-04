/****************************************************************************
;
;	 MODULE:		CreditsWin (.CPP)
;
;	PURPOSE:		Credits class - Windows specific stuff
;
;	HISTORY:		07/28/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "Windows.h"
#include "CreditsWin.h"
#include "clientheaders.h"


// Externs...

extern void* g_hLTDLLInstance;


// Statics...

HMODULE s_hModule = NULL;
HRSRC   s_hRes    = NULL;
HGLOBAL	s_hGlobal = NULL;
char*	s_sBuf    = NULL;


// Functions...

char* CreditsWin_GetTextBuffer(char* sName)
{
	//if (s_sBuf)
	//{
		//return(s_sBuf);
	//}
	//else
	{
		void* hModule;
		g_pLTClient->GetEngineHook("cres_hinstance",&hModule);
		s_hModule = (HINSTANCE)hModule;
		if (!s_hModule)	return(NULL);

		s_hRes = FindResource(s_hModule, sName, "TEXT");
		if (!s_hRes) return(NULL);

		s_hGlobal = LoadResource(s_hModule, s_hRes);
		if (!s_hGlobal) return(NULL);

		s_sBuf = (char*)LockResource(s_hGlobal);
		if (!s_sBuf) return(NULL);
		
		return(s_sBuf);
	}
}




