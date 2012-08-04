
#include "bdefs.h"
#include "version_info.h"


void LTVersionInfo::GetString(char *pStr, uint32 nStrBytes)
{
	char tempStr[512];

	LTSNPrintF(tempStr, sizeof(tempStr), "%lu.%lu", m_MajorVersion, m_MinorVersion);
	LTStrCpy(pStr, tempStr, nStrBytes);
}





