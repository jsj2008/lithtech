//----------------------------------------------------------------------------------
// DebugLog.cpp
//----------------------------------------------------------------------------------
#include "DebugLog.h"
#include "CustomInfo.h"


//----------------------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------------------
const char sDebugOutputFile[] = "C:\\SierraUpdate.log";


//----------------------------------------------------------------------------------
// DebugLog: Format and log the info string.
//----------------------------------------------------------------------------------
void DebugLog(const char* sFormat, ...)
{
	if (WONAPI::GetCustomInfo()->GetDebug())
	{
		// Initialize variable arguments.
		va_list ArgList;
		va_start(ArgList, sFormat); //lint !e1924

		// Format the string and dump it to the log file.
		FILE* pFile = fopen(sDebugOutputFile, "at+");
		if (pFile)
		{
			TCHAR sBuffer[8192];
			wvsprintf(sBuffer, sFormat, ArgList);
			fprintf(pFile, "%s", sBuffer);
			fclose(pFile);
		}

		// Reset variable arguments.
		va_end(ArgList); //lint !e1924
	}
}


//----------------------------------------------------------------------------------
// DeleteDebugLog: Delete the debug log file.
//----------------------------------------------------------------------------------
void DeleteDebugLog(void)
{
	DeleteFile(sDebugOutputFile);
}
