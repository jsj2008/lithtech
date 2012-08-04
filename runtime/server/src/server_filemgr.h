
// This module defines and implements the server-side file
// manager.  The file manager tracks which files the server
// is using at any given time, keeps track of a list of file
// trees for the server, and will open and close files for
// the server.

#ifndef __SERVER_FILEMGR_H__
#define __SERVER_FILEMGR_H__

#ifndef __SYSFILE_H__
#include "sysfile.h"
#endif

#ifndef __LTT_LIST_CIRCULAR_H__
#include "ltt_list_circular.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif


class HHashElement;
class HHashTable;
typedef void* HLTFileTree; // class HLTFileTree;



// UsedFile flags.
	// This is set when an object referencing the file is using a placeholder,
	// so when the file is finally done loading, it knows whether it should
	// search thru the objects looking for ones that need updating.
	#define UF_DEPENDENCIES		(1<<0)


// ------------------------------------------------------------ //
// Structures.
// ------------------------------------------------------------ //

// A server file tree.
//struct ServerFileTree {
//	HLTFileTree *m_hFileTree;
//};

//typedefs for our list types.
typedef CListCircularHead<HLTFileTree *> SERVERFILEMGR_HEAD;
typedef CListCircularElement<HLTFileTree *> SERVERFILEMGR_ELEMENT;


// An entry in the used file list.
struct UsedFile {
	char*			GetFilename();
	HHashElement    *m_hElement; // holds the file-table entry for filename
	uint32			m_FileSize;
	uint32			m_FileID;
	short			m_Flags;
	HLTFileTree     *m_hFileTree;
	void*			m_Data ;     // points to data.
};


// The file manager.
class IServerFileMgr : public IBase {
public:
    interface_version(IServerFileMgr, 0);

    //this interface is it's own implementation.
    declare_interface(IServerFileMgr);


public:
    IServerFileMgr();

    //clears everything.
    void Clear();

	// All the file trees in use.
    SERVERFILEMGR_HEAD file_tree_list;

	// Just a counter that is incremented.
	unsigned long	m_CurrentFileID;
	
	//The table of UsedFiles currently in use.
	HHashTable  *m_hFileTable;

	ObjectBank<UsedFile>	m_UsedFileBank;


public:
    //-------------------------------------------------------------
    //Functions that used to be non-class global sf_* functions.
    //-------------------------------------------------------------

    // One-time initialize and term.
    void Init();
    void Term();

    // Add resources to the list of available trees.
    // These are added at the end of the trees currently in there.
    // Trees later in the list override trees earlier in the list.
    void AddResources(const char **pTrees, int32 nTrees, 
        TreeType *pTreeTypes, int32 *nTreesLoaded);

    // Use these to open and close files.
    ILTStream* OpenFile(const char *pFilename);
    ILTStream* OpenFile2(const char *pFilename, int bAddUsedFile, short flags);
    ILTStream* OpenFile3(UsedFile *pUsedFile);

    // Copy a file.  Returns LT_OK, LT_ERROR, or LT_NOTFOUND.
    LTRESULT CopyFile(const char *pSrc, const char *pDest);

    //returns true if the file exists.  Will set hTree and the size of the file if those
    //parameters are not NULL.
    bool DoesFileExist(const char *pFilename, HLTFileTree **phTree, uint32 *pFileSize);

    // Get a file list.  Free it with ic_FreeFileList.
    FileEntry* GetFileList(const char *pDirName);

    // Add a file to the 'used file' list.
    // Fills in ppFile if it is non-NULL.
    // Returns 0 if the file doesn't exist.
    // Returns 1 if the file is already there (flags are ORed together).
    // Returns 2 if the file was added. 
    int AddUsedFile(const char *pFilename, short flags, UsedFile **ppFile);

	// Get a UsedFile that has already been added. 
	// returns false if failed
	// returns true if sucess.
	bool GetAddedUsedFile( const char *pFilename, UsedFile **pFile );

	// Get a UsedFile by file id... 
	bool GetAddedUsedFile( uint16 file_id, UsedFile **pFile );

    // Clear the used file list.
    void ClearUsedFiles();

    // Get the filename from a UsedFile.
    char* GetUsedFilename(UsedFile *pFile);

};
			

// ------------------------------------------------------------ //
// Functions.
// ------------------------------------------------------------ //



#endif  // __SERVER_FILEMGR_H__



