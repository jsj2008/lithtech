
#include "ltbasedefs.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "ltmem.h"
#ifdef _WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif
#define REZMGRDONTUNDEF
#include "rezmgr.h"
//#include "ltbasedefs.h"

#ifdef _CONSOLE
#define TRACE printf
#else
#define TRACE dprintf
#endif

#ifndef _WIN32
const unsigned int _MAX_DRIVE = 3;
const unsigned int _MAX_DIR = 256;
const unsigned int _MAX_FNAME = 256;
const unsigned int _MAX_EXT = 256;
void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext)
{
  ASSERT(FALSE && "no splitpath implementation on Linux");
}

struct _finddata_t {
	unsigned int attrib;
	time_t time_create;
	time_t time_access;
	time_t time_write;
	unsigned long size;
	char name[260];
};

enum { _S_IFDIR, _A_SUBDIR };
long _findfirst(char* filespec, _finddata_t* fileinfo) { return -1; }
int _findnext(long handle, _finddata_t* fileinfo) { return -1; }
int _findclose(long handle) { return -1; } 

#endif // !_WIN32


//***************************************************************************************************
// file format data structures

#pragma pack(1)
struct FileMainHeaderStruct {
  char CR1;
  char LF1;
  char FileType[RezMgrUserTitleSize];
  char CR2;
  char LF2;
  char UserTitle[RezMgrUserTitleSize];
  char CR3;
  char LF3;
  char EOF1;
  DWORD FileFormatVersion; 	// the file format version number only 1 is possible here right now
  DWORD RootDirPos; 		// Position of the root directory structure in the file
  DWORD RootDirSize;        // Size of root directory
  DWORD RootDirTime;        // Time Root dir was last updated
  DWORD NextWritePos;       // Position of first directory in the file
  DWORD Time; 				// Time resource file was last updated
  DWORD LargestKeyAry;		// Size of the largest key array in the resource file
  DWORD LargestDirNameSize; // Size of the largest directory name in the resource file (including 0 terminator)
  DWORD LargestRezNameSize;	// Size of the largest resource name in the resource file (including 0 terminator)
  DWORD LargestCommentSize;	// Size of the largest comment in the resource file (including 0 terminator)
  BYTE  IsSorted;           // If 0 then data is not sorted if 1 then it is sorted
};

enum FileDirEntryType {
  ResourceEntry = 0,
  DirectoryEntry = 1
};

struct FileDirEntryDirHeader {
  DWORD Pos; 					// File positon of dir entry
  DWORD Size;					// Size of directory data
  DWORD Time;					// Last time anything in directory was modified
//  char Name[];				// Name of this directory
};

struct FileDirEntryRezHeader {
  DWORD Pos; 					// File positon of dir entry
  DWORD Size;					// Size of directory data
  DWORD Time;					// Last time this resource was modified
  DWORD ID;                     // Resource ID number
  DWORD Type;					// Type of resource this is
  DWORD NumKeys;				// The number of keys to read in for this resource
//  char Name[];				// The name of this resource
//  char Comment[];             // The comment data for this resource
//  DWORD Keys[];				// The key values for this resource
};

struct FileDirEntryHeader {
  DWORD Type;
  union {
    FileDirEntryRezHeader Rez;
	FileDirEntryDirHeader Dir;
  };
};
#pragma pack()

//***************************************************************************************************
// CRezItm implementation

//---------------------------------------------------------------------------------------------------
CRezItm::CRezItm()
{
  m_pRezFile = NULL;
  m_pParentDir = NULL;
  m_sName = NULL;
  m_heName.SetRezItm(this);
};

//---------------------------------------------------------------------------------------------------
void CRezItm::InitRezItm(CRezDir* pParentDir, REZNAME sName, REZID nID, CRezTyp* pType, REZDESC sDesc,
                    REZSIZE nSize, DWORD nFilePos, REZTIME nTime, DWORD nNumKeys, REZKEYVAL* pKeyAry,
				    CBaseRezFile* pRezFile) {
  ASSERT(pParentDir != NULL);
  m_pRezFile = pRezFile;
  m_pParentDir = pParentDir;
  if (sName == NULL) m_sName = NULL;
  else {
    LT_MEM_TRACK_ALLOC(m_sName = new char [strlen(sName)+1],LT_MEM_TYPE_MISC);
    ASSERT(m_sName != NULL);
    if (m_sName != NULL) strcpy(m_sName,sName);
  }

  m_pType = pType;

  m_nSize = nSize;
  m_nFilePos = nFilePos;
  m_nTime = nTime;

  m_pData = NULL;
  m_nCurPos = 0;

  m_heName.SetRezItm(this);
};

//---------------------------------------------------------------------------------------------------
void CRezItm::TermRezItm() {
  // free up all members
  if (m_sName != NULL) delete [] m_sName;

  if (m_pParentDir != NULL) {
    if (m_pParentDir->m_pMemBlock == NULL) {
      if (m_pData != NULL) delete [] m_pData;
    }
  }
  else if (m_pData != NULL) delete [] m_pData;


  // set members back to default settings
  m_sName = NULL;

  m_pType = NULL;

  m_nTime = 0;
  m_nSize = 0;
  m_pData = NULL;

  m_pParentDir = NULL;
  m_nFilePos = 0;
  m_nCurPos = 0;

  m_heName.SetRezItm(NULL);
};

//---------------------------------------------------------------------------------------------------
REZTYPE	CRezItm::GetType() { 
  ASSERT(m_pType != NULL); 
  return m_pType->GetType(); 
};

//---------------------------------------------------------------------------------------------------
REZPATH	CRezItm::GetPath(char* Buf, unsigned int BufSize) {
  // Note! This is really slow, could be sped up!
  ASSERT(Buf != NULL);
  ASSERT(BufSize > 0);

  // if this is in the root directory just return that
  if (m_pParentDir->m_pParentDir == NULL) {
    LTStrCpy(Buf,"\\", BufSize);
  }

  // if this is not the root dir
  else {

    // allocate memory for temp buffer and clear initial buffer
    char* TempBuf;
	LT_MEM_TRACK_ALLOC(TempBuf = new char[BufSize],LT_MEM_TYPE_MISC);
    ASSERT(TempBuf != NULL);
    strcpy(Buf,"");

    // loop thru all directories appending them to the path
    ASSERT(m_pParentDir != NULL);
    CRezDir* pDir = m_pParentDir;
    while (pDir != NULL) {
      ASSERT((strlen(pDir->GetDirName())+strlen(Buf)+1) < BufSize);
      strcpy(TempBuf,Buf);
      if (pDir->m_pParentDir != NULL) strcpy(Buf,"\\");
  	  else Buf[0] = '\0';
      strcat(Buf,pDir->GetDirName());
      strcat(Buf,TempBuf);
      pDir = pDir->m_pParentDir;
    }

    // remove temp buffer
    delete [] TempBuf;
  }

  return Buf;
};

//---------------------------------------------------------------------------------------------------
REZDIRNAME CRezItm::GetDir() {
  ASSERT(m_pParentDir != NULL);
  return m_pParentDir->GetDirName();
};

//---------------------------------------------------------------------------------------------------
BYTE* CRezItm::Load() {
  ASSERT(m_pParentDir != NULL);
  ASSERT(m_pRezFile != NULL);

  // check if the whole directory is in memory already
  if (m_pParentDir->m_pMemBlock != NULL) {
    return m_pParentDir->m_pMemBlock+m_nFilePos-m_pParentDir->m_nItemsPos;
  }

  // check if the data is already in memory
  if (m_pData != NULL) return m_pData;

  // allocate memory for the data
  if (m_nSize == 0) return NULL;
  LT_MEM_TRACK_ALLOC(m_pData = new BYTE[m_nSize],LT_MEM_TYPE_MISC);
  ASSERT(m_pData != NULL);
  if (m_pData == NULL) return NULL;

  // load in the data from disk
  ASSERT(m_pParentDir->m_pRezMgr != NULL);
  if (m_pRezFile->Read(m_nFilePos,0,m_nSize,m_pData) != m_nSize) {
    delete [] m_pData;
    m_pData = NULL;
  }

  return m_pData;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezItm::UnLoad() {
  if (m_pData != NULL) {
    delete [] m_pData;
    m_pData = NULL;
  } 
  return TRUE;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezItm::IsLoaded() { 
  ASSERT(m_pParentDir != NULL);
  if (m_pParentDir->m_pMemBlock != NULL) return TRUE;
  else return (m_pData != NULL); 
}; 

//---------------------------------------------------------------------------------------------------
BOOL CRezItm::Get(BYTE* pBytes) {
  return (Get(pBytes,0,m_nSize));
};

//---------------------------------------------------------------------------------------------------
BOOL CRezItm::Get(BYTE* pBytes, DWORD startOffset, DWORD length) {
  ASSERT(m_pParentDir != NULL);
  ASSERT(m_pRezFile != NULL);
  ASSERT(pBytes != NULL);
  ASSERT(length > 0);
  ASSERT(length <= m_nSize+startOffset);

  // Check if the whole directory is in memory already and just copy it if it is
  if (m_pParentDir->m_pMemBlock != NULL) {
    memcpy(pBytes,m_pParentDir->m_pMemBlock+m_nFilePos+startOffset-m_pParentDir->m_nItemsPos,length);
    return TRUE;
  }

  // Check if this resource is in memory already and just copy it if so
  if (m_pData != NULL) {
    memcpy(pBytes,m_pData+startOffset,length);
    return TRUE;
  }

  // Load this part of the resource from disk
  ASSERT(m_pParentDir->m_pRezMgr != NULL);
  if (m_pRezFile->Read(m_nFilePos,startOffset,length,pBytes) != length) return FALSE;
  
  return TRUE;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezItm::Seek(DWORD offset) {
  m_nCurPos = offset;
  return TRUE;
};

//---------------------------------------------------------------------------------------------------
DWORD CRezItm::Read(BYTE* pBytes, DWORD length, DWORD seekPos) {
  ASSERT(m_pParentDir != NULL);
  ASSERT(pBytes != NULL);
  ASSERT(m_pRezFile != NULL);

  // do seek if necessary
  if (seekPos != 0xffffffff) Seek(seekPos);

  // check if we are already past the end of the file
  if (m_nCurPos > m_nSize) return 0;

  // truncate length if necessary
  if ((length+m_nCurPos) > m_nSize) length = m_nSize-m_nCurPos;

  // if length is zero just return
  if (length <= 0) return 0;

  // Check if the whole directory is in memory already and just copy it if it is
  if (m_pParentDir->m_pMemBlock != NULL) {
    memcpy(pBytes,m_pParentDir->m_pMemBlock+m_nFilePos+m_nCurPos-m_pParentDir->m_nItemsPos,length);
    m_nCurPos += length; 
    return length;
  }

  // Check if this resource is in memory already and just copy it if so
  if (m_pData != NULL) {
    memcpy(pBytes,m_pData+m_nCurPos,length);
    m_nCurPos += length; 
    return length;
  }

  // Load from disk
  ASSERT(m_pParentDir->m_pRezMgr != NULL);
  if (m_pRezFile->Read(m_nFilePos,m_nCurPos,length,pBytes) == length) {
    m_nCurPos += length;
    return length;
  }
  else {
    return 0;
  };
};

//---------------------------------------------------------------------------------------------------
BOOL CRezItm::EndOfRes() {
  if (m_nCurPos >= m_nSize) return TRUE;
  else return FALSE;
};

//---------------------------------------------------------------------------------------------------
char CRezItm::GetChar() {
  // This could be way more efficent!!!!!!
  char Ch;
  Read((BYTE*)&Ch,1);
  return Ch;
};


BYTE* CRezItm::Create(DWORD Size) {
  ASSERT(m_pParentDir != NULL);
  ASSERT(m_pParentDir->m_pRezMgr != NULL);
  ASSERT(m_pParentDir->m_pRezMgr->m_bReadOnly != TRUE); // You can't do this if the resource was open read only!

  // store the size the resource used to be
  DWORD nOldSize = m_nSize;

  // make sure old memory is removed
  UnLoad();

  // make sure parent does not have resources in memory (if so remove them)
  ASSERT(m_pParentDir != NULL);
  if (m_pParentDir->m_pMemBlock != NULL) m_pParentDir->UnLoad();

  // allocate the new memory and set the new size member
  m_nSize = Size;
  LT_MEM_TRACK_ALLOC(m_pData = new BYTE[m_nSize],LT_MEM_TYPE_MISC);
  ASSERT(m_pData != NULL);

  // mark this resource as not existing in the resource file yet
  m_nFilePos = 0;

  // update the directory items size in the parent directory
  m_pParentDir->m_nItemsSize += m_nSize;
  m_pParentDir->m_nItemsSize -= nOldSize;

  // mark data as not sorted
  m_pParentDir->m_pRezMgr->m_bIsSorted = FALSE;

  // update the timestamp for this item
  MarkCurTime();
  
  return m_pData;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezItm::Save() {
  ASSERT(m_pData != NULL);
  ASSERT(m_pRezFile != NULL);
  ASSERT(m_pParentDir != NULL);
  ASSERT(m_pParentDir->m_pRezMgr != NULL);
  ASSERT(m_pParentDir->m_pRezMgr->m_bReadOnly != TRUE); // You can't do this if the resource was open read only!

  // we don't have to do anything if the size is 0
  if (m_nSize <= 0) return TRUE;

  // if this resource has never been saved
  if (m_nFilePos == 0) {

    // mark resource file as not sorted
    m_pParentDir->m_pRezMgr->m_bIsSorted = FALSE;

    // write out the data
    if (m_pRezFile->Write(m_pParentDir->m_pRezMgr->m_nNextWritePos,0,m_nSize,m_pData) != m_nSize) {
      ASSERT(FALSE); // failed to write out data
      return FALSE;
    }

    // set the position we wrote the data to
    m_nFilePos = m_pParentDir->m_pRezMgr->m_nNextWritePos;

    // update start of dir data position
    m_pParentDir->m_pRezMgr->m_nNextWritePos += m_nSize;
  }

  // if this resource has been saved before (just write it where it belongs)
  else {

    // write out the data
    if (m_pRezFile->Write(m_nFilePos,0,m_nSize,m_pData) != m_nSize) {
      ASSERT(FALSE); // failed to write out data
      return FALSE;
    }
  }

  // update the timestamp for this item
  MarkCurTime();

  return TRUE;
};


void CRezItm::MarkCurTime() {
  ASSERT(m_pParentDir != NULL);
  ASSERT(m_pParentDir->m_pRezMgr != NULL);
  ASSERT(m_pParentDir->m_pRezMgr->m_bReadOnly != TRUE); // You can't do this if the resource was open read only!

  // get the current time
  REZTIME nTime = m_pParentDir->m_pRezMgr->GetCurTime();

  // mark this item
  m_nTime = nTime;

  // mark this items directory
  m_pParentDir->m_nLastTimeModified = nTime;

  // mark the main rez file
  m_pParentDir->m_pRezMgr->m_nLastTimeModified = nTime;
};


//***************************************************************************************************
// CRezType implementation

//---------------------------------------------------------------------------------------------------
CRezTyp::CRezTyp(REZTYPE nType, CRezDir* pParentDir, unsigned int nByIDNumHashBins, unsigned int nByNameNumHashBins) : m_haID(nByIDNumHashBins) , m_haName(nByNameNumHashBins) {
  m_nType = nType;
  m_heType.SetRezTyp(this);
  m_pParentDir = pParentDir;
};                                    

//---------------------------------------------------------------------------------------------------
CRezTyp::CRezTyp(REZTYPE nType, CRezDir* pParentDir, unsigned int nByNameNumHashBins) : m_haID() , m_haName(nByNameNumHashBins) {
  m_nType = nType;
  m_heType.SetRezTyp(this);
  m_pParentDir = pParentDir;
};                                    

//---------------------------------------------------------------------------------------------------
CRezTyp::~CRezTyp() {
  // remove all of the items in the ByID hash table 
  // Don't delete objectes here, they are deleted in the ByName table below!
  if (m_pParentDir->m_pRezMgr->m_bItemByIDUsed)
  {
    CRezItmHashByID* pItm = m_haID.GetFirst();
    CRezItmHashByID* pDel;
    while (pItm != NULL) {
      pDel = pItm;
      pItm = pItm->Next();
      m_haID.Delete(pDel);
    }
  }

  // remove all of the items in the ByName hash table and delete the objects
  {
    CRezItmHashByName* pItm = m_haName.GetFirst();
    CRezItmHashByName* pDel;
    while (pItm != NULL) {
      pDel = pItm;
      pItm = pItm->Next();
      m_haName.Delete(pDel);
      ASSERT(pDel->GetRezItm() != NULL);
//	  delete pDel->GetRezItm();
      pDel->GetRezItm()->TermRezItm();
      m_pParentDir->m_pRezMgr->DeAllocateRezItm(pDel->GetRezItm());
    }
  }

  m_nType = 0;
  m_heType.SetRezTyp(NULL);
};


//***************************************************************************************************
// CRezDir implementation

//---------------------------------------------------------------------------------------------------
CRezDir::CRezDir(CRezMgr* pRezMgr, CRezDir* pParentDir, REZDIRNAME szDirName, DWORD nDirPos, DWORD nDirSize, REZTIME nTime, unsigned int nDirNumHashBins, unsigned int nTypNumHashBins)
        : m_haDir(nDirNumHashBins), m_haTypes(nTypNumHashBins) {
  ASSERT(pRezMgr != NULL);
  ASSERT(szDirName != NULL);

  // allocate and copy name
  LT_MEM_TRACK_ALLOC(m_sDirName = new char[strlen(szDirName)+1],LT_MEM_TYPE_MISC);
  ASSERT(m_sDirName != NULL);
  if (m_sDirName != NULL) strcpy(m_sDirName,szDirName);

  // set valuse for other member functions
  m_nLastTimeModified = nTime;
  m_nDirSize = nDirSize;
  m_nDirPos = nDirPos;
  m_nItemsSize = 0;
  m_nItemsPos = 0;
  m_pMemBlock = NULL;
  m_pRezMgr = pRezMgr;
  m_pParentDir = pParentDir;
  m_heDir.SetRezDir(this);
};

//---------------------------------------------------------------------------------------------------
CRezDir::~CRezDir() {
  // remove all typ hash table contents and delete the objects
  {
    CRezTypeHash* pItm = m_haTypes.GetFirst();
    CRezTypeHash* pDel;
    while (pItm != NULL) {
      pDel = pItm;
      pItm = pItm->Next();
      m_haTypes.Delete(pDel);
      ASSERT(pDel->GetRezTyp() != NULL);
      delete pDel->GetRezTyp();
    }
  }

  // remove all dir hash table contents and delete the objects
  {
    CRezDirHash* pItm = m_haDir.GetFirst();
    CRezDirHash* pDel;
    while (pItm != NULL) {
      pDel = pItm;
      pItm = pItm->Next();
      m_haDir.Delete(pDel);
      ASSERT(pDel->GetRezDir() != NULL);
      delete pDel->GetRezDir();
    }
  }

  // remove simple member data
  if (m_sDirName != NULL) delete [] m_sDirName;
  if (m_pMemBlock != NULL) delete [] m_pMemBlock;

  // reset variables to default settings
  m_sDirName = NULL;
  m_nLastTimeModified = 0;
  m_nDirSize = 0;
  m_nDirPos = 0;
  m_nItemsSize = 0;
  m_nItemsPos = 0;
  m_pMemBlock = NULL;
  m_pRezMgr = NULL;
  m_pParentDir = NULL;
  m_heDir.SetRezDir(NULL);
};

//---------------------------------------------------------------------------------------------------
CRezItm* CRezDir::GetRez(REZNAME sRes, REZTYPE nType) {
  ASSERT(sRes != NULL);

  // get the Type for the resource
  CRezTyp* pTyp = m_haTypes.Find(nType);
  if (pTyp == NULL) return NULL;

  // get the item
  return pTyp->m_haName.Find(sRes,!GetParentMgr()->GetLowerCaseUsed());
};


//---------------------------------------------------------------------------------------------------
CRezItm* CRezDir::GetRezFromDosName(char* sDosName) {
  ASSERT(sDosName != NULL);
//  ASSERT(strlen(sDosName) < 13);
  char sExt[5];
  REZTYPE nType;

  // split the file name up into its parts
  char drive[_MAX_DRIVE+1];
  char dir[_MAX_DIR+1];
  char fname[_MAX_FNAME+1+_MAX_EXT+1];
  char ext[_MAX_EXT+1];
  _splitpath(sDosName, drive, dir, fname, ext );

  // check if the extension is too long
  BOOL bExtensionTooLong = FALSE;
  if (strlen(ext) > 5)
  {
 		bExtensionTooLong = TRUE;
		strncat(fname,ext,_MAX_FNAME+1+_MAX_EXT);
		fname[_MAX_FNAME] = '\0';
		nType = 0;
  }
  else
  {
  
	// figure out the Type for this file
	if (strlen(ext) > 0) {
		strcpy(sExt,&ext[1]);
		strupr(sExt);
		nType = m_pRezMgr->StrToType(sExt);
	}
	else nType = 0;
  }

  // call the normal GetRez function
  return GetRez(fname,nType);
};

//---------------------------------------------------------------------------------------------------
BOOL CRezDir::Load(BOOL LoadAllSubDirs) {

  // if this directory is already loaded then just return TRUE
  if (m_pMemBlock != NULL) return TRUE;

  // if the file is not sorted then we cannot load it by directory (actualy we could but it would take more effort)
  // we also can't do this if we have loaded more than 1 rez file
  if ((m_pRezMgr->m_bIsSorted == FALSE) || (m_pRezMgr->m_nNumRezFiles > 1)) {
//    TRACE("CRezDir::Load Failed! (File is not sorted!)\n");
    return FALSE;
  }

  // if the data size is 0 then we don't need to do anything
  if (m_nItemsSize > 0) {

    // allocate memory for data
    LT_MEM_TRACK_ALLOC(m_pMemBlock = new BYTE[m_nItemsSize],LT_MEM_TYPE_MISC);
    ASSERT(m_pMemBlock != NULL);
    if (m_pMemBlock != NULL) {

      // read in data
      ASSERT(m_pRezMgr != NULL);
      ASSERT(m_nItemsPos > 0);
      m_pRezMgr->m_pPrimaryRezFile->Read(m_nItemsPos,0,m_nItemsSize,m_pMemBlock);
    }
  }

  // load all sub directories data if we are supposed to
  if (LoadAllSubDirs) {
    CRezDirHash* pItm = m_haDir.GetFirst();
    while (pItm != NULL) {
      ASSERT(pItm->GetRezDir() != NULL);
      pItm->GetRezDir()->Load(TRUE);
      pItm = pItm->Next();
    }
  }

  return TRUE;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezDir::UnLoad(BOOL UnLoadAllSubDirs) {
  // delete this directory whole data if it exists
  if (m_pMemBlock != NULL) {
    delete [] m_pMemBlock;
    m_pMemBlock = NULL;
  }

  // if the memory is not stored globally then unload all items individually
  else {

    // go through all types in this directory 
    CRezTyp* pTyp = GetFirstType();
    while (pTyp != NULL) {

      // go through all items in this type and transfer them to the new dir
      CRezItm* pItm = GetFirstItem(pTyp);
      while (pItm != NULL) {
        pItm->UnLoad();
        pItm = GetNextItem(pItm);
      }

      pTyp = GetNextType(pTyp);
    }
  }

  // delete all sub directories data if we are supposed to
  if (UnLoadAllSubDirs) {
    CRezDirHash* pItm = m_haDir.GetFirst();
    while (pItm != NULL) {
      ASSERT(pItm->GetRezDir() != NULL);
      pItm->GetRezDir()->UnLoad(TRUE);
      pItm = pItm->Next();
    }
  }

  return TRUE;
};

//---------------------------------------------------------------------------------------------------
CRezDir* CRezDir::GetDir(REZCDIRNAME sDir) {
  ASSERT(sDir != NULL);
  if (sDir == NULL) return NULL;
  return m_haDir.Find(sDir,!GetParentMgr()->GetLowerCaseUsed());
};

//---------------------------------------------------------------------------------------------------
CRezDir* CRezDir::GetFirstSubDir() {
  CRezDirHash* pHash = m_haDir.GetFirst();
  if (pHash == NULL) return NULL;
  ASSERT(pHash->GetRezDir() != NULL);
  return pHash->GetRezDir();
};

//---------------------------------------------------------------------------------------------------
CRezDir* CRezDir::GetNextSubDir(CRezDir* pRezDir) {
  ASSERT(pRezDir != NULL);
  CRezDirHash* pHash = pRezDir->m_heDir.Next();
  if (pHash == NULL) return NULL;
  ASSERT(pHash->GetRezDir() != NULL);
  return pHash->GetRezDir();
};

//---------------------------------------------------------------------------------------------------
CRezTyp* CRezDir::GetRezTyp(REZTYPE nType) {
  return m_haTypes.Find(nType);;
};

//---------------------------------------------------------------------------------------------------
CRezTyp* CRezDir::GetFirstType() {
  CRezTypeHash* pHash = m_haTypes.GetFirst();
  if (pHash == NULL) return NULL;
  ASSERT(pHash->GetRezTyp() != NULL);
  return pHash->GetRezTyp();
};

//---------------------------------------------------------------------------------------------------
CRezTyp* CRezDir::GetNextType(CRezTyp* pRezType) {
  ASSERT(pRezType != NULL);
  CRezTypeHash* pHash = pRezType->m_heType.Next();
  if (pHash == NULL) return NULL;
  ASSERT(pHash->GetRezTyp() != NULL);
  return pHash->GetRezTyp();
};

//---------------------------------------------------------------------------------------------------
CRezItm* CRezDir::GetFirstItem(CRezTyp* pRezType) {
  ASSERT(pRezType != NULL);
  CRezItmHashByName* pHash = pRezType->m_haName.GetFirst();
  if (pHash == NULL) return NULL;
  return pHash->GetRezItm();
};

//---------------------------------------------------------------------------------------------------
CRezItm* CRezDir::GetNextItem(CRezItm* pRezItm) {
  ASSERT(pRezItm != NULL);
  CRezItmHashByName* pHash = pRezItm->m_heName.Next();
  if (pHash == NULL) return NULL;
  return pHash->GetRezItm();
};

//---------------------------------------------------------------------------------------------------
CRezDir* CRezDir::CreateDir(REZDIRNAME sDir) {
  ASSERT(sDir != NULL);
  ASSERT(m_pRezMgr != NULL);
//  ASSERT(m_pRezMgr->m_bReadOnly == FALSE);  // You can't do this if the resource was open read only!

  // make sure directory does not already exist
  CRezDir* pDir = m_haDir.Find(sDir,!GetParentMgr()->GetLowerCaseUsed());
  ASSERT(!(pDir != NULL));
  if (pDir != NULL) return NULL;

  // create new directory structure
  LT_MEM_TRACK_ALLOC(pDir = new CRezDir(m_pRezMgr, this, sDir, 0, 0, m_pRezMgr->GetCurTime(), m_pRezMgr->m_nDirNumHashBins, m_pRezMgr->m_nTypNumHashBins),LT_MEM_TYPE_MISC);
  ASSERT(pDir != NULL);
  if (pDir == NULL) return NULL;

  // add to this directories hash table
  m_haDir.Insert(&pDir->m_heDir);

  // update largest variables
  DWORD sNameLen = strlen(sDir);
  if (m_pRezMgr->m_nLargestDirNameSize <= sNameLen) m_pRezMgr->m_nLargestDirNameSize = sNameLen+1;

  return pDir;
};


//---------------------------------------------------------------------------------------------------
CRezItm* CRezDir::CreateRez(REZID nID, REZNAME sName, REZTYPE nType) {
  ASSERT(sName != NULL);
  ASSERT(m_pRezMgr != NULL);
  ASSERT(m_pRezMgr->m_bReadOnly == FALSE);  // You can't do this if the resource was open read only!

  // find or create the type for this resource
  CRezTyp* pTyp = GetOrMakeTyp(nType);
  ASSERT(pTyp != NULL);

  // make sure resource does not already exist in ID hash table
  CRezItm* pItm = NULL;

  // make sure resource does not already exist in Name hash table
  pItm = pTyp->m_haName.Find(sName,!GetParentMgr()->GetLowerCaseUsed());
  if (pItm != NULL) return NULL;

  // create new resource structure
//  pItm = new CRezItm(this,sName,nID,pTyp,NULL,0,0,m_pRezMgr->GetCurTime(),0,NULL,m_pRezMgr->m_pPrimaryRezFile);
  pItm = m_pRezMgr->AllocateRezItm();
  pItm->InitRezItm(this,sName,nID,pTyp,NULL,0,0,m_pRezMgr->GetCurTime(),0,NULL,m_pRezMgr->m_pPrimaryRezFile);
  ASSERT(pItm != NULL);
  if (pItm == NULL) return NULL;

  // add to the types hash tables
  pTyp->m_haName.Insert(&pItm->m_heName);

  // update largest variables
  DWORD sNameLen = strlen(sName);
  if (m_pRezMgr->m_nLargestRezNameSize <= sNameLen) m_pRezMgr->m_nLargestRezNameSize = sNameLen+1;

  return pItm;
};

//---------------------------------------------------------------------------------------------------
CRezItm* CRezDir::CreateRezInternal(REZID nID, REZNAME sName, CRezTyp* pTyp, CBaseRezFile* pRezFile) {
  ASSERT(sName != NULL);
  ASSERT(m_pRezMgr != NULL);

  // create new resource structure
//  CRezItm* pItm = new CRezItm(this,sName,nID,pTyp,NULL,0,0,m_pRezMgr->GetCurTime(),0,NULL,pRezFile);
  CRezItm* pItm = m_pRezMgr->AllocateRezItm();
  ASSERT(pItm != NULL);
  if (pItm == NULL) return NULL; 
  pItm->InitRezItm(this,sName,nID,pTyp,NULL,0,0,m_pRezMgr->GetCurTime(),0,NULL,pRezFile);

  // add to the types hash tables
  pTyp->m_haName.Insert(&pItm->m_heName);

  // update largest variables
  DWORD sNameLen = strlen(sName);
  if (m_pRezMgr->m_nLargestRezNameSize <= sNameLen) m_pRezMgr->m_nLargestRezNameSize = sNameLen+1;

  return pItm;
};

BOOL CRezDir::RemoveRezInternal(CRezTyp* pTyp, CRezItm* pItm) {
  ASSERT(pItm != NULL);

  // update the directory items size
  m_nItemsSize -= pItm->m_nSize;

  // remove from hash tables
  pTyp->m_haName.Delete(&pItm->m_heName);

  // delete item
//  delete pItm;
  pItm->TermRezItm();
  m_pRezMgr->DeAllocateRezItm(pItm);

  // mark data as not sorted
  m_pRezMgr->m_bIsSorted = FALSE;

  return TRUE;
};


//---------------------------------------------------------------------------------------------------
BOOL CRezDir::ReadAllDirs(CBaseRezFile* pRezFile, DWORD Pos, DWORD Size, BOOL bOverwriteItems) {
  BOOL bRetFlag = TRUE;
  ASSERT(Pos > 0);

  // return if this is an empty directory
  if (Size <= 0) return TRUE;

  // clear out the DirPos variables in any currently existing directories so we don't try to read a dir we shouldn't
  {
    CRezDirHash* pDir = m_haDir.GetFirst();
    while (pDir != NULL) {
      CRezDir* pRezDir = pDir->GetRezDir();
      ASSERT(pRezDir != NULL);
      pRezDir->m_nDirPos = 0;
      pDir = pDir->Next();
    }
  }

  // read in this directory
  if (ReadDirBlock(pRezFile, Pos, Size, bOverwriteItems)) {

    // loop through all directorys in this directory and recursivly call this function
    CRezDirHash* pDir = m_haDir.GetFirst();
    while (pDir != NULL) {
      CRezDir* pRezDir = pDir->GetRezDir();
      ASSERT(pRezDir != NULL);
	  if (pRezDir->m_nDirPos != 0) {
        if (!pRezDir->ReadAllDirs(pRezFile, pRezDir->m_nDirPos, pRezDir->m_nDirSize, bOverwriteItems)) bRetFlag = FALSE;
      }
      pDir = pDir->Next();
    }
  }

  else bRetFlag = FALSE;

  return bRetFlag;
};


//---------------------------------------------------------------------------------------------------
BOOL CRezDir::ReadDirBlock(CBaseRezFile* pRezFile, DWORD Pos, DWORD Size, BOOL bOverwriteItems) {
  ASSERT(Pos > 0);
  m_nItemsSize = 0;
  m_nItemsPos = 0xffffffff;
  DWORD nLastItemPos = 0;
  DWORD nLastItemSize = 0;

  // allocate memory for directory block
  BYTE* pBlk;
  LT_MEM_TRACK_ALLOC(pBlk = new BYTE[Size],LT_MEM_TYPE_MISC);
  ASSERT(pBlk != NULL);
  if (pBlk == NULL) return FALSE;

  // read in directory block (exit if error)
  if (pRezFile->Read(Pos,0,Size,pBlk) != Size) {
    delete [] pBlk;
    return FALSE;
  };

  // process all data in directory block 
  BYTE* pCur = pBlk;
  BYTE* pEnd = pBlk+Size;
  while (pCur < pEnd) {

    // if this is a directory entry
    if ((*(DWORD*)pCur) == DirectoryEntry) {
      pCur += sizeof(DWORD);

      // variables to store data read in
      DWORD Pos;
      DWORD Size;
      DWORD Time;
      char* sDirName;

      // convert simple header variables
      Pos = (*(DWORD*)pCur);
      pCur += sizeof(DWORD);
      Size = (*(DWORD*)pCur);
      pCur += sizeof(DWORD);
      Time = (*(DWORD*)pCur);
      pCur += sizeof(DWORD);

      // convert dir name
      sDirName = (char*)pCur;
      pCur += strlen(sDirName)+1;

	  // make sure this dir doesn't already exist if it does we don't need to add it again
	  CRezDir* pDir = m_haDir.Find(sDirName,!GetParentMgr()->GetLowerCaseUsed());
	  if (pDir == NULL) {	

        // construct new directory item
        LT_MEM_TRACK_ALLOC(pDir = new CRezDir(m_pRezMgr, this, sDirName, Pos, Size, Time, m_pRezMgr->m_nDirNumHashBins, m_pRezMgr->m_nTypNumHashBins),LT_MEM_TYPE_MISC);
        ASSERT(pDir != NULL);

        // insert new directory item in hash table
        m_haDir.Insert(&pDir->m_heDir);
      }
	  else {
	    pDir->m_nDirPos = Pos;
		pDir->m_nDirSize = Size;
		pDir->m_nLastTimeModified = Time;
	  }
    }

    // if this is a resource item entry
    else {
      ASSERT((*(DWORD*)pCur) == ResourceEntry); // Type is not a resource or a directory!!!!
      pCur += sizeof(DWORD);

      // variables to store data read in
      DWORD Pos;
      DWORD Size;
      DWORD Time;
      DWORD ID;
      DWORD Type;
      DWORD NumKeys;
      char* sName;
      char* sDescription;
      DWORD* pKeyAry;

      // convert simple header variables
      Pos = (*(DWORD*)pCur);
      pCur += sizeof(DWORD);
      Size = (*(DWORD*)pCur);
      pCur += sizeof(DWORD);
      Time = (*(DWORD*)pCur);
      pCur += sizeof(DWORD);
      ID = (*(DWORD*)pCur);
      pCur += sizeof(DWORD);
      Type = (*(DWORD*)pCur);
      pCur += sizeof(DWORD);
      NumKeys = (*(DWORD*)pCur);
      pCur += sizeof(DWORD);

      // convert item name
      sName = (char*)pCur;
      pCur += strlen(sName)+1;

      // find or create the type for this resource
      CRezTyp* pTyp = GetOrMakeTyp(Type);
      ASSERT(pTyp != NULL);

	  // if this gets set to true then this item will not be added
	  BOOL bSkipThisItem = FALSE;


	  // check if this name already exists and deal with it
	  {
        CRezItm* pDupNameItem = pTyp->m_haName.Find(sName);
		if (pDupNameItem != NULL) {
		  if (bOverwriteItems) {
		    RemoveRezInternal(pTyp,pDupNameItem);
		  }
		  else {
  		    bSkipThisItem = TRUE;
		  }
		}
      }

      // convert description name
      sDescription = (char*)pCur;
      pCur += strlen(sDescription)+1;
      if (sDescription[0] == '\0') sDescription = NULL;

      // convert KeyAry
      if (NumKeys > 0) {
        LT_MEM_TRACK_ALLOC(pKeyAry = new REZKEYVAL[NumKeys],LT_MEM_TYPE_MISC);
        ASSERT(pKeyAry != NULL);
        for (unsigned int i = 0; i < NumKeys; i++) {
          pKeyAry[i] = *(DWORD*)pCur;
          pCur += sizeof(DWORD);
        }
      }
      else pKeyAry = NULL;

	  // check if we are really going to add this item
 	  if (!bSkipThisItem) {

        // construct new resource item
//        CRezItm* pItm = new CRezItm(this,sName,ID,pTyp,sDescription,Size,Pos,Time,NumKeys,pKeyAry,pRezFile);
		CRezItm* pItm = m_pRezMgr->AllocateRezItm();
        ASSERT(pItm != NULL);
		pItm->InitRezItm(this,sName,ID,pTyp,sDescription,Size,Pos,Time,NumKeys,pKeyAry,pRezFile);

        // insert new resource item in hash tables
        pTyp->m_haName.Insert(&pItm->m_heName);

        // update the directory items size and position
        m_nItemsSize += pItm->m_nSize;
        if (pItm->m_nFilePos < m_nItemsPos) m_nItemsPos = pItm->m_nFilePos;
        if (pItm->m_nFilePos > nLastItemPos) {
          nLastItemPos = pItm->m_nFilePos;
          nLastItemSize = pItm->m_nSize;
        }
	  }

      // delete any needed allocations
      if (pKeyAry != NULL) delete [] pKeyAry;
    }
  };

  // check if resource data was sorted
//  if (m_nItemsPos != 0xffffffff) { // make sure there were items (if not we don't care)
//    if (m_nItemsSize != (nLastItemPos+nLastItemSize-m_nItemsPos)) m_pRezMgr->m_bIsSorted = FALSE;
//  }

  // free memory for block
  delete [] pBlk;

  return TRUE;
};

//---------------------------------------------------------------------------------------------------
CRezTyp* CRezDir::GetOrMakeTyp(REZTYPE nType) {
  // find the type 
  CRezTyp* pTyp = m_haTypes.Find(nType);

  // create a new type if this one is not found
  if (pTyp == NULL) {
    
    // create type structure
	if (m_pRezMgr->m_bItemByIDUsed)
	{
		LT_MEM_TRACK_ALLOC(pTyp = new CRezTyp(nType, this, m_pRezMgr->m_nByIDNumHashBins, m_pRezMgr->m_nByNameNumHashBins),LT_MEM_TYPE_MISC);
	}
	else
	{
		LT_MEM_TRACK_ALLOC(pTyp = new CRezTyp(nType, this, m_pRezMgr->m_nByNameNumHashBins),LT_MEM_TYPE_MISC);
	}
    ASSERT(pTyp != NULL);
    if (pTyp == NULL) return NULL;

    // add new type to typ hash table
    m_haTypes.Insert(&pTyp->m_heType);
  }

  return pTyp;
};


//***************************************************************************************************
// CRezMgr implementation

//---------------------------------------------------------------------------------------------------
CRezMgr::CRezMgr() {
  m_bFileOpened = FALSE; 
  m_bRenumberIDCollisions = TRUE;
  m_nNextIDNumToUse = 2000000000;	
  m_pPrimaryRezFile = NULL;
  m_nNumRezFiles = 0;
  m_nRootDirPos = 0;
  m_nNextWritePos = 0;
  m_bReadOnly = TRUE;
  m_pRootDir = NULL;
  m_nLastTimeModified = 0;
  m_bMustReWriteDirs = FALSE;
  m_nFileFormatVersion = 0;
  m_nLargestKeyAry = 0;
  m_nLargestDirNameSize = 0;
  m_nLargestRezNameSize = 0;
  m_nLargestCommentSize = 0;
  m_bIsSorted = TRUE;
  m_sFileName = NULL;
  m_nMaxOpenFilesInEmulatedDir = 3;
  m_sDirSeparators = NULL;
  m_bLowerCaseUsed = FALSE;
  m_bItemByIDUsed = FALSE;
  m_nByNameNumHashBins = kDefaultByNameNumHashBins;
  m_nByIDNumHashBins = kDefaultByIDNumHashBins;	
  m_nDirNumHashBins = kDefaultDirNumHashBins;	
  m_nTypNumHashBins = kDefaultTypNumHashBins;	
  m_nRezItmChunkSize = 100;	
  m_sUserTitle[0] = '\0';	
 };

//---------------------------------------------------------------------------------------------------
CRezMgr::CRezMgr(const char* FileName, BOOL ReadOnly, BOOL CreateNew) {
  CRezMgr();
  Open(FileName,ReadOnly,CreateNew);
};

//---------------------------------------------------------------------------------------------------
CRezMgr::~CRezMgr() {
  if (m_bFileOpened) Close();
  CBaseRezFile* pRezFile;
  while ((pRezFile = m_lstRezFiles.GetFirst()) != NULL) {
    m_lstRezFiles.Delete(pRezFile);
	m_nNumRezFiles--;
	delete pRezFile;
  }
  if (m_pRootDir != NULL) {
    delete m_pRootDir;
    m_pRootDir = NULL;
  }
  if (m_sFileName != NULL) {
    delete [] m_sFileName;
    m_sFileName = NULL;
  }
  if (m_sDirSeparators != NULL) 
  {
	  delete [] m_sDirSeparators;
	  m_sDirSeparators = NULL;
  }
  m_bFileOpened = FALSE; 
  m_pPrimaryRezFile = NULL;
  m_nRootDirPos = 0;
  m_nRootDirSize = 0;
  m_nRootDirTime = 0;
  m_nNextWritePos = 0;
  m_bReadOnly = TRUE;
  m_pRootDir = NULL;
  m_nLastTimeModified = 0;
  m_bMustReWriteDirs = FALSE;
  m_nFileFormatVersion = 1;
  m_nLargestKeyAry = 0;
  m_nLargestDirNameSize = 0;
  m_nLargestRezNameSize = 0;
  m_nLargestCommentSize = 0;
  m_bIsSorted = TRUE;
  m_sFileName = NULL;

  // remove all RezItm Chunks
  {
	CRezItmChunk* pChunk;
	while ((pChunk = m_lstRezItmChunks.GetFirst()) != NULL)
	{
		delete [] pChunk->m_pRezItmAry;
		m_lstRezItmChunks.Delete(pChunk);
		delete pChunk;
	}
  }
};

//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::Open(const char* FileName, BOOL ReadOnly, BOOL CreateNew) {
  ASSERT(FileName != NULL); // NULL file name!
  ASSERT(!(ReadOnly && CreateNew)); // gee, we can't do both!
  m_bReadOnly = ReadOnly;


  // store file name in class
  if (m_sFileName != NULL) delete [] m_sFileName;
  LT_MEM_TRACK_ALLOC(m_sFileName = new char[strlen(FileName)+1],LT_MEM_TYPE_MISC);
  ASSERT(m_sFileName != NULL);
  strcpy(m_sFileName,FileName);

  // check if user has asked us to open a directory instead of a rez file
  if (IsDirectory(FileName)) {

    // make sure this is only open for read only
	ASSERT(m_bReadOnly != FALSE);  // we can only read from directories not write to them!
    if (m_bReadOnly == FALSE) return FALSE;

    // Create the CRezFileDirectoryEmulation object
    CRezFileDirectoryEmulation* pRezFile;
	LT_MEM_TRACK_ALLOC(pRezFile = new CRezFileDirectoryEmulation(this,m_nMaxOpenFilesInEmulatedDir),LT_MEM_TYPE_MISC);
    ASSERT(pRezFile != NULL);
    if (pRezFile == NULL) {
	  delete [] m_sFileName;
	  m_sFileName = NULL;
      return FALSE;
    }
    m_pPrimaryRezFile = pRezFile;
    m_lstRezFiles.Insert(pRezFile);
    m_nNumRezFiles++;

    // open the file
    if (!pRezFile->Open(FileName,ReadOnly,CreateNew)) return FALSE;
    m_bFileOpened = TRUE;

	// create empty root directory
    LT_MEM_TRACK_ALLOC(m_pRootDir = new CRezDir(this, NULL, "", 0, 0, GetCurTime(), m_nDirNumHashBins, m_nTypNumHashBins),LT_MEM_TYPE_MISC);
    ASSERT(m_pRootDir != NULL);

	// read in data from directory and all sub directories
	ReadEmulationDirectory(pRezFile,m_pRootDir,m_sFileName,FALSE);

    return TRUE;
  }

  // create a new CRezFile object for the RezFile
  CRezFile* pRezFile;
  LT_MEM_TRACK_ALLOC(pRezFile = new CRezFile(this),LT_MEM_TYPE_MISC);
  ASSERT(pRezFile != NULL);
  if (pRezFile == NULL) {
	delete [] m_sFileName;
	m_sFileName = NULL;
    return FALSE;
  }
  m_pPrimaryRezFile = pRezFile;
  m_lstRezFiles.Insert(pRezFile);
  m_nNumRezFiles++;

  // open the file
  if (!pRezFile->Open(FileName,ReadOnly,CreateNew)) return FALSE;
  m_bFileOpened = TRUE;

  // set up variables if this is a new file
  if (CreateNew) {
    m_nNextWritePos = sizeof(FileMainHeaderStruct);
    m_bMustReWriteDirs = TRUE;

    LT_MEM_TRACK_ALLOC(m_pRootDir = new CRezDir(this, NULL, "", 0, 0, GetCurTime(), m_nDirNumHashBins, m_nTypNumHashBins),LT_MEM_TYPE_MISC);
    ASSERT(m_pRootDir != NULL);
  }

  // if this is an old file read in header and directories
  else {

    // read in the header
    FileMainHeaderStruct Header;
    pRezFile->Read(0,0,sizeof(Header),&Header);

    // store header values in RezMgr class
    m_nNextWritePos =       Header.NextWritePos;
    m_nRootDirPos =         Header.RootDirPos;
    m_nRootDirSize =        Header.RootDirSize;
    m_nRootDirTime =        Header.RootDirTime;
    m_nLastTimeModified =   Header.Time;
    m_nFileFormatVersion =  Header.FileFormatVersion;
    m_nLargestKeyAry =      Header.LargestKeyAry;
    m_nLargestDirNameSize = Header.LargestDirNameSize;
    m_nLargestRezNameSize = Header.LargestRezNameSize;
    m_nLargestCommentSize = Header.LargestCommentSize;
    m_bIsSorted =           Header.IsSorted;

	// set the internal value for the user title in the header
	memcpy(m_sUserTitle, Header.UserTitle, RezMgrUserTitleSize);
	m_sUserTitle[RezMgrUserTitleSize] = '\0';
	{
		for (int i = RezMgrUserTitleSize-1; i >= 0; i--)
		{
			if (m_sUserTitle[i] != ' ') break;
			m_sUserTitle[i] = '\0';
		}
	}

    // verify that this is a valid file
    ASSERT(Header.CR1 == 0x0d);
    ASSERT(Header.LF2 == 0x0a);
    ASSERT(Header.EOF1 == 0x1a);
    ASSERT(m_nFileFormatVersion == 1);
	if (Header.CR1 != 0x0d) return FALSE;
	if (Header.LF2 != 0x0a) return FALSE;
	if (Header.EOF1 != 0x1a) return FALSE;
	if (m_nFileFormatVersion != 1) return FALSE;

    // create root directory
    LT_MEM_TRACK_ALLOC(m_pRootDir = new CRezDir(this, NULL, "", m_nRootDirPos, m_nRootDirSize, m_nRootDirTime, m_nDirNumHashBins, m_nTypNumHashBins),LT_MEM_TYPE_MISC);
    ASSERT(m_pRootDir != NULL);

    // read in directories
    m_pRootDir->ReadAllDirs(pRezFile, m_nRootDirPos, m_nRootDirSize, FALSE);
  }

  return TRUE;
}; 

//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::OpenAdditional(const char* FileName, BOOL bOverwriteItems) {
  ASSERT(FileName != NULL); // NULL file name!
  ASSERT(m_pRootDir != NULL);  // must have already called open
  ASSERT(m_bReadOnly == TRUE); // if original rez wasn't read only we can't do this!
  BOOL ReadOnly = TRUE;
  BOOL CreateNew = FALSE;
  ASSERT(!(ReadOnly && CreateNew)); // gee, we can't do both!

  // we can not open additinoal files if the original was not read only
  if (m_bReadOnly == FALSE) return FALSE;

  // by definition nothing is sorted anymore because we have mutiple files
  m_bIsSorted = FALSE;

  // store file name in class
  if (m_sFileName != NULL) delete [] m_sFileName;
  LT_MEM_TRACK_ALLOC(m_sFileName = new char[strlen(FileName)+1],LT_MEM_TYPE_MISC);
  ASSERT(m_sFileName != NULL);
  strcpy(m_sFileName,FileName);

  // check if user has asked us to open a directory instead of a rez file
  if (IsDirectory(FileName)) {

    // Create the CRezFileDirectoryEmulation object
    CRezFileDirectoryEmulation* pRezFile;
	LT_MEM_TRACK_ALLOC(pRezFile = new CRezFileDirectoryEmulation(this,m_nMaxOpenFilesInEmulatedDir),LT_MEM_TYPE_MISC);
    ASSERT(pRezFile != NULL);
    if (pRezFile == NULL) {
	  delete [] m_sFileName;
	  m_sFileName = NULL;
      return FALSE;
    }
    m_lstRezFiles.Insert(pRezFile);
    m_nNumRezFiles++;

    // open the file
    if (!pRezFile->Open(FileName,ReadOnly,CreateNew)) return FALSE;
    m_bFileOpened = TRUE;

	// read in data from directory and all sub directories
	ReadEmulationDirectory(pRezFile,m_pRootDir,m_sFileName,bOverwriteItems);

    return TRUE;
  }

  // create a new CRezFile object for the RezFile
  CRezFile* pRezFile;
  LT_MEM_TRACK_ALLOC(pRezFile = new CRezFile(this),LT_MEM_TYPE_MISC);
  ASSERT(pRezFile != NULL);
  if (pRezFile == NULL) {
	delete [] m_sFileName;
	m_sFileName = NULL;
    return FALSE;
  }
  m_lstRezFiles.Insert(pRezFile);
  m_nNumRezFiles++;

  // open the file
  if (!pRezFile->Open(FileName,ReadOnly,CreateNew)) return FALSE;

  // read in the header
  FileMainHeaderStruct Header;
  pRezFile->Read(0,0,sizeof(Header),&Header);

  // store header values in RezMgr class
  if (Header.LargestKeyAry > m_nLargestKeyAry) m_nLargestKeyAry = Header.LargestKeyAry;
  if (Header.LargestDirNameSize > m_nLargestDirNameSize) m_nLargestDirNameSize = Header.LargestDirNameSize;
  if (Header.LargestRezNameSize > m_nLargestRezNameSize) m_nLargestRezNameSize = Header.LargestRezNameSize;
  if (Header.LargestCommentSize > m_nLargestCommentSize) m_nLargestCommentSize = Header.LargestCommentSize;

  // verify that this is a valid file
  ASSERT(Header.CR1 == 0x0d);
  ASSERT(Header.LF2 == 0x0a);
  ASSERT(Header.EOF1 == 0x1a);
  ASSERT(Header.FileFormatVersion == 1);

  // read in directories
  m_pRootDir->ReadAllDirs(pRezFile, Header.RootDirPos, Header.RootDirSize, bOverwriteItems);

  return TRUE;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::ReadEmulationDirectory(CRezFileDirectoryEmulation* pRezFileEmulation, CRezDir* pDir, char* sParamPath, BOOL bOverwriteItems) {
  ASSERT(pDir != NULL);
  ASSERT(this != NULL);
  ASSERT(sParamPath != NULL);
  ASSERT(m_bFileOpened);
  
  _finddata_t fileinfo;

  // figure out the path to this dir with added backslash
  char sPath[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+5];
  strcpy(sPath,sParamPath);
  if (sPath[strlen(sPath)-1] != '\\') strcat(sPath,"\\");

  // figure out the find search string by adding *.* to search for everything
  char sFindPath[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+5];
  strcpy(sFindPath,sPath);
  strcat(sFindPath,"*.*");

  // being search for everything in this directory using findfirst and findnext
  long nFindHandle = _findfirst( sFindPath, &fileinfo );
  if (nFindHandle >= 0) {

    // loop through all entries in this directory
    do {

      // skip the files . and ..
      if (strcmp(fileinfo.name,".") == 0) continue;
      if (strcmp(fileinfo.name,"..") == 0) continue;
      
      // if this is a subdirectory
      if ((fileinfo.attrib & _A_SUBDIR) == _A_SUBDIR) {

        // figure out the base name
        char sBaseName[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+5];
        strcpy(sBaseName,fileinfo.name);
        if (!m_bLowerCaseUsed) strupr(sBaseName);

        // figure out the path name we are working on
        char sPathName[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+5];
        strcpy(sPathName,sPath);
        strcat(sPathName,sBaseName);              
        strcat(sPathName,"\\");

        // create new directory entry in resource file (unless directory already exists)
        CRezDir* pNewDir;
		pNewDir = pDir->GetDir(sBaseName);
		if (pNewDir == NULL) pNewDir = pDir->CreateDir(sBaseName);

        // error if directory not found or created
        if (pNewDir == NULL) {
		  ASSERT(FALSE); 
          continue;
        }

        // call TransferDir on the new directory
        ReadEmulationDirectory(pRezFileEmulation,pNewDir,sPathName,bOverwriteItems);
      }

      // if this is a file add it to the resource file
      else {

        // figure out the file name we are working on
        char sFileName[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+5];
        strcpy(sFileName,sPath);
        strcat(sFileName,fileinfo.name);              
        
        // skip if file size is 0
//        if (fileinfo.size <= 0) continue;
        
        // split the file name up into its parts
        char drive[_MAX_DRIVE+1];
        char dir[_MAX_DIR+1];
        char fname[_MAX_FNAME+1+_MAX_EXT+1];
        char ext[_MAX_EXT+1];
         _splitpath(sFileName, drive, dir, fname, ext );
        
        // figure out the Name for this file
        char sName[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+5];
        ASSERT(strlen(fname) < (_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+5));
        strcpy(sName,fname);
        strupr(sName);
        
        // figure out the ID for this file (if name is all digits use it as ID number, otherwise assign a number)
        REZID nID;
        {
          int nNameLen = strlen(sName);
          int i;
          for (i = 0; i < nNameLen; i++) {
            if ((sName[i] < '0') || (sName[i] > '9')) break;
          }
          if (i < nNameLen) {
            nID = m_nNextIDNumToUse;
            m_nNextIDNumToUse++;
          }
          else {
            nID = atol(sName);
          }
        }
        
        REZTYPE nType;

	    // check if the extension is too long
	    if (strlen(ext) > 5)
		{
		  strncat(sName,ext,_MAX_FNAME+1+_MAX_EXT);
		  fname[_MAX_FNAME] = '\0';
		  nType = 0;
		}  
        else
		{ 
          // figure out the Type for this file
          char sExt[5];
          if (strlen(ext) > 0) {
            strcpy(sExt,&ext[1]);
            strupr(sExt);
            nType = StrToType(sExt);
		  }
          else nType = 0;
		}
        
        // convert type back to string 
        char sType[5];
        TypeToStr(nType,sType);
        
        // print out message to user
//        TRACE("Adding: Type = %-4s Name = %-12s Size = %-8i ID = %-8i\n",sType,sName,(int)fileinfo.size,(int)nID);

        // find or create the type for this resource
        CRezTyp* pTyp = pDir->GetOrMakeTyp(nType);
        ASSERT(pTyp != NULL);

        // create new resource if it doesn't exist
        CRezItm* pItm;
		pItm = pDir->GetRez(sName,nType);
		if (pItm == NULL) pItm = pDir->CreateRezInternal(nID,sName,pTyp,NULL);
		else {
		  if (bOverwriteItems) {
		    pDir->RemoveRezInternal(pTyp,pItm);
			pItm = pDir->CreateRezInternal(nID,sName,pTyp,NULL);
		  }
		  else {
		    pItm = NULL;
		  }
		}
        
        // make sure resource was created
        if (pItm != NULL) {

     	  // store the file time in the resource
		  pItm->SetTime((REZTIME)fileinfo.time_write);

		  // store the size of the file in the resource
		  pItm->m_nSize = fileinfo.size;

  		  // set the file pointer for the item
		  pItm->m_pRezFile = new CRezFileSingleFile(this, sFileName, pRezFileEmulation);
		  ASSERT(pItm->m_pRezFile != NULL);
        }
      }
        
    // get the next entry in this directory
    } while (_findnext(nFindHandle, &fileinfo) == 0);

    // close out the directory findfirst and findnext
    _findclose(nFindHandle);
  }

  return TRUE;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::Close(BOOL bCompact) {
  ASSERT(m_bFileOpened);
  BOOL bRetVal;

  // if this is not read only flush it
  if (!m_bReadOnly) Flush();

  bRetVal = m_pPrimaryRezFile->Close();

  // remove the primary RezFile from the list of RezFiles
  m_lstRezFiles.Delete(m_pPrimaryRezFile);
  m_nNumRezFiles--;
  delete m_pPrimaryRezFile;
  m_pPrimaryRezFile = NULL;

  // delete and close out all the rest of the RezFiles
  CBaseRezFile* pRezFile;
  while ((pRezFile = m_lstRezFiles.GetFirst()) != NULL) {
	pRezFile->Close();
    m_lstRezFiles.Delete(pRezFile);
	m_nNumRezFiles--;
	delete pRezFile;
  }

  // remove memory in this object associated with the open file
  if (m_pRootDir != NULL) {
    delete m_pRootDir;
    m_pRootDir = NULL;
  }
  if (m_sFileName != NULL) {
    delete [] m_sFileName;
    m_sFileName = NULL;
  }

  // mark resource as closed
  m_bFileOpened = FALSE;

  return bRetVal;
}; 

//---------------------------------------------------------------------------------------------------
CRezDir* CRezMgr::GetRootDir() {
  ASSERT(m_bFileOpened);
  return m_pRootDir;
};


//---------------------------------------------------------------------------------------------------
REZTYPE CRezMgr::StrToType(char* pStr) {
  ASSERT(pStr != NULL);
  if (pStr == NULL) return 0;
  REZTYPE nType = 0;
  BYTE* pType = (BYTE*)&nType;

  // figure out length of string
  int nStrLen = strlen(pStr);

  // store string in type
  if (nStrLen > 0) pType[nStrLen-1] = pStr[0];
  if (nStrLen > 1) pType[nStrLen-2] = pStr[1];
  if (nStrLen > 2) pType[nStrLen-3] = pStr[2];
  if (nStrLen > 3) pType[nStrLen-4] = pStr[3];

  return nType;
};

//---------------------------------------------------------------------------------------------------
void CRezMgr::TypeToStr(REZTYPE nType, char* sType) {
  ASSERT(sType != NULL);
  if (sType == NULL) return;
  BYTE* pType = (BYTE*)&nType;
  int nStrLen = 0;

  // figure out the length of the string
  if (pType[3] != '\0') nStrLen = 4;
  else if (pType[2] != '\0') nStrLen = 3;
  else if (pType[1] != '\0') nStrLen = 2;
  else if (pType[0] != '\0') nStrLen = 1;

  // store type in string
  if (nStrLen > 0) sType[0] = pType[nStrLen-1];
  if (nStrLen > 1) sType[1] = pType[nStrLen-2];
  if (nStrLen > 2) sType[2] = pType[nStrLen-3];
  if (nStrLen > 3) sType[3] = pType[nStrLen-4];

  // put terminator on string
  sType[nStrLen] = '\0';

  return;
};

//---------------------------------------------------------------------------------------------------
void* CRezMgr::Alloc(DWORD NumBytes) {
  ASSERT(FALSE); // Not implemented!
  return NULL;
};

//---------------------------------------------------------------------------------------------------
void CRezMgr::Free(void* Ptr) {
  ASSERT(FALSE); // Not implemented!
};

//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::DiskError() {
  return FALSE;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::VerifyFileOpen() {
  BOOL bRetVal = TRUE;
  CBaseRezFile* pRezFile = m_lstRezFiles.GetFirst();
  while (pRezFile != NULL) {
    if (pRezFile->VerifyFileOpen() == FALSE) bRetVal = FALSE; 		
	pRezFile = pRezFile->Next();
  }
  return bRetVal;
};

//---------------------------------------------------------------------------------------------------
void CRezMgr::SetHashTableBins(unsigned int nByNameNumHashBins, unsigned int nByIDNumHashBins, 
							   unsigned int nDirNumHashBins, unsigned int nTypNumHashBins)
{
	m_nByNameNumHashBins = nByNameNumHashBins;
	m_nByIDNumHashBins = nByIDNumHashBins;
	m_nDirNumHashBins =	nDirNumHashBins;
	m_nTypNumHashBins = nTypNumHashBins;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::Flush() {
  ASSERT(m_bReadOnly != TRUE);

  // write out directory and header if not read only
  if (!m_bReadOnly) {

    // save the next write pos for the header information
    unsigned long nSaveWritePos = m_nNextWritePos;

    // write out all of the directories
    m_pRootDir->WriteAllDirs(m_pPrimaryRezFile, &m_nRootDirPos,&m_nRootDirSize);
    
    // fill out the permant parts of the header
    FileMainHeaderStruct Header;
    Header.CR1 = 0x0d;
    Header.CR2 = 0x0d;
    Header.CR3 = 0x0d;
    Header.LF1 = 0x0a;
    Header.LF2 = 0x0a;
    Header.LF3 = 0x0a;
    Header.EOF1 = 0x1a;
    memset(Header.FileType,' ',RezMgrUserTitleSize);
    strcpy(Header.FileType,"RezMgr Version 1 Copyright (C) 1995 MONOLITH INC.");
    Header.FileType[strlen(Header.FileType)] = ' ';
    memset(Header.UserTitle,' ',RezMgrUserTitleSize);
	if (m_sUserTitle[0] != '\0') memcpy(Header.UserTitle,m_sUserTitle,strlen(m_sUserTitle));
    
    // fill out the variable parts of the header
    Header.FileFormatVersion      = 1;     
    Header.RootDirPos             = m_nRootDirPos; 			
    Header.RootDirSize            = m_nRootDirSize;              
    Header.RootDirTime            = m_nRootDirTime;           
    Header.NextWritePos           = nSaveWritePos;           
    Header.Time                   = m_nLastTimeModified; 					
    Header.LargestKeyAry          = m_nLargestKeyAry;			
    Header.LargestDirNameSize     = m_nLargestDirNameSize;    
    Header.LargestRezNameSize     = m_nLargestRezNameSize;	
    Header.LargestCommentSize     = m_nLargestCommentSize;	
    Header.IsSorted               = m_bIsSorted;                
    
    // write the header
    m_pPrimaryRezFile->Write(0,0,sizeof(Header),&Header);

    // actually flush the file 
    m_pPrimaryRezFile->Flush();

    return TRUE;
  }
  else return FALSE;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezDir::WriteAllDirs(CBaseRezFile* pRezFile, DWORD* Pos, DWORD* Size) {
  BOOL bRetFlag = TRUE;

  // first write out all directories contained in this directory
  CRezDirHash* pDir = m_haDir.GetFirst();
  while (pDir != NULL) {
    CRezDir* pRezDir = pDir->GetRezDir();
    ASSERT(pRezDir != NULL);
    if (!pRezDir->WriteAllDirs(pRezFile, &pRezDir->m_nDirPos,&pRezDir->m_nDirSize)) {
      bRetFlag = FALSE;
      break;
    }
    pDir = pDir->Next();
  }

  // now write out our own directory block
  *Pos = m_pRezMgr->m_nNextWritePos;
  if (!WriteDirBlock(pRezFile, m_pRezMgr->m_nNextWritePos,Size)) bRetFlag = FALSE;
  else m_pRezMgr->m_nNextWritePos += *Size;

  return bRetFlag;
}; 

//---------------------------------------------------------------------------------------------------
BOOL CRezDir::WriteDirBlock(CBaseRezFile* pRezFile, DWORD Pos, DWORD* Size) {
  ASSERT(Pos > 0);
  DWORD nCurPos = Pos;
  FileDirEntryHeader Header;
  BYTE Zero = 0;

  // write all dir hash table contents out to file
  {
    Header.Type = DirectoryEntry;
    CRezDirHash* pDir = m_haDir.GetFirst();
    while (pDir != NULL) {
      CRezDir* pRezDir = pDir->GetRezDir();
      ASSERT(pRezDir != NULL);

      // Fill out the header we are going to write out
      Header.Dir.Pos = pRezDir->m_nDirPos;
      Header.Dir.Size = pRezDir->m_nDirSize;
      Header.Dir.Time = pRezDir->m_nLastTimeModified;

      // write out dir entry
      nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Type),&Header.Type);
      nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Dir.Pos),&Header.Dir.Pos);
      nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Dir.Size),&Header.Dir.Size);
      nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Dir.Time),&Header.Dir.Time);
      if (pRezDir->m_sDirName == NULL) nCurPos += pRezFile->Write(nCurPos,0,1,&Zero);
      else nCurPos += pRezFile->Write(nCurPos,0,strlen(pRezDir->m_sDirName)+1,pRezDir->m_sDirName);

      pDir = pDir->Next();
    }
  }

  // write all typ hash table contents 
  {
    Header.Type = ResourceEntry;
    CRezTypeHash* pTyp = m_haTypes.GetFirst();
    CRezItmHashByName* pItm;
    while (pTyp != NULL) {
      ASSERT(pTyp->GetRezTyp() != NULL);

      // go through all items of this type
      pItm = pTyp->GetRezTyp()->m_haName.GetFirst();
      while (pItm != NULL) {
        CRezItm* pRezItm = pItm->GetRezItm();
        ASSERT(pRezItm != NULL);
        
        // Fill out the header we are going to write out
        Header.Rez.Pos = pRezItm->m_nFilePos;
        Header.Rez.Size = pRezItm->m_nSize;
        Header.Rez.Time = pRezItm->m_nTime;
        Header.Rez.ID = 0;
        Header.Rez.Type = pTyp->GetRezTyp()->GetType();
        Header.Rez.NumKeys = 0;

        // write out item header
        nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Type),&Header.Type);
        nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Rez.Pos),&Header.Rez.Pos);
        nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Rez.Size),&Header.Rez.Size);
        nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Rez.Time),&Header.Rez.Time);
        nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Rez.ID),&Header.Rez.ID);
        nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Rez.Type),&Header.Rez.Type);
        nCurPos += pRezFile->Write(nCurPos,0,sizeof(Header.Rez.NumKeys),&Header.Rez.NumKeys);
        if (pRezItm->m_sName == NULL) nCurPos += pRezFile->Write(nCurPos,0,1,&Zero);
        else nCurPos += pRezFile->Write(nCurPos,0,strlen(pRezItm->m_sName)+1,pRezItm->m_sName);
        nCurPos += pRezFile->Write(nCurPos,0,1,&Zero);

        // advance to next item
        pItm = pItm->Next();
      }

      // advance to next resource type
      pTyp = pTyp->Next();
    }
  }

  m_pRezMgr->m_nNextWritePos = nCurPos;
  *Size = nCurPos - Pos;
  return TRUE;
};
/*
//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::VerifyFileOpen() {
  // check if the files is even supposed to be open
  if (m_bFileOpened == FALSE) return FALSE;

  // test to see if the file is open
  if (ftell(m_pRezFile) != -1L) return TRUE;
  else {

    // try to open the file again
    if (!FileOpen(m_sFileName,m_bReadOnly,FALSE)) return FALSE;
  }
  
  return TRUE;
};

//---------------------------------------------------------------------------------------------------
DWORD CRezMgr::FileRead(DWORD Pos, DWORD Size, void* DataPtr) {
  ASSERT(DataPtr != NULL);

  // if size is zero just return
  if (Size <= 0) return 0;

  while (fseek(m_pRezFile,Pos,SEEK_SET) != 0) {
    if (!DiskError()) {
      ASSERT(FALSE); // Seek Failed!
      return 0;
    }
  }

  DWORD nRetVal;
  while ((nRetVal = fread(DataPtr,1,Size,m_pRezFile)) != Size) {
    if (!DiskError()) {
      ASSERT(FALSE); // Read Failed!
      return 0;
    };
  };

  return nRetVal;
};

//---------------------------------------------------------------------------------------------------
DWORD CRezMgr::FileWrite(DWORD Pos, DWORD Size, void* DataPtr) {
  ASSERT(DataPtr != NULL);

  // if size is zero just return
  if (Size <= 0) return 0;

  while (fseek(m_pRezFile,Pos,SEEK_SET) != 0) {
    if (!DiskError()) {
      ASSERT(FALSE); // Seek Failed!
      return 0;
    }
  }

  DWORD nRetVal;
  while ((nRetVal = fwrite(DataPtr,1,Size,m_pRezFile)) != Size) {
    if (!DiskError()) {
      ASSERT(FALSE); // Write Failed!
      return 0;
    };
  };

  return nRetVal;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::FileOpen(const char* FileName, BOOL ReadOnly, BOOL CreateNew) {
  do {
    if (CreateNew) {
      if (ReadOnly) return FALSE;
      else m_pRezFile = fopen(FileName,"w+b");
    }
    else {
      if (ReadOnly) m_pRezFile = fopen(FileName,"rb");
      else m_pRezFile = fopen(FileName,"r+b");
    }
    if (m_pRezFile == NULL) {
      if (!DiskError()) return FALSE;
    }
  } while (m_pRezFile == NULL);
  m_bReadOnly = ReadOnly;
  return TRUE;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::FileClose() {
  ASSERT(m_pRezFile != NULL);
  if (m_pRezFile != NULL) {
    BOOL bRetVal;
    while ((bRetVal = (fclose(m_pRezFile) == 0)) == FALSE) {
      if (!DiskError()) return FALSE;
    };
    return bRetVal;
  }
  else return FALSE;
};

//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::FileFlush() {
  ASSERT(m_pRezFile != NULL);
  if (m_pRezFile != NULL) {
    BOOL bRetVal;
    while ((bRetVal = (fflush(m_pRezFile) == 0)) == FALSE) {
      if (!DiskError()) return FALSE;
    };
    return bRetVal;
  }
  else return FALSE;
};
*/

//---------------------------------------------------------------------------------------------------
REZTIME CRezMgr::GetCurTime() {
  time_t nTimeVal;
  nTimeVal = time(&nTimeVal);
  return (REZTIME)nTimeVal;
};


//---------------------------------------------------------------------------------------------------
void CRezMgr::SetDirSeparators(const char* sDirSeparators) {
	if (m_sDirSeparators != NULL) delete [] m_sDirSeparators;
	int nLen = strlen(sDirSeparators);
	LT_MEM_TRACK_ALLOC(m_sDirSeparators = new char[nLen+1],LT_MEM_TYPE_MISC);
	strcpy(m_sDirSeparators,sDirSeparators);
};


//---------------------------------------------------------------------------------------------------
inline BOOL CRezDir::IsGoodChar(char c)
{
	if (m_pRezMgr->m_sDirSeparators != NULL)
	{
		return (strchr(m_pRezMgr->m_sDirSeparators,c) == NULL);
	}
	else
	{
		if (c >= ' ' && c <= '.') return(TRUE);
		if (c >= '0' && c <= '9') return(TRUE);
		if (c >= 'A' && c <= 'Z') return(TRUE);
		if (c >= 'a' && c <= 'z') return(TRUE);
		return(FALSE);
	}
}

// [blg]
//---------------------------------------------------------------------------------------------------
CRezDir* CRezDir::GetDirFromPath(REZCDIRNAME sPath)
{
	// Strip off leading slash...

	int len = strlen(sPath);

	if (len > 1)
	{
		if (!IsGoodChar(sPath[0]))
		{
			sPath = &sPath[1];
			len--;
		}
	}


	// Read the first directory name...

	int i = 0;
	char sDir[1024];

	while (IsGoodChar(sPath[i]))
	{
		sDir[i] = sPath[i];
		i++;
		if (i >= 1023) break;
	}

	sDir[i] = '\0';

	CRezDir* pDir = GetDir(sDir);
	if (!pDir) return(NULL);


	// If this is the last name in the path string, we're done...

	if (sPath[i] == '\0')
	{
		return(pDir);
	}


	// Skip any white space...

	while (!IsGoodChar(sPath[i]))
	{
		i++;
		if (sPath[i] == '\0') return(pDir);
	}


	// Recursively continue...

	return(pDir->GetDirFromPath(&sPath[i]));
}


// [blg]
//---------------------------------------------------------------------------------------------------
CRezItm* CRezDir::GetRezFromDosPath(const char* sPath)
{
	char	sDir[1024];
	char	sRez[1024];


	// Strip off leading slash...

	int len = strlen(sPath);

	if (len >= 1023) return NULL;

	if (len > 1)
	{
		if (!IsGoodChar(sPath[0]))
		{
			sPath = &sPath[1];
			len--;
		}
	}


	// Strip off the rez name...

	int i   = len - 1;

	while (IsGoodChar(sPath[i]))
	{
		i--;
		if (i < 0) break;
	}

	if (i == len) return(NULL);

	strncpy(sRez, &sPath[i+1], 1023);
	sRez[1023] = '\0';


	// If we were only given a rez name, try to get it...

	if (i <= 1)
	{
		return(GetRezFromDosName(sRez));
	}


	// Copy everything except for the rez name...

	strncpy(sDir, sPath, i);
	sDir[i] = '\0';


	// Find the directory for this path...

	CRezDir* pDir = GetDirFromPath(sDir);

	if (!pDir) return(NULL);


	// Try to get the rez from the dir we just got...

	return(pDir->GetRezFromDosName(sRez));
}


// [blg]
//---------------------------------------------------------------------------------------------------
CRezItm* CRezDir::GetRezFromPath(const char* sPath, REZTYPE type)
{
	char	sDir[1024];
	char	sRez[1024];


	// Strip off leading slash...

	int len = strlen(sPath);

	if (len >= 1023) return NULL;

	if (len > 1)
	{
		if (!IsGoodChar(sPath[0]))
		{
			sPath = &sPath[1];
			len--;
		}
	}


	// Strip off the rez name...

	int i   = len - 1;

	while (IsGoodChar(sPath[i]))
	{
		i--;
		if (i < 0) break;
	}

	if (i == len) return(NULL);

	strncpy(sRez, &sPath[i+1], 1023);
	sRez[1023] = '\0';

	// If we were only given a rez name, try to get it...

	if (i <= 1)
	{
		return(GetRez(sRez, type));
	}


	// Copy everything except for the rez name...

	strncpy(sDir, sPath, i);
	sDir[i] = '\0';


	// Find the directory for this path...

	CRezDir* pDir = GetDirFromPath(sDir);

	if (!pDir) return(NULL);


	// Try to get the rez from the dir we just got...

	return(pDir->GetRez(sRez, type));
}

//---------------------------------------------------------------------------------------------------
CRezItm* CRezMgr::GetRezFromPath(const char* sPath, REZTYPE type)
{
	return(GetRootDir()->GetRezFromPath(sPath, type));
}

//---------------------------------------------------------------------------------------------------
CRezItm* CRezMgr::GetRezFromDosPath(const char* sPath)
{
	return(GetRootDir()->GetRezFromDosPath(sPath));
}

//---------------------------------------------------------------------------------------------------
CRezDir* CRezMgr::GetDirFromPath(const char* sPath)
{
	return(GetRootDir()->GetDirFromPath(sPath));
}


// [blg]
//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::Reset()
{
	if (!IsOpen()) return(FALSE);

	Close();
	return(Open(m_sFileName));
}


//---------------------------------------------------------------------------------------------------
BOOL CRezMgr::IsDirectory(const char* sFileName) {
   struct stat buf;
   int result;

   // Get data associated with "stat.c"
   result = stat( sFileName, &buf );

   // Check if statistics are valid 
   if( result != 0 ) return FALSE;

   // is this a directory
   if ((buf.st_mode & _S_IFDIR) == _S_IFDIR) return TRUE;
   else return FALSE;
}


//---------------------------------------------------------------------------------------------------
CRezItm* CRezMgr::AllocateRezItm()
{
	CRezItm* pNewItem = NULL;
	CRezItmHashByName* pHash;

	pHash = m_hashRezItmFreeList.GetFirst();
	if (pHash != NULL) pNewItem = pHash->GetRezItm();

	// if we are out of free Rez Items then make a new chunk and allocate one from there
	if (pNewItem == NULL)
	{
		CRezMgr::CRezItmChunk* pNewChunk;
		LT_MEM_TRACK_ALLOC(pNewChunk = new CRezItmChunk,LT_MEM_TYPE_MISC);
		if (pNewChunk == NULL) return NULL;

		LT_MEM_TRACK_ALLOC(pNewChunk->m_pRezItmAry = new CRezItm[m_nRezItmChunkSize],LT_MEM_TYPE_MISC);
		if (pNewChunk->m_pRezItmAry == NULL)
		{
			delete pNewChunk;
			return NULL;
		}

		for (unsigned int i = 0; i < m_nRezItmChunkSize; i++)
		{
			pNewChunk->m_pRezItmAry[i].m_heName.SetRezItm(&pNewChunk->m_pRezItmAry[i]);
			m_hashRezItmFreeList.Insert(&pNewChunk->m_pRezItmAry[i].m_heName);
		}

		m_lstRezItmChunks.Insert(pNewChunk);		

		pNewItem = m_hashRezItmFreeList.GetFirst()->GetRezItm();
	}

	if (pNewItem != NULL)
	{
		m_hashRezItmFreeList.Delete(&pNewItem->m_heName);
	}

	return pNewItem;
}


//---------------------------------------------------------------------------------------------------
void CRezMgr::DeAllocateRezItm(CRezItm* pItem)
{

	if (pItem != NULL)
	{
		m_hashRezItmFreeList.Insert(&pItem->m_heName);
	}
}


//---------------------------------------------------------------------------------------------------
void CRezMgr::SetUserTitle(const char* sUserTitle)
{
	strncpy(m_sUserTitle, sUserTitle, RezMgrUserTitleSize);
	m_sUserTitle[RezMgrUserTitleSize] = '\0';
}
