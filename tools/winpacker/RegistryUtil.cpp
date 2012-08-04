#include "stdafx.h"
#include "RegistryUtil.h"

// Gets the value in a registry key, or returns the default if it cannot be found
// The current key must be one of the HKEY_CLASSES_ROOT, etc. For the path, it should
// be like "dir\dir\keyname"
CString GetRegistryKey(HKEY hCurrentKey, const char* pszKeyPath, const char* pszDefault)
{
	CString sPath(pszKeyPath);

	int nSlashPos = sPath.FindOneOf("\\/");

	if(nSlashPos == -1)
	{
		//we hit the end, we need to try and read the value
		LONG nSize;
		if(RegQueryValue(hCurrentKey, sPath, NULL, &nSize) == ERROR_SUCCESS)
		{
			//create a buffer
			char* pszBuffer = new char [nSize + 1];

			LONG nNewSize = nSize + 1;
			RegQueryValue(hCurrentKey, sPath, pszBuffer, &nNewSize);

			//make sure to end it
			pszBuffer[nSize] = '\0';

			CString sRV(pszBuffer);
			delete [] pszBuffer;

			return sRV;
		}
		else
		{
			return CString(pszDefault);
		}
	}
	else
	{
		CString sCurrDir = sPath.Left(nSlashPos);
		
		//trim off the path
		sPath = sPath.Mid(nSlashPos + 1);

		//we need to recurse
		HKEY hNewKey;
		if(RegOpenKey(hCurrentKey, sCurrDir, &hNewKey) == ERROR_SUCCESS)
		{
			CString sRV = GetRegistryKey(hNewKey, sPath, pszDefault);
			RegCloseKey(hNewKey);
			return sRV;
		}
		else
		{
			//not there
			return CString(pszDefault);
		}
	}
}

// This will write out the specified key to the registry. The current key must be one of
// the predefined keys, and the path should be in the format mentioned above
BOOL SetRegistryKey(HKEY hCurrentKey, const char* pszKeyPath, const char* pszValue)
{
	CString sPath(pszKeyPath);

	int nSlashPos = sPath.FindOneOf("\\/");

	//see if we are at the end
	if(nSlashPos == -1)
	{
		//we are at the end, write out the value
		RegSetValue(hCurrentKey, sPath, REG_SZ, pszValue, strlen(pszValue));
		return TRUE;
	}
	else
	{
		CString sCurrDir = sPath.Left(nSlashPos);
		
		//trim off the path
		sPath = sPath.Mid(nSlashPos + 1);

		//we need to recurse
		HKEY hNewKey;
		if(RegOpenKey(hCurrentKey, sCurrDir, &hNewKey) == ERROR_SUCCESS)
		{
			BOOL bRV = SetRegistryKey(hNewKey, sPath, pszValue);
			RegCloseKey(hNewKey);
			return bRV;
		}
		else
		{
			if(RegCreateKey(hCurrentKey, sCurrDir, &hNewKey) != ERROR_SUCCESS)
			{
				return FALSE;
			}
			BOOL bRV = SetRegistryKey(hNewKey, sPath, pszValue);
			RegCloseKey(hNewKey);
			return bRV;
		}
	}
}

//deletes the specified key from the registry
BOOL DeleteRegistryKey(HKEY hCurrentKey, const char* pszKeyPath)
{
	CString sPath(pszKeyPath);

	int nSlashPos = sPath.FindOneOf("\\/");

	//see if we are at the end
	if(nSlashPos == -1)
	{
		//we are at the end, delete the value
		RegDeleteKey(hCurrentKey, sPath);
		return TRUE;
	}
	else
	{
		CString sCurrDir = sPath.Left(nSlashPos);
		
		//trim off the path
		sPath = sPath.Mid(nSlashPos + 1);

		//we need to recurse
		HKEY hNewKey;
		if(RegOpenKey(hCurrentKey, sCurrDir, &hNewKey) == ERROR_SUCCESS)
		{
			BOOL bRV = DeleteRegistryKey(hNewKey, sPath);
			RegCloseKey(hNewKey);
			return bRV;
		}
		else
		{
			return TRUE;
		}
	}
}