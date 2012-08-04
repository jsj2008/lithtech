/****************************************************************************
;
;	MODULE:		RezFile (.H)
;
;	PURPOSE:
;
;	HISTORY:	10/06/97 
;
;	NOTICE:		Copyright (c) 1995, MONOLITH, Inc.
;
****************************************************************************/

#ifndef __REZFILE_H__
#define __REZFILE_H__

#ifndef __LITH_H__
#include "lith.h"
#endif


// forward classes
class CRezMgr;
class CBaseRezFile;
class CRezFileSingleFile;

// -----------------------------------------------------------------------------------------
// CBaseRezFileList

class CBaseRezFileList : public CVirtBaseList {
public:
	CBaseRezFile* GetFirst() { return (CBaseRezFile*)CVirtBaseList::GetFirst(); };
	CBaseRezFile* GetLast() { return (CBaseRezFile*)CVirtBaseList::GetLast(); };
	void VirtualFoo();
};


// -----------------------------------------------------------------------------------------
// CRezFileSingleFileList

class CRezFileSingleFileList : public CVirtBaseList {
public:
	CRezFileSingleFile* GetFirst() { return (CRezFileSingleFile*)CVirtBaseList::GetFirst(); };
	CRezFileSingleFile* GetLast() { return (CRezFileSingleFile*)CVirtBaseList::GetLast(); };
	void VirtualFoo();
};


// -----------------------------------------------------------------------------------------
// CBaseRezFile

class CBaseRezFile : public CVirtBaseListItem {
public:
  CBaseRezFile(CRezMgr* pRezMgr); 
  virtual ~CBaseRezFile();
  virtual DWORD Read(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData) = 0;
  virtual DWORD Write(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData) = 0;
  virtual BOOL Open(const char* sFileName, BOOL bReadOnly, BOOL bCreateNew) = 0;
  virtual BOOL Close() = 0;
  virtual BOOL Flush() = 0;
  virtual BOOL VerifyFileOpen() = 0;
  virtual char* GetFileName() = 0;
  CBaseRezFile* Next() { return (CBaseRezFile*)CVirtBaseListItem::Next(); };
  void VirtualFoo();
protected:
  CRezMgr* m_pRezMgr;
};


// -----------------------------------------------------------------------------------------
// CRezFile

class CRezFile : public CBaseRezFile {
public:
  CRezFile(CRezMgr* pRezMgr);
  ~CRezFile();
  virtual DWORD Read(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData);
  virtual DWORD Write(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData);
  virtual BOOL Open(const char* sFileName, BOOL bReadOnly, BOOL bCreateNew);
  virtual BOOL Close();
  virtual BOOL Flush();
  virtual BOOL VerifyFileOpen();
  virtual char* GetFileName();
private:
  FILE* m_pFile;
  char* m_sFileName;
  BOOL m_bReadOnly;
  BOOL m_bCreateNew;
  DWORD m_nLastSeekPos;
};

// -----------------------------------------------------------------------------------------
// CRezFileDirectory

class CRezFileDirectoryEmulation : public CBaseRezFile {
public:
  CRezFileDirectoryEmulation(CRezMgr* pRezMgr, int nMaxOpenFiles);
  ~CRezFileDirectoryEmulation();
  virtual DWORD Read(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData);
  virtual DWORD Write(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData);
  virtual BOOL Open(const char* sFileName, BOOL bReadOnly, BOOL bCreateNew);
  virtual BOOL Close();
  virtual BOOL Flush();
  virtual BOOL VerifyFileOpen();
  virtual char* GetFileName();
private:
  friend class CRezFileSingleFile;
  CRezFileSingleFileList lstOpenFiles;
  CRezFileSingleFileList lstClosedFiles;
  int m_nNumOpenFiles;
  int m_nMaxOpenFiles;
  BOOL m_bReadOnly;
  BOOL m_bCreateNew;
};

// -----------------------------------------------------------------------------------------
// CRezFileSingleFile

class CRezFileSingleFile : public CBaseRezFile {
public:
  CRezFileSingleFile(CRezMgr* pRezMgr, const char* sFileName, CRezFileDirectoryEmulation* pDirEmulation);
  ~CRezFileSingleFile();
  virtual DWORD Read(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData);
  virtual DWORD Write(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData);
  virtual BOOL Open(const char* sFileName, BOOL bReadOnly, BOOL bCreateNew);
  virtual BOOL Close();
  virtual BOOL Flush();
  virtual BOOL VerifyFileOpen();
  virtual char* GetFileName();
  void VirtualFoo();
private:
  friend class CRezFileDirectoryEmulation;
  BOOL ReallyOpen();
  BOOL ReallyClose();
  char* m_sFileName;
  FILE* m_pFile;
  CRezFileDirectoryEmulation* m_pDirEmulation;
};


#endif

