
#include "windows.h"
#include "regmgr.h"

BOOL CRegMgr::Init(const char* sCompany, const char* sApp, const char* sVersion, const char* sSubKey, HANDLE hRootKey, char* sRoot2)
{
  strcpy(m_sApp,sApp);
  if (sSubKey != NULL) strcpy(m_sSubKey,sSubKey);
  else m_sSubKey[0] = '\0';
  m_hRootKey = (HKEY)hRootKey;
  char sSoftware[] = "Software";
  char* pSoftware;
  if (sRoot2 == NULL) pSoftware = sSoftware;
  else pSoftware = sRoot2;
  if (CreateKey(m_hRootKey, pSoftware, m_hSoftwareKey)) {
    if (CreateKey(m_hSoftwareKey, sCompany, m_hCompanyKey)) {
      if (CreateKey(m_hCompanyKey, sApp, m_hAppKey)) {
        if (CreateKey(m_hAppKey, sVersion, m_hVersionKey)) {
          m_bInitialized = TRUE;
          if (SetSubKey(sSubKey)) {
            return TRUE;
          }
          else {
            m_bInitialized = FALSE;
            return FALSE;
          }
        }
        else return FALSE;
      }
      else return FALSE;
    }
    else return FALSE;
  }
  else return FALSE;
} 

void CRegMgr::Term() {
  if (!m_bInitialized) return;
  m_bInitialized = FALSE;
  RegCloseKey(m_hSoftwareKey);
  RegCloseKey(m_hCompanyKey);
  RegCloseKey(m_hAppKey);
  if (m_hVersionKey != m_hSubKey) RegCloseKey(m_hVersionKey);
  RegCloseKey(m_hSubKey);
};

BOOL CRegMgr::SetSubKey(const char* sSubKey) {
  if (!m_bInitialized) return FALSE;
  if (sSubKey == NULL) {
    m_hSubKey = m_hVersionKey;
    return TRUE;
  }
  else {
    if (CreateKey(m_hVersionKey, sSubKey, m_hSubKey)) return TRUE;
    else return FALSE;
  }
};                                                        

BOOL CRegMgr::Set(const char* sKey, const char* sValue) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  if (sValue == NULL) return NULL;
  if (RegSetValueEx(m_hSubKey,sKey,0,REG_SZ,(const unsigned char*)sValue,strlen(sValue)+1) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::Set(const char* sKey, void* pValue, int nLen) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  if (pValue == NULL) return FALSE;
  if (RegSetValueEx(m_hSubKey,sKey,0,REG_BINARY,(const unsigned char*)pValue,nLen) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::Set(const char* sKey, DWORD nValue) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  if (RegSetValueEx(m_hSubKey,sKey,0,REG_DWORD,(const unsigned char*)&nValue,sizeof(nValue)) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

char* CRegMgr::Get(const char* sKey, char* sBuf, UINT32& nBufSize, const char* sDef) {
  if (!m_bInitialized) goto Default;
  if (sKey == NULL) goto Default;
  if (sBuf == NULL) goto Default;
  if (nBufSize <= 0)  goto Default;
  DWORD nType;
  if (RegQueryValueEx(m_hSubKey,sKey,0,&nType,(unsigned char*)sBuf,( DWORD * )&nBufSize) == ERROR_SUCCESS) {
    if (nType != REG_SZ) goto Default;
    return sBuf;
  }
  else goto Default;
Default:
  if (sDef != NULL) {
    strcpy(sBuf,sDef);
    nBufSize = strlen(sBuf);
    return sBuf;
  }
  else {
    nBufSize = 0;
	return NULL;
  }
};

void* CRegMgr::Get(const char* sKey, void* pBuf, UINT32& nBufSize, void* pDef, UINT32 nDefSize) {
  if (!m_bInitialized) goto Default;
  if (sKey == NULL) goto Default;
  if (pBuf == NULL) goto Default;
  if (nBufSize <= 0) goto Default;
  DWORD nType;
  if (RegQueryValueEx(m_hSubKey,sKey,0,&nType,(unsigned char*)pBuf,( DWORD * )&nBufSize) == ERROR_SUCCESS) {
    if (nType != REG_BINARY) goto Default;
    return pBuf;
  }
  else goto Default;
Default:
  if ((pDef != NULL) && (nDefSize > 0)) {
    memcpy(pBuf,pDef,nDefSize);
	nBufSize = nDefSize;
	return pBuf;
  }
  else {
    nBufSize = 0;
	return NULL;
  }
};

DWORD CRegMgr::Get(const char* sKey, DWORD nDef) {
  if (!m_bInitialized) return nDef;
  if (sKey == NULL) return nDef;
  DWORD nType;
  DWORD nVal;
  DWORD nSize = sizeof(nVal);
  if (RegQueryValueEx(m_hSubKey,sKey,0,&nType,(unsigned char*)&nVal,&nSize) == ERROR_SUCCESS) {
    if (nType != REG_DWORD) return nDef;
    return nVal;
  }
  else return nDef;
};

BOOL CRegMgr::Delete(const char* sKey) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  if (RegDeleteValue(m_hSubKey,sKey) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::CreateKey(HKEY hKey, const char* sSubKey, HKEY& hNewKey) {
  DWORD nDisposition;
//  if (RegCreateKeyEx(hKey, sSubKey, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
  if (RegCreateKeyEx(hKey, sSubKey, 0, "", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hNewKey,
      &nDisposition) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::DeleteApp() {
  if (RegDeleteKey(m_hCompanyKey,m_sApp) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::DeleteSubKey() {
  if (RegDeleteKey(m_hCompanyKey,m_sSubKey) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::DeleteUnderSubKey(const char* sSubKey) {
  if (RegDeleteKey(m_hSubKey,sSubKey) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};
