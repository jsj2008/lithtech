
#include <windows.h>
#include <stdio.h>
#include "exportsys.h"
#include "estring.h"


#define SOFTWARE_KEY	"Software"
#define COMPANY_KEY		"LithTech Inc."
//#define VERSION_KEY		"1.0"
#define VERSION_KEY     IDS_VERSION_NUM


static EString g_NameStr, g_VersionStr;


BOOL CreateKey(HKEY hParent, char *pKeyName, HKEY &hNewKey)
{
	unsigned long disposition;

	return RegCreateKeyEx(hParent, pKeyName, 0, "", REG_OPTION_NON_VOLATILE, 
		KEY_READ|KEY_WRITE,
		NULL,
		&hNewKey,
		&disposition)
		== ERROR_SUCCESS;
}


HKEY GetBaseKey()
{
	HKEY hCur;

	hCur = HKEY_LOCAL_MACHINE;

	if(CreateKey(hCur, SOFTWARE_KEY, hCur))
	{
		if(CreateKey(hCur, COMPANY_KEY, hCur))
		{
			if(CreateKey(hCur, (char*)g_NameStr, hCur))
			{
				if(CreateKey(hCur, (char*)g_VersionStr, hCur))
				{
					return hCur;
				}
			}
		}
	}

	return NULL;
}


void sys_SetAppInfo(char *pName, char *pVersion)
{
	g_NameStr = pName;
	g_VersionStr = pVersion;
}
		

BOOL sys_GetStringKey(const char *pKeyName, char *pStr, DWORD maxLen)
{
	HKEY hBase;
	unsigned long nType;
	unsigned long ulMaxLen = maxLen ;
	pStr[0] = 0;

	hBase = GetBaseKey();
	if(!hBase)
		return FALSE;

	if(RegQueryValueEx(hBase, pKeyName, 0, &nType, 
		(unsigned char*)pStr, &ulMaxLen ) == ERROR_SUCCESS)
	{
		if(nType == REG_SZ)
		{
			return TRUE;
		}
	}

	return FALSE;
}


BOOL sys_GetStringKey2(const char *pKeyName, EString &str)
{
	char tempStr[1024];

	if(sys_GetStringKey(pKeyName, tempStr, sizeof(tempStr)))
	{
		str = tempStr;
		return TRUE;
	}
	else
	{
		str = "";
		return FALSE;
	}
}


BOOL sys_SetStringKey(const char *pKeyName, const char *pStr)
{
	HKEY hBase;

	hBase = GetBaseKey();
	if(!hBase)
		return FALSE;

	return RegSetValueEx(hBase, pKeyName, 0, REG_SZ, 
		(const unsigned char*)pStr, strlen(pStr)+1) == ERROR_SUCCESS;
}


