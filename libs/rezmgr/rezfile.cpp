
#include "ltbasedefs.h"
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#define REZMGRDONTUNDEF
#include "rezmgr.h"
#include <string.h>
#include <sys/stat.h>

// -----------------------------------------------------------------------------------------
// CBaseRezFileList

// -----------------------------------------------------------------------------------------
void CBaseRezFileList::VirtualFoo() {
};


// -----------------------------------------------------------------------------------------
// CRezFileSingleFileList

// -----------------------------------------------------------------------------------------
void CRezFileSingleFileList::VirtualFoo() {
};


// -----------------------------------------------------------------------------------------
// CBaseRezFile

// -----------------------------------------------------------------------------------------
CBaseRezFile::CBaseRezFile(CRezMgr* pRezMgr) {
  ASSERT(pRezMgr != NULL);
  m_pRezMgr = pRezMgr;
};


// -----------------------------------------------------------------------------------------
CBaseRezFile::~CBaseRezFile() {
  m_pRezMgr = NULL;
};


// -----------------------------------------------------------------------------------------
void CBaseRezFile::VirtualFoo() {
};

// -----------------------------------------------------------------------------------------
// CRezFile

// -----------------------------------------------------------------------------------------
CRezFile::CRezFile(CRezMgr* pRezMgr) : CBaseRezFile(pRezMgr) {
  ASSERT(pRezMgr != NULL);
  m_pFile = NULL;
  m_sFileName = NULL;
  m_nLastSeekPos = 0xFFFFFFFF;
};


// -----------------------------------------------------------------------------------------
CRezFile::~CRezFile() {
  if (m_pFile != NULL) Close();
  if (m_sFileName != NULL) delete [] m_sFileName;
};


// -----------------------------------------------------------------------------------------
DWORD CRezFile::Read(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData) {
  ASSERT(m_pFile != NULL);
  ASSERT(pData != NULL);
  ASSERT(m_pRezMgr != NULL);

  // if size is zero just return
  if (nSize <= 0) return 0;

  DWORD nSeekPos = nItemPos+nItemOffset;
  if (m_nLastSeekPos != nSeekPos)
  {
		while (fseek(m_pFile, nSeekPos, SEEK_SET) != 0) {
			if (!m_pRezMgr->DiskError()) {
				m_nLastSeekPos = 0xFFFFFFFF;
				ASSERT(FALSE); // Seek Failed!
				return 0;
			}
		}
  }

  DWORD nRetVal;
  while ((nRetVal = fread(pData, 1, nSize, m_pFile)) != nSize) {
		if (!m_pRezMgr->DiskError()) {
			m_nLastSeekPos = 0xFFFFFFFF;
			ASSERT(FALSE); // Read Failed!
			return 0;
		};
  };

  m_nLastSeekPos = nSeekPos+nRetVal;

  return nRetVal;
};


// -----------------------------------------------------------------------------------------
DWORD CRezFile::Write(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData) {
  ASSERT(m_pFile != NULL);
  ASSERT(pData != NULL);
  ASSERT(m_bReadOnly != TRUE);
  ASSERT(m_pRezMgr != NULL);

  m_nLastSeekPos = 0xFFFFFFFF;

  // if size is zero just return
  if (nSize <= 0) return 0;

  while (fseek(m_pFile, nItemPos+nItemOffset, SEEK_SET) != 0) {
    if (!m_pRezMgr->DiskError()) {
      ASSERT(FALSE); // Seek Failed!
      return 0;
    }
  }

  DWORD nRetVal;
  while ((nRetVal = fwrite(pData, 1, nSize, m_pFile)) != nSize) {
    if (!m_pRezMgr->DiskError()) {
      ASSERT(FALSE); // Write Failed!
      return 0;
    };
  };

  return nRetVal;
};


// -----------------------------------------------------------------------------------------
BOOL CRezFile::Open(const char* sFileName, BOOL bReadOnly, BOOL bCreateNew) {
  do {
    if (bCreateNew) {
      if (bReadOnly) return FALSE;
      else m_pFile = fopen(sFileName,"w+b");
    }
    else {
      if (bReadOnly) m_pFile = fopen(sFileName,"rb");
      else m_pFile = fopen(sFileName,"r+b");
    }
    if (m_pFile == NULL) {
      if (!m_pRezMgr->DiskError()) return FALSE;
    }
  } while (m_pFile == NULL);
  m_bReadOnly = bReadOnly;

  if (m_sFileName != NULL) 
	  delete [] m_sFileName;
  
  uint32 nNewStrLen = strlen(sFileName)+1;
  LT_MEM_TRACK_ALLOC(m_sFileName = new char[nNewStrLen],LT_MEM_TYPE_MISC);
  
  if (m_sFileName != NULL) 
	  LTStrCpy(m_sFileName,sFileName,nNewStrLen);

  m_nLastSeekPos = 0xFFFFFFFF;
  return TRUE;
};


// -----------------------------------------------------------------------------------------
BOOL CRezFile::Close() {
  ASSERT(m_pRezMgr != NULL);
  BOOL bRetVal;
  int nCheck;
  if (m_pFile != NULL) {
	do 
	{
	  nCheck = fclose(m_pFile);
 	  if (nCheck == 0) bRetVal = TRUE;
	  else 
	  {
	    bRetVal = FALSE;
        if (!m_pRezMgr->DiskError()) return FALSE;
	  }
    }
    while (bRetVal == FALSE);
  }
  else return FALSE;
  m_pFile = NULL;
  if (m_sFileName != NULL) delete [] m_sFileName;
  m_sFileName = NULL;
  m_nLastSeekPos = 0xFFFFFFFF;
  return bRetVal;
};


// -----------------------------------------------------------------------------------------
BOOL CRezFile::Flush() {
  ASSERT(m_pFile != NULL);
  ASSERT(m_pRezMgr != NULL);
  m_nLastSeekPos = 0xFFFFFFFF;
  if (m_pFile != NULL) {
    BOOL bRetVal;
    int nCheck;
	do 
	{
	  nCheck = fflush(m_pFile);
 	  if (nCheck == 0) bRetVal = TRUE;
	  else 
	  {
	    bRetVal = FALSE;
        if (!m_pRezMgr->DiskError()) return FALSE;
	  }
    }
    while (bRetVal == FALSE);
    return bRetVal;
  }
  else return FALSE;
};


// -----------------------------------------------------------------------------------------
BOOL CRezFile::VerifyFileOpen() {
  m_nLastSeekPos = 0xFFFFFFFF;

  // check if the files is even supposed to be open
  if (m_pFile == NULL) return FALSE;

  // test to see if the file is open
  if (ftell(m_pFile) != -1L) return TRUE;
  else {

    // try to open the file again
    if (!Open(m_sFileName,m_bReadOnly,FALSE)) return FALSE;
  }
  
  return TRUE;
};


// -----------------------------------------------------------------------------------------
char* CRezFile::GetFileName() {
  return m_sFileName;
};


// -----------------------------------------------------------------------------------------
// CRezFileDirectory

// -----------------------------------------------------------------------------------------
CRezFileDirectoryEmulation::CRezFileDirectoryEmulation(CRezMgr* pRezMgr, int nMaxOpenFiles) : CBaseRezFile(pRezMgr) {
  ASSERT(nMaxOpenFiles > 0);
  m_nNumOpenFiles = 0;
  m_nMaxOpenFiles = nMaxOpenFiles;
  m_bReadOnly = TRUE;
  m_bCreateNew = FALSE;
};

// -----------------------------------------------------------------------------------------
CRezFileDirectoryEmulation::~CRezFileDirectoryEmulation() {
  CRezFileSingleFile* pItm;

  // delete all the open single files
  while ((pItm = lstOpenFiles.GetFirst()) != NULL) {
	delete pItm;
  }; 

  // delete all the closed single files
  while ((pItm = lstClosedFiles.GetFirst()) != NULL) {
	delete pItm;
  }; 
};

// -----------------------------------------------------------------------------------------
DWORD CRezFileDirectoryEmulation::Read(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData) {
  ASSERT(FALSE); // this should never be called!
  return 0;
};

// -----------------------------------------------------------------------------------------
DWORD CRezFileDirectoryEmulation::Write(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData) {
  ASSERT(FALSE); // this should never be called!
  return 0;
};

// -----------------------------------------------------------------------------------------
BOOL CRezFileDirectoryEmulation::Open(const char* sFileName, BOOL bReadOnly, BOOL bCreateNew) {
  m_bReadOnly = bReadOnly;
  m_bCreateNew = bCreateNew;
  return TRUE;
};

// -----------------------------------------------------------------------------------------
BOOL CRezFileDirectoryEmulation::Close() {
 CRezFileSingleFile* pItm;
 while ((pItm = lstOpenFiles.GetFirst()) != NULL) {
	pItm->ReallyClose();
  }; 
  return TRUE;
};

// -----------------------------------------------------------------------------------------
BOOL CRezFileDirectoryEmulation::Flush() {
  return TRUE;
};

// -----------------------------------------------------------------------------------------
BOOL CRezFileDirectoryEmulation::VerifyFileOpen() {
  return TRUE;
};

// -----------------------------------------------------------------------------------------
char* CRezFileDirectoryEmulation::GetFileName() {
  return NULL;
};


// -----------------------------------------------------------------------------------------
// CRezFileSingleFile

// -----------------------------------------------------------------------------------------
CRezFileSingleFile::CRezFileSingleFile(CRezMgr* pRezMgr, const char* sFileName, CRezFileDirectoryEmulation* pDirEmulation) : CBaseRezFile(pRezMgr) {
  ASSERT(sFileName != NULL);
  ASSERT(pDirEmulation != NULL);

  m_pDirEmulation = pDirEmulation;
  m_pFile = NULL;

  LT_MEM_TRACK_ALLOC(m_sFileName = new char[strlen(sFileName)+1],LT_MEM_TYPE_MISC);
  ASSERT(m_sFileName != NULL);
  strcpy(m_sFileName,sFileName);

  m_pDirEmulation->lstClosedFiles.Insert(this);
};

// -----------------------------------------------------------------------------------------
CRezFileSingleFile::~CRezFileSingleFile() {
  if (m_pFile != NULL) ReallyClose();
  if (m_sFileName != NULL) delete [] m_sFileName;
  m_pDirEmulation->lstClosedFiles.Delete(this);
};

// -----------------------------------------------------------------------------------------
DWORD CRezFileSingleFile::Read(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData) {
  ASSERT(pData != NULL);
  ASSERT(m_pDirEmulation != NULL);
  ASSERT(m_pDirEmulation->m_pRezMgr != NULL);

  // if size is zero just return
  if (nSize <= 0) return 0;

  // make sure this file is open
  if (m_pFile == NULL) ReallyOpen();

  while (fseek(m_pFile, nItemOffset, SEEK_SET) != 0) {
    if (!m_pDirEmulation->m_pRezMgr->DiskError()) {
      ASSERT(FALSE); // Seek Failed!
      return 0;
    }
  }

  DWORD nRetVal;
  while ((nRetVal = fread(pData, 1, nSize, m_pFile)) != nSize) {
    if (!m_pDirEmulation->m_pRezMgr->DiskError()) {
      ASSERT(FALSE); // Read Failed!
      return 0;
    };
  };

  return nRetVal;
};

// -----------------------------------------------------------------------------------------
DWORD CRezFileSingleFile::Write(DWORD nItemPos, DWORD nItemOffset, DWORD nSize, void* pData) {
  ASSERT(pData != NULL);
  ASSERT(m_pDirEmulation != NULL);
  ASSERT(m_pDirEmulation->m_bReadOnly != TRUE);
  ASSERT(m_pDirEmulation->m_pRezMgr != NULL);

  // if size is zero just return
  if (nSize <= 0) return 0;

  // make sure this file is open
  if (m_pFile == NULL) ReallyOpen();

  while (fseek(m_pFile, nItemOffset, SEEK_SET) != 0) {
    if (!m_pDirEmulation->m_pRezMgr->DiskError()) {
      ASSERT(FALSE); // Seek Failed!
      return 0;
    }
  }

  DWORD nRetVal;
  while ((nRetVal = fwrite(pData, 1, nSize, m_pFile)) != nSize) {
    if (!m_pDirEmulation->m_pRezMgr->DiskError()) {
      ASSERT(FALSE); // Write Failed!
      return 0;
    };
  };

  return nRetVal;
};

// -----------------------------------------------------------------------------------------
BOOL CRezFileSingleFile::Open(const char* sFileName, BOOL bReadOnly, BOOL bCreateNew) {
  ASSERT(FALSE); // this should never be called!
  return FALSE;
};

// -----------------------------------------------------------------------------------------
BOOL CRezFileSingleFile::Close() {
  ASSERT(FALSE); // this should never be called!
  return FALSE;
};

// -----------------------------------------------------------------------------------------
BOOL CRezFileSingleFile::Flush() {
  ASSERT(m_pDirEmulation->m_pRezMgr != NULL);
  if (m_pFile != NULL) {
    BOOL bRetVal;
    while ((bRetVal = (fflush(m_pFile) == 0)) == FALSE) {
      if (!m_pDirEmulation->m_pRezMgr->DiskError()) return FALSE;
    };
    return bRetVal;
  }
  else return TRUE;
};

// -----------------------------------------------------------------------------------------
BOOL CRezFileSingleFile::VerifyFileOpen() {
  ASSERT(FALSE); // this should never be called!
  return FALSE;
};

// -----------------------------------------------------------------------------------------
BOOL CRezFileSingleFile::ReallyOpen() {
  ASSERT(m_sFileName != NULL);
  ASSERT(m_pDirEmulation != NULL);
  ASSERT(m_pDirEmulation->m_pRezMgr != NULL);
  if (m_pFile != NULL) return TRUE;

  // check if we need to close another file because we have too many open
  if (m_pDirEmulation->m_nNumOpenFiles > m_pDirEmulation->m_nMaxOpenFiles) {
    CRezFileSingleFile* pItm = m_pDirEmulation->lstOpenFiles.GetLast();
	ASSERT(pItm != NULL);
    if (pItm != NULL) pItm->ReallyClose();
  }

  do {
    if (m_pDirEmulation->m_bCreateNew) {
      if (m_pDirEmulation->m_bReadOnly) return FALSE;
      else m_pFile = fopen(m_sFileName,"w+b");
    }
    else {
      if (m_pDirEmulation->m_bReadOnly) m_pFile = fopen(m_sFileName,"rb");
      else m_pFile = fopen(m_sFileName,"r+b");
    }
    if (m_pFile == NULL) {
      if (!m_pDirEmulation->m_pRezMgr->DiskError()) return FALSE;
    }
  } while (m_pFile == NULL);

  m_pDirEmulation->lstClosedFiles.Delete(this);
  m_pDirEmulation->lstOpenFiles.InsertFirst(this);
  m_pDirEmulation->m_nNumOpenFiles++;

  return TRUE;
};

// -----------------------------------------------------------------------------------------
BOOL CRezFileSingleFile::ReallyClose() {
  ASSERT(m_pDirEmulation != NULL);
  ASSERT(m_pDirEmulation->m_pRezMgr != NULL);
  if (m_pFile == NULL) return TRUE;

  BOOL bRetVal;
  while ((bRetVal = (fclose(m_pFile) == 0)) == FALSE) {
    if (!m_pDirEmulation->m_pRezMgr->DiskError()) return FALSE;
  };

  m_pDirEmulation->m_nNumOpenFiles--;
  m_pDirEmulation->lstOpenFiles.Delete(this);
  m_pDirEmulation->lstClosedFiles.Insert(this);

  m_pFile = NULL;

  return bRetVal;
};

// -----------------------------------------------------------------------------------------
void CRezFileSingleFile::VirtualFoo() {
};

// -----------------------------------------------------------------------------------------
char* CRezFileSingleFile::GetFileName() {
  return m_sFileName;
};


