
// This has some useful system-dependent stuff for the exporters.
#ifndef __EXPORTSYS_H__
#define __EXPORTSYS_H__


	#include "estring.h"


	// Must call with this first with the app name so it can store the keys in the registry.
	void sys_SetAppInfo(char *pName, char *pVersion);

	BOOL sys_GetStringKey(const char *pKeyName, char *pStr, DWORD maxLen);
	BOOL sys_GetStringKey2(const char *pKeyName, EString &str);
	BOOL sys_SetStringKey(const char *pKeyName, const char *pStr);


#endif



