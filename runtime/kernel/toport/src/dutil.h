
// d_util defines lots of useful helper functions.

#ifndef __D_UTIL_H__
#define __D_UTIL_H__


    #include "ltbasetypes.h"


	inline char du_Toupper(const char theChar)
	{
		if(theChar >= 'a' && theChar <= 'z')
			return 'A' + (theChar - 'a');
		else
			return theChar;
	}


	void du_strupr(char *pStr);

	int du_UpperStrcmp(const char *pStr1, const char *pStr2);


#endif  // __D_UTIL_H__
