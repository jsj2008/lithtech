
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <stdlib.h>
#include "tdguard.h"


char *g_pFileMask;
char *g_pProgramName;

char **g_pExtraArgs;
int g_nExtraArgs;


int CheckExtension(char *pName)
{
	int len;
	char *pTestEnd, *pRealEnd;

	if(strlen(g_pFileMask) >= 3)
	{
		pRealEnd = &g_pFileMask[strlen(g_pFileMask)-3];
	}
	else
	{
		return 0;
	}

	len = strlen(pName);
	if(len >= 3)
	{
		pTestEnd = &pName[len-3];
		if(toupper(pTestEnd[0]) == toupper(pRealEnd[0]) && toupper(pTestEnd[1]) == toupper(pRealEnd[1]) &&
			toupper(pTestEnd[2]) == toupper(pRealEnd[2]))
		{
			return 1;
		}
	}

	return 0;
}


int RecurseAndRun(char *pBaseDir)
{
	char spec[256], cmd[256], tmp[100];
	long handle;
	_finddata_t data;
	int nConverted, i;

	nConverted = 0;
	
	
	// Go into subdirectories.
	sprintf(spec, "%s\\*.*", pBaseDir);
	handle = _findfirst(spec, &data);
	if(handle != -1)
	{
		do
		{
			sprintf(spec, "%s\\%s", pBaseDir, data.name);
				
			if(data.attrib & _A_SUBDIR)
			{
				if(data.name[0] != '.')
				{
					nConverted += RecurseAndRun(spec);
				}
			}
		}
		while(_findnext(handle, &data) != -1);
		
		_findclose(handle);
	}


	// Now check the files.
	sprintf(spec, "%s\\%s", pBaseDir, g_pFileMask);
	handle = _findfirst(spec, &data);
	if(handle != -1)			 
	{
		do
		{
			sprintf(spec, "%s\\%s", pBaseDir, data.name);

			if(!(data.attrib & _A_SUBDIR))
			{
				if(CheckExtension(data.name))
				{
					sprintf(cmd, "%s \"%s\"", g_pProgramName, spec);
					for(i=0; i < g_nExtraArgs; i++)
					{
						sprintf(tmp, " \"%s\" ", g_pExtraArgs[i]);
						strcat(cmd, tmp);
					}

					system(cmd);
				}
			}
		}
		while(_findnext(handle, &data) != -1);
		
		_findclose(handle);
	}

	return nConverted;
}


int main(int argc, char **argv)
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		return 1;
	}

	if(argc < 3)
	{
		printf("Recurse_run <file mask> <program> <extra arguments to progam...>\n");
		printf("Sample: recurse_run *.dtx dtx1to2 -someparam\n");
		printf("For each .dtx file, calls dtx1to2 <filename> -someparam\n\n");
		return 1;
	}

	g_pFileMask = argv[1];
	g_pProgramName = argv[2];
	g_pExtraArgs = &argv[3];
	g_nExtraArgs = argc - 3;
	RecurseAndRun(".");
}






