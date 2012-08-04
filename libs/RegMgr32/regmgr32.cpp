#include "stdafx.h"
#include "regmgr32.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CRegMgr32::Init()
{
	if(m_bIsInited)
		return FALSE;
	
	m_hKey = NULL;

	m_bIsInited = TRUE;
	return(TRUE);
} 

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CRegMgr32::Term()
{
	if(m_bIsInited)
	{
		CloseCurrentKey();
		m_bIsInited = FALSE;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::CloseCurrentKey
//
//	PURPOSE:	Closes any key associated with m_hKey
//
// ----------------------------------------------------------------------- //

BOOL CRegMgr32::CloseCurrentKey()
{
	if(!m_bIsInited)
		return FALSE;

	if(m_hKey)
	{
		if(RegCloseKey(m_hKey) != ERROR_SUCCESS)
			return FALSE;
		m_hKey = NULL;
	}
	m_csKey.Empty();
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::SetField
//
//	PURPOSE:	Uses the previously opened key stored in m_hKey and sets
//				a particular field.
//	
//	NOTE:		To set m_hKey, use CreateKey() or OpenKey();
//
// ----------------------------------------------------------------------- //

BOOL CRegMgr32::SetField(char *sField, char *sValue)
{
	if(!m_bIsInited)
		return FALSE;
	if(!m_hKey)
		return FALSE;
	if (RegSetValueEx(m_hKey,sField,0,REG_SZ,(const unsigned char*)sValue,strlen(sValue)+1) == ERROR_SUCCESS)
		return TRUE;
	return FALSE;
}

BOOL CRegMgr32::SetField(char *sField, void *pValue, int nLen)
{
	if(!m_bIsInited)
		return FALSE;
	if(!m_hKey)
		return FALSE;
	if (RegSetValueEx(m_hKey,sField,0,REG_BINARY,(const unsigned char*)pValue,nLen) == ERROR_SUCCESS)
		return TRUE;
	return FALSE;
}

BOOL CRegMgr32::SetField(char *sField, DWORD dwValue)
{
	if(!m_bIsInited)
		return FALSE;
	if(!m_hKey)
		return FALSE;
	if (RegSetValueEx(m_hKey,sField,0,REG_DWORD,(const unsigned char*)&dwValue,sizeof(dwValue)) == ERROR_SUCCESS)
		return TRUE;
	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::SetField
//
//	PURPOSE:	Sets a particular field in a particular registry key
//	
//	NOTE:		Use these functions if you don't want to call CreateKey()
//				or OpenKey() first
//
// ----------------------------------------------------------------------- //

BOOL CRegMgr32::SetField(char *sField, char *sValue, HKEY hRootKey, char *szKey1, char *szKey2, char *szKey3, char *szKey4,
				char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9)
{
	CString buf = CombineKeys(szKey1,szKey2,szKey3,szKey4,szKey5,szKey6,szKey7,szKey8,szKey9);

	HKEY hTempKey;
	if(RegOpenKeyEx(hRootKey,buf,0,KEY_READ | KEY_WRITE,&hTempKey) != ERROR_SUCCESS)
		return FALSE;

	if (RegSetValueEx(hTempKey,sField,0,REG_SZ,(const unsigned char*)sValue,strlen(sValue)+1) == ERROR_SUCCESS)
	{
		RegCloseKey(hTempKey);
		return TRUE;
	}
	RegCloseKey(hTempKey);
	return FALSE;
}

BOOL CRegMgr32::SetField(char *sField, void *pValue, int nLen, HKEY hRootKey, char *szKey1, char *szKey2, char *szKey3, char *szKey4,
				char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9)
{
	CString buf = CombineKeys(szKey1,szKey2,szKey3,szKey4,szKey5,szKey6,szKey7,szKey8,szKey9);

	HKEY hTempKey;
	if(RegOpenKeyEx(hRootKey,buf,0,KEY_READ | KEY_WRITE,&hTempKey) != ERROR_SUCCESS)
		return FALSE;

	if (RegSetValueEx(hTempKey,sField,0,REG_BINARY,(const unsigned char*)pValue,nLen) == ERROR_SUCCESS)	
	{
		RegCloseKey(hTempKey);
		return TRUE;
	}
	RegCloseKey(hTempKey);
	return FALSE;
}

BOOL CRegMgr32::SetField(char *sField, DWORD dwValue, HKEY hRootKey, char *szKey1, char *szKey2, char *szKey3, char *szKey4,
				char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9)
{
	CString buf = CombineKeys(szKey1,szKey2,szKey3,szKey4,szKey5,szKey6,szKey7,szKey8,szKey9);

	HKEY hTempKey;
	if(RegOpenKeyEx(hRootKey,buf,0,KEY_READ | KEY_WRITE,&hTempKey) != ERROR_SUCCESS)
		return FALSE;

	if (RegSetValueEx(hTempKey,sField,0,REG_DWORD,(const unsigned char*)&dwValue,sizeof(dwValue)) == ERROR_SUCCESS)	
	{
		RegCloseKey(hTempKey);
		return TRUE;
	}
	RegCloseKey(hTempKey);
	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::GetField
//
//	PURPOSE:	Uses the previously opened key stored in m_hKey and gets
//				the value from a particular field.
//	
//	NOTE:		To set m_hKey, use CreateKey() or OpenKey();
//
// ----------------------------------------------------------------------- //

BOOL CRegMgr32::GetField(char *sField, char *sValue, DWORD &dwSize)
{
	if(!m_bIsInited)
		return FALSE;
	if(!m_hKey)
		return FALSE;
	if(dwSize <= 0)
		return FALSE;
	DWORD dwType;
	
	if (RegQueryValueEx(m_hKey,sField,0,&dwType,(unsigned char*)sValue,&dwSize) == ERROR_SUCCESS)
	{
		if(dwType == REG_SZ)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CRegMgr32::GetField(char *sField, void *pValue, DWORD &dwSize)
{
	if(!m_bIsInited)
		return FALSE;
	if(!m_hKey)
		return FALSE;
	if(dwSize <= 0)
		return FALSE;
	DWORD dwType;
	
	if (RegQueryValueEx(m_hKey,sField,0,&dwType,(unsigned char*)pValue,&dwSize) == ERROR_SUCCESS)
	{
		if(dwType == REG_BINARY)
		{
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CRegMgr32::GetField(char *sField, DWORD *pdwValue)
{
	if(!m_bIsInited)
		return FALSE;
	if(!m_hKey)
		return FALSE;
	DWORD dwType;
	DWORD dwVal;
	DWORD dwSize = sizeof(dwVal);
	if (RegQueryValueEx(m_hKey,sField,0,&dwType,(unsigned char*)&dwVal,&dwSize) == ERROR_SUCCESS)
	{
		if (dwType == REG_DWORD)
		{
			*pdwValue = dwVal;
			return TRUE;
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::GetField
//
//	PURPOSE:	Gets a particular field in a particular registry key
//	
//	NOTE:		Use these functions if you don't want to call CreateKey()
//				or OpenKey() first
//
// ----------------------------------------------------------------------- //

BOOL CRegMgr32::GetField(char *sField, char *sValue, DWORD &dwSize, HKEY hRootKey, char *szKey1, char *szKey2, char *szKey3, char *szKey4,
				char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9)
{
	if(dwSize <= 0)
		return FALSE;

	CString buf = CombineKeys(szKey1,szKey2,szKey3,szKey4,szKey5,szKey6,szKey7,szKey8,szKey9);

	HKEY hTempKey;
	if(RegOpenKeyEx(hRootKey,buf,0,KEY_READ | KEY_WRITE,&hTempKey) != ERROR_SUCCESS)
		return FALSE;

	DWORD dwType;
	
	if (RegQueryValueEx(hTempKey,sField,0,&dwType,(unsigned char*)sValue,&dwSize) == ERROR_SUCCESS)
	{
		if(dwType == REG_SZ)
		{
			RegCloseKey(hTempKey);
			return TRUE;
		}
	}
	RegCloseKey(hTempKey);
	return FALSE;
}

BOOL CRegMgr32::GetField(char *sField, void *pValue, DWORD &dwSize, HKEY hRootKey, char *szKey1, char *szKey2, char *szKey3, char *szKey4,
				char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9)
{
	if(dwSize <= 0)
		return FALSE;

	CString buf = CombineKeys(szKey1,szKey2,szKey3,szKey4,szKey5,szKey6,szKey7,szKey8,szKey9);

	HKEY hTempKey;
	if(RegOpenKeyEx(hRootKey,buf,0,KEY_READ | KEY_WRITE,&hTempKey) != ERROR_SUCCESS)
		return FALSE;

	DWORD dwType;
	
	if (RegQueryValueEx(hTempKey,sField,0,&dwType,(unsigned char*)pValue,&dwSize) == ERROR_SUCCESS)
	{
		if(dwType == REG_BINARY)
		{
			RegCloseKey(hTempKey);
			return TRUE;
		}
	}
	RegCloseKey(hTempKey);
	return FALSE;
}

BOOL CRegMgr32::GetField(char *sField, DWORD *pdwValue, HKEY hRootKey, char *szKey1, char *szKey2, char *szKey3, char *szKey4,
				char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9)
{
	CString buf = CombineKeys(szKey1,szKey2,szKey3,szKey4,szKey5,szKey6,szKey7,szKey8,szKey9);

	HKEY hTempKey;
	if(RegOpenKeyEx(hRootKey,buf,0,KEY_READ | KEY_WRITE,&hTempKey) != ERROR_SUCCESS)
		return FALSE;

	DWORD dwType;
	DWORD dwVal;
	DWORD dwSize = sizeof(dwVal);
	if (RegQueryValueEx(hTempKey,sField,0,&dwType,(unsigned char*)&dwVal,&dwSize) == ERROR_SUCCESS)
	{
		if (dwType == REG_DWORD)
		{
			*pdwValue = dwVal;
			RegCloseKey(hTempKey);
			return TRUE;
		}
	}
	RegCloseKey(hTempKey);
	return FALSE;
}



BOOL CRegMgr32::DeleteField(char *sField)
{
	if(!m_bIsInited)
		return FALSE;
	if(!m_hKey)
		return FALSE;
	if (RegDeleteValue(m_hKey,sField) == ERROR_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::DeleteKey
//
//	PURPOSE:	Deletes a specified key (and all of its descendents)
//	
//	NOTE:		In WindowsNT this function will not remove a key if the 
//				key has any descendents.
//
// ----------------------------------------------------------------------- //

BOOL CRegMgr32::DeleteKey(HKEY hRootKey, char *szKey1, char *szKey2, char *szKey3, char *szKey4,
					   char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9)
{
	if(!m_bIsInited)
		return FALSE;

	if(!hRootKey || !szKey1)
		return FALSE;

	CloseCurrentKey();

	CString buf = CombineKeys(szKey1,szKey2,szKey3,szKey4,szKey5,szKey6,szKey7,szKey8,szKey9);
	
	if(RegDeleteKey(hRootKey,buf) == ERROR_SUCCESS)
	{
		return TRUE;
	}
	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::CreateKey
//
//	PURPOSE:	Creates a new key and stores it in m_hKey
//	
//	NOTE:		If the key already exists, CreateKey will return FALSE
//
// ----------------------------------------------------------------------- //

BOOL CRegMgr32::CreateKey(HKEY hRootKey, char *szKey1, char *szKey2, char *szKey3, char *szKey4,
					   char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9)
{
	if(!m_bIsInited)
		return FALSE;

	if(!hRootKey || !szKey1)
		return FALSE;

	CloseCurrentKey();

	m_csKey = CombineKeys(szKey1,szKey2,szKey3,szKey4,szKey5,szKey6,szKey7,szKey8,szKey9);
		
	DWORD dwResult;
	if(RegCreateKeyEx(hRootKey,m_csKey,0,"",REG_OPTION_NON_VOLATILE,KEY_READ | KEY_WRITE,NULL,&m_hKey,&dwResult) == ERROR_SUCCESS)
	{
		if(dwResult == REG_OPENED_EXISTING_KEY)
		{
			CloseCurrentKey();
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::OpenKey
//
//	PURPOSE:	Opens an existing key and stores it in m_hKey
//	
//	NOTE:		If the key doesn't exist, OpenKey will return FALSE
//
// ----------------------------------------------------------------------- //
		
BOOL CRegMgr32::OpenKey(HKEY hRootKey, char *szKey1, char *szKey2, char *szKey3, char *szKey4,
					   char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9)
{	
	if(!m_bIsInited)
		return FALSE;

	if(!hRootKey || !szKey1)
		return FALSE;

	CloseCurrentKey();

	m_csKey = CombineKeys(szKey1,szKey2,szKey3,szKey4,szKey5,szKey6,szKey7,szKey8,szKey9);
	
	if(RegOpenKeyEx(hRootKey,m_csKey,0,KEY_READ | KEY_WRITE,&m_hKey) == ERROR_SUCCESS)
	{
		return TRUE;
	}
	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRegMgr32::CombineKeys
//
//	PURPOSE:	Combines the individual key strings into one large string
//	
// ----------------------------------------------------------------------- //

CString CRegMgr32::CombineKeys(char *szKey1, char *szKey2, char *szKey3, char *szKey4,
					   char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9)
{
	CString buf;
	buf = szKey1;
	if(szKey2)
	{
		buf += '\\';
		buf += szKey2;
		if(szKey3)
		{
			buf += '\\';
			buf += szKey3;
			if(szKey4)
			{
				buf += '\\';
				buf += szKey4;
				if(szKey5)
				{
					buf += '\\';
					buf += szKey5;
					if(szKey6)
					{
						buf += '\\';
						buf += szKey6;
						if(szKey7)
						{
							buf += '\\';
							buf += szKey7;
							if(szKey8)
							{
								buf += '\\';
								buf += szKey8;
								if(szKey9)
								{
									buf += '\\';
									buf += szKey9;
								}
							}
						}
					}
				}
			}
		}
	}
	return buf;
}
