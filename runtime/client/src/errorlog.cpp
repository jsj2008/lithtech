#include "bdefs.h"

#include "concommand.h"
#include "consolecommands.h"
#include "console.h"


static FILE *g_ErrorLogFP = LTNULL;

extern int32	g_bErrorLog;
extern int32	g_bAlwaysFlushLog;
								 

void PrintToErrorLog(const char *pMsg)
{
	if(g_bErrorLog && g_ErrorLogFP)
	{
		fprintf(g_ErrorLogFP, pMsg);
		if (pMsg[strlen(pMsg)-1] != '\n')
			fprintf(g_ErrorLogFP, "\n");
		
		if(g_bAlwaysFlushLog)
			fflush(g_ErrorLogFP);
	}
}

void InitErrorLog()
{
	char *pFilename;
	LTCommandVar *pVar = cc_FindConsoleVar(&g_ClientConsoleState, "errorlogfile");

	pFilename = "error.log";
	if(pVar)
	{
		pFilename = pVar->pStringVal;
	}

	if(g_bErrorLog)
	{
		g_ErrorLogFP = fopen(pFilename, "wtc");
	}

	con_SetErrorLog(PrintToErrorLog);
}

void TermErrorLog()
{
	if(g_ErrorLogFP)
	{
		fflush(g_ErrorLogFP);
		fclose(g_ErrorLogFP);
		g_ErrorLogFP = LTNULL;
	}
}


