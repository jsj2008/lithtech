#include "precompile.h"

#include "renderstruct.h"
#include "common_stuff.h"
#include "d3d_init.h"
#include "rendererconsolevars.h"

RenderStruct *g_pStruct=NULL;

// Main globals.
HWND	g_hWnd;
bool	g_bRunWindowed;

uint32	g_ScreenWidth, g_ScreenHeight;

// The main list of ConVars.
BaseConVar* g_pConVars	= NULL;

void AddDebugMessage(uint32 debugLevel, const char *pMsg, ...)
{
	if(debugLevel <= (uint32)g_CV_RenderDebug.m_Val)
	{
		va_list marker;
		va_start(marker, pMsg);

		char msg[256];
		LTVSNPrintF(msg, sizeof(msg) - 1, pMsg, marker);

		va_end(marker);

		int32 len = strlen(msg);
		if (msg[len-1] != '\n')
		{
			msg[len] = '\n';
			msg[len+1] = '\0';
		}

		g_pStruct->ConsolePrint(msg);
	}
}


HLTPARAM d3d_MaybeCreateCVar(const char *pName, float defaultVal)
{
	HLTPARAM hRet;

	hRet = g_pStruct->GetParameter(const_cast<char*>(pName));
	if(hRet)
	{
		return hRet;
	}
	else
	{
		char str[256];
		LTSNPrintF(str, sizeof(str), "%s %f", pName, defaultVal);
		g_pStruct->RunConsoleString(str);
		return g_pStruct->GetParameter((char*)pName);
	}
}



void d3d_CreateConsoleVariables()
{
	BaseConVar *pCur;

	for(pCur=g_pConVars; pCur; pCur=pCur->m_pNext)
	{
		pCur->m_hParam = d3d_MaybeCreateCVar(pCur->m_pName, pCur->m_DefaultVal);
	}
}


void d3d_ReadConsoleVariables()
{
	BaseConVar *pCur;

	
	for(pCur=g_pConVars; pCur; pCur=pCur->m_pNext)
	{
		pCur->SetFloat(g_pStruct->GetParameterValueFloat(pCur->m_hParam));
	}

	d3d_ReadExtraConsoleVariables();
}



