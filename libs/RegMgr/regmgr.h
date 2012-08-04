#ifndef RegMgr_H
#define RegMgr_H


// Includes...

#include "lith.h"
#include "winreg.h"


// Classes...

class CRegMgr {
public:

  // public functions
  CRegMgr() { m_bInitialized = FALSE; };
  ~CRegMgr() { Term(); };
  BOOL Init(const char* sCompany, const char* sApp, const char* sVersion, const char* sSubKey = NULL, HANDLE hRootKey = HKEY_LOCAL_MACHINE, char* sRoot2 = NULL);
  void Term();
  BOOL SetSubKey(const char* sSubKey);
  BOOL Set(const char* sKey, const char* sValue);
  BOOL Set(const char* sKey, void* pValue, int nLen);
  BOOL Set(const char* sKey, DWORD nValue);
  char* Get(const char* sKey, char* sBuf, UINT32& nBufSize, const char* sDef = NULL);
  DWORD Get(const char* sKey, DWORD nDef = 0);
  void* Get(const char* sKey, void* pBuf, UINT32& nBufSize, void* pDef = NULL, UINT32 nDefSize = 0);
  BOOL Delete(const char* sKey);
  BOOL DeleteApp();
  BOOL DeleteSubKey();
  BOOL DeleteUnderSubKey(const char* sKey);
  BOOL IsValid() { return (this != NULL); };

private:

  // private member functions
  BOOL CreateKey(HKEY hKey, const char* sSubkey, HKEY& hNewKey);

  // private member variables
  BOOL m_bInitialized;
  HKEY m_hRootKey;
  HKEY m_hSoftwareKey;
  HKEY m_hCompanyKey;
  HKEY m_hAppKey;
  HKEY m_hVersionKey;
  HKEY m_hSubKey;
  char m_sApp[256];
  char m_sSubKey[256];
};            

#endif
