#include "windows.h"
#include "assert.h"
#include "genregmgr.h"


//////////////////////////////////////////////////////////////////////////////////////
//						public functions start here
//////////////////////////////////////////////////////////////////////////////////////


BOOL CGenRegMgr::CreateKey (LPCSTR strKey)
{
	HKEY hKey = OpenKey (strKey);
	if (!hKey)
	{
		return FALSE;
	}

	CloseKey (hKey);
	return TRUE;
}

BOOL CGenRegMgr::DeleteKey (LPCSTR strKey)
{
	// first make a copy of the string so we can mess with it

	char* strCopy = new char [strlen (strKey) + 1];
	strcpy (strCopy, strKey);

	// first get the name of the key to delete

	char* ptr = strCopy + strlen (strCopy) - 1;
	while ((*ptr == '\\' || *ptr == '/') && ptr != strCopy)
	{
		*ptr = '\0';
		ptr--;
	}
	while (*ptr != '\\' && *ptr != '/' && ptr != strCopy)
	{
		ptr--;
	}

	if (ptr == strCopy || *(++ptr) == '\0')
	{
		// error in the string
		delete [] strCopy;
		return FALSE;
	}

	char* strDelete = new char [strlen (ptr) + 1];
	strcpy (strDelete, ptr);
	*ptr = '\0';

	// now remove any trailing slashes off what's left of strCopy

	ptr--;
	while ((*ptr == '\\' || *ptr == '/') && ptr != strCopy)
	{
		*ptr = '\0';
		ptr--;
	}

	if (ptr == strCopy)
	{
		// error in the string
		delete [] strCopy;
		delete [] strDelete;
		return FALSE;
	}

	// attempt to open the key before the one to delete

	HKEY hKey = OpenKey (strCopy);
	if (!hKey)
	{
		delete [] strCopy;
		delete [] strDelete;
		return FALSE;
	}

	// now recursively delete keys...

	BOOL bResult = RecurseAndDeleteKey (hKey, strDelete);

	CloseKey (hKey);
	
	delete [] strCopy;
	delete [] strDelete;

	return bResult;
}

BOOL CGenRegMgr::SetStringBoolValue (LPCSTR strKey, LPCSTR strValueName, BOOL bValue)
{
	if ( bValue )
	{
		return SetStringValue(strKey, strValueName, "True");
	}
	else
	{
		return SetStringValue(strKey, strValueName, "False");
	}
}

BOOL CGenRegMgr::GetDwordValue (LPCSTR strKey, LPCSTR strValueName, unsigned int *pdwBuffer)
{
	return GetValue(strKey, strValueName, (void *)pdwBuffer, sizeof(unsigned int));
}

BOOL CGenRegMgr::GetStringBoolValue (LPCSTR strKey, LPCSTR strValueName, BOOL *pbBuffer)
{
	char szBuffer[256];
	if ( GetValue(strKey, strValueName, (char *)szBuffer, sizeof(szBuffer)) )
	{
		strupr(szBuffer);
		if ( strcmp(szBuffer, "TRUE") == 0 )
		{
			*pbBuffer=TRUE;
		}
		else
		{
			*pbBuffer=FALSE;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CGenRegMgr::SetStringValue (LPCSTR strKey, LPCSTR strValueName, LPCSTR strValue)
{
	HKEY hKey = OpenKey (strKey);
	if (!hKey)
	{
		return FALSE;
	}

	LONG lResult = RegSetValueEx (hKey, strValueName, 0, REG_SZ, (LPBYTE)strValue, strlen (strValue) + 1);

	CloseKey (hKey);

	return (lResult == ERROR_SUCCESS);
}

BOOL CGenRegMgr::SetDwordValue (LPCSTR strKey, LPCSTR strValueName, unsigned int dwValue)
{
	HKEY hKey = OpenKey (strKey);
	if (!hKey)
	{
		return FALSE;
	}

	LONG lResult = RegSetValueEx (hKey, strValueName, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof (unsigned int));

	CloseKey (hKey);

	return (lResult == ERROR_SUCCESS);
}

BOOL CGenRegMgr::SetBinaryValue (LPCSTR strKey, LPCSTR strValueName, void* pData, unsigned int dwDataSize)
{
	HKEY hKey = OpenKey (strKey);
	if (!hKey)
	{
		return FALSE;
	}

	LONG lResult = RegSetValueEx (hKey, strValueName, 0, REG_BINARY, (LPBYTE)pData, dwDataSize);

	CloseKey (hKey);

	return (lResult == ERROR_SUCCESS);
}

unsigned int CGenRegMgr::GetValueSize (LPCSTR strKey, LPCSTR strValueName)
{
	HKEY hKey = OpenKey (strKey);
	if (!hKey)
	{
		return 0;
	}

	unsigned int dwDataSize = 0;
	RegQueryValueEx (hKey, strValueName, NULL, NULL, NULL, (unsigned long *)&dwDataSize);

	CloseKey (hKey);

	return dwDataSize;
}

BOOL CGenRegMgr::GetValue (LPCSTR strKey, LPCSTR strValueName, void* pBuffer, unsigned int dwBufferSize)
{
	HKEY hKey = OpenKey (strKey);
	if (!hKey)
	{
		return FALSE;
	}

	LONG lResult = RegQueryValueEx (hKey, strValueName, NULL, NULL, (LPBYTE)pBuffer, (unsigned long *)&dwBufferSize);

	CloseKey (hKey);

	return (lResult == ERROR_SUCCESS);
}



//////////////////////////////////////////////////////////////////////////////////////
//						protected functions start here
//////////////////////////////////////////////////////////////////////////////////////



HKEY CGenRegMgr::OpenKey (LPCSTR strKey)
{
	// get to the first part of the string

	while (*strKey == '/' || *strKey == '\\')
	{
		strKey++;
	}

	// make a copy of the string so we have one to work with...

	char* strCopy = new char [strlen (strKey) + 1];
	strcpy (strCopy, strKey);

	// subsequent error handler

	if (0)
	{
	error:
		delete [] strCopy;

		if (m_pKeys)
		{
			for (int i = m_nKeys - 1; i > 0; i++)
			{
				if (m_pKeys[i])
				{
					RegCloseKey (m_pKeys[i]);
				}
			}
		}
		delete [] m_pKeys;
		m_pKeys = NULL;
		m_nKeys = 0;

		return NULL;
	}

	// get the number of keys in the string...

	char* ptr = strCopy;
	
	unsigned int nKeys = *ptr ? 1 : 0;
	while (*ptr)
	{
		if (*ptr == '/' || *ptr == '\\')
		{
			while (*ptr == '/' || *ptr == '\\')
			{
				ptr++;
			}
			if (*ptr)
			{
				nKeys++;
			}
		}

		ptr++;
	}

	// there should always be at least one keys (the root at least)

	if (nKeys < 1)
	{
		return NULL;
	}

	// get the root key

	HKEY hKeyRoot = NULL;
	char sep[] = "\\/";
	
	ptr = strtok (strCopy, sep);
	if (stricmp (ptr, "HKEY_CLASSES_ROOT") == 0)
	{
		hKeyRoot = HKEY_CLASSES_ROOT;
	}
	else if (stricmp (ptr, "HKEY_CURRENT_USER") == 0)
	{
		hKeyRoot = HKEY_CURRENT_USER;
	}
	else if (stricmp (ptr, "HKEY_LOCAL_MACHINE") == 0)
	{
		hKeyRoot = HKEY_LOCAL_MACHINE;
	}
	else if (stricmp (ptr, "HKEY_USERS") == 0)
	{
		hKeyRoot = HKEY_USERS;
	}

	if (!hKeyRoot)
	{
		return NULL;
	}

	if (nKeys == 1)
	{
		return hKeyRoot;
	}

	// allocate array of keys

	m_nKeys = nKeys - 1;
	m_pKeys = new HKEY [m_nKeys];
	memset (m_pKeys, 0, sizeof (HKEY) * m_nKeys);
	
	m_pKeys[0] = hKeyRoot;

	// create or open the intermediate keys...

	unsigned int dwDisposition = 0;
	for (unsigned int i = 1; i < m_nKeys; i++)
	{
		ptr = strtok (NULL, sep);
		assert (ptr);

		if (RegCreateKeyEx (m_pKeys[i - 1], ptr, 0, "", 0, (KEY_READ | KEY_WRITE), NULL, &m_pKeys[i], (unsigned long *)&dwDisposition) != ERROR_SUCCESS)
		{
			goto error;
		}

		if (dwDisposition == REG_CREATED_NEW_KEY)
		{
			if (RegOpenKeyEx (m_pKeys[i - 1], ptr, 0, (KEY_READ | KEY_WRITE), &m_pKeys[i]) != ERROR_SUCCESS)
			{
				goto error;
			}
		}
	}

	// now create and open the new key

	ptr = strtok (NULL, sep);
	assert (ptr);
	HKEY hKeyReturned = NULL;
	if (RegCreateKeyEx (m_pKeys[m_nKeys - 1], ptr, 0, "", 0, (KEY_READ | KEY_WRITE), NULL, &hKeyReturned, (unsigned long *)&dwDisposition) != ERROR_SUCCESS)
	{
		goto error;
	}

	if (dwDisposition == REG_CREATED_NEW_KEY)
	{
		if (RegOpenKeyEx (m_pKeys[m_nKeys - 1], ptr, 0, (KEY_READ | KEY_WRITE), &hKeyReturned) != ERROR_SUCCESS)
		{
			goto error;
		}
	}
	
	delete [] strCopy;

	return hKeyReturned;
}

void CGenRegMgr::CloseKey (HKEY hKey)
{
	assert (m_nKeys && hKey);

	// close the last key...

	RegCloseKey (hKey);

	// now go backwards up the tree closing keys...

	for (unsigned int i = m_nKeys - 1; i > 0; i--)
	{
		RegCloseKey (m_pKeys[i]);
	}

	// clean up

	delete [] m_pKeys;
	m_pKeys = NULL;
	m_nKeys = 0;
}

BOOL CGenRegMgr::RecurseAndDeleteKey (HKEY hKeyParent, LPCSTR strKey)
{
	// open this key to see if it has any children

	HKEY hKey = NULL;
	if (RegOpenKeyEx (hKeyParent, strKey, 0, (KEY_READ | KEY_WRITE), &hKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	// recurse through this key's children, deleting them first

	char strBuff[512];
	unsigned int dwSizeBuff = 512;
	FILETIME ftime;
	
	unsigned int dwIndex = 0;
	while (1)
	{
		LONG lResult = RegEnumKeyEx (hKey, dwIndex, strBuff, (unsigned long *)&dwSizeBuff, NULL, NULL, NULL, &ftime);
		if (lResult == ERROR_NO_MORE_ITEMS)
		{
			break;
		}
		if (lResult != ERROR_SUCCESS)
		{
			return FALSE;
		}

		if (!RecurseAndDeleteKey (hKey, strBuff))
		{
			return FALSE;
		}

		dwIndex++;
	}

	RegCloseKey (hKey);
	if (RegDeleteKey (hKeyParent, strKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	return TRUE;
}