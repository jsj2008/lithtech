#ifndef __GENREGMGR_H
#define __GENREGMGR_H

#ifdef USE_PRAGMA_LIB
	#pragma comment (lib, "GenRegMgr.lib")
#endif


class CGenRegMgr
{
public:

	CGenRegMgr()		{ m_pKeys = NULL; m_nKeys = 0; }
	~CGenRegMgr()		{ if (m_pKeys) delete [] m_pKeys; }

	// in all of the following functions, the first part of the "path" in 
	// the string must be one of the following or the function will fail:
	//
	//	HKEY_CLASSES_ROOT
	//	HKEY_CURRENT_USER
	//	HKEY_LOCAL_MACHINE
	//	HKEY_USERS
	//
	//  Example key string: "HKEY_CURRENT_USER/Software/Microsoft/Games/Riot/1.00"
	//

	// creates the key, creating any necessary intermediate keys
	BOOL	CreateKey (LPCSTR strKey);

	// deletes the specified key, deleting any sub-keys it may contain
	BOOL	DeleteKey (LPCSTR strKey);

	// set registry values
	BOOL	SetStringValue (LPCSTR strKey, LPCSTR strValueName, LPCSTR strValue);
	BOOL	SetDwordValue (LPCSTR strKey, LPCSTR strValueName, unsigned int dwValue);
	BOOL	SetBinaryValue (LPCSTR strKey, LPCSTR strValueName, void* pData, unsigned int dwDataSize);
	BOOL	SetStringBoolValue (LPCSTR strKey, LPCSTR strValueName, BOOL bValue);

	// retrieve registry value sizes
	unsigned int	GetValueSize (LPCSTR strKey, LPCSTR strValueName);

	// retrieve registry values
	BOOL	GetValue (LPCSTR strKey, LPCSTR strValueName, void* pBuffer, unsigned int dwBufferSize);
	BOOL	GetDwordValue (LPCSTR strKey, LPCSTR strValueName, unsigned int *pdwBuffer);
	BOOL	GetStringBoolValue (LPCSTR strKey, LPCSTR strValueName, BOOL *pbBuffer);

protected:

	HKEY	OpenKey (LPCSTR strKey);
	void	CloseKey (HKEY hKey);
	BOOL	RecurseAndDeleteKey (HKEY hKeyParent, LPCSTR strKey);

protected:

	HKEY*	m_pKeys;
	unsigned int	m_nKeys;
};

#endif
