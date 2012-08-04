
// d_util defines lots of useful helper functions.

#ifndef __DUTIL_H__
#define __DUTIL_H__

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif


inline char du_Toupper(const char theChar)
{
    if (theChar >= 'a' && theChar <= 'z')
        return 'A' + (theChar - 'a');
    else
        return theChar;
}


void du_strupr(char *pStr);

int du_UpperStrcmp(const char *pStr1, const char *pStr2);


#endif  // __DUTIL_H__

