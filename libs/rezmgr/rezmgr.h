/****************************************************************************
;
;	MODULE:		RezMgr (.H)
;
;	PURPOSE:
;
;	HISTORY:	04/08/95 [m]
;
;	NOTICE:		Copyright (c) 1995, MONOLITH, Inc.
;
***************************************************************************/

#ifndef __REZMGR_H__
#define __REZMGR_H__

// defines

#ifndef DWORD
typedef unsigned long int	DWORD;
#endif

#define RezMgrUserTitleSize     60

#ifndef __STDIO_H__
#include <stdio.h>
#define __STDIO_H__
#endif

#ifndef __REZTYPES_H__
#include "reztypes.h"
#endif

#ifndef __REZFILE_H__
#include "rezfile.h"
#endif

// Console Libs...


// forward class declerations
class CRezItm;
class CRezTyp;
class CRezDir;
class CRezMgr;

#ifndef __REZHASH_H__
#include "rezhash.h"
#endif


// -----------------------------------------------------------------------------------------
// CRezItm

class CRezItm 
{
public:
	// functions for the general user
	REZNAME		GetName() { return m_sName; };                                          // Returns the Name of this resource
	REZTYPE		GetType();                                                              // Returns the Type of this resource
	REZSIZE		GetSize() { return m_nSize; };                                          // Returns the Size of the data in this resource
	REZPATH		GetPath(char* Buf, unsigned int BufSize);                               // Returns the directory path that this resource is contained in
	REZDIRNAME	GetDir();                                                               // Returns the name of the directory this resource is located in
    CRezDir*    GetParentDir() { return m_pParentDir; };                                // Returns the parent directory that contains this item
	REZTIME		GetTime() { return m_nTime; };											// Last time resource was modified

	BOOL		Get(BYTE* pBytes);                                                      // Copies the data from this resource to the specified location
	BOOL		Get(void* pBytes) { return Get((BYTE*)pBytes); };						// void version of get
	BOOL		Get(BYTE* pBytes, DWORD startOffset, DWORD length);                     // Copies a portion of the data from this resource to the specified location
	BOOL	    Get(void* pBytes, DWORD startOffset, DWORD length) 
	            { return Get((BYTE*)pBytes,startOffset,length); };						// void version of get

	BYTE*		Load();                                                                 // Returns a pointer to the data for this item (loads from disk if not already in memory)
	BOOL		UnLoad();                                                               // Frees the memory for this item
	BOOL		IsLoaded();                                                             // Returns TRUE if this item is currently in memory

	DWORD		GetSeekPos() { return m_nCurPos; };                                     // Get the current position inside this resource
	BOOL		Seek(DWORD offset);                                                     // Set the current position inside this resource
	DWORD		Read(BYTE* pBytes, DWORD length, DWORD seekPos = 0xffffffff);           // Read data in from this resource at the current position (SeekPos sets a new position) and advances the position
	DWORD		Read(void* pBytes, DWORD length, DWORD seekPos = 0xffffffff) 
				{ return Read((BYTE*)pBytes,length,seekPos); };							// void version of read
	BOOL		EndOfRes();                                                             // Returns TRUE if the current position is at or beyond the end off the resource data
	char		GetChar();                                                              // Returns the BYTE in the resource at the current position and advances the position

	void		SetTime(REZTIME Time) { m_nTime = Time;} ;                              // Set the current modification time for this resource

	// functions that can be used to gain direct read access to the rez file (DANGER!!!!!)
	const char* DirectRead_GetFullRezName() { if (m_pRezFile == NULL) return NULL; else return m_pRezFile->GetFileName(); };
	DWORD		DirectRead_GetFileOffset() { return m_nFilePos; };

	BYTE*		Create(DWORD Size);														// Allocates memory for data (primairly for a new resource)
	BOOL		Save();																    // Saves the data for the internally kept memory to disk

	// NOT IMPLEMENTED!!!!
//	BOOL		DirectRead_IsPossible();			
//	FILE*		DirectRead_GetFileHandle();
//	FILE*		DirectRead_ReleaseFileHandle();

private:
    // constructors and destructors
	CRezItm::CRezItm();

	// init and term
    void InitRezItm(CRezDir* pParentDir, REZNAME sName, REZID nID, CRezTyp* pType, REZDESC sDesc,
				    REZSIZE nSize, DWORD nFilePos, REZTIME nTime, DWORD nNumKeys, REZKEYVAL* pKeyAry,
				    CBaseRezFile* pRezFile);
    void TermRezItm();

    // friends
    friend class CRezTyp;
    friend class CRezDir;
    friend class CRezMgr;

	// internal functions
    void        MarkCurTime();              // Mark the current modification time as now for this resource

	// internal data members
	REZNAME				m_sName;			// Name of the resource
    CRezTyp*            m_pType;            // Resource type
	REZTIME		 		m_nTime;			// The last time the data in the resource was updated (does not include keys or description)
	REZSIZE				m_nSize;			// The size in bytes of the data in this resource
	CRezDir*			m_pParentDir;		// Pointer to the directory structure in memory that this resource is in (the parent)
	DWORD				m_nFilePos;			// File position in the resource file for this resources data (note, this is relative to m_nDataPos in the directory)
    DWORD               m_nCurPos;          // Current seek position within this resource
    CRezItmHashByName   m_heName;           // Hash element for by name hash table
	CBaseRezFile*		m_pRezFile;			// Pointer to class that controls the base low level resource file that is associated with this resource
	BYTE*				m_pData; 			// Pointer to the data for this resoucre (if NULL then not in memory)
};


// -----------------------------------------------------------------------------------------
// CRezType

class CRezTyp 
{
public:
    REZTYPE GetType() { return m_nType; };

private:
    CRezTyp(REZTYPE nType, CRezDir* pParentDir, unsigned int nByIDNumHashBins, unsigned int nByNameNumHashBins);
    CRezTyp(REZTYPE nType, CRezDir* pParentDir, unsigned int nByNameNumHashBins); // non ID has table version
    ~CRezTyp();

    // friends
    friend class CRezItm;
    friend class CRezDir;
    friend class CRezMgr;

	// internal data members
    REZTYPE                 m_nType;        // The type 
    CRezTypeHash            m_heType;       // The hash element for the typ hash table
	CRezItmHashTableByID    m_haID; 		// Hash table item info for by ID hash table
	CRezItmHashTableByName  m_haName;	    // Hash table item info for by Name hash table
	CRezDir*				m_pParentDir;	// Pointer to the directory structure in memory that this type is in (the parent)
};


// -----------------------------------------------------------------------------------------
// CRezDir

class CRezDir 
{
public:
	// functions for the general user
	REZDIRNAME 	GetDirName() { return m_sDirName; };			    // Returns the name of this directory
    CRezDir*    GetParentDir() { return m_pParentDir; };            // Returns the parent directory that contains this directory (NULL if this is ROOT dir)
    CRezMgr*    GetParentMgr() { return m_pRezMgr; };               // Returns the parent rez mgr that contains this directory

	CRezItm*	GetRez(REZNAME sRes, REZTYPE Type);		            // get a resource item in this directory by name
    CRezItm*    GetRezFromDosName(char* sDosname);                  // gets a resource from an old style dos file name which includes the type in the extension
	CRezItm*	GetRezFromPath(const char* sPath, REZTYPE type);	// gets a resource item from a full path and type
	CRezItm*	GetRezFromDosPath(const char* sPath);				// gets a resource item from a full path and extension

	BOOL		Load(BOOL LoadAllSubDirs = FALSE);                  // Load all resource in this directory
	BOOL		UnLoad(BOOL UnLoadAllSubDirs = FALSE);              // Unload all resources in this directory
    BOOL        IsLoaded() { return (m_pMemBlock != NULL); };       // Returns TRUE if this whole directory of resources is currently loaded into memory

	CRezDir*	GetDir(REZCDIRNAME sDir);                           // Returns a directory relative to this one unless starts with a \ or /
	CRezDir*	GetDirFromPath(const char* pPath);					// Returns a directory based on a full path
	CRezDir*	GetFirstSubDir();                                   // Returns the first subdirectory inside this one
	CRezDir*	GetNextSubDir(CRezDir* pRezDir);                    // Returns the next subdirectory inside this one

    CRezTyp*    GetRezTyp(REZTYPE Type);                            // Return a pointer to a CRezType object
	CRezTyp*	GetFirstType();                                     // Returns the first type of resource stored in this directory
	CRezTyp*	GetNextType(CRezTyp* pRezType);                     // Returns the next type of resource stored in this directory

	CRezItm*	GetFirstItem(CRezTyp* pRezType);                    // Returns the first resoruce of the specified type stored in this directory
	CRezItm*	GetNextItem(CRezItm* pRezItm);                      // Returns the next resource of the specified type stored in this directory

	CRezDir*	CreateDir(REZDIRNAME sDir);                         // Creates the specified directory inside the current directory (should not contain \ or /)
	CRezItm*	CreateRez(REZID ID, REZNAME Name, REZTYPE Type);    // Creates the specified resource in the resource file
	REZTIME     GetTime() { return m_nLastTimeModified; };		    // Last time resource was modified in this dir

private:
    CRezDir(CRezMgr* pRezMgr, CRezDir* pParentDir, REZDIRNAME szDirName, DWORD nFilePos, 
			DWORD nSize, REZTIME nTime, unsigned int nDirNumHashBins, unsigned int nTypNumHashBins);
    ~CRezDir();

    // friends
    friend class CRezItm;
    friend class CRezTyp;
    friend class CRezMgr;

	// internal functions
    BOOL        ReadAllDirs(CBaseRezFile* pRezFile, DWORD Pos, DWORD Size, BOOL bOverwriteItems); // Recursivly read all directories in this dir into memory
	BOOL		ReadDirBlock(CBaseRezFile* pRezFile, DWORD Pos, DWORD Size, BOOL bOverwriteItems); // Reads in directory block for this directory
    CRezTyp*    GetOrMakeTyp(REZTYPE nType);                        // Gets the type if it exists, creates it if it does not
	BOOL		IsGoodChar(char c);									// Determines if the given character is non-white space and non-seperator
    CRezItm*    CreateRezInternal(REZID nID, REZNAME sName, CRezTyp* pTyp, CBaseRezFile* pRezFile);
	BOOL		RemoveRezInternal(CRezTyp* pTyp, CRezItm* pItm);
	BOOL		WriteAllDirs(CBaseRezFile* pRezFile, DWORD* Pos, DWORD* Size);
	BOOL		WriteDirBlock(CBaseRezFile* pRezFile, DWORD Pos, DWORD* Size);

	// internal data members
	REZDIRNAME	       m_sDirName;	   	    	                    // The name of this directory
    DWORD              m_nDirPos;                                  // Position in directory data block in file
    DWORD              m_nDirSize;                                 // Size of the directory data block
    DWORD              m_nItemsPos;                                // Position of resource items data for this directory
    DWORD              m_nItemsSize;                               // Size of resource items data for this directory
	REZTIME             m_nLastTimeModified;                        // The last time that the data in a resource file in this directory was modified (does not include data in sub directories)
    CRezMgr*            m_pRezMgr;                                  // The parent CRezMgr class that this dir belongs to
    CRezDir*            m_pParentDir;                               // The parent directory that this directory is in (if NULL is is root dir)
    CRezDirHash         m_heDir;                                    // The hash element for the dir hash table
	CRezDirHashTable 	m_haDir;				                    // Hash table of all of the directories contained in this directory
	CRezTypeHashTable   m_haTypes;		                            // Hash table of all of the types of resources in this directory
    BYTE*               m_pMemBlock;                                // Pointer to memory block (used if all resources allocated at once)
};


// -----------------------------------------------------------------------------------------
// CRezMgr

class CRezMgr 
{
public:
	CRezMgr();                                                                          // Creates an empty unopened rez mgr class
    CRezMgr(const char* FileName, BOOL ReadOnly = TRUE, BOOL CreateNew = FALSE);              // Creates and opens a rez mgr class
	~CRezMgr();                                                                         // Disposes of a rez mgr class (closes it if necessary!)

	// functions for the general user
	BOOL	 Open(const char* FileName, BOOL ReadOnly = TRUE, BOOL CreateNew = FALSE);     // Open the current resource file
	BOOL	 OpenAdditional(const char* FileName, BOOL bOverwriteItems = FALSE);	 // Open an additional resource file (the file is ReadOnly and not New by definition)
	BOOL	 Close(BOOL bCompact = FALSE);                                           // Closes the current resource file (if bCompact is TRUE also compacts the resource file)
	CRezDir* GetRootDir();                                                           // Returns the root directory in the resource file
	BOOL     IsOpen() { return m_bFileOpened; };                                     // Returns TRUE if resource file is open, FALSE if not
    BOOL     VerifyFileOpen();                                                       // Checks if the files is actually open and try's to open it again if it is not
	REZTIME  GetTime() { return m_nLastTimeModified; };	                             // Last time anything in resource file was modified
	BOOL     Reset();
    BOOL	 IsSorted() { return m_bIsSorted; };

	// functions for accessing RezMgr with full paths
	CRezItm*	GetRezFromPath(const char* sPath, REZTYPE type);	// gets a resource item from a full path and type
	CRezItm*	GetRezFromDosPath(const char* sPath);				// gets a resource item from a full path and extension
	CRezDir*	GetDirFromPath(const char* pPath);					// Returns a directory based on a full path

	// function to modify how path access for RezMgr works
	void SetDirSeparators(const char* sDirSeparators);		// set the separator characters used when working with full paths (default : "/\_")

	// functions the user can override if necessary
	virtual  void* Alloc(DWORD NumBytes);            // used for all memory allocation in resource manager (currently not used or implemented!)
	virtual  void Free(void* Ptr);                   // used for all memory allocation in resoruce manager (currently not used or implemented!)
    virtual  BOOL DiskError();                       // called whenever a disk error occurs (if user returns TRUE RezMgr trys again)


    // utility functions
    REZTYPE  StrToType(char* pStr);                  // Convert an ascii string to a rez type
    void     TypeToStr(REZTYPE nType, char* sType);  // Convert a rez type to an ascii string

	// control for how ID numbers work
	void SetRenumberIDCollisions(BOOL bFlag); // defaults to TRUE (if set to FALSE duplicate ID's will be handled according to the bOverwriteItems flag)
	void SetNextIDNumber(DWORD nID);		  // defaults to 2000000000 and is incremented for every collision or resource in a raw directory that is loaded

	// lower case support (should call set right after constructor but before open)
	BOOL GetLowerCaseUsed() { return m_bLowerCaseUsed; };
	void SetLowerCaseUsed(BOOL bLowerCaseUsed) { m_bLowerCaseUsed = bLowerCaseUsed; };

	// support for accessing resources by ID (should call set right after constructor but before open)
	BOOL GetItemByIDUsed() { return m_bItemByIDUsed; };
	void SetItemByIDUsed(BOOL bItemByIDUsed) { m_bItemByIDUsed = bItemByIDUsed; };

	// set the number of bin values for creating hash tables (should call right after constructor but before open)
	void SetHashTableBins(unsigned int nByNameNumHashBins, unsigned int nByIDNumHashBins, 
						  unsigned int nDirNumHashBins, unsigned int nTypNumHashBins);

	// functions that the user should not typically use
    void ForceIsSortedFlag(BOOL bFlag) { m_bIsSorted = bFlag; };   // For use in zmgr program to force the sorted flag
	void SetMaxOpenFilesInEmulatedDir(int nNumFiles) { m_nMaxOpenFilesInEmulatedDir = nNumFiles; };  

	// accessors for usertitle header information
	void SetUserTitle(const char* sUserTitle);
	char* GetUserTitle() { return m_sUserTitle; };

	void SetNextIDNumToUse( DWORD nNextIDNumToUse ) { m_nNextIDNumToUse = nNextIDNumToUse; }
	DWORD GetNextIDNumToUse( ) { return m_nNextIDNumToUse; }

private:
	friend class CRezDir;
    friend class CRezTyp;
	friend class CRezItm;
	
	// internal functions to access the resource file
// THESE HAVE BEEN REPLACED WITH THE NEW CREZFILE CLASSES IN REZFILE.H 
//	DWORD		FileRead(DWORD Pos, DWORD Size, void* DataPtr);
//	DWORD		FileWrite(DWORD Pos, DWORD Size, void* DataPtr);
//	BOOL		FileOpen(const char* FileName, BOOL ReadOnly, BOOL CreateNew);
//	BOOL		FileClose();
//  BOOL        FileFlush();

	// class CRezItmChunk
	class CRezItmChunk : public CBaseListItem
	{
	public:
	  CRezItmChunk* Next() { return (CRezItmChunk*)CBaseListItem::Next(); };
	  CRezItm* m_pRezItmAry;
	};

	// class CRezItmChunkList
	class CRezItmChunkList : public CLTBaseList
	{
	public:
   	  CRezItmChunk* GetFirst() { return (CRezItmChunk*)CLTBaseList::GetFirst(); };
	};

	// alloc and dealloc rez items
	CRezItm* AllocateRezItm();
	void DeAllocateRezItm(CRezItm* pItem);

    // other internal functions
    REZTIME     GetCurTime();                                                           // For use by any internal function that wants to get the current time
    BOOL		IsDirectory(const char* sFileName);
    BOOL        ReadEmulationDirectory(CRezFileDirectoryEmulation* pRezFileEmulation, CRezDir* pDir, char* sParamPath, BOOL bOverwriteItems);
	BOOL		Flush();

	// internal data members
	char*		m_sDirSeparators;		// Separator characters between directories (if NULL(default) use built in method)
    BOOL        m_bIsSorted;            // If TRUE the the data is sorted and compacted by directory if FALSE then it is not
	BOOL		m_bFileOpened;			// If TRUE the resource file is opened
// m_pRezFile HAS BEEN REPLACED WITH A LIST OF FILES
	CBaseRezFileList m_lstRezFiles;		// List of low level resource files
	DWORD		m_nNumRezFiles;			// The number of rez files that are open
	CBaseRezFile* m_pPrimaryRezFile;	// Pointer the main RezFile (one opened with open!)
	BOOL		m_bRenumberIDCollisions;// If TRUE then ID's of resources that collide will simply be re-numbered
	DWORD		m_nNextIDNumToUse;		// Next ID number to use for allocating collisions and assigning to directories
	int			m_nMaxOpenFilesInEmulatedDir; // Maximum number of files that can be open at one time in a emulated dir
//	FILE*		m_pRezFile;	  			// The system file pointer for the resource file
// MOST OF THE REST OF THE VARIABLES BELOW ONLY APPLY TO THE FIRST RESOURCE FILE IN THE m_lstRezFiles LIST
	DWORD		m_nRootDirPos;			// The seek position in the file where the root directory is located
    DWORD       m_nRootDirSize;         // The size of the root directory
    REZTIME     m_nRootDirTime;         // The last time the root dir was modified
    DWORD       m_nNextWritePos;        // The next position in the file to write data out to
	BOOL		m_bReadOnly;			// If TRUE then the resource file is only opend for reading
	CRezDir*	m_pRootDir;				// Pointer to the root directory structure in the resource
	REZTIME     m_nLastTimeModified;    // The last time that any data in any resource in this resource file was modified (does not include key values and descriptions)
	BOOL		m_bMustReWriteDirs;		// If TRUE we must write out the directories on close
    DWORD       m_nFileFormatVersion; 	// the file format version number only 1 is possible here right now
    DWORD       m_nLargestKeyAry;		// Size of the largest key array in the resource file
    DWORD       m_nLargestDirNameSize;  // Size of the largest directory name in the resource file (including 0 terminator)
    DWORD       m_nLargestRezNameSize;	// Size of the largest resource name in the resource file (including 0 terminator)
    DWORD       m_nLargestCommentSize;	// Size of the largest comment in the resource file (including 0 terminator)
    char*       m_sFileName;            // Original file name user passed in to open the file
	BOOL		m_bLowerCaseUsed;		// If TRUE then lower case may be present in file and directory names (DEFAULT IS FALSE)
	BOOL		m_bItemByIDUsed;		// If TRUE then Rez items can be accessed by their ID, if false then they can not be (DEFAULT IS FALSE)
	unsigned int m_nByNameNumHashBins ;	// number of hash bins in the ItmByName hash table
	unsigned int m_nByIDNumHashBins;	// number of hash bins in the ItmByID hash table 
	unsigned int m_nDirNumHashBins;		// number of hash bins in the Directory hash table
	unsigned int m_nTypNumHashBins;		// number of hash bins in the Type hash table
	CRezItmHashTableByName m_hashRezItmFreeList; // free list of RezItm's using the hash table element inside the RezItem
	CRezItmChunkList m_lstRezItmChunks; // list of RezItm chunks
	unsigned int m_nRezItmChunkSize;	// number of rez items to allocate at once in a chunk
	char		m_sUserTitle[RezMgrUserTitleSize+1]; // user title information found in file header
};


#endif

