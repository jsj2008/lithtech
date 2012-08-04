#include "bdefs.h"

#include "sysfile.h"
#include "server_filemgr.h"
#include "servermgr.h"
#include "impl_common.h"
#include "syscounter.h"
#include "s_client.h"
#include "dhashtable.h"
#include "ftserv.h"


//allocate our IServerFileMgr instance.
define_interface(IServerFileMgr, IServerFileMgr);


IServerFileMgr::IServerFileMgr() {
    Clear();
}

void IServerFileMgr::Clear() {
    m_CurrentFileID = 0;
    m_hFileTable = NULL;

    m_UsedFileBank.Term();

    //clear the list of trees.
    file_tree_list.Clear();
}

char *UsedFile::GetFilename() {
    return (char*)hs_GetElementKey(m_hElement, NULL);
}

void IServerFileMgr::Init() {
    Clear();

    m_hFileTable = hs_CreateHashTable(500, HASH_FILENAME);
    LT_MEM_TRACK_ALLOC(m_UsedFileBank.Init(64, 512), LT_MEM_TYPE_FILE);
}


void IServerFileMgr::Term() {
    // Clear out the used file list.
    ClearUsedFiles();

    // Destroy the hash table.
    if (m_hFileTable) {
        hs_DestroyHashTable(m_hFileTable);
        m_hFileTable = 0;
    }

    // Clear out the file trees.
    while (file_tree_list.IsEmpty() == false) {
        //close the first tree.
        df_CloseTree(file_tree_list.First()->Item());

        //remove the first item from the list
        delete file_tree_list.First();
    }
}


void IServerFileMgr::AddResources(const char **pTrees, int32 nTrees, 
    TreeType *pTreeTypes, int32 *pnTreesLoaded)
{
    int i, treeStatus;
    HLTFileTree *hTree;
    int nTreesLoaded;

    // Setup ServerFileTrees for them all.
    nTreesLoaded = 0;
    for(i=0; i < nTrees; i++) {
        //open the tree with this name.
        treeStatus = df_OpenTree(pTrees[i], hTree);

        //check that we got it
        if (treeStatus != 0) continue;

        //we loaded the tree

        //create a list element for this handle.
        SERVERFILEMGR_ELEMENT *element;
		LT_MEM_TRACK_ALLOC(element = new SERVERFILEMGR_ELEMENT,LT_MEM_TYPE_MISC);

        //set the tree handle.
        element->Item() = hTree;
        
        //insert the element into the list.
        file_tree_list.Add(element);

        //fill in the array of tree types.
        if (pTreeTypes != NULL) {
            pTreeTypes[nTreesLoaded] = df_GetTreeType(hTree);
        }

        //we loaded a tree.
        ++nTreesLoaded;
    }

    *pnTreesLoaded = nTreesLoaded;
}


ILTStream* IServerFileMgr::OpenFile(const char *pFilename) {
    return OpenFile2(pFilename, 0, 0);
}


ILTStream* IServerFileMgr::OpenFile2(const char *pFilename, int bAddUsedFile, short flags) {
    // First, see if it's in the used file list, in which case we'll use that one.
    HHashElement *hElement = hs_FindElement(m_hFileTable, pFilename, strlen(pFilename)+1);
    if (hElement) {
        UsedFile *pUsedFile = (UsedFile*)hs_GetElementUserData(hElement);
        if (pUsedFile) {
            return df_Open(pUsedFile->m_hFileTree, pFilename, 0);
        }
    }


    // Now try all the file trees..
    SERVERFILEMGR_ELEMENT *cur = file_tree_list.First();
    while (file_tree_list.IsHead(cur) == false) {
        //get the file tree handle.
        HLTFileTree *hTree = cur->Item();

        //try to open the file.
        ILTStream *pRet = df_Open(hTree, pFilename, 0);
        if (pRet) {
            //found the file.
            if(bAddUsedFile) {
                AddUsedFile(pFilename, flags, NULL);
            }

            return pRet;
        }

        //go to the next file tree.
        cur = cur->Next();
    }

    return NULL;
}



ILTStream* IServerFileMgr::OpenFile3(UsedFile *pUsedFile) {
    char *pFilename;
    char formattedFilename[256];

    pFilename = GetUsedFilename(pUsedFile);
	CHelpers::FormatFilename(pFilename, formattedFilename, sizeof(formattedFilename));

    //try and open the file.
    return df_Open(pUsedFile->m_hFileTree, formattedFilename, DFOPEN_READ);
}


LTRESULT IServerFileMgr::CopyFile(const char *pSrc, const char *pDest) {
    //get first item in file tree list.
    SERVERFILEMGR_ELEMENT *cur = file_tree_list.First();

    //go through entire list.
    for (; file_tree_list.IsHead(cur) == false; cur = cur->Next()) {
        //get the file tree handle.
        HLTFileTree *hTree = cur->Item();

        //try and open the file.
        ILTStream *pStream = df_Open(hTree, pSrc, 0);

        //check if we got it.
        if (pStream == NULL) continue;

        //found the file.

        //save the file to the dest name.
        int status = df_Save(pStream, pDest);

        //release the stream.
        pStream->Release();

        //return ok if the file was saved correctly.
        return status ? LT_OK : LT_ERROR;
    }

    //could not find the file.
    return LT_NOTFOUND;
}


bool IServerFileMgr::DoesFileExist(const char *pFilename, HLTFileTree **phTree, uint32 *pFileSize) {
    //get the first file tree list element.
    SERVERFILEMGR_ELEMENT *cur = file_tree_list.First();

    //go through all the trees in the list.
    while (file_tree_list.IsHead(cur) == false) {
        //get the file tree handle.
        HLTFileTree *hTree = cur->Item();

        //try to get the file info.
        LTFindInfo info;
        if (df_GetFileInfo(hTree, pFilename, &info) == false) {
            //go to next element.
            cur = cur->Next();
            continue;
        }

        //the file existed.

        //check if the optional parameters were passed in.
        if (phTree != NULL) {
            *phTree = hTree;
        }

        if (pFileSize != NULL) {
            *pFileSize = info.m_Size;
        }

        //we found the file.
        return true;
    }

    //could not find the file.
    return false;
}


FileEntry* IServerFileMgr::GetFileList(const char *pDirName) {
    //put all our file tree handles in an array.
    HLTFileTree *trees[MAX_FILETREES_TO_SEARCH];
    int nTrees = 0;

    SERVERFILEMGR_ELEMENT *cur = file_tree_list.First();
    for (; file_tree_list.IsHead(cur) == false; cur = cur->Next()) {
        //add this file tree handle to the array.
        trees[nTrees] = cur->Item();

        //increment tree count.
        nTrees++;

        //make sure we dont go past the end of the array.
        if (nTrees >= MAX_FILETREES_TO_SEARCH) break;
    }

    //get the files in all those trees.
    return ic_GetFileList(trees, nTrees, pDirName);
}

// ------------------------------------------------------------------------
// GetAddedUsedFile( filename, ret-used-file )
// find a used file based on the filename.
// ------------------------------------------------------------------------
bool IServerFileMgr::GetAddedUsedFile( const char *pFilename, UsedFile **ppFile )
{

    HHashElement *hElement;
	UsedFile *pFile;    
	uint32 strLen ;

    if(ppFile)
        *ppFile = NULL;

    strLen = strlen(pFilename);

    hElement = hs_FindElement(m_hFileTable, pFilename, strLen+1);
    if(hElement)
    {
        pFile = (UsedFile*)hs_GetElementUserData(hElement);

        if(ppFile)
            *ppFile = pFile;
        
        return true;
    }
	
	return false ;
}

// ------------------------------------------------------------------------
// GetAddedUsedFile( file_id, ret_used_file )
// find the used file based on a file id. 
// returns true if something was found,
// false if the file has not been loaded.
// ------------------------------------------------------------------------
bool IServerFileMgr::GetAddedUsedFile( uint16 file_id, UsedFile **ppFile )
{
	HHashIterator *hIterator;
    HHashElement *hElement;
    UsedFile *pFile;

    hIterator = hs_GetFirstElement(m_hFileTable);
    while(hIterator)
    {
        hElement = hs_GetNextElement(hIterator);
        
        pFile = (UsedFile*)hs_GetElementUserData(hElement);
		if( pFile->m_FileID == file_id )
		{
			*ppFile = pFile ;
			return true ;
		}
    }
	return false ;
}

// ------------------------------------------------------------------------
// add_state AddUsedFile( filename, flags, out_used_file )
// Add a file to the 'used file' list.
// Fills in ppFile if it is non-NULL.
// Returns 0 if the file doesn't exist.
// Returns 1 if the file is already there (flags are ORed together).
// Returns 2 if the file was added. 
// ------------------------------------------------------------------------
int IServerFileMgr::AddUsedFile(const char *pFilename, short flags, UsedFile **ppFile) 
{
    UsedFile *pFile;
    HHashElement *hElement;
    uint32 strLen, file_size;
    HLTFileTree *hTree;
    LTLink *pCur, *pListHead;
    Client *pClient;

    if(ppFile)
        *ppFile = NULL;

    strLen = strlen(pFilename);

    hElement = hs_FindElement(m_hFileTable, pFilename, strLen+1);
    if(hElement)
    {
        pFile = (UsedFile*)hs_GetElementUserData(hElement);

        if(ppFile)
            *ppFile = pFile;
        
        return 1;
    }
    else
    {
        if(!DoesFileExist(pFilename, &hTree, &file_size))
            return 0;

        // Ok, setup a new file.
        pFile = m_UsedFileBank.Allocate();

        hElement = hs_AddElement(m_hFileTable, pFilename, strlen(pFilename)+1);
        pFile->m_Flags = flags;
        pFile->m_hFileTree = hTree;
        pFile->m_FileSize = file_size;
        pFile->m_FileID = m_CurrentFileID++;
        hs_SetElementUserData(hElement, pFile);
        pFile->m_hElement = hElement;
        pFile->m_Data = NULL;

        if(ppFile)
            *ppFile = pFile;

        // Tell the file transfer servers to add this file.
        pListHead = &g_pServerMgr->m_Clients.m_Head;
        for(pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
        {
            pClient = (Client*)pCur->m_pData;
            fts_AddFile(pClient->m_hFTServ, pFile->GetFilename(), pFile->m_FileSize, pFile->m_FileID, pFile->m_Flags);
        }

        return 2;
    }
}


void IServerFileMgr::ClearUsedFiles() 
{
    HHashIterator *hIterator;
    HHashElement *hElement;
    UsedFile *pFile;

    hIterator = hs_GetFirstElement(m_hFileTable);
    while(hIterator)
    {
        hElement = hs_GetNextElement(hIterator);
        
        pFile = (UsedFile*)hs_GetElementUserData(hElement);
        m_UsedFileBank.Free(pFile);
        hs_RemoveElement(m_hFileTable, hElement);
    }

    m_CurrentFileID = 0;
}


char *IServerFileMgr::GetUsedFilename(UsedFile *pFile) 
{
    if (pFile != NULL) {
        return (char*)hs_GetElementKey(pFile->m_hElement, NULL);
    }
    else {
        return "";
    }
}





