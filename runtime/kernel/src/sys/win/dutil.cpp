
#include "bdefs.h"
#include "dutil.h"


void du_strupr(char *pStr)
{
	while(*pStr != 0)
	{
		*pStr = du_Toupper(*pStr);
		++pStr;
	}
}


int du_UpperStrcmp(const char *pStr1, const char *pStr2)
{
	for(;;)
	{
		if(du_Toupper(*pStr1) != du_Toupper(*pStr2))
			return 0;
		
		if(*pStr1 == 0)
			return 1;

		++pStr1;
		++pStr2;
	}

	return 0;
}



