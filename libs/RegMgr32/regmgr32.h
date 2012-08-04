#ifndef RegMgr32_H
#define RegMgr32_H

#include "winreg.h"

class CRegMgr32 {
protected:

  BOOL		m_bIsInited;
  HKEY		m_hKey;
  CString	m_csKey;
  CString	CombineKeys(char *szKey1, char *szKey2, char *szKey3, char *szKey4,
					   char *szKey5, char *szKey6, char *szKey7, char *szKey8, char *szKey9);

  
public:
	CRegMgr32() { m_bIsInited = FALSE; };
	~CRegMgr32() { Term(); };
	BOOL	Init();
	void	Term();
	BOOL	OpenKey(HKEY hRootKey, char *szKey1, char *szKey2 = NULL, char *szKey3 = NULL, char *szKey4 = NULL,
					char *szKey5 = NULL, char *szKey6 = NULL, char *szKey7 = NULL, char *szKey8 = NULL, char *szKey9 = NULL);
	BOOL	CreateKey(HKEY hRootKey, char *szKey1, char *szKey2 = NULL, char *szKey3 = NULL, char *szKey4 = NULL,
					char *szKey5 = NULL, char *szKey6 = NULL, char *szKey7 = NULL, char *szKey8 = NULL, char *szKey9 = NULL);
	BOOL	DeleteKey(HKEY hRootKey, char *szKey1, char *szKey2 = NULL, char *szKey3 = NULL, char *szKey4 = NULL,
					char *szKey5 = NULL, char *szKey6 = NULL, char *szKey7 = NULL, char *szKey8 = NULL, char *szKey9 = NULL);

	BOOL	CloseCurrentKey();

	// These functions use the key (m_hKey) opened in OpenKey() or CreateKey()
	BOOL	SetField(char *sField, char *sValue);
	BOOL	SetField(char *sField, void *pValue, int nLen);
	BOOL	SetField(char *sField, DWORD dwValue);
	BOOL	GetField(char *sField, char *sValue, DWORD &dwSize);
	BOOL	GetField(char *sField, void *pValue, DWORD &dwSize);
	BOOL	GetField(char *sField, DWORD *pdwValue);
	BOOL	DeleteField(char *sField);
	//BOOL	GetDefault();

	// These functions will use a temporary key, get or set the field, then close the temporary key
	BOOL	SetField(char *sField, char *sValue, HKEY hRootKey, char *szKey1, char *szKey2 = NULL, char *szKey3 = NULL, char *szKey4 = NULL,
				char *szKey5 = NULL, char *szKey6 = NULL, char *szKey7 = NULL, char *szKey8 = NULL, char *szKey9 = NULL);
	BOOL	SetField(char *sField, void *pValue, int nLen, HKEY hRootKey, char *szKey1, char *szKey2 = NULL, char *szKey3 = NULL, char *szKey4 = NULL,
				char *szKey5 = NULL, char *szKey6 = NULL, char *szKey7 = NULL, char *szKey8 = NULL, char *szKey9 = NULL);
	BOOL	SetField(char *sField, DWORD dwValue, HKEY hRootKey, char *szKey1, char *szKey2 = NULL, char *szKey3 = NULL, char *szKey4 = NULL,
				char *szKey5 = NULL, char *szKey6 = NULL, char *szKey7 = NULL, char *szKey8 = NULL, char *szKey9 = NULL);
	BOOL	GetField(char *sField, char *sValue, DWORD &dwSize, HKEY hRootKey, char *szKey1, char *szKey2 = NULL, char *szKey3 = NULL, char *szKey4 = NULL,
				char *szKey5 = NULL, char *szKey6 = NULL, char *szKey7 = NULL, char *szKey8 = NULL, char *szKey9 = NULL);
	BOOL	GetField(char *sField, void *pValue,DWORD &dwSize, HKEY hRootKey, char *szKey1, char *szKey2 = NULL, char *szKey3 = NULL, char *szKey4 = NULL,
				char *szKey5 = NULL, char *szKey6 = NULL, char *szKey7 = NULL, char *szKey8 = NULL, char *szKey9 = NULL);
	BOOL	GetField(char *sField, DWORD *pdwValue, HKEY hRootKey, char *szKey1, char *szKey2 = NULL, char *szKey3 = NULL, char *szKey4 = NULL,
				char *szKey5 = NULL, char *szKey6 = NULL, char *szKey7 = NULL, char *szKey8 = NULL, char *szKey9 = NULL);
	BOOL	DeleteField(char *sField,HKEY hRootKey, char *szKey1, char *szKey2 = NULL, char *szKey3 = NULL, char *szKey4 = NULL,
				char *szKey5 = NULL, char *szKey6 = NULL, char *szKey7 = NULL, char *szKey8 = NULL, char *szKey9 = NULL);
};            

#endif
