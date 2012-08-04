//////////////////////////////////////////////////////////////////////////////
// String manipulation tools

#ifndef __STRTOOLS_H__
#define __STRTOOLS_H__

// Generic case-sensitive string hash code generation function
uint32 st_GetHashCode(const char *pString);

// Generic case-insensitive string hash code generation function
uint32 st_GetHashCode_ic(const char *pString);


#endif //__STRTOOLS_H__
